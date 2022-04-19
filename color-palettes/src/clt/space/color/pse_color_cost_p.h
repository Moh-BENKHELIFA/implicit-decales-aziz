#ifndef PSE_COLOR_COST_P_H
#define PSE_COLOR_COST_P_H

#include "pse_color_cost.h"

#include <pse.h>

PSE_API_BEGIN

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

enum pse_color_cost_arity_mode_t {
  PSE_COLOR_COST_ARITY_MODE_PER_COLOR,
  PSE_COLOR_COST_ARITY_MODE_PER_COMPONENT
};

/*! This structure is used to store implementation information for color cost
 * functors. We use this intermediate structure to simplify the implementation
 * of new color cost functors, letting the transformation to the
 * ::pse_relshp_cost_func_params_t structure to the generic function
 * ::pseColorCostFunctorRegister.
 *
 * \note The interpretation of \p costs_count is related to the value of
 *    \p color_cost_arity_mode: either it's the costs associated to a color,
 *    whatever the number of components, or it's the costs per component.
 */
struct pse_color_cost_func_implem_t {
  pse_clt_cost_func_uid_t uid;

  pse_clt_relshp_cost_func_compute_cb compute;

  struct pse_clt_type_info_t ctxt_type_info;
  pse_clt_relshps_cost_func_ctxts_init_cb ctxts_init;
  pse_clt_relshps_cost_func_ctxts_clean_cb ctxts_clean;

  enum pse_cost_arity_mode_t cost_arity_mode;
  enum pse_color_cost_arity_mode_t color_cost_arity_mode;
  size_t costs_count;
};

typedef enum pse_res_t
(*pse_color_cost_func_implem_get_cb)
  (const pse_color_cost_func_id_t cf,
   const pse_color_components_flags_t components,
   struct pse_color_cost_func_implem_t* implem);

/******************************************************************************
 *
 * PRIVATE CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_COST_FUNC_IMPLEM_NULL_                                       \
  { PSE_CLT_COST_FUNC_UID_INVALID_, NULL, PSE_CLT_TYPE_INFO_NULL_, NULL, NULL, \
    PSE_COST_ARITY_MODE_PER_POINT, PSE_COLOR_COST_ARITY_MODE_PER_COLOR, 0 }

static const struct pse_color_cost_func_implem_t PSE_COLOR_COST_FUNC_IMPLEM_NULL =
  PSE_COLOR_COST_FUNC_IMPLEM_NULL_;

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

PSE_COLOR_API enum pse_res_t
pseColorPaletteCostFunctorsCleanAll
  (struct pse_color_palette_t* cp);

LOCAL_SYMBOL enum pse_res_t
pseColorCostFunctorRegister
  (struct pse_cpspace_t* cps,
   const struct pse_color_cost_func_config_t* cfg,
   const struct pse_color_cost_func_implem_t* implem,
   pse_relshp_cost_func_id_t* fid);

PSE_API_END

#endif /* PSE_COLOR_COST_P_H */
