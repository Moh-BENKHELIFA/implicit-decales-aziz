#include "pse_color_cost_RGB_p.h"
#include "pse_color_cost_generic_p.h"
#include "pse_color_space_RGB.h"
#include "pse_color_palette_constraints_p.h"

/******************************************************************************
 *
 * HELPER FUNCTIONS
 *
 ******************************************************************************/

static enum pse_res_t
pseColorRGBCostFuncContextsMemInit
  (void* user_ctxt,
   const pse_clt_type_uid_t type_id,
   const size_t count,
   void** mems_to_init)
{
  (void)user_ctxt, (void)count, (void)mems_to_init;

  switch(type_id) {
    case PSE_COLOR_COST_FUNC_CTXT_UID_RGB_InGamut: /* fallthrough */
    case PSE_COLOR_COST_FUNC_CTXT_UID_RGB_OLEDScreenEnergyUsage:
      /* No contexts */
    break;
    default: assert(false); return RES_NOT_SUPPORTED;
  }
  return RES_OK;
}

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

enum pse_res_t
pseColorRGBCostFuncContextsInit
  (pse_clt_cost_func_config_t user_cfunc_config,
   const pse_clt_cost_func_uid_t user_cfunc_uid,
   const struct pse_eval_ctxt_t* eval_ctxt,
   struct pse_eval_relshps_t* eval_relshps,
   pse_clt_cost_func_ctxt_init_params_t user_init_params)
{
  enum pse_res_t res = RES_OK;
  (void)user_cfunc_config;
  (void)eval_ctxt, (void)eval_relshps;
  (void)user_init_params;

  switch(user_cfunc_uid) {
    case PSE_COLOR_COST_FUNC_CTXT_UID_RGB_InGamut: /* fallthrough */
    case PSE_COLOR_COST_FUNC_CTXT_UID_RGB_OLEDScreenEnergyUsage:
      /* No contexts */
    break;
    default: assert(false); res = RES_NOT_SUPPORTED; goto exit;
  }
exit:
  return res;
}

enum pse_res_t
pseColorRGBCostFuncContextsClean
  (pse_clt_cost_func_config_t user_cfunc_config,
   const pse_clt_cost_func_uid_t user_cfunc_uid,
   const struct pse_eval_ctxt_t* eval_ctxt,
   struct pse_eval_relshps_t* eval_relshps)
{
  (void)user_cfunc_config, (void)user_cfunc_uid;
  (void)eval_ctxt, (void)eval_relshps;
  /* Nothing to do right now */
  return RES_OK;
}

enum pse_res_t
pseColorCostFuncCompute_RGB_InGamut
  (const struct pse_eval_ctxt_t* eval_ctxt,
   const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs)
{
  size_t cost_idx = 0;
  size_t i,j;
  assert(eval_coords->pspace_uid == PSE_COLOR_FORMAT_RGBr);
  (void)eval_ctxt;

  /* TODO: compute distances at once in order to enable SIMD */
  for(i = 0; i < eval_relshps->count; ++i) {
    const struct pse_eval_relshp_data_t* data = eval_relshps->data[i];
    const struct pse_color_constraint_generic_weighted_config_t* cfg =
      eval_relshps->configs[i];
    for(j = 0; j < data->ppoints_count; ++j) {
      const pse_ppoint_id_t ppid = data->ppoints[j];
      struct pse_color_RGB_t clr = PSE_COLOR_RGB_BLACKr;
      clr.as.RGBr.R = eval_coords->coords[ppid*3+0];
      clr.as.RGBr.G = eval_coords->coords[ppid*3+1];
      clr.as.RGBr.B = eval_coords->coords[ppid*3+2];
      costs[cost_idx++] =
        cfg->weight * pseColorRGBDistanceSquaredFromGamut(&clr);
    }
  }
  return RES_OK;
}

enum pse_res_t
pseColorCostFuncCompute_RGB_OLEDScreenEnergyUsage
  (const struct pse_eval_ctxt_t* eval_ctxt,
   const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs)
{
  size_t cost_idx = 0;
  size_t i,j;
  assert(eval_coords->pspace_uid == PSE_COLOR_FORMAT_RGBr);
  (void)eval_ctxt;

  for(i = 0; i < eval_relshps->count; ++i) {
    const struct pse_eval_relshp_data_t* data = eval_relshps->data[i];
    const struct pse_color_constraint_generic_weighted_config_t* cfg =
      eval_relshps->configs[i];
    for(j = 0; j < data->ppoints_count; ++j) {
      const pse_ppoint_id_t ppid = data->ppoints[j];

      #define PSE_MAX3(a,b,c) PSE_MAX(PSE_MAX((a),(b)),(c))
      costs[cost_idx++] = cfg->weight
        * PSE_MAX3
            (eval_coords->coords[ppid*3+0],
             eval_coords->coords[ppid*3+1],
             eval_coords->coords[ppid*3+2]);
      #undef PSE_MAX3
    }
  }
  return RES_OK;
}

enum pse_res_t
pseColorRGBCostFunctorImplemGet
  (const pse_color_cost_func_id_t cf,
   const pse_color_components_flags_t components,
   struct pse_color_cost_func_implem_t* implem)
{
  static const struct pse_color_cost_func_implem_t IMPLEMS[PSE_COLOR_COST_FUNC_RGB_COUNT+1] = {
  #define PSE_COLOR_COST_FUNC(Name, Arity, ColorArity, CostsCount)             \
    { PSE_CONCAT(PSE_COLOR_COST_FUNC_CTXT_UID_,Name),                          \
      PSE_CONCAT(pseColorCostFuncCompute_,Name),                               \
      PSE_CONCAT(PSE_CONCAT(PSE_COLOR_CTXT_TYPE_INFO_,Name),_),                \
      pseColorRGBCostFuncContextsInit,                                         \
      pseColorRGBCostFuncContextsClean,                                        \
      PSE_CONCAT(PSE_COST_ARITY_MODE_,Arity),                                  \
      PSE_CONCAT(PSE_COLOR_COST_ARITY_MODE_,ColorArity),                       \
      CostsCount },
    PSE_COLOR_COST_FUNC_RGB_ALL
  #undef PSE_COLOR_COST_FUNC

    PSE_COLOR_COST_FUNC_IMPLEM_NULL_
  };

  if( cf < PSE_COLOR_COST_FUNC_GENERIC_COUNT )
    return pseColorGenericCostFunctorImplemGet(cf, components, implem);

  assert(implem && pseIsColorRGBCostFunctorValid(cf));

  *implem = IMPLEMS[cf-PSE_COLOR_COST_FUNC_RGB_BEGIN-1];
  implem->ctxt_type_info.user_ctxt = (void*)((uintptr_t)components);
  implem->ctxt_type_info.meminit = pseColorRGBCostFuncContextsMemInit;
  return RES_OK;
}

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

enum pse_res_t
pseColorRGBCostFunctorRegister
  (struct pse_cpspace_t* cps,
   const pse_color_format_t fmt,
   const enum pse_color_cost_func_RGB_t cf,
   const pse_color_components_flags_t components,
   pse_relshp_cost_func_id_t* fid)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_cost_func_config_t cfg = PSE_COLOR_COST_FUNC_CONFIG_NULL;
  struct pse_color_cost_func_implem_t implem = PSE_COLOR_COST_FUNC_IMPLEM_NULL;
  if(  !cps
    || (PSE_COLOR_SPACE_FROM(fmt) == PSE_COLOR_SPACE_RGB)
    || !pseIsColorRGBCostFunctorValid(cf)
    || (components == PSE_COLOR_COMPONENTS_NONE) )
    return RES_BAD_ARG;

  cfg.format = fmt;
  cfg.id = cf;
  cfg.components = components;

  PSE_CALL_OR_RETURN(res, pseColorRGBCostFunctorImplemGet
    (cf, components, &implem));
  PSE_CALL_OR_RETURN(res, pseColorCostFunctorRegister
    (cps, &cfg, &implem, fid));
  return res;
}
