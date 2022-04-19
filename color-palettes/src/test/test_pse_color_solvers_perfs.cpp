#include <pse/pse_solver_exploration.hpp>
#include <pse/pse_solver_interpolation.hpp>
#include <pse/pse_cps.hpp>
#include <pse/pse_levenberg_utils.hpp>
#include <pse/color/pse_color.hpp>
#include <pse/color/pse_cost_color.hpp>
#include <pse/color/pse_cost_color_energy.hpp>

#include <Functionnal/constrainedBezier.h>

#include <iostream>
#include <chrono>

#define PERFORM_CPS
#define PERFORM_EXPLORATION
//#define PERFORM_INTERPOLATION

//#define PRINT_INITIAL_PPOINTS
//#define PRINT_SPACES
//#define PRINT_FINAL_PPOINTS
#define PRINT_PPM

//#define USE_VALGRIND
//#define CHECK_MEM_USAGE   // reported value of Valgrind are deprecated

#define RAND_SEED 123456

static constexpr size_t COLORS_COUNT = 5;  // per palette
static constexpr LayerId SRC_PALETTE = 0;
static constexpr LayerId DST_PALETTE = 1;
static constexpr LayerId INVALID_PALETTE = 2;
static constexpr size_t PALETTES_COUNT = 2;
static constexpr int INTERPOLATION_SAMPLING = 50;
static constexpr int SOLVER_FORCED_ITERATIONS = 1;

#ifdef USE_VALGRIND
#include <valgrind/memcheck.h>
#else
#define VALGRIND_DO_ADDED_LEAK_CHECK  /* disabled */
#endif

#define TEST(truth) if(!(truth)) { assert(false); exit(1); }

// Overload global new/delete to get trace of the allocated memory size. Not
// perfect, but will do the job.
// We alloc more space each time to store the allocated size in order to
// retreive it on delete.
// FIXME: this computation is the ideal memory usage, without alignement and
// mem block size taken into account. The values provided by Valgrind is
// probably more exact regarding this reality!
size_t curr_alloc_mem_size = 0;
size_t total_alloc_mem_size = 0;
#ifdef CHECK_MEM_USAGE
void* operator new(size_t s, const std::nothrow_t&) noexcept {
  curr_alloc_mem_size += s;
  total_alloc_mem_size += s;

  const size_t adjusted_s = s + sizeof(size_t);
  void* p = malloc(adjusted_s);
  if( p != nullptr ) {
    *((size_t*)p) = s;
    p = (void*)((uintptr_t)p + sizeof(size_t));
  }
  return p;
}
void* operator new(size_t s) {
  void* p = ::operator new(s, std::nothrow);
  if(!p)
    throw std::bad_alloc();
  return p;
}
//void* operator new(size_t s, void* p); // Not usefull to overload it
void operator delete(void* p) {
  p = (void*)((uintptr_t)p - sizeof(size_t));
  curr_alloc_mem_size -= *((size_t*)p);
  free(p);
}
void operator delete(void* p, size_t s) {
  assert(*((size_t*)((uintptr_t)p - sizeof(size_t))) == s);
  ::operator delete(p);
}

#define MEMCHECK_START(name)                                                   \
  const size_t name##_cmem_start = curr_alloc_mem_size;                        \
  const size_t name##_tmem_start = total_alloc_mem_size
#define MEMCHECK_STOP(name)                                                    \
  const size_t name##_cmem_end = curr_alloc_mem_size;                          \
  const size_t name##_tmem_end = total_alloc_mem_size
#define MEMCHECK_GET_ALLOCATED(name) (ssize_t)(name##_cmem_end - name##_cmem_start)
#define MEMCHECK_GET_TOTAL_ALLOCATED(name) (ssize_t)(name##_tmem_end - name##_tmem_start)
#else
#define MEMCHECK_START(name)
#define MEMCHECK_STOP(name)
#endif

typedef double Real;
using RealColor = LABColor<Real>;
constexpr int Dim = 3;
constexpr int Degree = 3;
using ParameterValidationHelper = ::Color::InRGBspaceClampingHelper<Real>;
using ComponentFunctor = Functionnal::ConstrainedBezierMap<Real, Degree, Dim> ;
using ConstComponentFunctor = Functionnal::ConstConstrainedBezierMap<Real, Degree, Dim> ;

using CPS = ConstrainedParameterSpace<RealColor>;
using PPParams = CPS::ParametricPointParams;
using PParams = CPS::PairingParams;
using CParams = CPS::ConstraintParams;
using GCParams = CPS::GlobalConstraintParams;

using CPSExplorationProblemBase =
  SolverExploration<RealColor, ParameterValidationHelper>;
using CPSExplorationProblem =
  Utils::Levenberg::Functor_w_df<CPSExplorationProblemBase>;
using CPSExplorationSolver =
  Eigen::LevenbergMarquardt<CPSExplorationProblem>;

using CPSInterpolationProblemBase =
  SolverInterpolation<
    RealColor,
    ParameterValidationHelper,
    ComponentFunctor,
    ConstComponentFunctor,
    INTERPOLATION_SAMPLING
  >;
using CPSInterpolationProblem =
  Utils::Levenberg::Functor_w_df<CPSInterpolationProblemBase>;
using CPSInterpolationSolver =
  Eigen::LevenbergMarquardt<CPSInterpolationProblem>;

static void
generate_colors
  (const size_t count,
   std::vector<RealColor>& colors)
{
  for(size_t i = 0; i < count; ++i) {
    colors.push_back(RealColor{
      std::max(0.001, RealColor::Scalar(rand() % 100000) / RealColor::Scalar(100000)),
      std::max(0.001, RealColor::Scalar(rand() % 100000) / RealColor::Scalar(100000)),
      std::max(0.001, RealColor::Scalar(rand() % 100000) / RealColor::Scalar(100000))
    });
  }
}

using ParametricPointFunctor1 =
  ColorDiscrepancy::AnchorLAB_LuminanceDiscrepancyFunctor<RealColor>;
using ParametricPointFunctor2 =
  ColorDiscrepancy::AnchorLAB_aDiscrepancyFunctor<RealColor>;
using ParametricPointFunctor3 =
  ColorDiscrepancy::AnchorLAB_bDiscrepancyFunctor<RealColor>;
using ConstraintFunctor1 =
  ColorDiscrepancy::LAB_LuminanceDiscrepancyFunctor<RealColor>;
using ConstraintFunctor2 =
  ColorDiscrepancy::LAB_aDiscrepancyFunctor<RealColor>;
using ConstraintFunctor3 =
  ColorDiscrepancy::LAB_bDiscrepancyFunctor<RealColor>;
using GlobalConstraintFunctor =
  ColorDiscrepancy::GlobalAnchorLAB_LuminanceDiscrepancyFunctor<RealColor>;

struct CPSContext {
  std::vector<std::vector<RealColor>> colors;
  std::vector<std::vector<CPS::UnaryConstraintFunctor*>> ppoints_functors;
  std::vector<std::vector<CPS::BinaryConstraintFunctor*>> constraints_functors;
  std::vector<std::vector<CPS::NaryConstraintFunctor*>> global_functors;
};

static inline void
print_poly_color(const ConstComponentFunctor& poly, ...)
{
  typename ConstComponentFunctor::EmbeddedVectorType input;
  input << 0.5f;
  std::cout << RealColor(poly.eval(input)) << std::endl;
}
static inline void
print_cps_vertices(CPS* space)
{
  RealColor c;
  for(ParametricPointId id = 0; id < space->getAllParametricPointsCount(); ++id) {
    (void)space->getParametricPointValue(id, c);
    RealColor::CVector unscaled_c = c.getUnscaledNative();
    std::cout << "Vertex " << id
              << ": " << Color::UnscaledSpaceNames[c.getUnscaledSpace()]
              << "{ " << unscaled_c(0)
              << ", " << unscaled_c(1)
              << ", " << unscaled_c(2)
              << " }, " << c << std::endl;
  }
}

static void
create_cps_context
  (CPSContext& ctxt,
   const std::vector<std::vector<RealColor>>& colors)
{
  const size_t palettes_count = colors.size();
  ctxt.colors.resize(palettes_count);
  ctxt.ppoints_functors.resize(palettes_count);
  ctxt.constraints_functors.resize(palettes_count);
  ctxt.global_functors.resize(palettes_count);

  for(size_t p = 0; p < palettes_count; ++p) {
    ctxt.colors[p] = colors[p];
    const size_t ppoints_count = 0;//colors[p].size();
    for(size_t i = 0; i < ppoints_count; ++i) {
      ctxt.ppoints_functors[p].push_back(new ParametricPointFunctor1{});
      ctxt.ppoints_functors[p].push_back(new ParametricPointFunctor2{});
      ctxt.ppoints_functors[p].push_back(new ParametricPointFunctor3{});
    }
    const size_t const_count =
        (colors[p].size() * (colors[p].size() - 1)) / 2; // full graph edges count
    for(size_t i = 0; i < const_count; ++i) {
      ctxt.constraints_functors[p].push_back(new ConstraintFunctor1{});
      ctxt.constraints_functors[p].push_back(new ConstraintFunctor2{});
      ctxt.constraints_functors[p].push_back(new ConstraintFunctor3{});
    }
    const size_t gconst_count = 0;
    for(size_t i = 0; i < gconst_count; ++i) {
      ctxt.global_functors[p].push_back(new GlobalConstraintFunctor{});
    }
  }
}
static void
print_cps_context
  (CPSContext& ctxt)
{
  for(size_t p = 0; p < ctxt.colors.size(); ++p) {
    std::cout
      << "Palette " << p << ":" << std::endl
      << "==========" << std::endl
      << "Parametric points functors:" << std::endl;
    size_t functors_count = ctxt.ppoints_functors[p].size();
    for(size_t i = 0; i < functors_count; i += 3) {
      CPS::UnaryConstraintFunctor* fg1 = ctxt.ppoints_functors[p][i+0];
      CPS::UnaryConstraintFunctor* fg2 = ctxt.ppoints_functors[p][i+1];
      CPS::UnaryConstraintFunctor* fg3 = ctxt.ppoints_functors[p][i+2];
      ParametricPointFunctor1* f1 = static_cast<ParametricPointFunctor1*>(fg1);
      std::cout
        << f1->getTypeName() << ": "
        << "weight(" << f1->getW() << ")" << ", "
        << "ref(" << f1->getRef() << ")"
        << std::endl;
      ParametricPointFunctor2* f2 = static_cast<ParametricPointFunctor2*>(fg2);
      std::cout
        << f2->getTypeName() << ": "
        << "weight(" << f2->getW() << ")" << ", "
        << "ref(" << f2->getRef() << ")"
        << std::endl;
      ParametricPointFunctor3* f3 = static_cast<ParametricPointFunctor3*>(fg3);
      std::cout
        << f3->getTypeName() << ": "
        << "weight(" << f3->getW() << ")" << ", "
        << "ref(" << f3->getRef() << ")"
        << std::endl;
    }
    std::cout << "Constraints functors:" << std::endl;
    functors_count = ctxt.constraints_functors[p].size();
    for(size_t i = 0; i < functors_count; i += 3) {
      CPS::BinaryConstraintFunctor* fg1 = ctxt.constraints_functors[p][i+0];
      CPS::BinaryConstraintFunctor* fg2 = ctxt.constraints_functors[p][i+1];
      CPS::BinaryConstraintFunctor* fg3 = ctxt.constraints_functors[p][i+2];
      ConstraintFunctor1* f1 = static_cast<ConstraintFunctor1*>(fg1);
      std::cout
        << f1->getTypeName() << ": "
        << "weight(" << f1->getW() << ")"
        << std::endl;
      ConstraintFunctor2* f2 = static_cast<ConstraintFunctor2*>(fg2);
      std::cout
        << f2->getTypeName() << ": "
        << "weight(" << f2->getW() << ")"
        << std::endl;
      ConstraintFunctor3* f3 = static_cast<ConstraintFunctor3*>(fg3);
      std::cout
        << f3->getTypeName() << ": "
        << "weight(" << f3->getW() << ")"
        << std::endl;
    }
    std::cout << "Global constraints functors:" << std::endl;
    for(CPS::NaryConstraintFunctor* fg: ctxt.global_functors[p]) {
      GlobalConstraintFunctor* f = static_cast<GlobalConstraintFunctor*>(fg);
      std::cout
        << f->getTypeName() << ": "
        << "weight(" << f->getW() << ")" << ", "
        << "ref(" << f->getRef() << ")"
        << std::endl;
    }
  }
}
static void
destroy_cps_context
  (CPSContext& ctxt)
{
  for(size_t p = 0; p < ctxt.colors.size(); ++p) {
    for(auto v: ctxt.ppoints_functors[p])
      delete v;
    for(auto v: ctxt.constraints_functors[p])
      delete v;
    for(auto v: ctxt.global_functors[p])
      delete v;
  }
  ctxt.colors.clear();
}

static void
create_cps
  (CPSContext& ctxt,
   CPS& space)
{
  ParametricPointIdList ppids;
  std::vector<PPParams> ppparams;
  for(size_t p = 0; p < ctxt.colors.size(); ++p) {
    ppparams.clear();
    const size_t colors_count = ctxt.colors[p].size();
    for(size_t i = 0; i < colors_count; ++i) {
      //ppparams.push_back({ctxt.colors[p][i], 1.0f, 3, &ctxt.ppoints_functors[p][i*3]});
      ppparams.push_back({ctxt.colors[p][i], 1.0f, 0, nullptr});
    }
    TEST(space.addParametricPoints(p, ppparams, ppids) == ERet_OK);
  }

  std::vector<PParams> pparams;
  PairingIdList pids;
  for(size_t i = 0; i < ctxt.colors[0].size(); ++i) {
    pparams.push_back({ppids[i], ppids[i + ctxt.colors[0].size()]});
  }
  TEST(space.addPairings(pparams, pids) == ERet_OK);

  std::vector<CParams> cparams;
  ConstraintIdList cids;
  size_t idx;
  for(size_t p = 0; p < ctxt.colors.size(); ++p) {
    const size_t colors_count = ctxt.colors[p].size();
    idx = 0;
    for(size_t i = 0; i < colors_count - 1; ++i) {
      for(size_t j = i + 1; j < colors_count; ++j) {
        cparams.push_back({
          {ppids[p*colors_count + i],
           ppids[p*colors_count + j]},
          3, &ctxt.constraints_functors[p][idx*3]
        });
        ++idx;
      }
    }
  }
  if( !cparams.empty() ) {
    TEST(space.addConstraints(cparams, cids) == ERet_OK);
  }

  std::vector<GCParams> gcparams;
  GlobalConstraintIdList gcids;
  for(size_t p = 0; p < ctxt.colors.size(); ++p) {
    gcparams.clear();
    for(auto f: ctxt.global_functors[p]) {
      gcparams.push_back({f});
    }
    if( !gcparams.empty() ) {
      TEST(space.addGlobalConstraints(p, gcparams, gcids) == ERet_OK);
    }
  }
}

static void
print_ppm
  (CPS& space)
{
#define _STR(a) #a
#define STR(a)  _STR(a)
#define MAXVAL  255
  fprintf(stdout,
    "P3\n"  /* Magick number: P3 -> ASCII portable RGB color image */
    "%d"    /* Image width */
    " 2\n"  /* Image height */
    "%d\n", /* Max color value. We use the maximum to have more precision */
  (int)COLORS_COUNT, (int)MAXVAL);
  space.visitLayerParametricPoints(0, [&] (const ParametricPointId ppid) {
    RealColor color;
    TEST(space.getParametricPointValue(ppid, color) == ERet_OK);
    RGBColor<RealColor::Scalar> rgb = color;
    fprintf(stdout,"%d %d %d\t",
      (int)((RealColor::Scalar)MAXVAL * rgb.getNative()[0]),
      (int)((RealColor::Scalar)MAXVAL * rgb.getNative()[1]),
      (int)((RealColor::Scalar)MAXVAL * rgb.getNative()[2])
    );
  });
#undef MAXVAL
#undef STR
#undef _STR
}

int main()
{
  using namespace std::chrono;
  system_clock::time_point before, after;
  Eigen::LevenbergMarquardtSpace::Status status;
  int iterations;
  std::vector<std::vector<RealColor>> initial_colors;

  srand(RAND_SEED);
  initial_colors.resize(PALETTES_COUNT);
  for(auto& ic: initial_colors) {
    generate_colors(COLORS_COUNT, ic);
  }

#ifdef PERFORM_CPS
  srand(RAND_SEED);
  std::cerr << "CPS: CREATION ==============" << std::endl;
  VALGRIND_DO_ADDED_LEAK_CHECK;
  MEMCHECK_START(cps);
  before = system_clock::now();
    CPSContext space_ctxt;
    create_cps_context(space_ctxt, initial_colors);
    CPS* space = new CPS{PALETTES_COUNT};
    create_cps(space_ctxt, *space);
  after = system_clock::now();
  MEMCHECK_STOP(cps);
  VALGRIND_DO_ADDED_LEAK_CHECK;
  std::cerr << "============================" << std::endl;

  std::cout
    << "Execution time: " << duration_cast<milliseconds>(after - before).count() << " ms" << std::endl
#ifdef CHECK_MEM_USAGE
    << "Memory usage for ConstrainedParameterSpace:" << std::endl
    << " * as reported by Valgrind: "
      << "2952" << " bytes" << std::endl
    << " * as computed by overloading new operator: "
      << MEMCHECK_GET_ALLOCATED(cps) << " bytes" << std::endl
    << "Total memory allocated during construction of the ConstrainedParameterSpace: "
      << MEMCHECK_GET_TOTAL_ALLOCATED(cps) << " bytes" << std::endl
    << std::endl
#endif
  ;

#ifdef PERFORM_EXPLORATION
  std::cerr << "CPS: USE FOR EXPLORATION ===" << std::endl;
  VALGRIND_DO_ADDED_LEAK_CHECK;
  MEMCHECK_START(cpsexp);
  #ifdef PRINT_INITIAL_PPOINTS
    print_cps_vertices(space);
  #endif
  #ifdef PRINT_PPM
    print_ppm(*space);
  #endif
  before = system_clock::now();
  CPSExplorationProblem* cpspbexp = new CPSExplorationProblem{};
  space->initConstraintsInternals(SRC_PALETTE);
  #ifdef PRINT_SPACES
    print_cps_context(space_ctxt);
  #endif
  cpspbexp->setSpace(space);
  CPSExplorationSolver* cpssolverexp = new CPSExplorationSolver(*cpspbexp);
  cpspbexp->optimizeStartPalette(true);
  RealColor color;
  space->getParametricPointValue(0, color);
  HSVColor<RealColor::Scalar> hsv = color;
  hsv = HSVColor<RealColor::Scalar>{hsv.getNative()[0], hsv.getNative()[1], 0};
  space->setParametricPointValue(0, hsv);
  cpspbexp->lockParametricPoints({0});
  cpspbexp->setup();
  CPSExplorationProblem::InputType cpsexpvars;
  cpspbexp->updateVariablesFromGraph(cpsexpvars);
  iterations = 0;
  for(int i = 0; i < SOLVER_FORCED_ITERATIONS; ++i) {
    status = cpssolverexp->minimize(cpsexpvars);
    iterations += cpssolverexp->iterations();
  }
  cpspbexp->updateGraphFromVariables(cpsexpvars);
  after = system_clock::now();
  #ifdef PRINT_FINAL_PPOINTS
    print_cps_vertices(space);
  #endif
  #ifdef PRINT_PPM
    print_ppm(*space);
  #endif
  MEMCHECK_STOP(cpsexp);
  VALGRIND_DO_ADDED_LEAK_CHECK;
  std::cerr << "============================" << std::endl;

  std::cout
    << "Last execution status: " << status << std::endl
    << "Execution time: " << duration_cast<milliseconds>(after - before).count() << " ms" << std::endl
    << "Iterations: " << iterations << std::endl
#ifdef CHECK_MEM_USAGE
    << "Memory usage after exploration of the ConstrainedParameterSpace:" << std::endl
    << " * as reported by Valgrind: "
      << "???" << " bytes" << std::endl
    << " * as computed by overloading new operator: "
      << MEMCHECK_GET_ALLOCATED(cpsexp) << " bytes" << std::endl
    << "Total memory allocated during the exploration of the ConstrainedParameterSpace: "
      << MEMCHECK_GET_TOTAL_ALLOCATED(cpsexp) << " bytes" << std::endl
    << std::endl
#endif
  ;
#endif // PERFORM_EXPLORATION

#ifdef PERFORM_INTERPOLATION
  std::cerr << "CPS: USE FOR INTERPOLATION =" << std::endl;
  VALGRIND_DO_ADDED_LEAK_CHECK;
  MEMCHECK_START(cpsint);
  #ifdef PRINT_INITIAL_PPOINTS
    print_cps_vertices(space);
  #endif
  before = system_clock::now();
  CPSInterpolationProblem* cpspbint = new CPSInterpolationProblem{};
  cpspbint->setParametricStepsCount(INTERPOLATION_SAMPLING);
  #ifdef PRINT_SPACES
    print_cps_context(space_ctxt);
  #endif
  cpspbint->setSpace(space);
  cpspbint->setStartLayerId(SRC_PALETTE);
  cpspbint->setEndLayerId(DST_PALETTE);
  CPSInterpolationSolver* cpssolverint = new CPSInterpolationSolver(*cpspbint);
  CALL(cpspbint->initInternals());
  CPSInterpolationProblem::InputType cpsintvars;
  cpsintvars.resize(cpspbint->inputs());
  cpspbint->visitPolynomials
    ([](ComponentFunctor& poly, const PairingId) {
       poly.initCoeffs();
     },
     cpsintvars);
  space->initConstraintsInternals(SRC_PALETTE);
  iterations = 0;
  for(int i = 0; i < SOLVER_FORCED_ITERATIONS; ++i) {
    status = cpssolverint->minimize(cpsintvars);
    iterations += cpssolverint->iterations();
  }
  after = system_clock::now();
  #ifdef PRINT_FINAL_PPOINTS
    cpspbint->visitConstPolynomials(print_poly_color, cpsintvars);
  #endif
  MEMCHECK_STOP(cpsint);
  VALGRIND_DO_ADDED_LEAK_CHECK;
  std::cerr << "============================" << std::endl;

  std::cout
    << "Last execution status: " << status << std::endl
    << "Execution time: " << duration_cast<milliseconds>(after - before).count() << " ms" << std::endl
    << "Iterations: " << iterations << std::endl
#ifdef CHECK_MEM_USAGE
    << "Memory usage after interpolation of the ConstrainedParameterSpace:" << std::endl
    << " * as reported by Valgrind: "
      << "???" << " bytes" << std::endl
    << " * as computed by overloading new operator: "
      << MEMCHECK_GET_ALLOCATED(cpsint) << " bytes" << std::endl
    << "Total memory allocated during the interpolation of the ConstrainedParameterSpace: "
      << MEMCHECK_GET_TOTAL_ALLOCATED(cpsint) << " bytes" << std::endl
    << std::endl
#endif
  ;
#endif // PERFORM_INTERPOLATION

  std::cerr << "CPS: DELETION ==============" << std::endl;
  VALGRIND_DO_ADDED_LEAK_CHECK;
  MEMCHECK_START(cpsdel);
  before = system_clock::now();
    #ifdef PERFORM_EXPLORATION
    delete cpssolverexp;
    delete cpspbexp;
    #endif
    #ifdef PERFORM_INTERPOLATION
    delete cpssolverint;
    delete cpspbint;
    #endif
    delete space;
    destroy_cps_context(space_ctxt);
  after = system_clock::now();
  MEMCHECK_STOP(cpsdel);
  VALGRIND_DO_ADDED_LEAK_CHECK;
  std::cerr << "============================" << std::endl;

  std::cout
    << "Execution time: " << duration_cast<milliseconds>(after - before).count() << " ms" << std::endl
#ifdef CHECK_MEM_USAGE
    << "Memory usage after deletion for ConstrainedParameterSpace:" << std::endl
    << " * as reported by Valgrind: "
      << "-2952" << " bytes" << std::endl
    << " * as computed by overloading new operator: "
      << MEMCHECK_GET_ALLOCATED(cpsdel) << " bytes" << std::endl
    << "Total memory allocated during deletion of the ConstrainedParameterSpace: "
      << MEMCHECK_GET_TOTAL_ALLOCATED(cpsdel) << " bytes" << std::endl
    << std::endl
#endif
;
#endif

  return 0;
}
