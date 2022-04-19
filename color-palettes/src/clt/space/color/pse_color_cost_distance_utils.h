#ifndef PSE_COLOR_COST_DISTANCE_UTILS_H
#define PSE_COLOR_COST_DISTANCE_UTILS_H

#include "pse_color.h"

#include <clt/pse_clt_cost3.h>

#include <string.h>

PSE_API_BEGIN

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

/*! Helper function to initialize memory of ::pse_clt_cost3_ctxt_t objects from
 * color components.
 */
static PSE_FINLINE void
pseColorCost3FuncContextsMemInit
  (const pse_color_components_flags_t components,
   const size_t count,
   void** mems_to_init)
{
  size_t comps_count = 0;
  size_t comps_idx[3] = { 0, 1, 2 };
  size_t i;

  /* Fill the components buffer */
  for(i = 0; i < PSE_COLOR_COMPONENTS_COUNT_MAX; ++i) {
    if( PSE_HAS_COLOR_COMPONENT(components,i) ) {
      comps_idx[comps_count] = i;
      ++comps_count;
    }
  }

  /* Prepare the contexts memory */
  for(i = 0; i < count; ++i) {
    struct pse_clt_cost3_ctxt_t* ctxt =
      (struct pse_clt_cost3_ctxt_t*)mems_to_init[i];
    pseCltCost3ContextInit(ctxt, comps_count);
    memcpy(ctxt->comps_idx, comps_idx, sizeof(size_t)*comps_count);
  }
}

/*! Helper function to initialize contexts of evaluation of relationships. This
 * version works on weighted binary relationships, and the computed reference
 * value is the signed L1 distance between each color component. It uses the
 * type ::pse_clt_cost3_ctxt_t to store the reference value and the weight.
 */
static PSE_INLINE enum pse_res_t
pseColorCost3FuncContextsInitFromL1Distance
  (const struct pse_colors_t* colors,
   const enum pse_clt_L1_distance_kind_t kind,
   struct pse_eval_relshps_t* eval_relshps,
   const bool* must_filter,
   pse_clt_L1_distance_per_component_filter_cb filter_signed,
   pse_clt_L1_distance_per_component_filter_cb filter_unsigned)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_t clr1 = PSE_COLOR_INVALID;
  struct pse_color_t clr2 = PSE_COLOR_INVALID;
  size_t i;

  for(i = 0; i < eval_relshps->count; ++i) {
    struct pse_clt_cost3_ctxt_t* ctxt =
      (struct pse_clt_cost3_ctxt_t*)eval_relshps->ctxts[i];
    struct pse_color_constraint_distance_config_t* cfg =
      (struct pse_color_constraint_distance_config_t*)eval_relshps->configs[i];
    pse_ppoint_id_t ppid1 = PSE_PPOINT_ID_INVALID;
    pse_ppoint_id_t ppid2 = PSE_PPOINT_ID_INVALID;

    /* Initialize the contexts */
    pseColorFormatSet(&clr1, cfg->as_weighted.format);
    pseColorFormatSet(&clr2, cfg->as_weighted.format);
    assert(eval_relshps->data[i]->ppoints_count == 2);
    ppid1 = eval_relshps->data[i]->ppoints[0];
    ppid2 = eval_relshps->data[i]->ppoints[1];
    PSE_CALL_OR_GOTO(res,exit, pseColorsExtractAt(colors, ppid1, &clr1));
    PSE_CALL_OR_GOTO(res,exit, pseColorsExtractAt(colors, ppid2, &clr2));
    PSE_CALL_OR_GOTO(res,exit,
      pseCltCostRefInitFromL1Distance
        (&ctxt->ref, kind, cfg->as_weighted.weight,
         clr1.as.any.comps.mem, clr2.as.any.comps.mem,
         must_filter, filter_signed, filter_unsigned));
  }
exit:
  return res;
}

PSE_API_END

#endif /* PSE_COLOR_COST_DISTANCE_UTILS_H */


