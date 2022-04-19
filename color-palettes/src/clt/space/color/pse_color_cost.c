#include "pse_color_cost_p.h"
#include "pse_color_palette_p.h"

#include "pse_color_cost_Cat02_LMS_p.h"
#include "pse_color_cost_HSV_p.h"
#include "pse_color_cost_Lab_p.h"
#include "pse_color_cost_RGB_p.h"
#include "pse_color_cost_XYZ_p.h"

#include <stb_ds.h>

/******************************************************************************
 *
 * HELPERS FUNCTIONS
 *
 ******************************************************************************/

static PSE_FINLINE pse_clt_cost_func_uid_t
pseColorCostFuncUIDFromConfig
  (const struct pse_color_cost_func_config_t* cfg)
{
  PSE_STATIC_ASSERT
    (  ( sizeof(pse_color_type_t)
       + sizeof(pse_color_space_t)
       + sizeof(pse_color_cost_func_id_t))
     <= sizeof(pse_clt_cost_func_uid_t),
     Invalid_type_size);
  assert((cfg->format <= 0xFFFF) && (cfg->id <= 0xFFFF));
  return
      ((0xFFFF & cfg->format) << 16)
    | ((0xFFFF & cfg->id)     << 0);
}

static PSE_FINLINE size_t
pseComponentsCountFromFlags
  (pse_color_components_flags_t comps)
{
  size_t i, count = 0;
  for(i = 0; i < PSE_COLOR_COMPONENTS_COUNT_MAX; ++i) {
    if( (1 << i) & comps )
      ++count;
  }
  return count;
}

static PSE_FINLINE pse_color_cost_func_implem_get_cb
pseColorCostFuncImplemGetCallbackFromSpace
  (const pse_color_space_t space)
{
  switch(space) {
    case PSE_COLOR_SPACE_Cat02LMS: return pseColorCat02LMSCostFunctorImplemGet;
    case PSE_COLOR_SPACE_HSV: return pseColorHSVCostFunctorImplemGet;
    case PSE_COLOR_SPACE_LAB: return pseColorLABCostFunctorImplemGet;
    case PSE_COLOR_SPACE_RGB: return pseColorRGBCostFunctorImplemGet;
    case PSE_COLOR_SPACE_XYZ: return pseColorXYZCostFunctorImplemGet;
    default: assert(false);
  }
  return NULL;
}

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

enum pse_res_t
pseColorPaletteCostFunctorsCleanAll
  (struct pse_color_palette_t* cp)
{
  enum pse_res_t res = RES_OK;
  assert(cp);
  hmfree(cp->cfuncs);
  return res;
}

enum pse_res_t
pseColorCostFunctorRegister
  (struct pse_cpspace_t* cps,
   const struct pse_color_cost_func_config_t* cfg,
   const struct pse_color_cost_func_implem_t* implem,
   pse_relshp_cost_func_id_t* fid)
{
  struct pse_relshp_cost_func_params_t params = PSE_RELSHP_COST_FUNC_PARAMS_NULL;
  assert(cps && cfg && implem && fid);
  assert(cfg->components != PSE_COLOR_COMPONENTS_NONE);

  params.uid = implem->uid;
  params.expected_pspace = cfg->format;
  params.compute = implem->compute;
  params.compute_df = NULL;
  params.cost_arity_mode = implem->cost_arity_mode;
  params.costs_count =
      (implem->color_cost_arity_mode == PSE_COLOR_COST_ARITY_MODE_PER_COLOR)
    ? implem->costs_count
    : implem->costs_count * pseComponentsCountFromFlags(cfg->components);
  params.ctxt_type_info = implem->ctxt_type_info;
  params.ctxts_init = implem->ctxts_init;
  params.ctxts_clean = implem->ctxts_clean;
  params.user_config = NULL;

  return pseConstrainedParameterSpaceRelationshipCostFunctorsRegister
    (cps, 1, &params, fid);
}

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

enum pse_res_t
pseColorPaletteCostFunctorsCreateOrGetFromConfig
  (struct pse_color_palette_t* cp,
   const size_t count,
   const struct pse_color_cost_func_config_t* configs,
   pse_relshp_cost_func_id_t* fids)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_cost_func_implem_t implem = PSE_COLOR_COST_FUNC_IMPLEM_NULL;
  struct pse_color_cost_func_data_t new_cfd = PSE_COLOR_COST_FUNC_DATA_NULL;
  size_t i, j;
  if( !cp || (count && (!configs || !fids)) )
    return RES_BAD_ARG;
  if( !count )
    return RES_OK; /* Nothing to do */

  for(i = 0; i < count; ++i) {
    const struct pse_color_cost_func_config_t* cfg = &configs[i];
    const pse_clt_cost_func_uid_t cfuid = pseColorCostFuncUIDFromConfig(cfg);
    if( hmgeti(cp->cfuncs, cfuid) < 0 ) {
      /* We do not have this cost functor right now. Create it! */

      /* First, retreive the implementation of the cost functor. */
      pse_color_cost_func_implem_get_cb implem_get_cb =
        pseColorCostFuncImplemGetCallbackFromSpace
          (PSE_COLOR_SPACE_FROM(cfg->format));
      PSE_VERIFY_OR_ELSE(implem_get_cb != NULL, res = RES_INTERNAL; goto error);
      implem = PSE_COLOR_COST_FUNC_IMPLEM_NULL;
      PSE_CALL_OR_GOTO(res,error, implem_get_cb
        (cfg->id, cfg->components, &implem));

      /* Then register this cost functor to the CPS associated to the color
       * palette. */
      new_cfd = PSE_COLOR_COST_FUNC_DATA_NULL;
      new_cfd.key = cfuid;
      new_cfd.cfg = *cfg;
      PSE_CALL_OR_GOTO(res,error, pseColorCostFunctorRegister
        (cp->cps, cfg, &implem, &new_cfd.fid));

      /* Everything is OK, keep this new cost functor for later use. */
      (void)hmputs(cp->cfuncs, new_cfd);
    }
    assert(hmgeti(cp->cfuncs, cfuid) >= 0);
    fids[i] = hmgets(cp->cfuncs, cfuid).fid;
  }

exit:
  return res;
error:
  /* Reset already retreived relationship cost functors. */
  for(j = 0; j < i; ++j) {
    fids[j] = PSE_RELSHP_COST_FUNC_ID_INVALID;
  }
  goto exit;
}

enum pse_res_t
pseColorPaletteCostFunctorsClean
  (struct pse_color_palette_t* cp,
   const size_t count,
   const struct pse_color_cost_func_config_t* configs)
{
  size_t i;
  if( !cp || (count && !configs) )
    return RES_BAD_ARG;

  /* First, check that all configurations exist. */
  for(i = 0; i < count; ++i) {
    const pse_clt_cost_func_uid_t cfuid =
      pseColorCostFuncUIDFromConfig(&configs[i]);
    PSE_VERIFY_OR_ELSE
      (hmgeti(cp->cfuncs, cfuid) >= 0,
       return RES_NOT_FOUND);
  }

  /* Then, go through all configurations and clean and remove them. */
  for(i = 0; i < count; ++i) {
    const pse_clt_cost_func_uid_t cfuid =
      pseColorCostFuncUIDFromConfig(&configs[i]);
    (void)hmdel(cp->cfuncs, cfuid);
  }

  return RES_OK;
}
