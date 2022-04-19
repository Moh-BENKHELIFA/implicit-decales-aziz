#ifndef PSE_CLT_COST3_H
#define PSE_CLT_COST3_H

#include "pse_clt_cost.h"
#include "pse_clt_cost_L1.h"

PSE_API_BEGIN

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

struct pse_clt_cost3_ctxt_t {
  struct pse_clt_costN_ctxt_ref_t ref;
  struct pse_clt_cost1_ctxt_t comps[3];
  size_t comps_idx[3];
};

/******************************************************************************
 *
 * PUBLIC CONSTANTS
 *
 ******************************************************************************/

#define PSE_CLT_COST3_CTXT_NULL_                                               \
  { PSE_CLT_COSTN_CTXT_REF_NULL_,                                           \
    { PSE_CLT_COST1_CTXT_DEFAULT_,                                          \
      PSE_CLT_COST1_CTXT_DEFAULT_,                                          \
      PSE_CLT_COST1_CTXT_DEFAULT_ },                                        \
    { 0, 0, 0 } }

static const struct pse_clt_cost3_ctxt_t PSE_CLT_COST3_CTXT_NULL =
  PSE_CLT_COST3_CTXT_NULL_;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_CLT_INLINE_API void
pseCltCost3ContextInit
  (struct pse_clt_cost3_ctxt_t* ctxt,
   const size_t count)
{
  assert(ctxt);
  pseCltCostContextRefInitFromBuffer
    (&ctxt->ref, count, ctxt->comps, ctxt->comps_idx);
}

PSE_CLT_INLINE_API pse_real_t
pseCltCost3ComputeL1SignedDistance
  (struct pse_clt_cost3_ctxt_t* ctxt,
   const pse_real_t* val1,
   const pse_real_t* val2,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  return pseCltL1SignedDistanceSumCompute
    (val1, val2, ctxt->ref.count, ctxt->comps_idx, must_filter, filter);
}

PSE_CLT_INLINE_API pse_real_t
pseCltCost3ComputeL1UnsignedDistance
  (struct pse_clt_cost3_ctxt_t* ctxt,
   const pse_real_t* val1,
   const pse_real_t* val2,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  return pseCltL1UnsignedDistanceSumCompute
    (val1, val2, ctxt->ref.count, ctxt->comps_idx, must_filter, filter);
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCost3InitFromL1DistanceSignedPerComponent
  (struct pse_clt_cost3_ctxt_t* ctxt,
   const pse_real_t weight,
   const pse_real_t* ref_val1,
   const pse_real_t* ref_val2,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  return pseCltCostRefInitFromL1DistanceSignedPerComponent
    (&ctxt->ref, weight, ref_val1, ref_val2, must_filter, filter);
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCost3InitFromL1DistanceUnsignedPerComponent
  (struct pse_clt_cost3_ctxt_t* ctxt,
   const pse_real_t weight,
   const pse_real_t* ref_val1,
   const pse_real_t* ref_val2,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  return pseCltCostRefInitFromL1DistanceUnsignedPerComponent
    (&ctxt->ref, weight, ref_val1, ref_val2, must_filter, filter);
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCost3InitFromL1DistanceSigned
  (struct pse_clt_cost3_ctxt_t* ctxt,
   const pse_real_t weight,
   const pse_real_t* ref_val1,
   const pse_real_t* ref_val2,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  return pseCltCostRefInitFromL1DistanceSigned
    (&ctxt->ref, weight, ref_val1, ref_val2, must_filter, filter);
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCost3InitFromL1DistanceUnsigned
  (struct pse_clt_cost3_ctxt_t* ctxt,
   const pse_real_t weight,
   const pse_real_t* ref_val1,
   const pse_real_t* ref_val2,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  return pseCltCostRefInitFromL1DistanceUnsigned
    (&ctxt->ref, weight, ref_val1, ref_val2, must_filter, filter);
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostFunc3ComputeAtL1DistanceSignedPerComponent
  (const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs,
   const size_t comp_idx,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  return pseCltCostFunc1ComputeL1DistanceSignedPerComponent
    (eval_coords, eval_relshps, costs, 3, comp_idx, filter);
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostFunc3ComputeAtL1DistanceUnsignedPerComponent
  (const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs,
   const size_t comp_idx,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  return pseCltCostFunc1ComputeL1DistanceUnsignedPerComponent
    (eval_coords, eval_relshps, costs, 3, comp_idx, filter);
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostFunc3ComputeL1DistanceUnsigned
  (const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  return pseCltCostFuncRefComputeL1DistanceUnsigned
    (eval_coords, eval_relshps, costs, 3, must_filter, filter);
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostFunc3ComputeL1DistanceSigned
  (const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  return pseCltCostFuncRefComputeL1DistanceUnsigned
    (eval_coords, eval_relshps, costs, 3, must_filter, filter);
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostFunc3ComputeL1DistanceSignedPerComponent
  (const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  return pseCltCostFuncRefComputeL1DistanceSignedPerComponent
    (eval_coords, eval_relshps, costs, 3, must_filter, filter);
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostFunc3ComputeL1DistanceUnsignedPerComponent
  (const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  return pseCltCostFuncRefComputeL1DistanceUnsignedPerComponent
    (eval_coords, eval_relshps, costs, 3, must_filter, filter);
}

PSE_API_END

#endif /* PSE_CLT_COST3_H */
