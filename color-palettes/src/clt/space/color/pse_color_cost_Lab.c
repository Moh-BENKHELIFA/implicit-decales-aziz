#include "pse_color_cost_Lab_p.h"
#include "pse_color_cost_generic_p.h"
#include "pse_color_palette_constraints_p.h"
#include "pse_color_space_Lab.h"

/******************************************************************************
 *
 * HELPERS FUNCTIONS
 *
 ******************************************************************************/

static enum pse_res_t
pseColorLABCostFuncContextsMemInit
  (void* user_ctxt,
   const pse_clt_type_uid_t type_id,
   const size_t count,
   void** mems_to_init)
{
  (void)user_ctxt, (void)count, (void)mems_to_init;

  switch(type_id) {
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
pseColorLABCostFuncContextsInit
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
    default: assert(false); res = RES_NOT_SUPPORTED; goto exit;
  }
exit:
  return res;
}

enum pse_res_t
pseColorLABCostFuncContextsClean
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
pseColorLABCostFunctorImplemGet
  (const pse_color_cost_func_id_t cf,
   const pse_color_components_flags_t components,
   struct pse_color_cost_func_implem_t* implem)
{
  static const struct pse_color_cost_func_implem_t IMPLEMS[PSE_COLOR_COST_FUNC_LAB_COUNT+1] = {
  #define PSE_COLOR_COST_FUNC(Name, Arity, ColorArity, CostsCount)             \
    { PSE_CONCAT(PSE_COLOR_COST_FUNC_CTXT_UID_,Name),                          \
      PSE_CONCAT(pseColorCostFuncCompute_,Name),                               \
      PSE_CONCAT(PSE_CONCAT(PSE_COLOR_CTXT_TYPE_INFO_,Name),_),                \
      pseColorLABCostFuncContextsInit,                                         \
      pseColorLABCostFuncContextsClean,                                        \
      PSE_CONCAT(PSE_COST_ARITY_MODE_,Arity),                                  \
      PSE_CONCAT(PSE_COLOR_COST_ARITY_MODE_,ColorArity),                       \
      CostsCount },
    PSE_COLOR_COST_FUNC_LAB_ALL
  #undef PSE_COLOR_COST_FUNC

    PSE_COLOR_COST_FUNC_IMPLEM_NULL_
  };

  if( cf < PSE_COLOR_COST_FUNC_GENERIC_COUNT )
    return pseColorGenericCostFunctorImplemGet(cf, components, implem);

  assert(implem && pseIsColorLABCostFunctorValid(cf));

  *implem = IMPLEMS[cf-PSE_COLOR_COST_FUNC_LAB_BEGIN-1];
  implem->ctxt_type_info.user_ctxt = (void*)((uintptr_t)components);
  implem->ctxt_type_info.meminit = pseColorLABCostFuncContextsMemInit;
  return RES_OK;
}

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

enum pse_res_t
pseColorLABCostFunctorRegister
  (struct pse_cpspace_t* cps,
   const pse_color_format_t fmt,
   const enum pse_color_cost_func_LAB_t cf,
   const pse_color_components_flags_t components,
   pse_relshp_cost_func_id_t* fid)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_cost_func_config_t cfg = PSE_COLOR_COST_FUNC_CONFIG_NULL;
  struct pse_color_cost_func_implem_t implem = PSE_COLOR_COST_FUNC_IMPLEM_NULL;
  if(  !cps
    || (PSE_COLOR_SPACE_FROM(fmt) == PSE_COLOR_SPACE_LAB)
    || !pseIsColorLABCostFunctorValid(cf)
    || (components == PSE_COLOR_COMPONENTS_NONE) )
    return RES_BAD_ARG;

  cfg.format = fmt;
  cfg.id = cf;
  cfg.components = components;

  PSE_CALL_OR_RETURN(res, pseColorLABCostFunctorImplemGet
    (cf, components, &implem));
  PSE_CALL_OR_RETURN(res, pseColorCostFunctorRegister
    (cps, &cfg, &implem, fid));
  return res;
}
