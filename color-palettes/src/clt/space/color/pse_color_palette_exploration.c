#include "pse_color_palette_exploration.h"
#include "pse_color_palette_p.h"
#include "pse_color_conversion_p.h"
#include "pse_color_values_p.h"

#include <stb_ds.h>
#include <stretchy_buffer.h>

/* Static checks needed to ensure proper conversion of colors */
PSE_STATIC_ASSERT
  (offsetof(struct pse_color_any_components_t, mem) == 0,
   Inconsistent_color_component_structure_for_conversion);

/******************************************************************************
 *
 * HELPER FUNCTIONS
 *
 ******************************************************************************/

static PSE_INLINE enum pse_res_t
pseColorPaletteParameterSpaceConvert
  (void* user_data,
   const pse_color_format_t from,
   const pse_color_format_t to,
   const size_t values_count,
   const pse_real_t* values_from,
   pse_real_t* values_to)
{
  enum pse_res_t res = RES_OK;
  struct pse_colors_ref_t src, dst;
  assert(user_data && (from != to));
  (void)user_data;

  /* TODO: how to ensure that the values_from will not be modified by the apply
   * function? */
  PSE_CALL_OR_RETURN(res, pseColorsRefMapBuffer
    (&src, from, values_count, (pse_real_t*)values_from));
  PSE_CALL_OR_RETURN(res, pseColorsRefMapBuffer
    (&dst, to, values_count, values_to));
  PSE_VERIFY_OR_ELSE
    (NULL != pseNRefToNRef(&src, to, &dst),
     res = RES_NOT_SUPPORTED);

  return res;
}

static PSE_INLINE enum pse_res_t
pseColorSpaceVariationApply
  (void* user_data,
   const pse_color_format_t in,
   const pse_color_variation_uid_t to,
   const size_t values_count,
   const pse_real_t* values_from,
   pse_real_t* values_to)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_palette_t* cp = (struct pse_color_palette_t*)user_data;
  struct pse_color_variation_data_t* cvd = NULL;
  struct pse_colors_ref_t src, dst;
  assert(cp);

  cvd = &hmgets(cp->variations, to);
  assert(cvd);

  /* TODO: how to ensure that the values_from will not be modified by the apply
   * function? */
  PSE_CALL_OR_RETURN(res, pseColorsRefMapBuffer
    (&src, (pse_color_format_t)in, values_count, (pse_real_t*)values_from));
  PSE_CALL_OR_RETURN(res, pseColorsRefMapBuffer
    (&dst, (pse_color_format_t)in, values_count, values_to));
  PSE_CALL_OR_RETURN(res, cvd->params.apply
    (cvd->params.apply_user_data, &src, in, to, &dst));

  return res;
}

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

enum pse_res_t
pseColorPaletteExplorationContextCreate
  (struct pse_color_palette_t* cp,
   struct pse_color_palette_exploration_ctxt_params_t* params,
   struct pse_cpspace_exploration_ctxt_t** ctxt)
{
  enum pse_res_t res = RES_OK;
  struct pse_cpspace_exploration_ctxt_params_t cpsp;
  if( !cp || !params || !ctxt )
    return RES_BAD_ARG;
  if( params->explore_in == PSE_COLOR_FORMAT_INVALID )
    return RES_BAD_ARG;

  cpsp = PSE_CPSPACE_EXPLORATION_CTXT_PARAMS_NULL;
  cpsp.pspace.explore_in = params->explore_in;
  cpsp.pspace.convert = pseColorPaletteParameterSpaceConvert;
  cpsp.pspace.convert_user_data = cp;
  cpsp.variations.count = sb_count(cp->variations_uid);
  cpsp.variations.to_explore = cp->variations_uid;
  cpsp.variations.apply = pseColorSpaceVariationApply;
  cpsp.variations.apply_user_data = cp;
  cpsp.options = params->options;
  PSE_CALL_OR_RETURN(res, pseConstrainedParameterSpaceExplorationContextCreate
    (cp->cps, &cpsp, ctxt));

  return res;
}

enum pse_res_t
pseColorPaletteExplorationContextInitFromValues
  (struct pse_cpspace_exploration_ctxt_t* ctxt,
   struct pse_colors_t* refs)
{
  enum pse_res_t res = RES_OK;
  struct pse_cpspace_exploration_ctxt_params_t params;
  struct pse_color_palette_t* cp = NULL;
  struct pse_color_palette_values_t* cpvals = NULL;
  if( !ctxt || !refs )
    return RES_BAD_ARG;

  PSE_CALL_OR_RETURN(res,
    pseConstrainedParameterSpaceExplorationContextParamsGet
      (ctxt, &params));
  assert /* Ensure we have the right type */
    (  params.pspace.convert_user_data
    && (params.pspace.convert == pseColorPaletteParameterSpaceConvert) );

  /* Create values with the right number of colors by taking the variations into
   * account. */
  cp = (struct pse_color_palette_t*)params.pspace.convert_user_data;
  PSE_CALL_OR_GOTO(res,exit, pseColorPaletteValuesCreateFrom
    (cp, (pse_color_format_t)params.pspace.explore_in, refs, 0, NULL, &cpvals));

  /* Use the internal buffer of the cpvalues to feed the contexts */
  /* TODO: we should have a function able to build a pse_colors_t from
   * variations, without going through the cpvals */
  PSE_CALL_OR_GOTO(res,exit,
    pseConstrainedParameterSpaceExplorationRelationshipsAllContextsInit
      (ctxt, cpvals->colors));

exit:
  if( cpvals )
    PSE_CALL(pseColorPaletteValuesDestroy(cpvals));
  return res;
}

enum pse_res_t
pseColorPaletteExplorationSolve
  (struct pse_cpspace_exploration_ctxt_t* ctxt,
   struct pse_colors_t* smpls_clrs,
   const size_t locked_count,
   const pse_color_idx_t* locked)
{
  enum pse_res_t res = RES_OK;
  struct pse_cpspace_exploration_ctxt_params_t params;
  struct pse_cpspace_exploration_samples_t smpls =
    PSE_CPSPACE_EXPLORATION_SAMPLES_NULL;
  struct pse_color_palette_t* cp = NULL;
  struct pse_color_palette_values_t* cpvals = NULL;
  size_t i;
  if( !ctxt || !smpls_clrs || (locked_count && !locked) )
    return RES_BAD_ARG;

  /* Check the validity of the locked indices */
  for(i = 0; i < locked_count; ++i) {
    if( locked[i] >= smpls_clrs->as.any.count )
      return RES_BAD_ARG;
  }

  PSE_CALL_OR_RETURN(res,
    pseConstrainedParameterSpaceExplorationContextParamsGet
      (ctxt, &params));
  assert /* Ensure we have the right type */
    (  params.pspace.convert_user_data
    && (params.pspace.convert == pseColorPaletteParameterSpaceConvert) );

  /* Create values with the right number of colors by taking the variations into
   * account. */
  cp = (struct pse_color_palette_t*)params.pspace.convert_user_data;
  PSE_CALL_OR_GOTO(res,exit, pseColorPaletteValuesCreateFrom
    (cp, (pse_color_format_t)params.pspace.explore_in,
     smpls_clrs, locked_count, locked, &cpvals));
  smpls.values = cpvals->cps_values;

  /* Solve using the temporary samples values */
  PSE_TRY_CALL_OR_GOTO(res,exit, pseConstrainedParameterSpaceExplorationSolve
    (ctxt, &smpls));

exit:
  if( cpvals )
    PSE_CALL(pseColorPaletteValuesDestroy(cpvals));
  return res;
}

enum pse_res_t
pseColorPaletteExplorationResultsRetreive
  (struct pse_cpspace_exploration_ctxt_t* ctxt,
   struct pse_colors_t* optimized_clrs,
   struct pse_cpspace_exploration_extra_results_t* extra_results)
{
  enum pse_res_t res = RES_OK;
  struct pse_cpspace_exploration_ctxt_params_t params;
  struct pse_color_palette_t* cp = NULL;
  struct pse_color_palette_values_t* cpvals = NULL;
  if( !ctxt || !optimized_clrs )
    return RES_BAD_ARG;

  PSE_CALL_OR_RETURN(res,
    pseConstrainedParameterSpaceExplorationContextParamsGet
      (ctxt, &params));
  assert /* Ensure we have the right type */
    (  params.pspace.convert_user_data
    && (params.pspace.convert == pseColorPaletteParameterSpaceConvert) );

  /* Create a CPS values to retreive the results in the right format */
  cp = (struct pse_color_palette_t*)params.pspace.convert_user_data;
  PSE_CALL_OR_RETURN(res, pseColorPaletteValuesCreateMapping
    (cp, (pse_color_format_t)params.pspace.explore_in, optimized_clrs, &cpvals));

  /* This will call our setAttribs and then fill the colors associated to
   * optimized_vals */
  PSE_CALL_OR_GOTO(res,exit,
    pseConstrainedParameterSpaceExplorationLastResultsRetreive
      (ctxt, cpvals->cps_values, extra_results));

  /* As our values was a mapping to the optimized_clrs parameter, we have
   * already modified it. */

exit:
  if( cpvals )
    PSE_CALL(pseColorPaletteValuesDestroy(cpvals));
  return res;
}
