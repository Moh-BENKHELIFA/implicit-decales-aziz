#ifndef PSE_COLOR_COST_RGB_P_H
#define PSE_COLOR_COST_RGB_P_H

#include "pse_color_cost_RGB.h"
#include "pse_color_types.h"

PSE_API_BEGIN

struct pse_eval_ctxt_t;
struct pse_eval_relshps_t;
struct pse_eval_coordinates_t;
struct pse_color_cost_func_implem_t;

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

enum pse_color_cost_func_ctxt_RGB_uid_t {
#define PSE_COLOR_COST_FUNC(Name, _a,_b,_c)                                    \
  PSE_CONCAT(PSE_COLOR_COST_FUNC_CTXT_UID_,Name) =                             \
    PSE_CONCAT(PSE_COLOR_COST_FUNC_,Name),
  PSE_COLOR_COST_FUNC_RGB_ALL
#undef PSE_COLOR_COST_FUNC

  PSE_COLOR_COST_FUNC_CTXT_RGB_UID_COUNT
};

/******************************************************************************
 *
 * PRIVATE CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_CTXT_TYPE_INFO_RGB_InGamut_                                  \
  { PSE_COLOR_COST_FUNC_CTXT_UID_RGB_InGamut,                                  \
    0, 0, NULL, NULL, NULL, NULL }
#define PSE_COLOR_CTXT_TYPE_INFO_RGB_OLEDScreenEnergyUsage_                    \
  { PSE_COLOR_COST_FUNC_CTXT_UID_RGB_OLEDScreenEnergyUsage,                    \
    0, 0, NULL, NULL, NULL, NULL }

static const struct pse_clt_type_info_t PSE_COLOR_CTXT_TYPE_INFO_RGB_InGamut =
  PSE_COLOR_CTXT_TYPE_INFO_RGB_InGamut_;
static const struct pse_clt_type_info_t PSE_COLOR_CTXT_TYPE_INFO_RGB_OLEDScreenEnergyUsage =
  PSE_COLOR_CTXT_TYPE_INFO_RGB_OLEDScreenEnergyUsage_;

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

LOCAL_SYMBOL enum pse_res_t
pseColorRGBCostFuncContextsInit
  (pse_clt_cost_func_config_t user_cfunc_config,
   const pse_clt_cost_func_uid_t user_cfunc_uid,
   const struct pse_eval_ctxt_t* eval_ctxt,
   struct pse_eval_relshps_t* eval_relshps,
   pse_clt_cost_func_ctxt_init_params_t user_init_params);

LOCAL_SYMBOL enum pse_res_t
pseColorRGBCostFuncContextsClean
  (pse_clt_cost_func_config_t user_cfunc_config,
   const pse_clt_cost_func_uid_t user_cfunc_uid,
   const struct pse_eval_ctxt_t* eval_ctxt,
   struct pse_eval_relshps_t* eval_relshps);

#define PSE_COLOR_COST_FUNC(Name,_a,_b,_c)                                     \
  LOCAL_SYMBOL enum pse_res_t                                                  \
  PSE_CONCAT(pseColorCostFuncCompute_,Name)                                    \
    (const struct pse_eval_ctxt_t* eval_ctxt,                                  \
     const struct pse_eval_coordinates_t* eval_coords,                         \
     struct pse_eval_relshps_t* eval_relshps,                                  \
     pse_real_t* costs);
PSE_COLOR_COST_FUNC_RGB_ALL
#undef PSE_COLOR_COST_FUNC

LOCAL_SYMBOL enum pse_res_t
pseColorRGBCostFunctorImplemGet
  (const pse_color_cost_func_id_t cf,
   const pse_color_components_flags_t components,
   struct pse_color_cost_func_implem_t* implem);

PSE_API_END

#endif /* PSE_COLOR_COST_RGB_P_H */
