#include "pse_eigen_exploration.h"
#include "pse_eigen_drv.h"
#include "pse_eigen_values.h"

#include <pse.h>
#include <pse_logger.h>
#include <stretchy_buffer.h>

#include <Eigen/Dense>
#include <unsupported/Eigen/LevenbergMarquardt>

#include <unordered_map>
#include <unordered_set>
#include <inttypes.h>

//#define PSE_EIGEN_REF

#if 0 // Use to check the perfs of each function in this file and to debug easily
#undef PSE_FINLINE
#undef PSE_INLINE
#define PSE_FINLINE
#define PSE_INLINE
#endif

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

enum pse_eigen_access_right_t {
  PSE_EIGEN_ACCESS_RIGHT_WRITE,
  PSE_EIGEN_ACCESS_RIGHT_READ
};

struct pse_costs_mem_chunk_t {
  size_t offset;
  size_t count;
};

struct pse_eigen_relshps_for_ppoint_precomputations_t {
  pse_clt_ppoint_variation_uid_t variation;
  std::vector<pse_relshp_id_t> relshps;
  std::vector<pse_clt_cost_func_ctxt_t> relshps_ctxt;
  std::vector<pse_clt_cost_func_ctxt_config_t> relshps_config;
  std::vector<const struct pse_eval_relshp_data_t*> relshps_eval_data;
  std::vector<struct pse_costs_mem_chunk_t> relshps_costs_chunks;
  size_t costs_count;
};

/* TODO: use our allocator */
typedef std::unordered_set<
  pse_ppoint_id_t
> pse_eigen_ppoint_id_set_t;

struct pse_eigen_relshp_precomputations_t {
  const struct pse_cpspace_instance_relshp_data_t* idata;
  pse_eigen_ppoint_id_set_t ppoints_involved_set;
  size_t costs_count;
};

/* TODO: use our allocator */
typedef std::pair<
  pse_ppoint_id_t,
  pse_clt_ppoint_variation_uid_t
> pse_variated_ppoint_id_t;
typedef std::map<
  pse_variated_ppoint_id_t,
  struct pse_eigen_relshps_for_ppoint_precomputations_t
> pse_eigen_variated_ppoint_to_relshps_t;

struct pse_eigen_relshp_cost_func_t {
  const struct pse_cpspace_instance_cost_func_data_t* idata;
  size_t costs_count_per_variation;
  pse_eigen_variated_ppoint_to_relshps_t relshps_per_ppoint;
};

/* TODO: use our allocator */
typedef std::unordered_map<
  pse_relshp_cost_func_id_t,
  struct pse_eigen_relshp_cost_func_t
> pse_eigen_relshp_cost_funcs_precomputations_t;

/* TODO: use our allocator */
typedef std::unordered_map<
  pse_relshp_id_t,
  struct pse_eigen_relshp_precomputations_t
> pse_eigen_relshps_precomputations_t;

struct pse_eigen_cps_precomputations_t {
  pse_eigen_relshp_cost_funcs_precomputations_t relshp_cost_funcs;
  pse_eigen_relshps_precomputations_t relshps;
  size_t costs_needed;
};

struct pse_eigen_cps_exploration_solver_context_t;

struct PseEigenExplorationFunctor {
  /* NOTE: these declarations are required by Eigen */
  static constexpr int InputsAtCompileTime = Eigen::Dynamic;
  static constexpr int ValuesAtCompileTime = Eigen::Dynamic;
  using Scalar       = pse_real_t;
  using FakeBase     = Eigen::DenseFunctor<Scalar, InputsAtCompileTime, ValuesAtCompileTime>;
  using InputType    = typename FakeBase::InputType;
  using ValueType    = typename FakeBase::ValueType;
  using JacobianType = typename FakeBase::JacobianType;
  using QRSolver     = typename FakeBase::QRSolver;

  using VectorBlock         = Eigen::VectorBlock<Scalar>;
  using JacobianVectorBlock = Eigen::VectorBlock<Eigen::Block<JacobianType>>;

  struct pse_eigen_cps_exploration_t* exp;
  struct pse_eigen_cps_precomputations_t precomp;
  struct pse_eigen_cps_exploration_solver_context_t* ctxt;
  mutable enum pse_res_t last_res;

  PSE_INLINE PseEigenExplorationFunctor()
    : exp(nullptr)
    , ctxt(nullptr)
    , last_res(RES_OK)
  {}
  PSE_INLINE virtual ~PseEigenExplorationFunctor() {}

  PSE_FINLINE int values() const; /* NOTE: required by Eigen */

  PSE_FINLINE enum pse_res_t
  converted_inputs_get
    (const pse_clt_ppoint_variation_uid_t var,
     pse_clt_pspace_uid_t& dst,
     InputType*& input_converted) const;

  /* Main function used by Eigen to compute the costs, given the input */
  PSE_FINLINE int operator()(const InputType& input, ValueType& costs) const;

  /* Do the computation for one cost function */
  PSE_FINLINE enum pse_res_t
  compute
    (const struct pse_eigen_relshp_cost_func_t& rcf,
     typename ValueType::SegmentReturnType costs) const;

#ifndef PSE_EIGEN_REF
  /* Specific version used by df() to compute costs only for a modified value */
  PSE_FINLINE enum pse_res_t
  finite_diffs_compute_df
    (const struct pse_eigen_relshp_cost_func_t& rcf,
     const InputType& x,
     JacobianType& jac,
     size_t costs_start_idx) const;
#endif

  /* Needed by Eigen LM to compute the gradients. */
  PSE_FINLINE int df(const InputType& x, JacobianType& jac) const;
};

using PseEigenExplorationProblem = PseEigenExplorationFunctor;
using PseEigenExplorationSolver = Eigen::LevenbergMarquardt<PseEigenExplorationProblem>;

/* TODO: use our allocator */
typedef std::pair<
  pse_clt_pspace_uid_t,
  pse_clt_ppoint_variation_uid_t
> pse_eigen_cps_inputs_version_t;
typedef std::map<
  pse_eigen_cps_inputs_version_t,
  PseEigenExplorationProblem::InputType
> pse_eigen_cps_inputs_by_pspace_t;

/*! Stores the information related to a specific exploration computation,
 * allowing to work independently from the world and to store the results for
 * later use by the user. */
struct pse_eigen_cps_exploration_solver_context_t {
  struct pse_eval_ctxt_t eval_ctxt;
  PseEigenExplorationProblem::InputType input_full;
  PseEigenExplorationProblem::InputType input;  /*!< in the main pspace */
  PseEigenExplorationProblem::ValueType costs_ref;
  PseEigenExplorationProblem::ValueType costs_tmp1;
  PseEigenExplorationProblem::ValueType costs_tmp2;
  pse_eigen_cps_inputs_by_pspace_t input_full_by_pspace;
  bool need_costs_ref;
  size_t components_count;
  size_t costs_count;
  pse_ppoint_id_t* optimizable_ppoints; /* stretchy buffer */
  Eigen::ComputationInfo algo_info;
  Eigen::LevenbergMarquardtSpace::Status algo_status;
  struct pse_cpspace_exploration_extra_results_t extra;
};

struct pse_eigen_cps_exploration_t {
  pse_eigen_device_t* dev;
  struct pse_cpspace_exploration_ctxt_params_t params;
  struct pse_cpspace_exploration_ctxt_t* clt_ctxt;
  const struct pse_cpspace_instance_t* cpsi;

  PseEigenExplorationProblem* problem;
  PseEigenExplorationSolver* lm;  /* Levenberg-Marquardt */

  /*! We use triple buffering to allow the user to get the last results during
   * an exploration. This allow the user to get the results when it wants, and
   * the exploration to use double buffering for its own efficiency. */
  struct pse_eigen_cps_exploration_solver_context_t ctxts[3];
  size_t last_ctxt_idx; /*! Last context used for computation, so with newest results */
  size_t curr_ctxt_idx; /*! Context used for computation */

  pse_atomic_t lock_internal; /*! Lock to change internal state of the solver */
};

/******************************************************************************
 *
 * PRIVATE CONSTANTS
 *
 ******************************************************************************/

#define PSE_OFFSET_INVALID                                                     \
  ((size_t)-1)
#define PSE_INDEX_INVALID                                                      \
  ((size_t)-1)
#define PSE_COSTS_MEM_CHUNK_EMPTY_                                             \
  { PSE_OFFSET_INVALID, 0 }
#define PSE_EIGEN_RELSHPS_FOR_PPOINT_PRECOMPUTATIONS_EMPTY_                    \
  { PSE_CLT_PPOINT_VARIATION_UID_INVALID_, {}, {}, {}, {}, {}, 0 }
#define PSE_EIGEN_RELSHP_PRECOMPUTATIONS_EMPTY_                                \
  { nullptr, {}, 0 }
#define PSE_EIGEN_RELSHP_COST_FUNC_NULL_                                       \
  { nullptr, 0, {} }
#define PSE_EIGEN_CPS_PRECOMPUTATIONS_EMPTY_                                   \
  { {}, {}, 0 }
#define PSE_EIGEN_CPS_EXPLORATION_SOLVER_CONTEXT_NULL_                         \
  { PSE_EVAL_CTXT_NULL_,                                                       \
    {}, {}, {}, {}, {}, {}, true, 0, 0, nullptr,                               \
    Eigen::NoConvergence,                                                      \
    Eigen::LevenbergMarquardtSpace::NotStarted,                                \
    PSE_CPSPACE_EXPLORATION_EXTRA_RESULTS_NULL_ }
#define PSE_EIGEN_CPS_EXPLORATION_NULL_                                        \
  { nullptr, PSE_CPSPACE_EXPLORATION_CTXT_PARAMS_NULL_, nullptr, nullptr,      \
    nullptr, nullptr,                                                          \
    { PSE_EIGEN_CPS_EXPLORATION_SOLVER_CONTEXT_NULL_,                          \
      PSE_EIGEN_CPS_EXPLORATION_SOLVER_CONTEXT_NULL_,                          \
      PSE_EIGEN_CPS_EXPLORATION_SOLVER_CONTEXT_NULL_ },                        \
    PSE_INDEX_INVALID, PSE_INDEX_INVALID,                                      \
    0 }

static const struct pse_costs_mem_chunk_t PSE_COSTS_MEM_CHUNK_EMPTY =
  PSE_COSTS_MEM_CHUNK_EMPTY_;
static const struct pse_eigen_relshps_for_ppoint_precomputations_t PSE_EIGEN_RELSHPS_FOR_PPOINT_PRECOMPUTATIONS_EMPTY =
  PSE_EIGEN_RELSHPS_FOR_PPOINT_PRECOMPUTATIONS_EMPTY_;
static const struct pse_eigen_relshp_precomputations_t PSE_EIGEN_RELSHP_PRECOMPUTATIONS_EMPTY =
  PSE_EIGEN_RELSHP_PRECOMPUTATIONS_EMPTY_;
static const struct pse_eigen_relshp_cost_func_t PSE_EIGEN_RELSHP_COST_FUNC_NULL =
  PSE_EIGEN_RELSHP_COST_FUNC_NULL_;
static const struct pse_eigen_cps_precomputations_t PSE_EIGEN_CPS_PRECOMPUTATIONS_EMPTY =
  PSE_EIGEN_CPS_PRECOMPUTATIONS_EMPTY_;
static const struct pse_eigen_cps_exploration_solver_context_t PSE_EIGEN_CPS_EXPLORATION_SOLVER_CONTEXT_NULL =
  PSE_EIGEN_CPS_EXPLORATION_SOLVER_CONTEXT_NULL_;
static const struct pse_eigen_cps_exploration_t PSE_EIGEN_CPS_EXPLORATION_NULL =
  PSE_EIGEN_CPS_EXPLORATION_NULL_;

/******************************************************************************
 *
 * EXPLORATION FUNCTOR IMPLEMENTATION
 *
 ******************************************************************************/

PSE_FINLINE int
PseEigenExplorationFunctor::values() const
{
  assert(ctxt->costs_count <= (size_t)INT_MAX);
  return (int)ctxt->costs_count;
}

PSE_FINLINE enum pse_res_t
PseEigenExplorationFunctor::converted_inputs_get
  (const pse_clt_ppoint_variation_uid_t var,
   pse_clt_pspace_uid_t& to,
   PseEigenExplorationFunctor::InputType*& input_converted) const
{
  const pse_clt_pspace_uid_t from = exp->params.pspace.explore_in;
  enum pse_res_t res = RES_OK;
  const pse_eigen_cps_inputs_version_t key(to,var);
  const pse_eigen_cps_inputs_version_t key_var_only(from,var);
  const bool need_pspace_variation =
    (key.second != PSE_CLT_PPOINT_VARIATION_UID_INVALID);
  const bool need_pspace_conversion = (key.first != from);

  if( need_pspace_variation || need_pspace_conversion ) {
    /* We have to do a variation and/or a conversion */
    PseEigenExplorationFunctor::InputType* input = &ctxt->input_full;
    PseEigenExplorationFunctor::InputType* output = nullptr;

    if( need_pspace_variation ) {
      const bool has_done_variation =
        (ctxt->input_full_by_pspace.count(key_var_only) > 0);
      output = &ctxt->input_full_by_pspace[key_var_only];

      if( !has_done_variation ) {
        assert(exp->params.variations.apply);

        /* Get the buffer of the main pspace for the given variation. */
        *output = *input;
        PSE_CALL_OR_RETURN(res, exp->params.variations.apply
          (exp->params.variations.apply_user_data,
           from, var, sb_count(exp->cpsi->ppoints),
           input->data(), output->data()));
      }
    }

    if( need_pspace_conversion ) {
      const bool has_done_conversion =
        (ctxt->input_full_by_pspace.count(key) > 0);
      output = &ctxt->input_full_by_pspace[key];

      if( !has_done_conversion ) {
        assert(exp->params.pspace.convert);

        /* Get the buffer of the given pspace for the given variation. */
        *output = *input;
        PSE_CALL_OR_RETURN(res, exp->params.pspace.convert
          (exp->params.pspace.convert_user_data,
           from, to, sb_count(exp->cpsi->ppoints),
           input->data(), output->data()));
      }
    }

    /* The last output is our input variated and/or converted */
    input_converted = output;
  } else {
    /* No variation in the main pspace: return the main buffer. */
    to = from;
    input_converted = &ctxt->input_full;
  }
  assert(input_converted);
  return RES_OK;
}

/* Main function used by Eigen to compute the costs, given the input */
PSE_FINLINE int
PseEigenExplorationFunctor::operator()
  (const InputType& input,
   ValueType& costs) const
{
  size_t i, j, costs_start_idx = 0;
  assert(ctxt);

  /* We entering a new computation, clear the spaces conversion cache */
  ctxt->input_full_by_pspace.clear();

  /* TODO: check if relationships are needed and avoid to compute them if not:
   * If the ppoints implied are all locked, the relationship is not needed. */

  /* Copy the input buffer to our internal buffer in order to have all ppoint
   * (locked and optimizable) in the same buffer. Note that the locked ppoints
   * already have their values set and thus we only go through the ppoints we
   * have to optimize. */
  /* TODO: use a loop on chunks instead of looping on optimizable ppoints, that
   * will allow to copy faster */
  assert(ctxt->input_full.size() >= input.size());
  assert(sb_count(ctxt->optimizable_ppoints)*ctxt->components_count == (size_t)input.size());
  const size_t comps_count = ctxt->components_count;
  for(i = 0; i < sb_count(ctxt->optimizable_ppoints); ++i) {
    for(j = 0; j < comps_count; ++j) {
      const pse_ppoint_id_t ppid = ctxt->optimizable_ppoints[i];
      ctxt->input_full[ppid*comps_count+j] = input[i*comps_count+j];
    }
  }
  /* Converted inputs will use ctxt->input_full as the up-to-date values. */

  last_res = RES_OK;
  for(const auto& fpcp: precomp.relshp_cost_funcs) {
    const struct pse_eigen_relshp_cost_func_t& rcf = fpcp.second;
    const size_t costs_count =
      rcf.costs_count_per_variation * rcf.idata->variations_count;
    if( costs_count <= 0 )
      continue;
    assert(costs_start_idx + costs_count <= ctxt->costs_count);
    PSE_CALL_OR_GOTO(last_res,exit, compute
      (rcf, costs.segment(costs_start_idx, costs_count)));
    costs_start_idx += costs_count;
  }
  assert(costs_start_idx <= ctxt->costs_count);

  /* Set remaining unused costs to 0, if any */
  if( costs_start_idx < ctxt->costs_count ) {
    costs
      .segment(costs_start_idx, ctxt->costs_count - costs_start_idx)
      .setZero();
  }

exit:
  return last_res != RES_OK ? -1 : 0;  // On error, ask to stop
}

PSE_FINLINE enum pse_res_t
PseEigenExplorationFunctor::compute
  (const struct pse_eigen_relshp_cost_func_t& rcf,
   typename ValueType::SegmentReturnType costs) const
{
  enum pse_res_t res = RES_OK;

  /* Get the converted inputs, for each variation of the instance. */
  pse_clt_pspace_uid_t func_pspace = rcf.idata->params.expected_pspace;
  PseEigenExplorationFunctor::InputType* input_converted = nullptr;
  for(size_t i = 0; i < rcf.idata->variations_count; ++i) {
    const struct pse_cpspace_instance_variated_cost_func_data_t* ivcfd =
      &rcf.idata->variations[i];
    PSE_CALL_OR_RETURN(res, converted_inputs_get
      (ivcfd->uid, func_pspace, input_converted));

    struct pse_eval_relshps_t eval_relshps = PSE_EVAL_RELSHPS_NULL;
    eval_relshps.count = ivcfd->relshps_count;
    eval_relshps.ids = ivcfd->relshps_ids;
    eval_relshps.data = ivcfd->relshps_data;
    eval_relshps.ctxts = ivcfd->relshps_ctxts;
    eval_relshps.configs = ivcfd->relshps_configs;

    struct pse_eval_coordinates_t eval_coords = PSE_EVAL_COORDINATES_NULL;
    eval_coords.pspace_uid = func_pspace;
    eval_coords.scalars_count = input_converted->size();
    eval_coords.coords = input_converted->data();

    PSE_CALL_OR_RETURN(last_res, rcf.idata->params.compute
      (&ctxt->eval_ctxt, &eval_coords, &eval_relshps,
       costs.segment
         (i*rcf.costs_count_per_variation,
          rcf.costs_count_per_variation).data()));
  }
  return RES_OK;
}

#ifdef PSE_EIGEN_REF
// Sligthly adjusted df function of Eigen::NumericalDiff class, for easier
// tweaking purpose in order to compare with our own implementation.
PSE_FINLINE int
PseEigenExplorationFunctor::df
  (const InputType& _x,
   JacobianType& jac) const
{
  const Scalar epsfcn = exp->params.options.auto_df_epsilon;
  using std::sqrt;
  using std::abs;
  /* Local variables */
  Scalar h;
  int nfev=0;
  const typename InputType::Index n = _x.size();
  const Scalar eps = sqrt(((std::max)(epsfcn,Eigen::NumTraits<Scalar>::epsilon() )));
  ValueType val1, val2;
  InputType x = _x;
  // TODO : we should do this only if the size is not already known
  val1.resize(values());
  val2.resize(values());

  // Function Body
  for (int j = 0; j < n; ++j) {
    h = eps * abs(x[j]);
    if (h == 0.) {
        h = eps;
    }
    x[j] += h;
    operator()(x, val2); nfev++;
    x[j] -= 2*h;
    operator()(x, val1); nfev++;
    x[j] = _x[j];
    jac.col(j) = (val2-val1)/(2*h);
  }
  return nfev;
}
#else
PSE_FINLINE int
PseEigenExplorationFunctor::df
  (const InputType& x,
   JacobianType& jac) const
{
  /* Local variables */
  int nfev = 0;
  jac.setZero();

  assert(ctxt->input.size() == x.size());

  size_t cost_start_idx = 0;
  for(auto& fpcp: precomp.relshp_cost_funcs) {
    const struct pse_eigen_relshp_cost_func_t& rcf = fpcp.second;
    if( rcf.costs_count_per_variation <= 0 )
      continue;
    if( rcf.idata->params.compute_df ) {
      // TODO: call the compute_df function
      ++nfev;
    } else {
      // The cost function do not provide a compute_df function, so we do this
      // computation "by hand" by using finite differences.

      // First, compute the cost for the given input, as precomputed costs
      // that we will need to keep for later use.
      if( ctxt->need_costs_ref ) {
        // This call will fill the 'input_full' buffer with values of 'x'
        // TODO: we should separate the initialization of the 'input_full'
        // buffer from the computation of the costs as here, we know which
        // function we want to evaluate.
        if( this->operator ()(x, ctxt->costs_ref) < 0 )
          return -1;
      }
      // It's only the first time, whichever the cost function we want to
      // evaluate.
      ctxt->need_costs_ref = false;

      PSE_CALL_OR_GOTO(last_res,error, finite_diffs_compute_df
        (rcf, x, jac, cost_start_idx));
    }
    cost_start_idx +=
      rcf.costs_count_per_variation * rcf.idata->variations_count;
  }

  // fake number of function evaluation in order to have the same behavior than
  // before.
  nfev += 2 * (int)x.size();
exit:
  ctxt->need_costs_ref = true;  // Next time, 'x' will be different!
  return nfev;
error:
  nfev = -1;
  goto exit;
}

PSE_FINLINE enum pse_res_t
PseEigenExplorationFunctor::finite_diffs_compute_df
  (const struct pse_eigen_relshp_cost_func_t& rcf,
   const InputType& x,
   JacobianType& jac,
   size_t costs_start_idx) const
{
  enum pse_res_t res = RES_OK;
  pse_real_t h, ref_value;
  size_t i, j, k, v;
  size_t input_val_idx_in_x, input_val_idx_in_full;
  size_t relshp_costs_start_idx;
  pse_ppoint_id_t ppid;
  const Scalar func_eps = PSE_REAL_EPS; // TODO: get the function epsilon
  const Scalar scalar_eps = Eigen::NumTraits<Scalar>::epsilon();
  const Scalar eps = std::sqrt(PSE_MAX(func_eps, scalar_eps));

  /* Get the converted inputs if needed, for each variation */
  pse_clt_pspace_uid_t func_pspace = rcf.idata->params.expected_pspace;
  PseEigenExplorationFunctor::InputType* input_converted = nullptr;
  for(v = 0; v < rcf.idata->variations_count; ++v) {
    const pse_clt_ppoint_variation_uid_t variation = rcf.idata->variations[v].uid;
    PSE_CALL_OR_RETURN(res, converted_inputs_get
      (variation, func_pspace, input_converted));

    struct pse_eval_coordinates_t eval_coords = PSE_EVAL_COORDINATES_NULL;
    eval_coords.pspace_uid = func_pspace;
    eval_coords.scalars_count = input_converted->size();
    eval_coords.coords = input_converted->data();

    assert(costs_start_idx + rcf.costs_count_per_variation <= ctxt->costs_count);

    for(i = 0; i < sb_count(ctxt->optimizable_ppoints); ++i) {
      ppid = ctxt->optimizable_ppoints[i];

      const pse_variated_ppoint_id_t vppid(ppid, variation);
      assert(rcf.relshps_per_ppoint.count(vppid) > 0);

      const struct pse_eigen_relshps_for_ppoint_precomputations_t& rfppp =
        rcf.relshps_per_ppoint.at(vppid);
      if( rfppp.costs_count <= 0 )
        continue; /* No relationships which use this ppoint */

      struct pse_eval_relshps_t eval_relshps = PSE_EVAL_RELSHPS_NULL;
      eval_relshps = PSE_EVAL_RELSHPS_NULL;
      eval_relshps.count = rfppp.relshps.size();
      eval_relshps.ids = rfppp.relshps.data(),
      eval_relshps.data = rfppp.relshps_eval_data.data(),
      eval_relshps.ctxts = rfppp.relshps_ctxt.empty()
        ? nullptr
        : rfppp.relshps_ctxt.data();
      eval_relshps.configs = rfppp.relshps_config.empty()
        ? nullptr
        : rfppp.relshps_config.data();

      ctxt->costs_tmp1.resize(rfppp.costs_count);
      ctxt->costs_tmp2.resize(rfppp.costs_count);

      for(j = 0; j < ctxt->components_count; ++j) {
        input_val_idx_in_x = i*ctxt->components_count+j;
        input_val_idx_in_full = ppid*ctxt->components_count+j;
        ref_value = x[input_val_idx_in_x];
        h = eps * std::abs(x[j]);
        if (h == 0.) {
          h = eps;
        }

        /* Compute the cost at -delta */
        (*input_converted)[input_val_idx_in_full] = ref_value - h;
        PSE_CALL_OR_GOTO(res,exit, rcf.idata->params.compute
          (&ctxt->eval_ctxt, &eval_coords,
           &eval_relshps, ctxt->costs_tmp1.data()));

        /* Compute the cost at +delta */
        (*input_converted)[input_val_idx_in_full] = ref_value + h;
        PSE_CALL_OR_GOTO(res,exit, rcf.idata->params.compute
          (&ctxt->eval_ctxt, &eval_coords,
           &eval_relshps, ctxt->costs_tmp2.data()));

        /* restore the value for this ppoint */
        (*input_converted)[input_val_idx_in_full] = ref_value;

        /* Compute the difference to get the gradients */
        ctxt->costs_tmp2 -= ctxt->costs_tmp1;
        ctxt->costs_tmp2 /= (2*h);

        /* Now, copy meaningfull gradients in the jacobian */
        relshp_costs_start_idx = 0;
        JacobianType::ColXpr::SegmentReturnType jac_segment =
          jac.col(input_val_idx_in_x)
             .segment(costs_start_idx, rcf.costs_count_per_variation);
        for(k = 0; k < rfppp.relshps_costs_chunks.size(); ++k) {
          const struct pse_costs_mem_chunk_t& chunk = rfppp.relshps_costs_chunks[k];
          jac_segment.segment(chunk.offset, chunk.count) =
            ctxt->costs_tmp2.segment(relshp_costs_start_idx, chunk.count);
          relshp_costs_start_idx += chunk.count;
        }
      }
    }
    /* For each variation, we have a full costs set. */
    costs_start_idx += rcf.costs_count_per_variation;
  }

exit:
  return res;
}
#endif

/******************************************************************************
 *
 * HELPER FUNCTIONS
 *
 ******************************************************************************/

static PSE_FINLINE const char*
pseEigenComputationInfoToCString
  (const Eigen::ComputationInfo info)
{
  switch(info) {
    case Eigen::Success: return "Success";
    case Eigen::NumericalIssue: return "NumericalIssue";
    case Eigen::NoConvergence: return "NoConvergence";
    case Eigen::InvalidInput: return "InvalidInput";
    default: break;
  }
  return "unknown";
}

static PSE_FINLINE const char*
pseEigenLMStatusToCString
  (const Eigen::LevenbergMarquardtSpace::Status status)
{
  switch(status) {
    case Eigen::LevenbergMarquardtSpace::NotStarted:
      return "NotStarted";
    case Eigen::LevenbergMarquardtSpace::Running:
      return "Running";
    case Eigen::LevenbergMarquardtSpace::ImproperInputParameters:
      return "ImproperInputParameters";
    case Eigen::LevenbergMarquardtSpace::RelativeReductionTooSmall:
      return "RelativeReductionTooSmall";
    case Eigen::LevenbergMarquardtSpace::RelativeErrorTooSmall:
      return "RelativeErrorTooSmall";
    case Eigen::LevenbergMarquardtSpace::RelativeErrorAndReductionTooSmall:
      return "RelativeErrorAndReductionTooSmall";
    case Eigen::LevenbergMarquardtSpace::CosinusTooSmall:
      return "CosinusTooSmall";
    case Eigen::LevenbergMarquardtSpace::TooManyFunctionEvaluation:
      return "TooManyFunctionEvaluation";
    case Eigen::LevenbergMarquardtSpace::FtolTooSmall:
      return "FtolTooSmall";
    case Eigen::LevenbergMarquardtSpace::XtolTooSmall:
      return "XtolTooSmall";
    case Eigen::LevenbergMarquardtSpace::GtolTooSmall:
      return "GtolTooSmall";
    case Eigen::LevenbergMarquardtSpace::UserAsked:
      return "UserAsked";
    default: break;
  }
  return "unknown";
}

static PSE_INLINE enum pse_res_t
pseEigenExplorationCopySamples
  (const struct pse_cpspace_values_data_t* smpls,
   const pse_ppoint_id_t* ppoints,
   PseEigenExplorationFunctor::InputType& input)
{
  const size_t ppoints_count = sb_count(ppoints);
  const struct pse_attrib_value_accessors_t* accessors = nullptr;
  enum pse_res_t res = RES_OK;
  assert(smpls && ppoints);

  accessors = pseEigenValuesAccessorsGet(smpls, PSE_POINT_ATTRIB_COORDINATES);
  PSE_VERIFY_OR_ELSE(accessors && accessors->get, res = RES_BAD_ARG; goto exit);
  PSE_CALL_OR_GOTO(res,exit, accessors->get(accessors->ctxt,
    PSE_POINT_ATTRIB_COORDINATES,
    PSE_TYPE_REAL,
    ppoints_count,
    ppoints,
    input.data()));

exit:
  return res;
}

static PSE_FINLINE enum pse_res_t
pseEigenExplorationOptimizableParametricPointsCompute
  (const struct pse_cpspace_instance_t* cpsi,
   const struct pse_cpspace_values_data_t* smpls,
   pse_ppoint_id_t*& optimizable_ppoints)
{
  enum pse_res_t res = RES_OK;
  const struct pse_attrib_value_accessors_t* accessors = nullptr;
  size_t locked_count = 0;
  assert(cpsi && smpls && !sb_count(optimizable_ppoints));

  accessors = pseEigenValuesAccessorsGet(smpls, PSE_POINT_ATTRIB_LOCK_STATUS);
  assert(accessors);
  if( accessors->get ) {
    std::vector<uint8_t> lock_status(cpsi->ppoints_count, false);
    PSE_CALL_OR_RETURN(res, accessors->get(accessors->ctxt,
      PSE_POINT_ATTRIB_LOCK_STATUS,
      PSE_TYPE_BOOL_8,
      cpsi->ppoints_count,
      cpsi->ppoints,
      lock_status.data()));
    for(size_t i = 0; i < cpsi->ppoints_count; ++i) {
      if( lock_status[i] ) {
        ++locked_count;
      } else {
        sb_push(optimizable_ppoints, cpsi->ppoints[i]);
      }
    }
  } else {
    /* No accessors, so all parametric points are unlocked */
    for(size_t i = 0; i < cpsi->ppoints_count; ++i) {
      sb_push(optimizable_ppoints, cpsi->ppoints[i]);
    }
  }

  assert(sb_count(optimizable_ppoints) == (cpsi->ppoints_count - locked_count));
  if( sb_count(optimizable_ppoints) <= 0 )
    return RES_NOT_FOUND;
  return res;
}

static PSE_INLINE void
pseEigenExplorationSolverContextClean
  (struct pse_eigen_cps_exploration_solver_context_t* ctxt)
{
  sb_free(ctxt->optimizable_ppoints);
  ctxt->input.resize(0);
  *ctxt = PSE_EIGEN_CPS_EXPLORATION_SOLVER_CONTEXT_NULL;
}

static PSE_INLINE enum pse_res_t
pseEigenExplorationSolverContextSetup
  (const struct pse_eigen_cps_exploration_t* exp,
   const struct pse_cpspace_values_data_t* smpls,
   struct pse_eigen_cps_exploration_solver_context_t* ctxt)
{
  enum pse_res_t res = RES_OK;
  struct pse_cpspace_instance_pspace_data_t* psd = nullptr;
  size_t coords_comps_count;
  size_t optimizable_ppoints_count;
  size_t optimizable_ppoints_scalars_count;
  size_t i;
  assert(exp && smpls && ctxt);

  *ctxt = PSE_EIGEN_CPS_EXPLORATION_SOLVER_CONTEXT_NULL;
  assert(sb_count(ctxt->optimizable_ppoints) == 0);
  PSE_TRY_CALL_OR_RETURN(res,
    pseEigenExplorationOptimizableParametricPointsCompute
      (exp->cpsi, smpls, ctxt->optimizable_ppoints));

  optimizable_ppoints_count = sb_count(ctxt->optimizable_ppoints);

  /* Compute the number of scalars that are associated to the parametric points
   * we have to optimize. */
  for(i = 0; i < exp->cpsi->pspaces_count; ++i) {
    psd = &exp->cpsi->pspaces[i];
    if( psd->key == exp->params.pspace.explore_in )
      break;
  }
  assert(exp->params.pspace.explore_in == psd->key);
  coords_comps_count =
    psd->ppoint_attribs_comps_count[PSE_POINT_ATTRIB_COORDINATES];
  optimizable_ppoints_scalars_count =
    optimizable_ppoints_count * coords_comps_count;

  ctxt->components_count = coords_comps_count;
  ctxt->input.resize(optimizable_ppoints_scalars_count);
  ctxt->input_full.resize(sb_count(exp->cpsi->ppoints) * coords_comps_count);

  /* TODO: is it normal to use a MAX here? Is this a problem for the results if
   * some costs are allocated but not used? */
  ctxt->costs_count = PSE_MAX
    (optimizable_ppoints_scalars_count,
     exp->problem->precomp.costs_needed);
  ctxt->costs_ref.resize(ctxt->costs_count);
  ctxt->costs_tmp1.resize(ctxt->costs_count);
  ctxt->costs_tmp2.resize(ctxt->costs_count);
  ctxt->need_costs_ref = true;

  PSE_CALL_OR_GOTO(res,error, pseEigenExplorationCopySamples
    (smpls, ctxt->optimizable_ppoints, ctxt->input));
  PSE_CALL_OR_GOTO(res,error, pseEigenExplorationCopySamples
    (smpls, exp->cpsi->ppoints, ctxt->input_full));

  ctxt->eval_ctxt.dev = exp->dev->clt_dev;
  ctxt->eval_ctxt.cps = exp->cpsi->cps;
  ctxt->eval_ctxt.exp_ctxt = exp->clt_ctxt;

exit:
  return res;
error:
  pseEigenExplorationSolverContextClean(ctxt);
  goto exit;
}

static PSE_FINLINE enum pse_res_t
pseEigenExplorationSolverContextStatusCheck
  (struct pse_eigen_cps_exploration_solver_context_t* ctxt,
   const enum pse_res_t last_res)
{
  assert(ctxt);
  switch(ctxt->algo_status) {
    case Eigen::LevenbergMarquardtSpace::UserAsked:
      return last_res; /* result from internal calls of the minimization */
    case Eigen::LevenbergMarquardtSpace::ImproperInputParameters:
      return (ctxt->algo_info == Eigen::NumericalIssue)
        ? RES_NUMERICAL_ISSUE
        : RES_BAD_ARG;
    case Eigen::LevenbergMarquardtSpace::NotStarted: /* fallthrough */
    case Eigen::LevenbergMarquardtSpace::Running:
      return RES_NOT_CONVERGED;
    case Eigen::LevenbergMarquardtSpace::RelativeReductionTooSmall: /* fallthrough */
    case Eigen::LevenbergMarquardtSpace::RelativeErrorTooSmall: /* fallthrough */
    case Eigen::LevenbergMarquardtSpace::RelativeErrorAndReductionTooSmall: /* fallthrough */
    case Eigen::LevenbergMarquardtSpace::CosinusTooSmall: /* fallthrough */
    case Eigen::LevenbergMarquardtSpace::FtolTooSmall: /* fallthrough */
    case Eigen::LevenbergMarquardtSpace::XtolTooSmall: /* fallthrough */
    case Eigen::LevenbergMarquardtSpace::GtolTooSmall:
      return RES_OK;
    case Eigen::LevenbergMarquardtSpace::TooManyFunctionEvaluation:
      return RES_NOT_CONVERGED;
    default: assert(false); /* We should have managed all cases! */
  }
  return RES_INTERNAL;
}

static PSE_FINLINE void
pseEigenExplorationSolverPrecomputationClean
  (struct pse_eigen_cps_exploration_t* exp)
{
  exp->problem->precomp.relshps.clear();
  exp->problem->precomp.relshp_cost_funcs.clear();

  for(size_t i = 0; i < 3; ++i) {
    pseEigenExplorationSolverContextClean(&exp->ctxts[i]);
  }

  sb_free(exp->cpsi->ppoints);
}

#ifndef PSE_EIGEN_REF
static PSE_FINLINE enum pse_res_t
pseEigenExplorationSolverRelationshipPerPointAddAndPrecompute
  (const struct pse_eigen_cps_precomputations_t& cpsp,
   struct pse_eigen_relshp_cost_func_t& rcf,
   const pse_ppoint_id_t ppid)
{
  for(size_t i = 0; i < rcf.idata->variations_count; ++i) {
    struct pse_cpspace_instance_variated_cost_func_data_t* ivcfd =
      &rcf.idata->variations[i];
    const size_t relshps_count = ivcfd->relshps_count;
    struct pse_eigen_relshps_for_ppoint_precomputations_t& rfppp =
      rcf.relshps_per_ppoint[pse_variated_ppoint_id_t(ppid, ivcfd->uid)];

    rfppp = PSE_EIGEN_RELSHPS_FOR_PPOINT_PRECOMPUTATIONS_EMPTY;
    rfppp.variation = ivcfd->uid;
    rfppp.relshps.reserve(relshps_count);
    rfppp.relshps_ctxt.reserve(relshps_count);
    rfppp.relshps_config.reserve(relshps_count);
    rfppp.relshps_eval_data.reserve(relshps_count);
    rfppp.relshps_costs_chunks.reserve(relshps_count/2);

    // TODO: this loop cost a lot and may be precomputed in the PSE library!
    rfppp.costs_count = 0;
    struct pse_costs_mem_chunk_t chunk = PSE_COSTS_MEM_CHUNK_EMPTY;
    size_t relshp_costs_start_idx = 0;
    for(size_t j = 0; j < relshps_count; ++j) {
      const pse_relshp_id_t rid = ivcfd->relshps_ids[j];
      const struct pse_eigen_relshp_precomputations_t& rp = cpsp.relshps.at(rid);
      const pse_eigen_ppoint_id_set_t& ppoints_involved =
        rp.ppoints_involved_set;
      if( ppoints_involved.count(ppid) > 0 ) {
        rfppp.relshps.push_back(rid);
        rfppp.relshps_ctxt.push_back(ivcfd->relshps_ctxts[j]);
        rfppp.relshps_config.push_back(ivcfd->relshps_configs[j]);
        rfppp.relshps_eval_data.push_back(ivcfd->relshps_data[j]);
        if( chunk.offset == PSE_OFFSET_INVALID ) {
          chunk.offset = relshp_costs_start_idx;
        } else {
          assert(chunk.offset + chunk.count == relshp_costs_start_idx);
        }
        chunk.count += rp.costs_count;
        rfppp.costs_count += rp.costs_count;
      } else if( chunk.count > 0 ) {
        // This chunk is finished
        rfppp.relshps_costs_chunks.push_back(chunk);
        chunk = PSE_COSTS_MEM_CHUNK_EMPTY;
      }
      relshp_costs_start_idx += rp.costs_count;
    }
    if( chunk.count > 0 ) {
      // Keep the last chunk
      rfppp.relshps_costs_chunks.push_back(chunk);
    }
    assert(relshp_costs_start_idx == rcf.costs_count_per_variation);
    assert(rfppp.costs_count <= rcf.costs_count_per_variation);
  }
  return RES_OK;
}
#endif

static PSE_INLINE enum pse_res_t
pseEigenExplorationSolverPrecompute
  (struct pse_eigen_cps_exploration_t* exp)
{
  enum pse_res_t res = RES_OK;
  PseEigenExplorationProblem* pb = nullptr;
  const struct pse_cpspace_instance_t* cpsi = nullptr;
  assert(exp);

  pb = exp->problem;
  cpsi = exp->cpsi;

  /* Prepare the relationships precomputations. */
  for(size_t i = 0; i < cpsi->relshps_count; ++i) {
    const struct pse_cpspace_instance_relshp_data_t* ird = &cpsi->relshps[i];
    const pse_relshp_id_t rid = (pse_relshp_id_t)ird->key;

    assert(pb->precomp.relshps.count(rid) == 0);
    struct pse_eigen_relshp_precomputations_t& rpc = pb->precomp.relshps[rid];

    rpc.idata = ird;
    rpc.costs_count = 0;

    /* We keep a set to quickly check if a ppoint is involved in a relatioship */
    rpc.ppoints_involved_set.reserve(ird->eval_data.ppoints_count);
    for(size_t j = 0; j < ird->eval_data.ppoints_count; ++j) {
      rpc.ppoints_involved_set.insert(ird->eval_data.ppoints[j]);
    }
  }

  /* Prepare the cost functors precomputations. */
  for(size_t i = 0; i < cpsi->cfuncs_count; ++i) {
    const struct pse_cpspace_instance_cost_func_data_t* icfd = &cpsi->cfuncs[i];
    const pse_relshp_cost_func_id_t rcfid = (pse_relshp_cost_func_id_t)icfd->key;

    assert(pb->precomp.relshp_cost_funcs.count(rcfid) == 0);
    struct pse_eigen_relshp_cost_func_t& rcfpc =
      pb->precomp.relshp_cost_funcs[rcfid];

    rcfpc.idata = icfd;
    rcfpc.costs_count_per_variation = 0;
  }

  /* Second pass to compute the final costs count */
  /* TODO: this pass may be merged with the first one */
  pb->precomp.costs_needed = 0;
  for(auto& fpcp: pb->precomp.relshp_cost_funcs) {
    struct pse_eigen_relshp_cost_func_t& rcf = fpcp.second;
    const struct pse_relshp_cost_func_params_t* rcfp = &rcf.idata->params;
    assert(rcf.idata->variations_count > 0);
    const struct pse_cpspace_instance_variated_cost_func_data_t* main_ivcfd =
      &rcf.idata->variations[0];
    assert(main_ivcfd->uid == PSE_CLT_PPOINT_VARIATION_UID_INVALID);
    /* The main variated cost func will have the list of all relationships, not
     * filtered by applicability of the variations. */

    /* Compute the number of costs needed by this functor */
    rcf.costs_count_per_variation = 0;
    switch(rcfp->cost_arity_mode) {
      case PSE_COST_ARITY_MODE_PER_RELATIONSHIP: {
        rcf.costs_count_per_variation =
          rcfp->costs_count * main_ivcfd->relshps_count;
        /* Keep the costs count for each relationship */
        for(size_t i = 0; i < main_ivcfd->relshps_count; ++i) {
          const pse_relshp_id_t rid = main_ivcfd->relshps_ids[i];
          pb->precomp.relshps[rid].costs_count = rcfp->costs_count;
        }
      } break;
      case PSE_COST_ARITY_MODE_PER_POINT: {
        for(size_t i = 0; i < main_ivcfd->relshps_count; ++i) {
          const pse_relshp_id_t rid = main_ivcfd->relshps_ids[i];
          struct pse_eigen_relshp_precomputations_t& rp = pb->precomp.relshps[rid];
          /* Keep the costs count for each relationship */
          rp.costs_count = rcfp->costs_count * rp.idata->eval_data.ppoints_count;
          rcf.costs_count_per_variation += rp.costs_count;
        }
      } break;
      default: assert(false); res = RES_INTERNAL; goto error;
    }
    pb->precomp.costs_needed +=
      rcf.costs_count_per_variation * rcf.idata->variations_count;

#ifndef PSE_EIGEN_REF
    for(size_t i = 0; i < sb_count(cpsi->ppoints); ++i) {
      const pse_ppoint_id_t ppid = cpsi->ppoints[i];
      PSE_CALL_OR_GOTO(res,error,
        pseEigenExplorationSolverRelationshipPerPointAddAndPrecompute
          (pb->precomp, rcf, ppid));
    }
#endif
  }

exit:
  return res;
error:
  pseEigenExplorationSolverPrecomputationClean(exp);
  goto exit;
}

static PSE_FINLINE void
pseEigenExplorationCountersPrepare
  (const struct pse_cpspace_exploration_options_t* opts,
   struct pse_cpspace_exploration_extra_results_t* extra)
{
  assert(opts && extra);
  (void)opts;
  extra->counter_costs_calls.last_call = 0;
  extra->counter_iterations.last_call = 0;
}

static PSE_FINLINE void
pseEigenExplorationCountersFinalize
  (const struct pse_cpspace_exploration_options_t* opts,
   struct pse_cpspace_exploration_extra_results_t* extra)
{
  assert(opts && extra);
  (void)opts;
  extra->counter_costs_calls.total += extra->counter_costs_calls.last_call;
  extra->counter_iterations.total += extra->counter_iterations.last_call;
}

static PSE_FINLINE void
pseEigenExplorationResultLog
  (struct pse_logger_t* logger,
   const char* name,
   struct pse_eigen_cps_exploration_solver_context_t* ctxt)
{
  char buff[16] = {0};
  PSE_LOG(logger, DEBUG, name);
  PSE_LOG(logger, DEBUG, " -> Eigen LM status: ");
  PSE_LOG(logger, DEBUG, pseEigenLMStatusToCString(ctxt->algo_status));
  PSE_LOG(logger, DEBUG, "\n");

  PSE_LOG(logger, DEBUG, name);
  PSE_LOG(logger, DEBUG, " -> Eigen computation info: ");
  PSE_LOG(logger, DEBUG, pseEigenComputationInfoToCString(ctxt->algo_info));
  PSE_LOG(logger, DEBUG, "\n");

  PSE_LOG(logger, DEBUG, name);
  PSE_LOG(logger, DEBUG, " -> Number of costs calls (last call/total): ");
  sprintf(buff, "%I64u", (uint64_t)ctxt->extra.counter_costs_calls.last_call);
  PSE_LOG(logger, DEBUG, buff);
  PSE_LOG(logger, DEBUG, "/");
  sprintf(buff, "%I64u", (uint64_t)ctxt->extra.counter_costs_calls.total);
  PSE_LOG(logger, DEBUG, buff);
  PSE_LOG(logger, DEBUG, "\n");

  PSE_LOG(logger, DEBUG, name);
  PSE_LOG(logger, DEBUG, " -> Number of iterations (last call/total): ");
  sprintf(buff, "%I64u", (uint64_t)ctxt->extra.counter_iterations.last_call);
  PSE_LOG(logger, DEBUG, buff);
  PSE_LOG(logger, DEBUG, "/");
  sprintf(buff, "%I64u", (uint64_t)ctxt->extra.counter_iterations.total);
  PSE_LOG(logger, DEBUG, buff);
  PSE_LOG(logger, DEBUG, "\n");
}

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

/* TODO: add a lock in order to avoid to setup an exploration during another
 * computation. We also could do a double buffer implementation in order to
 * avoid such lock. Right now, we do something that will not work properly: we
 * check if the 'values' pointer is NULL or not. But it's not modified
 * atomically. */

enum pse_res_t
pseEigenExplorationCreate
  (struct pse_eigen_device_t* dev,
   struct pse_cpspace_exploration_ctxt_t* ctxt,
   const struct pse_cpspace_instance_t* cpsi,
   const struct pse_cpspace_exploration_ctxt_params_t* params,
   struct pse_eigen_cps_exploration_t** out_exp)
{
  enum pse_res_t res = RES_OK;
  struct pse_eigen_cps_exploration_t* exp = nullptr;
  assert(out_exp && ctxt && cpsi && params);
  //assert(cpsi->pspaces_count > 0);
  //assert(cpsi->cfuncs_count > 0);
  //assert(cpsi->ppoints_count > 0);
  //assert(cpsi->relshps_count > 0); /* Or nothing to do */

  exp = PSE_TYPED_ALLOC(dev->allocator, struct pse_eigen_cps_exploration_t);
  PSE_VERIFY_OR_ELSE(exp != nullptr, res = RES_MEM_ERR; goto error);
  exp = new(exp) pse_eigen_cps_exploration_t{};
  *exp = PSE_EIGEN_CPS_EXPLORATION_NULL;
  exp->problem = new PseEigenExplorationProblem{};
  PSE_VERIFY_OR_ELSE(exp->problem != nullptr, res = RES_MEM_ERR; goto error);
  exp->lm = new PseEigenExplorationSolver{*exp->problem};
  PSE_VERIFY_OR_ELSE(exp->lm != nullptr, res = RES_MEM_ERR; goto error);

  exp->dev = dev;
  exp->cpsi = cpsi;
  exp->clt_ctxt = ctxt;
  exp->params = *params;
  exp->problem->exp = exp;

  PSE_CALL_OR_GOTO(res,error, pseEigenExplorationSolverPrecompute(exp));

  *out_exp = exp;

exit:
  return res;
error:
  if( exp ) {
    delete exp->lm;
    delete exp->problem;
    PSE_FREE(exp->dev->allocator, exp);
  }
  goto exit;
}

enum pse_res_t
pseEigenExplorationDestroy
  (struct pse_eigen_cps_exploration_t* exp)
{
  assert(exp);

  pseEigenExplorationSolverPrecomputationClean(exp);
  delete exp->lm;
  delete exp->problem;
  PSE_FREE(exp->dev->allocator, exp);

  return RES_OK;
}

enum pse_res_t
pseEigenExplorationSolveBegin
  (struct pse_eigen_cps_exploration_t* exp,
   const struct pse_cpspace_values_data_t* smpls)
{
  enum pse_res_t res = RES_OK;
  struct pse_eigen_cps_exploration_solver_context_t* ctxt = nullptr;
  assert(exp && smpls);

  // Fast check without locking everyone
  if( exp->curr_ctxt_idx != PSE_INDEX_INVALID )
    return RES_BUSY;

  // Lock the solver to update the indices
  if( PSE_ATOMIC_SET(&exp->lock_internal, 1) == 1 )
    return RES_BUSY; // Already locked

  // Secured check within the lock scope, just in case something append just
  // before the lock.
  PSE_TRY_VERIFY_OR_ELSE
    (exp->curr_ctxt_idx == PSE_INDEX_INVALID,
     res = RES_BUSY; goto error);

  exp->curr_ctxt_idx = (exp->last_ctxt_idx + 1) % 3;
  assert(exp->curr_ctxt_idx != exp->last_ctxt_idx);
  ctxt = &exp->ctxts[exp->curr_ctxt_idx];

  res = pseEigenExplorationSolverContextSetup(exp, smpls, ctxt);
  if( res == RES_NOT_FOUND ) {
    /* There is no point to optimize, just leave silently with the same result
     * as the caller must be aware of this impossibility to avoid step/end
     * calls. */
    PSE_LOG(exp->dev->logger, DEBUG, "Begin -> No point to optimize\n");
    goto error;
  }
  PSE_VERIFY_OR_ELSE(res == RES_OK, goto error);

  pseEigenExplorationCountersPrepare(&exp->params.options, &ctxt->extra);
  exp->problem->ctxt = ctxt;
  ctxt->algo_status = exp->lm->minimizeInit(ctxt->input);
  ctxt->algo_info = exp->lm->info();
  ctxt->extra.counter_costs_calls.last_call = exp->lm->nfev();
  ctxt->extra.counter_iterations.last_call = 1;
  pseEigenExplorationCountersFinalize(&exp->params.options, &ctxt->extra);

  pseEigenExplorationResultLog(exp->dev->logger, "Begin", ctxt);
  res = pseEigenExplorationSolverContextStatusCheck
    (ctxt, exp->problem->last_res);
  PSE_VERIFY_OR_ELSE(res == RES_OK || res == RES_NOT_CONVERGED, goto error);

exit:
  PSE_ATOMIC_SET(&exp->lock_internal, 0);
  return res;
error:
  exp->curr_ctxt_idx = PSE_INDEX_INVALID;
  pseEigenExplorationSolverContextClean(ctxt);
  goto exit;
}

enum pse_res_t
pseEigenExplorationSolveStep
  (struct pse_eigen_cps_exploration_t* exp)
{
  enum pse_res_t res = RES_OK;
  struct pse_eigen_cps_exploration_solver_context_t* ctxt = nullptr;
  assert(exp);
  if( exp->curr_ctxt_idx == PSE_INDEX_INVALID )
    return RES_NOT_READY;

  ctxt = &exp->ctxts[exp->curr_ctxt_idx];

  const size_t prev_iterations_count = ctxt->extra.counter_iterations.last_call;
  pseEigenExplorationCountersPrepare(&exp->params.options, &ctxt->extra);
  ctxt->algo_status = exp->lm->minimizeOneStep(ctxt->input);
  ctxt->algo_info = exp->lm->info();
  ctxt->extra.counter_costs_calls.last_call = exp->lm->nfev();
  ctxt->extra.counter_iterations.last_call = exp->lm->iterations() - prev_iterations_count;
  pseEigenExplorationCountersFinalize(&exp->params.options, &ctxt->extra);

  pseEigenExplorationResultLog(exp->dev->logger, "Step", ctxt);
  res = pseEigenExplorationSolverContextStatusCheck
    (ctxt, exp->problem->last_res);
  PSE_VERIFY_OR_ELSE(res == RES_OK || res == RES_NOT_CONVERGED, return res);
  return res;
}

enum pse_res_t
pseEigenExplorationSolveEnd
  (struct pse_eigen_cps_exploration_t* exp)
{
  enum pse_res_t res = RES_OK;
  assert(exp);

  if( exp->curr_ctxt_idx == PSE_INDEX_INVALID )
    return RES_NOT_READY;

  // Lock the solver to update the indices
  if( PSE_ATOMIC_SET(&exp->lock_internal, 1) == 1 )
    return RES_BUSY; // Already locked

  // Secured check within the lock scope, just in case something append just
  // before the lock.
  PSE_TRY_VERIFY_OR_ELSE
    (exp->curr_ctxt_idx != PSE_INDEX_INVALID,
     res = RES_NOT_READY; goto exit);

  // TODO: rework the swapping and this indices as I am quite sure it is not
  // working properly.
  exp->problem->ctxt = nullptr;
  exp->last_ctxt_idx = exp->curr_ctxt_idx;
  exp->curr_ctxt_idx = PSE_INDEX_INVALID;

exit:
  PSE_ATOMIC_SET(&exp->lock_internal, 0);
  return res;
}

enum pse_res_t
pseEigenExplorationSolve
  (struct pse_eigen_cps_exploration_t* exp,
   const struct pse_cpspace_values_data_t* smpls)
{
  enum pse_res_t res = RES_OK;
  struct pse_eigen_cps_exploration_solver_context_t* ctxt = nullptr;
  assert(exp && smpls);

  // Fast check without locking everyone
  if( exp->curr_ctxt_idx != PSE_INDEX_INVALID )
    return RES_BUSY;

  // Lock the solver to update the indices
  if( PSE_ATOMIC_SET(&exp->lock_internal, 1) == 1 )
    return RES_BUSY; // Already locked

  // Secured check within the lock scope, just in case something append just
  // before the lock.
  PSE_TRY_VERIFY_OR_ELSE
    (exp->curr_ctxt_idx == PSE_INDEX_INVALID,
     res = RES_BUSY; goto error);

  exp->curr_ctxt_idx = (exp->last_ctxt_idx + 1) % 3;
  assert(exp->curr_ctxt_idx != exp->last_ctxt_idx);
  ctxt = &exp->ctxts[exp->curr_ctxt_idx];

  res = pseEigenExplorationSolverContextSetup(exp, smpls, ctxt);
  if( res == RES_NOT_FOUND ) {
    /* There is no point to optimize, just leave silently as if we finished
     * successfully as there was nothing to do. */
    PSE_LOG(exp->dev->logger, DEBUG, "Solve -> No point to optimize\n");
    goto exit;
  }
  PSE_VERIFY_OR_ELSE(res == RES_OK, goto error);

  pseEigenExplorationCountersPrepare(&exp->params.options, &ctxt->extra);
  exp->problem->ctxt = ctxt;
  if( exp->params.options.until_convergence ) {
    /* Tries until converged or max tries reached */
    size_t i;
    for(i = 0; i < exp->params.options.max_convergence_tries; ++i) {
      ctxt->algo_status = exp->lm->minimize(ctxt->input);
      ctxt->algo_info = exp->lm->info();
      ctxt->extra.counter_costs_calls.last_call += exp->lm->nfev();
      ctxt->extra.counter_iterations.last_call += exp->lm->iterations();
      res = pseEigenExplorationSolverContextStatusCheck
        (ctxt, exp->problem->last_res);
      if( res != RES_NOT_CONVERGED )
        break; /* Converged or error -> stop */
    }
  } else {
    /* Only one try */
    ctxt->algo_status = exp->lm->minimize(ctxt->input);
    ctxt->algo_info = exp->lm->info();
    ctxt->extra.counter_costs_calls.last_call = exp->lm->nfev();
    ctxt->extra.counter_iterations.last_call = exp->lm->iterations();
    res = pseEigenExplorationSolverContextStatusCheck
      (ctxt, exp->problem->last_res);
  }
  pseEigenExplorationCountersFinalize(&exp->params.options, &ctxt->extra);
  pseEigenExplorationResultLog(exp->dev->logger, "Solve", ctxt);
  PSE_VERIFY_OR_ELSE(res == RES_OK || res == RES_NOT_CONVERGED, goto error);

  exp->problem->ctxt = nullptr;
  exp->last_ctxt_idx = exp->curr_ctxt_idx;

exit:
  exp->curr_ctxt_idx = PSE_INDEX_INVALID;
  PSE_ATOMIC_SET(&exp->lock_internal, 0);
  return res;
error:
  pseEigenExplorationSolverContextClean(ctxt);
  goto exit;
}

enum pse_res_t
pseEigenExplorationLastResultsRetreive
  (struct pse_eigen_cps_exploration_t* exp,
   struct pse_cpspace_values_data_t* where,
   struct pse_cpspace_exploration_extra_results_t* extra)
{
  enum pse_res_t res = RES_OK;
  size_t results_ctxt_idx = PSE_INDEX_INVALID;
  const struct pse_eigen_cps_exploration_solver_context_t* ctxt = nullptr;
  const struct pse_attrib_value_accessors_t* accessors = nullptr;
  assert(exp && where);

  accessors = pseEigenValuesAccessorsGet(where, PSE_POINT_ATTRIB_COORDINATES);
  PSE_VERIFY_OR_ELSE(accessors && accessors->set, return RES_BAD_ARG);

  /* If there is no current context results, just leave without any
   * modification. */
  if( exp->last_ctxt_idx == PSE_INDEX_INVALID )
    return RES_OK;

  // TODO: this solution is not working: if the retreive takes years, the solver
  // could overwrite the context kept here!
  results_ctxt_idx = exp->last_ctxt_idx;
  ctxt = &exp->ctxts[results_ctxt_idx];

  PSE_CALL_OR_RETURN(res, accessors->set
    (accessors->ctxt,
     PSE_POINT_ATTRIB_COORDINATES,
     PSE_TYPE_REAL,
     sb_count(ctxt->optimizable_ppoints),
     ctxt->optimizable_ppoints,
     ctxt->input.data()));

  if( extra )
    *extra = ctxt->extra;

  return res;
}
