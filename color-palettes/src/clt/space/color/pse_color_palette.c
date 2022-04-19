#include "pse_color_palette_p.h"
#include "pse_color_palette_constraints_p.h"
#include "pse_color_cost_p.h"

#include <pse_allocator.h>

#define STB_DS_IMPLEMENTATION
#include <stb_ds.h>
#include <stretchy_buffer.h>

/******************************************************************************
 *
 * HELPER FUNCTIONS
 *
 ******************************************************************************/

static PSE_FINLINE enum pse_res_t
pseColorPaletteVariationsAreNotDeclared
  (struct pse_color_palette_t* cp,
   const size_t count,
   const pse_color_variation_uid_t* uids)
{
  size_t i;
  assert(cp && count && uids);
  for(i = 0; i < count; ++i) {
    if( uids[i] == PSE_CLT_PPOINT_VARIATION_UID_INVALID )
      return RES_BAD_ARG;
    if( hmgeti(cp->variations, uids[i]) >= 0 )
      return RES_ALREADY_EXISTS;
  }
  return RES_OK;
}

static PSE_FINLINE enum pse_res_t
pseColorPaletteVariationsAreDeclared
  (struct pse_color_palette_t* cp,
   const size_t count,
   const pse_color_variation_uid_t* uids)
{
  size_t i;
  assert(cp && count && uids);
  for(i = 0; i < count; ++i) {
    if( uids[i] == PSE_CLT_PPOINT_VARIATION_UID_INVALID )
      return RES_BAD_ARG;
    if( hmgeti(cp->variations, uids[i]) < 0 )
      return RES_NOT_FOUND;
  }
  return RES_OK;
}

static PSE_FINLINE enum pse_res_t
pseColorPaletteVariationParamsCheck
  (struct pse_color_palette_t* cp,
   const struct pse_color_variation_params_t* params)
{
  (void)cp;
  return !params->apply ? RES_BAD_ARG : RES_OK;
}

static PSE_INLINE enum pse_res_t
pseColorPaletteParameterSpaceParamsFill
  (const pse_color_format_t format,
   struct pse_pspace_params_t* params)
{
  enum pse_res_t res = RES_OK;
  switch(format) {
    case PSE_COLOR_FORMAT_Cat02LMSr_: {
      *params = PSE_COLOR_SPACE_PARAMS_Cat02LMSr;
    } break;
    case PSE_COLOR_FORMAT_HSVr_: {
      *params = PSE_COLOR_SPACE_PARAMS_HSVr;
    } break;
    case PSE_COLOR_FORMAT_LABr_: {
      *params = PSE_COLOR_SPACE_PARAMS_LABr;
    } break;
    case PSE_COLOR_FORMAT_RGBr_: {
      *params = PSE_COLOR_SPACE_PARAMS_RGBr;
    } break;
    case PSE_COLOR_FORMAT_XYZr_: {
      *params = PSE_COLOR_SPACE_PARAMS_XYZr;
    } break;
    default: assert(false); res = RES_NOT_SUPPORTED;
  }
  return res;
}

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

enum pse_res_t
pseColorPaletteCreate
  (const struct pse_color_palette_params_t* params,
   struct pse_color_palette_t** out_cp)
{
  enum pse_res_t res = RES_OK;
  struct pse_allocator_t* alloc = NULL;
  struct pse_color_palette_t* cp = NULL;
  struct pse_cpspace_params_t cpsp = PSE_CPSPACE_PARAMS_NULL;
  struct pse_pspace_params_t psps[PSE_COLOR_PALETTE_SPACES_MANAGED_COUNT] =
    { PSE_PSPACE_PARAMS_NULL_, PSE_PSPACE_PARAMS_NULL_ };
  struct pse_color_space_data_t csd;
  size_t i;
  if( !params || !out_cp || !params->dev )
    return RES_BAD_ARG;

  alloc = params->alloc ? params->alloc : &PSE_ALLOCATOR_DEFAULT;

  cp = PSE_TYPED_ALLOC(alloc, struct pse_color_palette_t);
  PSE_VERIFY_OR_ELSE(cp != NULL, res = RES_MEM_ERR; goto error);
  *cp = PSE_COLOR_PALETTE_NULL;
  cp->alloc = alloc; /* TODO: use a proxy allocator */
  cp->dev = params->dev;
  cp->logger = params->logger;

  /* Build formats list */
  sb_reserve_more(cp->formats, PSE_COLOR_PALETTE_SPACES_MANAGED_COUNT);
  for(i = 0; i < PSE_COLOR_PALETTE_SPACES_MANAGED_COUNT; ++i) {
    sb_push(cp->formats, PSE_COLOR_PALETTE_COMPUTATION_FORMATS_MANAGED[i]);
  }

  /* Get the managed parameter spaces */
  for(i = 0; i < PSE_COLOR_PALETTE_SPACES_MANAGED_COUNT; ++i) {
    PSE_CALL_OR_GOTO(res,error, pseColorPaletteParameterSpaceParamsFill
      (cp->formats[i], &psps[i]));

    /* Fill our pspace map */
    csd = PSE_COLOR_SPACE_DATA_NULL;
    csd.key = cp->formats[i];
    (void)hmputs(cp->pspaces, csd);
  }

  PSE_CALL_OR_GOTO(res,error, pseConstrainedParameterSpaceCreate
    (params->dev, &cpsp, &cp->cps));
  PSE_CALL_OR_GOTO(res,error, pseConstrainedParameterSpaceParameterSpacesDeclare
    (cp->cps, PSE_COLOR_PALETTE_SPACES_MANAGED_COUNT, cp->formats, psps));

  *out_cp = cp;

exit:
  return res;
error:
  if( cp != NULL ) {
    sb_free(cp->formats);
    hmfree(cp->pspaces);
    if( cp->cps )
      PSE_CALL(pseConstrainedParameterSpaceRefSub(cp->cps));
    PSE_FREE(alloc, cp);
  }
  goto exit;
}

enum pse_res_t
pseColorPaletteDestroy
  (struct pse_color_palette_t* cp)
{
  if( !cp )
    return RES_BAD_ARG;

  if( cp->cps ) {
    /* Remove the constraints before releasing the CPS to ensure is it alive! */
    while( hmlen(cp->constraints) ) {
      PSE_CALL(pseColorPaletteConstraintRemove(cp, cp->constraints[0].key));
    }
    PSE_CALL(pseConstrainedParameterSpaceRefSub(cp->cps));
  }
  PSE_CALL(pseColorPaletteCostFunctorsCleanAll(cp));
  sb_free(cp->ppoints_id);
  sb_free(cp->formats);
  hmfree(cp->pspaces);
  sb_free(cp->variations_uid);
  hmfree(cp->variations);
  hmfree(cp->constraints);
  PSE_FREE(cp->alloc, cp);
  return RES_OK;
}

struct pse_cpspace_t*
pseColorPaletteConstrainedParameterSpaceGet
  (struct pse_color_palette_t* cp)
{
  return cp ? cp->cps : NULL;
}

size_t
pseColorPalettePointCountGet
  (struct pse_color_palette_t* cp)
{
  return cp ? sb_count(cp->ppoints_id) : 0;
}

enum pse_res_t
pseColorPalettePointCountSet
  (struct pse_color_palette_t* cp,
   const size_t count)
{
  enum pse_res_t res = RES_OK;
  size_t ppoints_count = 0;
  struct pse_ppoint_params_t* params = NULL; /* Useless */
  if( !cp )
    return RES_BAD_ARG;
  ppoints_count = sb_count(cp->ppoints_id);
  if( ppoints_count == count )
    return RES_OK;

  /* TODO: what about the values created with the previous size? Do we need to
   * fit them? Do we need to invalidate them like iterators? */
  /* TODO: we have to modify some constraints to fit the new number of colors:
   * - translation_only
   * By deleting and recreating everything, I think it will work.
   * Do not forget to take variations into account. */

  if( ppoints_count < count ) {
    /* We want more points than before */
    const size_t count_diff = count - ppoints_count;
    sb_setn(params, count_diff);
    sb_setn(cp->ppoints_id, count);
    PSE_CALL_OR_GOTO(res,error, pseConstrainedParameterSpaceParametricPointsAdd
      (cp->cps,
       count_diff,
       params,
       &cp->ppoints_id[ppoints_count]));
  } else {
    /* We want less points than before */
    const size_t count_diff = ppoints_count - count;
    PSE_CALL_OR_GOTO(res,error, pseConstrainedParameterSpaceParametricPointsRemove
      (cp->cps, count_diff, &cp->ppoints_id[count]));
    sb_setn(cp->ppoints_id, count);
  }
  ppoints_count = count;

exit:
  sb_free(params);
  return res;
error:
  sb_setn(cp->ppoints_id, ppoints_count);
  goto exit;
}

enum pse_res_t
pseColorPaletteColorsIndexToParametricPointsId
  (struct pse_color_palette_t* cp,
   const size_t count,
   const pse_color_idx_t* indices,
   pse_ppoint_id_t* ids)
{
  enum pse_res_t res = RES_OK;
  size_t i, ppoints_count = 0;
  if( !cp || (count && (!indices || !ids)) )
    return RES_BAD_ARG;
  if( !count )
    return RES_OK;

  ppoints_count = sb_count(cp->ppoints_id);
  for(i = 0; i < count; ++i) {
    const pse_color_idx_t idx = indices[i];
    PSE_VERIFY_OR_ELSE(idx < ppoints_count, return RES_BAD_ARG);
    ids[i] = cp->ppoints_id[idx];
  }

  return res;
}

enum pse_res_t
pseColorPaletteVariationsDeclare
  (struct pse_color_palette_t* cp,
   const size_t count,
   const pse_color_variation_uid_t* uids,
   const struct pse_color_variation_params_t* params)
{
  enum pse_res_t res = RES_OK;
  size_t i;
  if( !cp || (count && (!uids || !params)) )
    return RES_BAD_ARG;
  if( count == 0 )
    return RES_OK;

  /* First, check the existance of the variations */
  PSE_TRY_CALL_OR_RETURN(res, pseColorPaletteVariationsAreNotDeclared
    (cp, count, uids));

  /* Check that the params are valid */
  PSE_TRY_CALL_OR_RETURN(res, pseColorPaletteVariationParamsCheck(cp, params));

  /* Add the variations to the parametric spaces */
  PSE_CALL_OR_RETURN(res,
    pseConstrainedParameterSpaceParameterSpacesVariationsAdd
      (cp->cps, sb_count(cp->formats), cp->formats, count, uids));

  /* If everything went OK, keep track of the variations params */
  sb_reserve_more(cp->variations_uid, count);
  for(i = 0; i < count; ++i) {
    struct pse_color_variation_data_t var = PSE_COLOR_VARIATION_DATA_NULL;
    var.key = uids[i];
    var.params = params[i];
    sb_push(cp->variations_uid, var.key);
    (void)hmputs(cp->variations, var);
  }

  return res;
}

enum pse_res_t
pseColorPaletteVariationsForget
  (struct pse_color_palette_t* cp,
   const size_t count,
   const pse_color_variation_uid_t* uids)
{
  enum pse_res_t res = RES_OK;
  size_t i, j;
  if( !cp || (count && !uids) )
    return RES_BAD_ARG;

  /* First, check the existance of the variations */
  PSE_TRY_CALL_OR_RETURN(res, pseColorPaletteVariationsAreDeclared
    (cp, count, uids));

  PSE_CALL_OR_RETURN(res,
    pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist
      (cp->cps, sb_count(cp->formats), cp->formats, count, uids));

  /* If everything went OK, remove the variations params */
  for(i = 0; i < count; ++i) {
    (void)hmdel(cp->variations, uids[i]);
    for(j = 0; j < sb_count(cp->variations_uid); ++j) {
      if( cp->variations_uid[j] == uids[i] ) {
        sb_delat(cp->variations_uid, j);
        break;
      }
    }
  }

  return res;
}
