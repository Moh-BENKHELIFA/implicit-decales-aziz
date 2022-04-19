#ifndef PSE_CLT_COST_L1_H
#define PSE_CLT_COST_L1_H

#include "pse_clt_cost.h"

#include <pse.h>

PSE_API_BEGIN

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

enum pse_clt_L1_distance_kind_t {
  PSE_CLT_L1_DISTANCE_KIND_SIGNED,
  PSE_CLT_L1_DISTANCE_KIND_UNSIGNED,
  PSE_CLT_L1_DISTANCE_KIND_SIGNED_PER_COMPONENT,
  PSE_CLT_L1_DISTANCE_KIND_UNSIGNED_PER_COMPONENT
};

typedef pse_real_t
(* pse_clt_L1_distance_per_component_filter_cb)
  (const size_t comp_idx,
   const pse_real_t dist);

/******************************************************************************
 *
 * PUBLIC CONSTANTS
 *
 ******************************************************************************/

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_CLT_INLINE_API pse_real_t
pseCltL1SignedDistanceComputeAt
  (const pse_real_t* val1,
   const pse_real_t* val2,
   const size_t comp_idx,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  pse_real_t dist;
  assert(val1 && val2);
  dist = val2[comp_idx] - val1[comp_idx];
  if( filter ) {
    return filter(comp_idx, dist);
  }
  return dist;
}

PSE_CLT_INLINE_API pse_real_t
pseCltL1UnsignedDistanceComputeAt
  (const pse_real_t* val1,
   const pse_real_t* val2,
   const size_t comp_idx,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  pse_real_t dist;
  assert(val1 && val2);
  dist = PSE_REAL_ABS(val2[comp_idx] - val1[comp_idx]);
  if( filter ) {
    return filter(comp_idx, dist);
  }
  return dist;
}

PSE_CLT_INLINE_API pse_real_t
pseCltL1SignedDistanceSumCompute
  (const pse_real_t* val1,
   const pse_real_t* val2,
   const size_t count,
   const size_t* comps_idx,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  pse_real_t dist = 0;
  size_t i;
  assert(val1 && val2 && (count > 0));
  for(i = 0; i < count; ++i) {
    const size_t comp_idx = comps_idx[i];
    dist += pseCltL1SignedDistanceComputeAt
      (val1, val2, comp_idx, must_filter[comp_idx] ? filter : NULL);
  }
  return dist;
}

/* L1 distance, also known as the Manhattan distance. */
PSE_CLT_INLINE_API pse_real_t
pseCltL1UnsignedDistanceSumCompute
  (const pse_real_t* val1,
   const pse_real_t* val2,
   const size_t count,
   const size_t* comps_idx,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  pse_real_t dist = 0;
  size_t i;
  assert(val1 && val2 && (count > 0));
  for(i = 0; i < count; ++i) {
    const size_t comp_idx = comps_idx[i];
    dist += pseCltL1UnsignedDistanceComputeAt
      (val1, val2, comp_idx, must_filter[comp_idx] ? filter : NULL);
  }
  return dist;
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCost1InitFromL1DistanceSignedAt
  (struct pse_clt_cost1_ctxt_t* comp_ctxt,
   const size_t comp_idx,
   const pse_real_t weight,
   const pse_real_t* ref_val1,
   const pse_real_t* ref_val2,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  assert(comp_ctxt && ref_val1 && ref_val2);
  comp_ctxt->weight = weight;
  comp_ctxt->ref = pseCltL1SignedDistanceComputeAt
    (ref_val1, ref_val2, comp_idx, filter);
  return RES_OK;
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCost1InitFromL1DistanceUnsignedAt
  (struct pse_clt_cost1_ctxt_t* comp_ctxt,
   const size_t comp_idx,
   const pse_real_t weight,
   const pse_real_t* ref_val1,
   const pse_real_t* ref_val2,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  assert(comp_ctxt && ref_val1 && ref_val2);
  comp_ctxt->weight = weight;
  comp_ctxt->ref = pseCltL1UnsignedDistanceComputeAt
    (ref_val1, ref_val2, comp_idx, filter);
  return RES_OK;
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCost1InitFromL1DistanceSignedPerComponent
  (const size_t count,
   struct pse_clt_cost1_ctxt_t* comps_ctxts,
   const size_t* comps_idx, /* of size count */
   const pse_real_t weight,
   const pse_real_t* ref_val1,
   const pse_real_t* ref_val2,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  size_t i;
  assert((count > 0) && comps_idx && comps_ctxts && ref_val1 && ref_val2);
  for(i = 0; i < count; ++i) {
    const size_t comp_idx = comps_idx[i];
    comps_ctxts[i].weight = weight;
    comps_ctxts[i].ref = pseCltL1SignedDistanceComputeAt
      (ref_val1, ref_val2, comp_idx,
       must_filter && must_filter[comp_idx] ? filter : NULL);
  }
  return RES_OK;
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCost1InitFromL1DistanceUnsignedPerComponent
  (const size_t count,
   struct pse_clt_cost1_ctxt_t* comps_ctxts,
   const size_t* comps_idx, /* of size count */
   const pse_real_t weight,
   const pse_real_t* ref_val1,
   const pse_real_t* ref_val2,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  size_t i;
  assert((count > 0) && comps_idx && comps_ctxts && ref_val1 && ref_val2);
  for(i = 0; i < count; ++i) {
    const size_t comp_idx = comps_idx[i];
    comps_ctxts[i].weight = weight;
    comps_ctxts[i].ref = pseCltL1UnsignedDistanceComputeAt
      (ref_val1, ref_val2, comp_idx,
       must_filter && must_filter[comp_idx] ? filter : NULL);
  }
  return RES_OK;
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostRefInitFromL1DistanceUnsigned
  (struct pse_clt_costN_ctxt_ref_t* ref,
   const pse_real_t weight,
   const pse_real_t* ref_val1,
   const pse_real_t* ref_val2,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  assert(ref && ref_val1 && ref_val2 && (ref->count > 0));
  ref->comps[0].weight = weight;
  ref->comps[0].ref = pseCltL1UnsignedDistanceSumCompute
    (ref_val1, ref_val2, ref->count, ref->comps_idx, must_filter, filter);
  return RES_OK;
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostRefInitFromL1DistanceSigned
  (struct pse_clt_costN_ctxt_ref_t* ref,
   const pse_real_t weight,
   const pse_real_t* ref_val1,
   const pse_real_t* ref_val2,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  assert(ref && ref_val1 && ref_val2 && (ref->count > 0));
  ref->comps[0].weight = weight;
  ref->comps[0].ref = pseCltL1SignedDistanceSumCompute
    (ref_val1, ref_val2, ref->count, ref->comps_idx, must_filter, filter);
  return RES_OK;
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostRefInitFromL1DistanceSignedPerComponent
  (struct pse_clt_costN_ctxt_ref_t* ref,
   const pse_real_t weight,
   const pse_real_t* ref_val1,
   const pse_real_t* ref_val2,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  return pseCltCost1InitFromL1DistanceSignedPerComponent
    (ref->count, ref->comps, ref->comps_idx,
     weight, ref_val1, ref_val2, must_filter, filter);
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostRefInitFromL1DistanceUnsignedPerComponent
  (struct pse_clt_costN_ctxt_ref_t* ref,
   const pse_real_t weight,
   const pse_real_t* ref_val1,
   const pse_real_t* ref_val2,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  return pseCltCost1InitFromL1DistanceUnsignedPerComponent
    (ref->count, ref->comps, ref->comps_idx,
     weight, ref_val1, ref_val2, must_filter, filter);
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostRefInitFromL1Distance
  (struct pse_clt_costN_ctxt_ref_t* ref,
   const enum pse_clt_L1_distance_kind_t kind,
   const pse_real_t weight,
   const pse_real_t* ref_val1,
   const pse_real_t* ref_val2,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter_signed,
   pse_clt_L1_distance_per_component_filter_cb filter_unsigned)
{
  switch(kind) {
    case PSE_CLT_L1_DISTANCE_KIND_SIGNED:
      return pseCltCostRefInitFromL1DistanceSigned
        (ref, weight, ref_val1, ref_val2, must_filter, filter_signed);
    case PSE_CLT_L1_DISTANCE_KIND_UNSIGNED:
      return pseCltCostRefInitFromL1DistanceUnsigned
        (ref, weight, ref_val1, ref_val2, must_filter, filter_unsigned);
    case PSE_CLT_L1_DISTANCE_KIND_SIGNED_PER_COMPONENT:
      return pseCltCostRefInitFromL1DistanceSignedPerComponent
        (ref, weight, ref_val1, ref_val2, must_filter, filter_signed);
    case PSE_CLT_L1_DISTANCE_KIND_UNSIGNED_PER_COMPONENT:
      return pseCltCostRefInitFromL1DistanceUnsignedPerComponent
        (ref, weight, ref_val1, ref_val2, must_filter, filter_unsigned);
    default: assert(false);
  }
  return RES_NOT_SUPPORTED;
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostFunc1ComputeL1DistanceSignedPerComponent
  (const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs,
   const size_t value_comps_count,
   const size_t comp_idx,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  const pse_real_t* values = eval_coords->coords;
  const struct pse_eval_relshp_data_t* data;
  struct pse_clt_cost1_ctxt_t* ctxt;
  pse_ppoint_id_t ppid1;
  pse_ppoint_id_t ppid2;
  pse_real_t curr_L1;
  size_t i;

  for(i = 0; i < eval_relshps->count; ++i) {
    data = eval_relshps->data[i];
    ctxt = (struct pse_clt_cost1_ctxt_t*)eval_relshps->ctxts[i];
    assert(data->ppoints_count == 2);
    ppid1 = data->ppoints[0];
    ppid2 = data->ppoints[1];
    curr_L1 = pseCltL1SignedDistanceComputeAt
      (&values[ppid1*value_comps_count],
       &values[ppid2*value_comps_count],
       comp_idx, filter);
    costs[i] = ctxt->weight * (curr_L1 - ctxt->ref);
  }
  return RES_OK;
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostFunc1ComputeL1DistanceUnsignedPerComponent
  (const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs,
   const size_t value_comps_count,
   const size_t comp_idx,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  const pse_real_t* values = eval_coords->coords;
  const struct pse_eval_relshp_data_t* data;
  struct pse_clt_cost1_ctxt_t* ctxt;
  pse_ppoint_id_t ppid1;
  pse_ppoint_id_t ppid2;
  pse_real_t curr_L1;
  size_t i;

  for(i = 0; i < eval_relshps->count; ++i) {
    data = eval_relshps->data[i];
    ctxt = (struct pse_clt_cost1_ctxt_t*)eval_relshps->ctxts[i];
    assert(data->ppoints_count == 2);
    ppid1 = data->ppoints[0];
    ppid2 = data->ppoints[1];
    curr_L1 = pseCltL1UnsignedDistanceComputeAt
      (&values[ppid1*value_comps_count],
       &values[ppid2*value_comps_count],
       comp_idx, filter);
    costs[i] = ctxt->weight * (curr_L1 - ctxt->ref);
  }
  return RES_OK;
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostFuncRefComputeL1DistanceUnsigned
  (const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs,
   const size_t value_comps_count,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  const pse_real_t* values = eval_coords->coords;
  const struct pse_eval_relshp_data_t* data;
  struct pse_clt_costN_ctxt_ref_t* ctxt;
  pse_ppoint_id_t ppid1;
  pse_ppoint_id_t ppid2;
  pse_real_t curr_L1;
  size_t i, c = 0;

  for(i = 0; i < eval_relshps->count; ++i) {
    data = eval_relshps->data[i];
    ctxt = (struct pse_clt_costN_ctxt_ref_t*)eval_relshps->ctxts[i];
    assert(data->ppoints_count == 2);
    ppid1 = data->ppoints[0];
    ppid2 = data->ppoints[1];
    curr_L1 = pseCltL1UnsignedDistanceSumCompute
      (&values[ppid1*value_comps_count],
       &values[ppid2*value_comps_count],
       ctxt->count, ctxt->comps_idx,
       must_filter, filter);
    costs[c++] = ctxt->comps[0].weight * (curr_L1 - ctxt->comps[0].ref);
  }
  return RES_OK;
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostFuncRefComputeL1DistanceSigned
  (const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs,
   const size_t value_comps_count,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  const pse_real_t* values = eval_coords->coords;
  const struct pse_eval_relshp_data_t* data;
  struct pse_clt_costN_ctxt_ref_t* ctxt;
  pse_ppoint_id_t ppid1;
  pse_ppoint_id_t ppid2;
  pse_real_t curr_L1;
  size_t i, c = 0;

  for(i = 0; i < eval_relshps->count; ++i) {
    data = eval_relshps->data[i];
    ctxt = (struct pse_clt_costN_ctxt_ref_t*)eval_relshps->ctxts[i];
    assert(data->ppoints_count == 2);
    ppid1 = data->ppoints[0];
    ppid2 = data->ppoints[1];
    curr_L1 = pseCltL1SignedDistanceSumCompute
      (&values[ppid1*value_comps_count],
       &values[ppid2*value_comps_count],
       ctxt->count, ctxt->comps_idx,
       must_filter, filter);
    costs[c++] = ctxt->comps[0].weight * (curr_L1 - ctxt->comps[0].ref);
  }
  return RES_OK;
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostFuncRefComputeL1DistanceSignedPerComponent
  (const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs,
   const size_t value_comps_count,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  const pse_real_t* values = eval_coords->coords;
  const struct pse_eval_relshp_data_t* data;
  struct pse_clt_costN_ctxt_ref_t* ctxt;
  pse_ppoint_id_t ppid1;
  pse_ppoint_id_t ppid2;
  pse_real_t curr_L1;
  size_t i, j, c = 0;

  for(i = 0; i < eval_relshps->count; ++i) {
    data = eval_relshps->data[i];
    ctxt = (struct pse_clt_costN_ctxt_ref_t*)eval_relshps->ctxts[i];
    assert(data->ppoints_count == 2);
    ppid1 = data->ppoints[0];
    ppid2 = data->ppoints[1];
    for(j = 0; j < ctxt->count; ++j) {
      const size_t comp_idx = ctxt->comps_idx[j];
      curr_L1 = pseCltL1SignedDistanceComputeAt
        (&values[ppid1*value_comps_count],
         &values[ppid2*value_comps_count],
         comp_idx,
         must_filter && must_filter[comp_idx] ? filter : NULL);
      costs[c++] = ctxt->comps[j].weight * (curr_L1 - ctxt->comps[j].ref);
    }
  }
  return RES_OK;
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostFuncRefComputeL1DistanceUnsignedPerComponent
  (const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs,
   const size_t value_comps_count,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  const pse_real_t* values = eval_coords->coords;
  const struct pse_eval_relshp_data_t* data;
  struct pse_clt_costN_ctxt_ref_t* ctxt;
  pse_ppoint_id_t ppid1;
  pse_ppoint_id_t ppid2;
  pse_real_t curr_L1;
  size_t i, j, c = 0;

  for(i = 0; i < eval_relshps->count; ++i) {
    data = eval_relshps->data[i];
    ctxt = (struct pse_clt_costN_ctxt_ref_t*)eval_relshps->ctxts[i];
    assert(data->ppoints_count == 2);
    ppid1 = data->ppoints[0];
    ppid2 = data->ppoints[1];
    for(j = 0; j < ctxt->count; ++j) {
      const size_t comp_idx = ctxt->comps_idx[j];
      curr_L1 = pseCltL1UnsignedDistanceComputeAt
        (&values[ppid1*value_comps_count],
         &values[ppid2*value_comps_count],
         comp_idx,
         must_filter && must_filter[comp_idx] ? filter : NULL);
      costs[c++] = ctxt->comps[j].weight * (curr_L1 - ctxt->comps[j].ref);
    }
  }
  return RES_OK;
}

PSE_CLT_INLINE_API enum pse_res_t
pseCltCostFuncRefComputeL1Distance
  (const struct pse_eval_coordinates_t* eval_coords,
   const enum pse_clt_L1_distance_kind_t kind,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs,
   const size_t value_comps_count,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter)
{
  switch(kind) {
    case PSE_CLT_L1_DISTANCE_KIND_SIGNED:
      return pseCltCostFuncRefComputeL1DistanceSigned
        (eval_coords, eval_relshps, costs,
         value_comps_count, must_filter, filter);
    case PSE_CLT_L1_DISTANCE_KIND_UNSIGNED:
      return pseCltCostFuncRefComputeL1DistanceUnsigned
        (eval_coords, eval_relshps, costs,
         value_comps_count, must_filter, filter);
    case PSE_CLT_L1_DISTANCE_KIND_SIGNED_PER_COMPONENT:
      return pseCltCostFuncRefComputeL1DistanceSignedPerComponent
        (eval_coords, eval_relshps, costs,
         value_comps_count, must_filter, filter);
    case PSE_CLT_L1_DISTANCE_KIND_UNSIGNED_PER_COMPONENT:
      return pseCltCostFuncRefComputeL1DistanceUnsignedPerComponent
        (eval_coords, eval_relshps, costs,
         value_comps_count, must_filter, filter);
    default: assert(false);
  }
  return RES_NOT_SUPPORTED;
}

PSE_API_END

#endif /* PSE_CLT_COST_L1_H */
