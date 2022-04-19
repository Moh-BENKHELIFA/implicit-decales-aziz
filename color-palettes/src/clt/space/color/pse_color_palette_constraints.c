#include "pse_color_palette_constraints_p.h"
#include "pse_color_palette_p.h"

#include <stb_ds.h>
#include <stretchy_buffer.h>

/******************************************************************************
 *
 * HELPER FUNCTIONS
 *
 ******************************************************************************/

static PSE_FINLINE enum pse_res_t
pseColorSpaceConstraintParamsCheck
  (struct pse_color_palette_t* cp,
   const struct pse_color_space_constraint_params_t* params)
{
  size_t i, count;
  count = sb_count(cp->ppoints_id);
  for(i = 0; i < params->colors_ref.colors_count; ++i) {
    if( params->colors_ref.colors_idx[i] >= count )
      return RES_BAD_ARG;
  }
  return (params->components != PSE_COLOR_COMPONENTS_NONE)
      && pseIsColorSpaceManaged(params->space)
      && (   (params->variations_count == 0)
          || (params->variations != NULL))
    ? RES_OK
    : RES_BAD_ARG;
}

static PSE_FINLINE enum pse_res_t
pseColorPaletteConstraintParamsCheck
  (struct pse_color_palette_t* cp,
   const struct pse_color_palette_constraint_params_t* params)
{
  if( !params->create )
    return RES_BAD_ARG;
  return pseColorSpaceConstraintParamsCheck(cp, &params->space);
}

static PSE_INLINE enum pse_res_t
pseColorPaletteConstraintInGamutRelationshipsCreate
  (pse_color_constraint_custom_data_t custom_data,
   struct pse_color_palette_t* cp,
   const pse_color_palette_constraint_id_t id,
   const struct pse_color_space_constraint_params_t* params)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_constraint_generic_weighted_config_t* cnstr_cfg =
    (struct pse_color_constraint_generic_weighted_config_t*)custom_data;
  struct pse_color_cost_func_config_t cfcfg = PSE_COLOR_COST_FUNC_CONFIG_NULL;
  struct pse_cpspace_relshp_params_t relshp = PSE_CPSPACE_RELSHP_PARAMS_NULL;
  pse_relshp_cost_func_id_t fid = PSE_RELSHP_COST_FUNC_ID_INVALID;
  pse_ppoint_id_t* ppoints_id = NULL;
  assert(cp && params && id != PSE_COLOR_PALETTE_CONSTRAINT_ID_INVALID);

  /* First, just get the parametric points associated to the color indices. */
  sb_setn(ppoints_id, params->colors_ref.colors_count);
  PSE_CALL_OR_GOTO(res,exit, pseColorPaletteColorsIndexToParametricPointsId
    (cp, params->colors_ref.colors_count, params->colors_ref.colors_idx,
     ppoints_id));

  switch(params->space) {
    case PSE_COLOR_SPACE_RGB: cfcfg.id = PSE_COLOR_COST_FUNC_RGB_InGamut; break;
    default: assert(false); res = RES_NOT_SUPPORTED; goto exit;
  }

  /* Get the cost functor configuration. */
  cfcfg.format = cnstr_cfg->format;
  cfcfg.components = params->components;
  PSE_CALL_OR_GOTO(res,exit,
    pseColorPaletteCostFunctorsCreateOrGetFromConfig
      (cp, 1, &cfcfg, &fid));

  relshp.kind = params->colors_ref.kind;
  relshp.ppoints_count = params->colors_ref.colors_count;
  relshp.ppoints_id = ppoints_id;
  relshp.variations_count = params->variations_count;
  relshp.variations = params->variations;
  relshp.cnstrs.funcs_count = 1;
  relshp.cnstrs.funcs = &fid;
  relshp.cnstrs.ctxts_config = &custom_data;

  PSE_CALL_OR_GOTO(res,exit, pseColorPaletteConstraintRelationshipsPush
    (cp, id, 1, &relshp));

exit:
  sb_free(ppoints_id);
  return res;
}

static PSE_INLINE enum pse_res_t
pseColorPaletteConstraintOLEDScreenEnergyUsageRelationshipsCreate
  (pse_color_constraint_custom_data_t custom_data,
   struct pse_color_palette_t* cp,
   const pse_color_palette_constraint_id_t id,
   const struct pse_color_space_constraint_params_t* params)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_constraint_generic_weighted_config_t* cnstr_cfg =
    (struct pse_color_constraint_generic_weighted_config_t*)custom_data;
  struct pse_color_cost_func_config_t cfcfg = PSE_COLOR_COST_FUNC_CONFIG_NULL;
  struct pse_cpspace_relshp_params_t relshp = PSE_CPSPACE_RELSHP_PARAMS_NULL;
  pse_relshp_cost_func_id_t fid = PSE_RELSHP_COST_FUNC_ID_INVALID;
  pse_ppoint_id_t* ppoints_id = NULL;
  assert(cp && params && id != PSE_COLOR_PALETTE_CONSTRAINT_ID_INVALID);

  /* First, just get the parametric points associated to the color indices. */
  sb_setn(ppoints_id, params->colors_ref.colors_count);
  PSE_CALL_OR_GOTO(res,exit, pseColorPaletteColorsIndexToParametricPointsId
    (cp, params->colors_ref.colors_count, params->colors_ref.colors_idx,
     ppoints_id));

  switch(params->space) {
    case PSE_COLOR_SPACE_RGB: cfcfg.id = PSE_COLOR_COST_FUNC_RGB_OLEDScreenEnergyUsage; break;
    default: assert(false); res = RES_NOT_SUPPORTED; goto exit;
  }

  /* Get the cost functor configuration. */
  cfcfg.format = cnstr_cfg->format;
  cfcfg.components = params->components;
  PSE_CALL_OR_GOTO(res,exit,
    pseColorPaletteCostFunctorsCreateOrGetFromConfig
      (cp, 1, &cfcfg, &fid));

  relshp.kind = params->colors_ref.kind;
  relshp.ppoints_count = params->colors_ref.colors_count;
  relshp.ppoints_id = ppoints_id;
  relshp.variations_count = params->variations_count;
  relshp.variations = params->variations;
  relshp.cnstrs.funcs_count = 1;
  relshp.cnstrs.funcs = &fid;
  relshp.cnstrs.ctxts_config = &custom_data;

  PSE_CALL_OR_GOTO(res,exit, pseColorPaletteConstraintRelationshipsPush
    (cp, id, 1, &relshp));

exit:
  sb_free(ppoints_id);
  return res;
}

static PSE_INLINE enum pse_res_t
pseColorPaletteConstraintDistanceRelationshipsCreate
  (pse_color_constraint_custom_data_t custom_data,
   struct pse_color_palette_t* cp,
   const pse_color_palette_constraint_id_t id,
   const struct pse_color_space_constraint_params_t* params)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_constraint_distance_config_t* cnstr_cfg =
    (struct pse_color_constraint_distance_config_t*)custom_data;
  struct pse_color_cost_func_config_t cfcfg = PSE_COLOR_COST_FUNC_CONFIG_NULL;
  struct pse_cpspace_relshp_params_t* relshps = NULL;
  pse_relshp_cost_func_id_t fid = PSE_RELSHP_COST_FUNC_ID_INVALID;
  pse_ppoint_id_t* all_ppids = NULL;
  pse_ppoint_id_t* ppoints_id = NULL;
  size_t ppoints_count = 0, relshps_count = 0;
  size_t i, j, ippid;
  assert(cp && params && (id != PSE_COLOR_PALETTE_CONSTRAINT_ID_INVALID));

  /* Deduce the ppoints involved in this constraint. */
  PSE_CALL_OR_GOTO(res,exit, pseColorPaletteConstraintColorsRefResolveNow
      (cp, &params->colors_ref, &ppoints_count, &ppoints_id));

  /* Check if we can do something */
  PSE_VERIFY_OR_ELSE(ppoints_count > 0, res = RES_BAD_ARG; goto exit);
  if( ppoints_count == 1 )
    goto exit;  /* Not enough ppoints to add relationships! */

  /* Get the cost functor configuration. */
  cfcfg.format = cnstr_cfg->as_weighted.format;
  cfcfg.components = params->components;
  /* TODO: here, we should check the cnstr_cfg->dist_params to see if wee need
   * to enable the threshold management or not. This way, having 2 versions of
   * the functions, we could avoid a test in the "hot loop" and win some
   * percents. Could we manage this without using the constraint id?
   */
  switch(params->space) {
    case PSE_COLOR_SPACE_HSV: {
      cfcfg.id = cnstr_cfg->mode == PSE_COLOR_CONSTRAINT_MODE_PER_COLOR
        ? PSE_COLOR_COST_FUNC_HSV_L1DistanceSigned
        : PSE_COLOR_COST_FUNC_HSV_L1DistanceSignedPerComponent;
    } break;
    default: {
      cfcfg.id = cnstr_cfg->mode == PSE_COLOR_CONSTRAINT_MODE_PER_COLOR
        ? PSE_COLOR_COST_FUNC_Generic_L1DistanceSigned
        : PSE_COLOR_COST_FUNC_Generic_L1DistanceSignedPerComponent;
    } break;
  }

  PSE_CALL_OR_GOTO(res,exit,
    pseColorPaletteCostFunctorsCreateOrGetFromConfig
      (cp, 1, &cfcfg, &fid));

  /* The number of relationships we want is the sum of first naturals. */
  relshps_count = ppoints_count * (ppoints_count-1) / 2;

  /* Allocate memory */
  sb_reserve_more(relshps, relshps_count);
  sb_setn(all_ppids, relshps_count*2);  /* binary relationships */

  ippid = 0;
  for(i = 0; i < ppoints_count - 1; ++i) {
    for(j = i+1; j < ppoints_count; ++j) {
      struct pse_cpspace_relshp_params_t rparams =
        PSE_CPSPACE_RELSHP_PARAMS_NULL;
      rparams.kind = PSE_RELSHP_KIND_INCLUSIVE;
      rparams.ppoints_count = 2;
      rparams.ppoints_id = &all_ppids[ippid];
      rparams.variations_count = params->variations_count;
      rparams.variations = params->variations;
      rparams.cnstrs.funcs_count = 1;
      rparams.cnstrs.funcs = &fid;
      rparams.cnstrs.ctxts_config = &custom_data;

      all_ppids[ippid++] = ppoints_id[i];
      all_ppids[ippid++] = ppoints_id[j];
      assert(rparams.ppoints_id == &all_ppids[ippid - 2]);

      sb_push(relshps, rparams);
    }
  }

  res = pseColorPaletteConstraintRelationshipsPush
    (cp, id, relshps_count, relshps);
  sb_free(relshps);
  sb_free(all_ppids);
  PSE_VERIFY_OR_ELSE(res == RES_OK, goto exit);

exit:
  /* Do not forget to clean the buffer */
  pseColorPaletteConstraintColorsRefBufferClean(cp, ppoints_count, ppoints_id);
  return res;
}

static PSE_INLINE enum pse_res_t
pseColorPaletteConstraintGenericDestroy
  (pse_color_constraint_custom_data_t custom_data,
   struct pse_color_palette_t* cp,
   const pse_color_palette_constraint_id_t id)
{
  assert(custom_data && cp && (id != PSE_COLOR_PALETTE_CONSTRAINT_ID_INVALID));
  (void)id;
  PSE_FREE(cp->alloc, custom_data);
  return RES_OK;
}

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

enum pse_res_t
pseColorPaletteConstraintCreate
  (struct pse_color_palette_t* cp,
   const struct pse_color_palette_constraint_params_t* params,
   struct pse_color_palette_constraint_t** out_cnstr)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_palette_constraint_t cnstr =
    PSE_COLOR_PALETTE_CONSTRAINT_NULL;
  struct pse_color_space_constraint_params_t* space_params = NULL;
  size_t i;
  assert(cp && params && out_cnstr);
  PSE_TRY_CALL_OR_RETURN(res, pseColorPaletteConstraintParamsCheck(cp, params));

  cnstr.key = cp->constraint_next_id++;
  cnstr.params = *params;

  space_params = &cnstr.params.space;
  space_params->variations = NULL; /* duplicate this buffer */
  sb_setn(space_params->variations, space_params->variations_count);
  for(i = 0; i < space_params->variations_count; ++i) {
    space_params->variations[i] = params->space.variations[i];
  }
  space_params->colors_ref.colors_idx = NULL;
  sb_setn
    (space_params->colors_ref.colors_idx,
     space_params->colors_ref.colors_count);
  for(i = 0; i < space_params->colors_ref.colors_count; ++i) {
    space_params->colors_ref.colors_idx[i] =
      params->space.colors_ref.colors_idx[i];
  }

  (void)hmputs(cp->constraints, cnstr);
  *out_cnstr = &hmgets(cp->constraints, cnstr.key);

  return res;
}

PSE_INLINE enum pse_res_t
pseColorPaletteConstraintDestroy
  (struct pse_color_palette_t* cp,
   struct pse_color_palette_constraint_t* cnstr)
{
  sb_free(cnstr->params.space.variations);
  sb_free(cnstr->params.space.colors_ref.colors_idx);

  /* Remove the constraint */
  (void)hmdel(cp->constraints, cnstr->key);
  return RES_OK;
}

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

enum pse_res_t
pseColorPaletteConstraintAdd
  (struct pse_color_palette_t* cp,
   struct pse_color_palette_constraint_params_t* params,
   pse_color_palette_constraint_id_t* id)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_palette_constraint_t* cnstr = NULL;
  if( !cp || !params || !id )
    return RES_BAD_ARG;

  PSE_CALL_OR_GOTO(res,error, pseColorPaletteConstraintCreate
    (cp, params, &cnstr));
  PSE_CALL_OR_GOTO(res,error, cnstr->params.create
    (cnstr->params.custom_data, cp, cnstr->key, &cnstr->params.space));

  *id = cnstr->key;

exit:
  return res;
error:
  if( cnstr != NULL ) {
    PSE_CALL(pseColorPaletteConstraintDestroy(cp, cnstr));
  }
  goto exit;
}

enum pse_res_t
pseColorPaletteConstraintRemove
  (struct pse_color_palette_t* cp,
   const pse_color_palette_constraint_id_t id)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_palette_constraint_t* cnstr = NULL;
  if( !cp )
    return RES_BAD_ARG;
  if( hmgeti(cp->constraints, id) < 0 )
    return RES_NOT_FOUND;

  /* Destroy the constraint */
  cnstr = &hmgets(cp->constraints, id);
  assert(cnstr && cnstr->key == id);
  if( cnstr->params.destroy ) {
    PSE_CALL(cnstr->params.destroy(cnstr->params.custom_data, cp, id));
  }
  /* Accept NOT_FOUND as if we don't have generated relationships, that may
   * occur. */
  res = pseConstrainedParameterSpaceRelationshipsRemoveByGroup(cp->cps, id);
  PSE_VERIFY_OR_ELSE(res == RES_OK || res == RES_NOT_FOUND, return res);
  PSE_CALL(pseColorPaletteConstraintDestroy(cp, cnstr));

  return RES_OK;
}

enum pse_res_t
pseColorPaletteConstraintSwitch
  (struct pse_color_palette_t* cp,
   const pse_color_palette_constraint_id_t id,
   bool enable)
{
  enum pse_res_t res = RES_OK;
  if( !cp )
    return RES_BAD_ARG;
  if( hmgeti(cp->constraints, id) < 0 )
    return RES_NOT_FOUND;

  PSE_CALL_OR_RETURN(res,
    pseConstrainedParameterSpaceRelationshipsStateSetByGroup
      (cp->cps, id, enable));
  return res;
}

enum pse_res_t
pseColorPaletteConstraintRelationshipsPush
  (struct pse_color_palette_t* cp,
   const pse_color_palette_constraint_id_t id,
   const size_t relshps_count,
   struct pse_cpspace_relshp_params_t* relshps)
{
  enum pse_res_t res = RES_OK;
  pse_relshp_id_t* relshps_id = NULL;
  if( !cp || (relshps_count && !relshps) )
    return RES_BAD_ARG;
  if( hmgeti(cp->constraints, id) < 0 )
    return RES_NOT_FOUND;

  sb_setn(relshps_id, relshps_count);
  PSE_CALL_OR_GOTO(res,error, pseConstrainedParameterSpaceRelationshipsAdd
    (cp->cps, id, relshps_count, relshps, relshps_id));

exit:
  sb_free(relshps_id);
  return res;
error:
  goto exit;
}

/******************************************************************************
 *
 * PUBLIC HELPERS API
 *
 ******************************************************************************/

enum pse_res_t
pseColorPaletteConstraintColorsRefResolveNow
   (struct pse_color_palette_t* cp,
    const struct pse_color_space_constraint_colors_ref_t* relshp,
    size_t* cnstr_ppoints_count,
    pse_ppoint_id_t** cnstr_ppoints_id) /* managed as a stretchy buffer */
{
  enum pse_res_t res = RES_OK;
  size_t i, all_count;
  if( !cp || !relshp || !cnstr_ppoints_count || !cnstr_ppoints_id )
    return RES_BAD_ARG;
  if( relshp->colors_count && !relshp->colors_idx )
    return RES_BAD_ARG;

  /* First, check color indices */
  all_count = sb_count(cp->ppoints_id);
  for(i = 0; i < relshp->colors_count; ++i) {
    const pse_color_idx_t idx = relshp->colors_idx[i];
    PSE_VERIFY_OR_ELSE(idx < all_count, return RES_BAD_ARG);
  }

  switch(relshp->kind) {
    case PSE_RELSHP_KIND_INCLUSIVE: {
      /* We just have to duplicate the ppoints buffer of the constraint. */
      *cnstr_ppoints_count = relshp->colors_count;
      if( *cnstr_ppoints_count > 0 ) {
        sb_setn(*cnstr_ppoints_id, *cnstr_ppoints_count);
        for(i = 0; i < *cnstr_ppoints_count; ++i) {
          (*cnstr_ppoints_id)[i] = cp->ppoints_id[relshp->colors_idx[i]];
        }
      } else {
        *cnstr_ppoints_id = NULL;
      }
    } break;
    case PSE_RELSHP_KIND_EXCLUSIVE: {
      /* We have to find ppoints that are not in the buffer of the constraint.*/
      const size_t excluded_count = relshp->colors_count;
      PSE_VERIFY_OR_ELSE
        (all_count >= excluded_count,
         res = RES_BAD_ARG; goto error);

      if( all_count == excluded_count ) {
        /* Simple case: all ppoints are excluded! */
        *cnstr_ppoints_count = 0;
        *cnstr_ppoints_id = NULL;
      } else if( excluded_count == 0 ) {
        /* Simple case: no ppoint excluded, we copy all the color palette
         * ppoints. */
        *cnstr_ppoints_count = all_count;
        sb_setn(*cnstr_ppoints_id, *cnstr_ppoints_count);
        for(i = 0; i < *cnstr_ppoints_count; ++i) {
          (*cnstr_ppoints_id)[i] = cp->ppoints_id[i];
        }
      } else {
        /* We have to exclude some ppoints */
        size_t j;
        *cnstr_ppoints_count = all_count - excluded_count;
        sb_setn(*cnstr_ppoints_id, *cnstr_ppoints_count);
        for(i = 0; i < all_count; ++i) {
          const pse_ppoint_id_t ppid = cp->ppoints_id[i];
          bool excluded = false;
          for(j = 0; j < excluded_count; ++j) {
            const pse_color_idx_t idx = relshp->colors_idx[i];
            if( ppid == cp->ppoints_id[idx] ) {
              excluded = true;
              break;
            }
          }
          if( !excluded ) {
            PSE_VERIFY_OR_ELSE
              (i < *cnstr_ppoints_count,
               res = RES_BAD_ARG; goto error);
            (*cnstr_ppoints_id)[i] = ppid;
          }
        }
      }
    } break;
    default: assert(false); res = RES_NOT_SUPPORTED;
  }
exit:
  return res;
error:
  sb_free(*cnstr_ppoints_id);
  *cnstr_ppoints_id = NULL;
  *cnstr_ppoints_count = 0;
  goto exit;
}

void
pseColorPaletteConstraintColorsRefBufferClean
   (struct pse_color_palette_t* cp,
    const size_t cnstr_ppoints_count,
    pse_ppoint_id_t* cnstr_ppoints_id)
{
  (void)cnstr_ppoints_count;
  if( !cp )
    return;
  sb_free(cnstr_ppoints_id);
}

enum pse_res_t
pseColorPaletteConstraintInGamutAdd
  (struct pse_color_palette_t* cp,
   const struct pse_color_space_constraint_params_t* params,
   pse_color_palette_constraint_id_t* id)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_palette_constraint_params_t cnstr_params =
    PSE_COLOR_PALETTE_CONSTRAINT_PARAMS_NULL;
  struct pse_color_constraint_generic_weighted_config_t* cnstr_cfg = NULL;
  if( !cp || !params || !id )
    return RES_BAD_ARG;
  if( params->space != PSE_COLOR_SPACE_RGB )
    return RES_BAD_ARG;

  /* Create the weighted configuration for this constraint. We will pass this
   * configuration to all the relationships associated with the constraint. */
  cnstr_cfg = PSE_TYPED_ALLOC
    (cp->alloc, struct pse_color_constraint_generic_weighted_config_t);
  PSE_VERIFY_OR_ELSE(cnstr_cfg != NULL, return RES_MEM_ERR);
  *cnstr_cfg = PSE_COLOR_CONSTRAINT_GENERIC_WEIGHTED_CONFIG_INVALID;
  cnstr_cfg->format = pseColorFormatManagedFromSpace(params->space);
  cnstr_cfg->weight = params->weight;

  cnstr_params.create = pseColorPaletteConstraintInGamutRelationshipsCreate;
  cnstr_params.destroy = pseColorPaletteConstraintGenericDestroy;
  cnstr_params.space = *params;
  cnstr_params.custom_data = cnstr_cfg;
  PSE_CALL_OR_GOTO(res, error, pseColorPaletteConstraintAdd
    (cp, &cnstr_params, id));
exit:
  return res;
error:
  PSE_FREE(cp->alloc, cnstr_cfg);
  goto exit;
}


enum pse_res_t
pseColorPaletteConstraintDistanceAdd
  (struct pse_color_palette_t* cp,
   const struct pse_color_space_constraint_params_t* params,
   const struct pse_color_distance_params_t* dist_params,
   pse_color_palette_constraint_id_t* id)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_palette_constraint_params_t cnstr_params =
    PSE_COLOR_PALETTE_CONSTRAINT_PARAMS_NULL;
  struct pse_color_constraint_distance_config_t* cnstr_cfg = NULL;
  if( !cp || !params || !dist_params || !id )
    return RES_BAD_ARG;

  /* TODO: externalize the mode in order to avoid to have 2 functions that takes
   * exactly the same parameters. */

  /* Create the weighted distance configuration for this constraint. We will
   * pass this configuration to all the relationships associated with the
   * constraint. */
  cnstr_cfg = PSE_TYPED_ALLOC
    (cp->alloc, struct pse_color_constraint_distance_config_t);
  PSE_VERIFY_OR_ELSE(cnstr_cfg != NULL, res = RES_MEM_ERR; goto error);
  *cnstr_cfg = PSE_COLOR_CONSTRAINT_DISTANCE_PARAMS_NULL;
  cnstr_cfg->as_weighted.format = pseColorFormatManagedFromSpace(params->space);
  cnstr_cfg->as_weighted.weight = params->weight;
  cnstr_cfg->dist_params = *dist_params;
  cnstr_cfg->mode = PSE_COLOR_CONSTRAINT_MODE_PER_COLOR;

  cnstr_params.create = pseColorPaletteConstraintDistanceRelationshipsCreate;
  cnstr_params.destroy = pseColorPaletteConstraintGenericDestroy;
  cnstr_params.space = *params;
  cnstr_params.custom_data = cnstr_cfg;
  PSE_CALL_OR_GOTO(res,error, pseColorPaletteConstraintAdd
    (cp, &cnstr_params, id));

exit:
  return res;
error:
  PSE_FREE(cp->alloc, cnstr_cfg);
  goto exit;
}

enum pse_res_t
pseColorPaletteConstraintDistancePerComponentAdd
  (struct pse_color_palette_t* cp,
   const struct pse_color_space_constraint_params_t* params,
   const struct pse_color_distance_params_t* dist_params,
   pse_color_palette_constraint_id_t* id)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_palette_constraint_params_t cnstr_params =
    PSE_COLOR_PALETTE_CONSTRAINT_PARAMS_NULL;
  struct pse_color_constraint_distance_config_t* cnstr_cfg = NULL;
  if( !cp || !params || !id )
    return RES_BAD_ARG;

  /* TODO: externalize the mode in order to avoid to have 2 functions that takes
   * exactly the same parameters. */

  /* Create the weighted distance configuration for this constraint. We will
   * pass this configuration to all the relationships associated with the
   * constraint. */
  cnstr_cfg = PSE_TYPED_ALLOC
    (cp->alloc, struct pse_color_constraint_distance_config_t);
  PSE_VERIFY_OR_ELSE(cnstr_cfg != NULL, res = RES_MEM_ERR; goto error);
  *cnstr_cfg = PSE_COLOR_CONSTRAINT_DISTANCE_PARAMS_NULL;
  cnstr_cfg->as_weighted.format = pseColorFormatManagedFromSpace(params->space);
  cnstr_cfg->as_weighted.weight = params->weight;
  cnstr_cfg->dist_params = *dist_params;
  cnstr_cfg->mode = PSE_COLOR_CONSTRAINT_MODE_PER_COMPONENT;

  cnstr_params.create = pseColorPaletteConstraintDistanceRelationshipsCreate;
  cnstr_params.destroy = pseColorPaletteConstraintGenericDestroy;
  cnstr_params.space = *params;
  cnstr_params.custom_data = cnstr_cfg;
  PSE_CALL_OR_GOTO(res, error, pseColorPaletteConstraintAdd
    (cp, &cnstr_params, id));
exit:
  return res;
error:
  PSE_FREE(cp->alloc, cnstr_cfg);
  goto exit;
}

enum pse_res_t
pseColorPaletteConstraintOLEDScreenEnergyConsumptionAdd
  (struct pse_color_palette_t* cp,
   const struct pse_color_space_constraint_params_t* params,
   pse_color_palette_constraint_id_t* id)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_palette_constraint_params_t cnstr_params =
    PSE_COLOR_PALETTE_CONSTRAINT_PARAMS_NULL;
  struct pse_color_constraint_generic_weighted_config_t* cnstr_cfg = NULL;
  if( !cp || !params || !id )
    return RES_BAD_ARG;
  if( params->space != PSE_COLOR_SPACE_RGB )
    return RES_BAD_ARG;

  /* Create the weighted configuration for this constraint. We will pass this
   * configuration to all the relationships associated with the constraint. */
  cnstr_cfg = PSE_TYPED_ALLOC
    (cp->alloc, struct pse_color_constraint_generic_weighted_config_t);
  PSE_VERIFY_OR_ELSE(cnstr_cfg != NULL, return RES_MEM_ERR);
  *cnstr_cfg = PSE_COLOR_CONSTRAINT_GENERIC_WEIGHTED_CONFIG_INVALID;
  cnstr_cfg->format = pseColorFormatManagedFromSpace(params->space);
  cnstr_cfg->weight = params->weight;

  cnstr_params.create = pseColorPaletteConstraintOLEDScreenEnergyUsageRelationshipsCreate;
  cnstr_params.destroy = pseColorPaletteConstraintGenericDestroy;
  cnstr_params.space = *params;
  cnstr_params.custom_data = cnstr_cfg;
  PSE_CALL_OR_GOTO(res, error, pseColorPaletteConstraintAdd
    (cp, &cnstr_params, id));
exit:
  return res;
error:
  PSE_FREE(cp->alloc, cnstr_cfg);
  goto exit;
}
