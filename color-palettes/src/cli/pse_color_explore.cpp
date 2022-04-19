#include <pse/serialization/pse_serializer_yaml.hpp>
#include <pse/serialization/pse_slz_cps.hpp>
#include <pse/serialization/pse_slz_cost_generic.hpp>
#include <pse/color/serialization/pse_slz_color.hpp>
#include <pse/color/serialization/pse_slz_cost_color.hpp>
#include <pse/color/serialization/pse_slz_cost_color_energy.hpp>

#include <pse/pse_solver_exploration.hpp>
#include <pse/pse_levenberg_utils.hpp>
#include <pse/color/pse_color.hpp>
#include <pse/color/pse_cost_color.hpp>
#include <pse/color/pse_cost_color_energy.hpp>

#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

//#define PLX_DEBUG_OUTPUT

#define PLX_ITERATIONS  1

using PlxColorRGBd = Color::ColorBase<double, Color::RGB>;
using PlxColorLABd = Color::ColorBase<double, Color::LAB>;

template<typename ParametricPoint_>
struct implem
{
  using ParametricPoint = ParametricPoint_;
  using Scalar = typename ParametricPoint::Scalar;
  using CPS = ConstrainedParameterSpace<ParametricPoint>;
  using ParameterValidationHelper = Color::InRGBspaceClampingHelper<Scalar>;
  using CPSExplorationProblemBase =
    SolverExploration<ParametricPoint, ParameterValidationHelper>;
  using CPSExplorationProblem =
    Utils::Levenberg::Functor_w_df<CPSExplorationProblemBase>;
  using CPSExplorationSolver =
    Eigen::LevenbergMarquardt<CPSExplorationProblem>;

  template<typename Params_>
  static ESRet
  serialize
    (Params_ params,
     CPS* cps,
     const char* format)
  {
    ESRet ret = ESRet_OK;
    assert(strncmp(format, "yaml", 4) == 0);

    SerializerYAML s;

    Factory f;
    FACTORY_ADD_TYPE(f, ParametricPoint);
    FACTORY_ADD_TYPE(f, CPS);
    colorDiscrepenciesTypesAddToFactory<ParametricPoint>(f);
    colorEnergyDiscrepenciesTypesAddToFactory<ParametricPoint>(f);
    s.addFactory(&f);

    SCALL_OR_RETURN(ret, colorTypedObjectSerializationFunctionsRegister<Scalar>(s));
    SCALL_OR_RETURN(ret, cpsTypedObjectSerializationFunctionsRegister<ParametricPoint>(s));
    SCALL_OR_RETURN(ret, colorDiscrepenciesTypedObjectSerializationFunctionsRegister<ParametricPoint>(s));
    SCALL_OR_RETURN(ret, colorEnergyDiscrepenciesTypedObjectSerializationFunctionsRegister<ParametricPoint>(s));

    SCALL_OR_RETURN(ret, s.beginStream(params));
    SCALL_OR_RETURN(ret, serializeCPS<ParametricPoint>(s, cps));
    SCALL_OR_RETURN(ret, s.endStream());
    CHECK_OR_DO(cps != NULL, return ESRet_Invalid);

    return ret;
  }

  static ERet
  explore
    (CPSExplorationProblem* pb,
     CPSExplorationSolver* solver,
     CPS* cps,
     typename CPSExplorationProblem::InputType& vars,
     const ParametricPointId id,
     const ParametricPoint& val)
  {
    auto before = std::chrono::system_clock::now();
    cps->setParametricPointValue(id, val);

#ifdef PLX_DEBUG_OUTPUT
    for(const ParametricPointId ppid: cps->getLayerParametricPointIdList(0)) {
      ParametricPoint cval;
      CALL(cps->getParametricPointValue(ppid, cval));
      std::cout << "\"" << cval.getRGBHex() << "\"," << std::endl;
    }
#endif

    pb->lockParametricPoints({id});
    pb->setup();
    pb->updateVariablesFromGraph(vars);
    for(int i = 0; i < PLX_ITERATIONS; ++i ) {
      solver->minimize(vars);
    }
    pb->updateGraphFromVariables(vars);
    pb->unlockParametricPoints({id});

    auto after = std::chrono::system_clock::now();
    std::cout
      << "Execution time: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count()
      << " ms" << std::endl;
#ifdef PLX_DEBUG_OUTPUT
    for(const ParametricPointId ppid: cps->getLayerParametricPointIdList(0)) {
      ParametricPoint cval;
      CALL(cps->getParametricPointValue(ppid, cval));
      std::cout << ppid << ": " << cval << std::endl;
    }
    for(const ParametricPointId ppid: cps->getLayerParametricPointIdList(0)) {
      ParametricPoint cval;
      CALL(cps->getParametricPointValue(ppid, cval));
      std::cout << "\"" << cval.getRGBHex() << "\"," << std::endl;
    }
#endif
    return ERet_OK;
  }
};

int main(int argc, char** argv)
{
  const char* format = "yaml";
  const char* input_filepath = "/dev/stdin";

  for(int i = 1; i < argc; ++i ) {
    if( strncmp(argv[i], "--format=", 9) == 0 ) {
      format = &argv[i][9];
    } else if( strncmp(argv[i], "-f", 2) == 0 ) {
      format = &argv[i][2];
    } else if( strncmp(argv[i], "--input=", 8) == 0 ) {
      input_filepath = &argv[i][8];
    }
  }

  if( strncasecmp(format, "yaml", 4) != 0 ) {
    fprintf(stderr, "Error: format '%s' not managed.\n", format);
    return 1;
  }

  // Read the buffer from input file (manage also the default case of STDIN)
  std::ifstream stream(input_filepath);
  std::stringstream buffer;
  buffer << stream.rdbuf();

  using PlxImplemLABd = implem<PlxColorLABd>;

  PlxImplemLABd::CPSExplorationProblem::InputType vars;
  PlxImplemLABd::CPS* cps = new PlxImplemLABd::CPS{1};
  PlxImplemLABd::CPSExplorationProblem* pb = new PlxImplemLABd::CPSExplorationProblem{};
  pb->setSpace(cps);
  pb->optimizeStartPalette(true);
  PlxImplemLABd::CPSExplorationSolver* solver = new PlxImplemLABd::CPSExplorationSolver{*pb};

  // Create the CPS from the input buffer
  SCALL(PlxImplemLABd::serialize<const std::string&>(buffer.str(), cps, format));
  cps->initConstraintsInternals(0);

  // Launch exploration on it
  PlxImplemLABd::ParametricPoint val { 0.354287, 0.486776, 0.498008 };
  /*PlxImplemLABd::ParametricPoint val;
  val.setRGB<PlxColorRGBd::CVector>({
    double(77) / 255.0,
    double(85) / 255.0, 
    double(84) / 255.0
  });*/
  CALL(PlxImplemLABd::explore(pb, solver, cps, vars, 2, val));

  delete solver;
  delete pb;
  delete cps;

  return 0;
}
