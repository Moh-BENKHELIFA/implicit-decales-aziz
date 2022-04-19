#ifndef PSE_COLOR_COST_GENERIC_P_H
#define PSE_COLOR_COST_GENERIC_P_H

#include "pse_color_cost_generic.h"
#include "pse_color_cost_p.h"

#include <clt/pse_clt_cost3.h>

PSE_API_BEGIN

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

enum pse_color_cost_func_ctxt_generic_uid_t {
#define PSE_COLOR_COST_FUNC(Name, _a,_b,_c)                                    \
  PSE_CONCAT(PSE_COLOR_COST_FUNC_CTXT_UID_,Name) =                             \
    PSE_CONCAT(PSE_COLOR_COST_FUNC_,Name),
  PSE_COLOR_COST_FUNC_GENERIC_ALL
#undef PSE_COLOR_COST_FUNC

  PSE_COLOR_COST_FUNC_CTXT_UID_GENERIC_COUNT
};

/******************************************************************************
 *
 * PRIVATE CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_CTXT_TYPE_INFO_Generic_L1DistanceSignedPerComponent_         \
  { PSE_COLOR_COST_FUNC_CTXT_UID_Generic_L1DistanceSignedPerComponent,         \
    sizeof(struct pse_clt_cost3_ctxt_t), 0,                                    \
    &PSE_CLT_COST3_CTXT_NULL,                                                  \
    NULL, NULL, NULL }
#define PSE_COLOR_CTXT_TYPE_INFO_Generic_L1DistanceUnsignedPerComponent_       \
  { PSE_COLOR_COST_FUNC_CTXT_UID_Generic_L1DistanceUnsignedPerComponent,       \
    sizeof(struct pse_clt_cost3_ctxt_t), 0,                                    \
    &PSE_CLT_COST3_CTXT_NULL,                                                  \
    NULL, NULL, NULL }
#define PSE_COLOR_CTXT_TYPE_INFO_Generic_L1DistanceSigned_                     \
  { PSE_COLOR_COST_FUNC_CTXT_UID_Generic_L1DistanceSigned,                     \
    sizeof(struct pse_clt_cost3_ctxt_t), 0,                                    \
    &PSE_CLT_COST3_CTXT_NULL,                                                  \
    NULL, NULL, NULL }
#define PSE_COLOR_CTXT_TYPE_INFO_Generic_L1DistanceUnsigned_                   \
  { PSE_COLOR_COST_FUNC_CTXT_UID_Generic_L1DistanceUnsigned,                   \
    sizeof(struct pse_clt_cost3_ctxt_t), 0,                                    \
    &PSE_CLT_COST3_CTXT_NULL,                                                  \
    NULL, NULL, NULL }

static const struct pse_clt_type_info_t PSE_COLOR_CTXT_TYPE_INFO_Generic_L1DistanceSignedPerComponent =
  PSE_COLOR_CTXT_TYPE_INFO_Generic_L1DistanceSignedPerComponent_;
static const struct pse_clt_type_info_t PSE_COLOR_CTXT_TYPE_INFO_Generic_L1DistanceUnsignedPerComponent =
  PSE_COLOR_CTXT_TYPE_INFO_Generic_L1DistanceUnsignedPerComponent_;
static const struct pse_clt_type_info_t PSE_COLOR_CTXT_TYPE_INFO_Generic_L1DistanceSigned =
  PSE_COLOR_CTXT_TYPE_INFO_Generic_L1DistanceSigned_;
static const struct pse_clt_type_info_t PSE_COLOR_CTXT_TYPE_INFO_Generic_L1DistanceUnsigned =
  PSE_COLOR_CTXT_TYPE_INFO_Generic_L1DistanceUnsigned_;

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

LOCAL_SYMBOL enum pse_res_t
pseColorGenericCostFuncContextsInit
  (pse_clt_cost_func_config_t user_cfunc_config,
   const pse_clt_cost_func_uid_t user_cfunc_uid,
   const struct pse_eval_ctxt_t* eval_ctxt,
   struct pse_eval_relshps_t* eval_relshps,
   pse_clt_cost_func_ctxt_init_params_t user_init_params);

LOCAL_SYMBOL enum pse_res_t
pseColorGenericCostFuncContextsClean
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
PSE_COLOR_COST_FUNC_GENERIC_ALL
#undef PSE_COLOR_COST_FUNC

LOCAL_SYMBOL enum pse_res_t
pseColorGenericCostFunctorImplemGet
  (const pse_color_cost_func_id_t cf,
   const pse_color_components_flags_t components,
   struct pse_color_cost_func_implem_t* implem);

PSE_API_END

#endif /* PSE_COLOR_COST_GENERIC_P_H */
