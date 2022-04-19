#include "pse_color_cost_generic_p.h"
#include "pse_color_palette_constraints_p.h"
#include "pse_color.h"
#include "pse_color_cost_distance_utils.h"

#include <stretchy_buffer.h>

/******************************************************************************
 *
 * HELPER FUNCTIONS
 *
 ******************************************************************************/

static enum pse_res_t
pseColorGenericCostFuncContextsMemInit
  (void* user_ctxt,
   const pse_clt_type_uid_t type_id,
   const size_t count,
   void** mems_to_init)
{
  switch(type_id) {
    case PSE_COLOR_COST_FUNC_CTXT_UID_Generic_L1DistanceSignedPerComponent:
    case PSE_COLOR_COST_FUNC_CTXT_UID_Generic_L1DistanceUnsignedPerComponent:
    case PSE_COLOR_COST_FUNC_CTXT_UID_Generic_L1DistanceSigned:
    case PSE_COLOR_COST_FUNC_CTXT_UID_Generic_L1DistanceUnsigned: {
      const pse_color_components_flags_t components =
        (pse_color_components_flags_t)((uintptr_t)user_ctxt);
      pseColorCost3FuncContextsMemInit(components, count, mems_to_init);
    } break;

    default: assert(false); return RES_NOT_SUPPORTED;
  }
  return RES_OK;
}

static PSE_FINLINE enum pse_clt_L1_distance_kind_t
pseColorGenericL1DistanceKindFromUID
  (const pse_clt_cost_func_uid_t user_cfunc_uid)
{
  switch(user_cfunc_uid) {
    case PSE_COLOR_COST_FUNC_CTXT_UID_Generic_L1DistanceSignedPerComponent:
      return PSE_CLT_L1_DISTANCE_KIND_SIGNED_PER_COMPONENT;
    case PSE_COLOR_COST_FUNC_CTXT_UID_Generic_L1DistanceUnsignedPerComponent:
      return PSE_CLT_L1_DISTANCE_KIND_UNSIGNED_PER_COMPONENT;
    case PSE_COLOR_COST_FUNC_CTXT_UID_Generic_L1DistanceSigned:
      return PSE_CLT_L1_DISTANCE_KIND_SIGNED;
    case PSE_COLOR_COST_FUNC_CTXT_UID_Generic_L1DistanceUnsigned:
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
#define PSE_COLOR_SPACE_NAME  Generic
#define PSE_COLOR_SPACE_COMPS_COUNT 3
#define PSE_L1DISTANCE_KIND_FROM_CFUNC_UID pseColorGenericL1DistanceKindFromUID
#include "pse_color_cost_distance_tmpl.h"

enum pse_res_t
pseColorGenericCostFunctorImplemGet
  (const pse_color_cost_func_id_t cf,
   const pse_color_components_flags_t components,
   struct pse_color_cost_func_implem_t* implem)
{
  static const struct pse_color_cost_func_implem_t IMPLEMS[PSE_COLOR_COST_FUNC_GENERIC_COUNT+1] = {
  #define PSE_COLOR_COST_FUNC(Name, Arity, ColorArity, CostsCount)             \
    { PSE_CONCAT(PSE_COLOR_COST_FUNC_CTXT_UID_,Name),                          \
      PSE_CONCAT(pseColorCostFuncCompute_,Name),                               \
      PSE_CONCAT(PSE_CONCAT(PSE_COLOR_CTXT_TYPE_INFO_,Name),_),                \
      pseColorGenericCostFuncContextsInit,                                     \
      pseColorGenericCostFuncContextsClean,                                    \
      PSE_CONCAT(PSE_COST_ARITY_MODE_,Arity),                                  \
      PSE_CONCAT(PSE_COLOR_COST_ARITY_MODE_,ColorArity),                       \
      CostsCount },
    PSE_COLOR_COST_FUNC_GENERIC_ALL
  #undef PSE_COLOR_COST_FUNC

    PSE_COLOR_COST_FUNC_IMPLEM_NULL_
  };

  assert(implem && (cf < PSE_COLOR_COST_FUNC_GENERIC_COUNT));

  *implem = IMPLEMS[cf];
  implem->ctxt_type_info.user_ctxt = (void*)((uintptr_t)components);
  implem->ctxt_type_info.meminit = pseColorGenericCostFuncContextsMemInit;
  return RES_OK;
}

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

enum pse_res_t
pseColorGenericCostFunctorRegister
  (struct pse_cpspace_t* cps,
   const pse_color_format_t fmt,
   const enum pse_color_cost_func_generic_t cf,
   const pse_color_components_flags_t components,
   pse_relshp_cost_func_id_t* fid)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_cost_func_config_t cfg = PSE_COLOR_COST_FUNC_CONFIG_NULL;
  struct pse_color_cost_func_implem_t implem = PSE_COLOR_COST_FUNC_IMPLEM_NULL;
  if(  !cps
    || (cf >= PSE_COLOR_COST_FUNC_GENERIC_COUNT)
    || (components == PSE_COLOR_COMPONENTS_NONE) )
    return RES_BAD_ARG;

  cfg.format = fmt;
  cfg.id = cf;
  cfg.components = components;

  PSE_CALL_OR_RETURN(res, pseColorGenericCostFunctorImplemGet
    (cf, components, &implem));
  PSE_CALL_OR_RETURN(res, pseColorCostFunctorRegister
    (cps, &cfg, &implem, fid));
  return res;
}
