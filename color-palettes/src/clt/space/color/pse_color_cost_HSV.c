#include "pse_color_cost_HSV_p.h"
#include "pse_color_cost_generic_p.h"
#include "pse_color_space_HSV.h"
#include "pse_color_palette_constraints_p.h"
#include "pse_color_cost_distance_utils.h"

/******************************************************************************
 *
 * HELPER FUNCTIONS
 *
 ******************************************************************************/

static enum pse_res_t
pseColorHSVCostFuncContextsMemInit
  (void* user_ctxt,
   const pse_clt_type_uid_t type_id,
   const size_t count,
   void** mems_to_init)
{
  switch(type_id) {
    case PSE_COLOR_COST_FUNC_CTXT_UID_HSV_L1DistanceSignedPerComponent:
    case PSE_COLOR_COST_FUNC_CTXT_UID_HSV_L1DistanceUnsignedPerComponent:
    case PSE_COLOR_COST_FUNC_CTXT_UID_HSV_L1DistanceSigned:
    case PSE_COLOR_COST_FUNC_CTXT_UID_HSV_L1DistanceUnsigned: {
      const pse_color_components_flags_t components =
        (pse_color_components_flags_t)((uintptr_t)user_ctxt);
      pseColorCost3FuncContextsMemInit(components, count, mems_to_init);
    } break;

    default: assert(false); return RES_NOT_SUPPORTED;
  }
  return RES_OK;
}

static PSE_FINLINE pse_real_t
pseColorHSVSignedDistanceFilter
  (const size_t comp_idx,
   const pse_real_t comp_dist)
{
  /* We do a H%360. */
  assert(comp_idx == 0);
  (void)comp_idx;
  if( comp_dist <= (pse_real_t)-0.5 ) {
    return comp_dist + (pse_real_t)1;
  } else if( comp_dist >= (pse_real_t)0.5 ) {
    return comp_dist - (pse_real_t)1;
  }
  return comp_dist;
}

static PSE_FINLINE pse_real_t
pseColorHSVUnsignedDistanceFilter
  (const size_t comp_idx,
   const pse_real_t comp_dist)
{
  /* Unsigned comp_distance means that we receive abs(comp_distance). */
  /* We do a abs(H)%360. */
  assert(comp_idx == 0);
  (void)comp_idx;
  if( comp_dist >= (pse_real_t)0.5 ) {
    return (pse_real_t)1 - comp_dist;
  }
  return comp_dist;
}

static PSE_FINLINE enum pse_clt_L1_distance_kind_t
pseColorHSVL1DistanceKindFromUID
  (const pse_clt_cost_func_uid_t user_cfunc_uid)
{
  switch(user_cfunc_uid) {
    case PSE_COLOR_COST_FUNC_CTXT_UID_HSV_L1DistanceSignedPerComponent:
      return PSE_CLT_L1_DISTANCE_KIND_SIGNED_PER_COMPONENT;
    case PSE_COLOR_COST_FUNC_CTXT_UID_HSV_L1DistanceUnsignedPerComponent:
      return PSE_CLT_L1_DISTANCE_KIND_UNSIGNED_PER_COMPONENT;
    case PSE_COLOR_COST_FUNC_CTXT_UID_HSV_L1DistanceSigned:
      return PSE_CLT_L1_DISTANCE_KIND_SIGNED;
    case PSE_COLOR_COST_FUNC_CTXT_UID_HSV_L1DistanceUnsigned:
      return PSE_CLT_L1_DISTANCE_KIND_UNSIGNED;
    default: assert(false);
  }
  return PSE_CLT_L1_DISTANCE_KIND_SIGNED;
}

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

/* Instanciate the template for the common distance cost functors */
#define PSE_COLOR_SPACE_NAME HSV
#define PSE_COLOR_SPACE_COMPS_COUNT 3
#define PSE_L1DISTANCE_KIND_FROM_CFUNC_UID pseColorHSVL1DistanceKindFromUID
#define PSE_COLOR_SPACE_FILTER_COMPONENTS { true, false, false }
#define PSE_COLOR_SPACE_FILTER_SIGNED pseColorHSVSignedDistanceFilter
#define PSE_COLOR_SPACE_FILTER_UNSIGNED pseColorHSVUnsignedDistanceFilter
#include "pse_color_cost_distance_tmpl.h"

enum pse_res_t
pseColorHSVCostFunctorImplemGet
  (const pse_color_cost_func_id_t cf,
   const pse_color_components_flags_t components,
   struct pse_color_cost_func_implem_t* implem)
{
  static const struct pse_color_cost_func_implem_t IMPLEMS[PSE_COLOR_COST_FUNC_HSV_COUNT+1] = {
  #define PSE_COLOR_COST_FUNC(Name, Arity, ColorArity, CostsCount)             \
    { PSE_CONCAT(PSE_COLOR_COST_FUNC_CTXT_UID_,Name),                          \
      PSE_CONCAT(pseColorCostFuncCompute_,Name),                               \
      PSE_CONCAT(PSE_CONCAT(PSE_COLOR_CTXT_TYPE_INFO_,Name),_),                \
      pseColorHSVCostFuncContextsInit,                                         \
      pseColorHSVCostFuncContextsClean,                                        \
      PSE_CONCAT(PSE_COST_ARITY_MODE_,Arity),                                  \
      PSE_CONCAT(PSE_COLOR_COST_ARITY_MODE_,ColorArity),                       \
      CostsCount },
    PSE_COLOR_COST_FUNC_HSV_ALL
  #undef PSE_COLOR_COST_FUNC

    PSE_COLOR_COST_FUNC_IMPLEM_NULL_
  };

  if( cf < PSE_COLOR_COST_FUNC_GENERIC_COUNT )
    return pseColorGenericCostFunctorImplemGet(cf, components, implem);

  assert(implem && pseIsColorHSVCostFunctorValid(cf));

  *implem = IMPLEMS[cf-PSE_COLOR_COST_FUNC_HSV_BEGIN-1];
  implem->ctxt_type_info.user_ctxt = (void*)((uintptr_t)components);
  implem->ctxt_type_info.meminit = pseColorHSVCostFuncContextsMemInit;
  return RES_OK;
}

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

enum pse_res_t
pseColorHSVCostFunctorRegister
  (struct pse_cpspace_t* cps,
   const pse_color_format_t fmt,
   const enum pse_color_cost_func_HSV_t cf,
   const pse_color_components_flags_t components,
   pse_relshp_cost_func_id_t* fid)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_cost_func_config_t cfg = PSE_COLOR_COST_FUNC_CONFIG_NULL;
  struct pse_color_cost_func_implem_t implem = PSE_COLOR_COST_FUNC_IMPLEM_NULL;
  if(  !cps
    || (PSE_COLOR_SPACE_FROM(fmt) == PSE_COLOR_SPACE_HSV)
    || !pseIsColorHSVCostFunctorValid(cf)
    || (components == PSE_COLOR_COMPONENTS_NONE) )
    return RES_BAD_ARG;

  cfg.format = fmt;
  cfg.id = cf;
  cfg.components = components;

  PSE_CALL_OR_RETURN(res, pseColorHSVCostFunctorImplemGet
    (cf, components, &implem));
  PSE_CALL_OR_RETURN(res, pseColorCostFunctorRegister
    (cps, &cfg, &implem, fid));
  return res;
}
