#include "pse_color_values_p.h"
#include "pse_color_palette_p.h"
#include "pse_color_conversion_p.h"

#include <pse.h>

#include <stb_ds.h>
#include <stretchy_buffer.h>

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

enum pse_values_access_right_t {
  PSE_VALUES_ACCESS_RIGHT_WRITE,
  PSE_VALUES_ACCESS_RIGHT_READ
};

/******************************************************************************
 *
 * HELPER FUNCTIONS
 *
 ******************************************************************************/

enum pse_res_t
pseColorPaletteValuesAttribGet
  (void* ctxt,
   const enum pse_point_attrib_t attrib,
   const enum pse_type_t as_type,
   const size_t count,
   const pse_ppoint_id_t* values_idx,
   void* attrib_values)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_palette_values_t* values =
    (struct pse_color_palette_values_t*)ctxt;
  size_t i;

  const pse_color_format_t values_format =
    PSE_COLOR_FORMAT_FROM(values->colors->space, values->colors->as.any.type);

  (void)as_type;
  switch(attrib) {
    case PSE_POINT_ATTRIB_COORDINATES: {
      pse_real_t* coords = (pse_real_t*)attrib_values;
      struct pse_colors_ref_t coords_ref = PSE_COLORS_REF_INVALID;
      assert(as_type == PSE_TYPE_REAL);
      PSE_CALL_OR_RETURN(res, pseColorsRefMapBuffer
        (&coords_ref, values->exchange_format, count, coords));
      assert(coords_ref.kind == PSE_COLOR_REF_KIND_RAW_AOS);

      /* TODO: we should try to do the conversion chunk by chunk, instead of
       * color per color. */
      for(i = 0; i < count; ++i) {
        const pse_ppoint_id_t ppid = values_idx[i];
        PSE_VERIFY_OR_ELSE(NULL != pseAnyCompsToAnyComps
          (values_format,
           &values->colors->as.any.comps[ppid],
           values->exchange_format,
           pseColorsRawAOSGetCompsAt(&coords_ref.as.raw_aos, i)),
           return RES_NOT_SUPPORTED);
      }
    } break;
    case PSE_POINT_ATTRIB_LOCK_STATUS: {
      bool* lock_status = (bool*)attrib_values;
      assert(as_type == PSE_TYPE_BOOL_8);
      for(i = 0; i < count; ++i) {
        const pse_ppoint_id_t ppid = values_idx[i];
        lock_status[i] = values->lock_status[ppid];
      }
    } break;
    default: assert(false); res = RES_NOT_SUPPORTED;
  }
  return res;
}

enum pse_res_t
pseColorPaletteValuesAttribSet
  (void* ctxt,
   const enum pse_point_attrib_t attrib,
   const enum pse_type_t as_type,
   const size_t count,
   const pse_ppoint_id_t* values_idx,
   const void* attrib_values)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_palette_values_t* values =
    (struct pse_color_palette_values_t*)ctxt;
  size_t i;

  const pse_color_format_t values_format =
    PSE_COLOR_FORMAT_FROM(values->colors->space, values->colors->as.any.type);

  (void)as_type;
  switch(attrib) {
    case PSE_POINT_ATTRIB_COORDINATES: {
      const pse_real_t* coords = (const pse_real_t*)attrib_values;
      struct pse_colors_ref_t coords_ref = PSE_COLORS_REF_INVALID;
      assert(as_type == PSE_TYPE_REAL);
      PSE_CALL_OR_RETURN(res, pseColorsRefMapBuffer
        (&coords_ref, values->exchange_format, count, (void*)coords));
      assert(coords_ref.kind == PSE_COLOR_REF_KIND_RAW_AOS);

      /* TODO: we should try to do the conversion chunk by chunk, instead of
       * color per color. */
      for(i = 0; i < count; ++i) {
        const pse_ppoint_id_t ppid = values_idx[i];
        PSE_VERIFY_OR_ELSE(NULL != pseAnyCompsToAnyComps
          (values->exchange_format,
           pseColorsRawAOSGetCompsAt(&coords_ref.as.raw_aos, i),
           values_format,
           &values->colors->as.any.comps[ppid]),
           return RES_NOT_SUPPORTED);
      }
    } break;
    default: assert(false); res = RES_NOT_SUPPORTED;
  }
  return res;
}

/* TODO: transform bool write_mode into enum for clarity */
static PSE_FINLINE enum pse_res_t
pseColorPaletteValuesInternalCreate
  (struct pse_color_palette_t* cp,
   const pse_color_format_t exchange_format,
   struct pse_colors_t* clrs,
   const size_t locked_count,
   const pse_color_idx_t* locked,
   struct pse_color_palette_values_t** out_cpvals)
{
  enum pse_res_t res = RES_OK;
  struct pse_cpspace_values_data_t data = PSE_CPSPACE_VALUES_DATA_NULL;
  struct pse_color_palette_values_t* cpvals = NULL;
  struct pse_cpspace_values_t* vals = NULL;
  size_t ppoints_count;
  size_t i;
  assert(cp && clrs && (!locked_count || locked) && out_cpvals);
  assert(exchange_format != PSE_COLOR_FORMAT_INVALID);

  cpvals = PSE_TYPED_ALLOC(cp->alloc, struct pse_color_palette_values_t);
  PSE_VERIFY_OR_ELSE(cpvals != NULL, res = RES_MEM_ERR; goto error);
  *cpvals = PSE_COLOR_PALETTE_VALUES_NULL;

  /* Allocate lock status buffer */
  ppoints_count = sb_count(cp->ppoints_id);
  cpvals->lock_status = PSE_TYPED_ALLOC_ARRAY(cp->alloc, bool, ppoints_count);
  PSE_VERIFY_OR_ELSE(cpvals->lock_status != NULL, res = RES_MEM_ERR; goto error);

  /* Initialize the lock status buffer */
  memset(cpvals->lock_status, 0, sizeof(bool)*ppoints_count);
  for(i = 0; i < locked_count; ++i) {
    cpvals->lock_status[locked[i]] = true;
  }

  /* In read mode, it's just a mapping to the given colors */
  cpvals->colors = clrs;
  cpvals->exchange_format = exchange_format;

  /* Create the associated CPS values */
  data.pspace = exchange_format;
  data.storage = PSE_CPSPACE_VALUES_STORAGE_ACCESSORS_GLOBAL;
  data.as.global.accessors.get = pseColorPaletteValuesAttribGet;
  data.as.global.accessors.set = pseColorPaletteValuesAttribSet;
  data.as.global.accessors.ctxt = cpvals;
  PSE_CALL_OR_GOTO(res,error, pseConstrainedParameterSpaceValuesCreate
    (cp->cps, &data, &vals));
  cpvals->cps_values = vals;
  cpvals->cp = cp;

  *out_cpvals = cpvals;

exit:
  return res;
error:
  PSE_FREE(cp->alloc, cpvals->lock_status);
  PSE_FREE(cp->alloc, cpvals);
  goto exit;
}


/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

enum pse_res_t
pseColorPaletteValuesCreateFrom
  (struct pse_color_palette_t* cp,
   const pse_color_format_t target_format,
   struct pse_colors_t* clrs,
   const size_t locked_count,
   const pse_color_idx_t* locked,
   struct pse_color_palette_values_t** out_cpvals)
{
  return pseColorPaletteValuesInternalCreate
    (cp, target_format, clrs, locked_count, locked, out_cpvals);
}

enum pse_res_t
pseColorPaletteValuesCreateMapping
  (struct pse_color_palette_t* cp,
   const pse_color_format_t received_format,
   struct pse_colors_t* clrs,
   struct pse_color_palette_values_t** out_cpvals)
{
  return pseColorPaletteValuesInternalCreate
    (cp, received_format, clrs, 0, NULL, out_cpvals);
}

enum pse_res_t
pseColorPaletteValuesCopy
  (struct pse_color_palette_values_t* vals,
   struct pse_colors_t* dst)
{
  enum pse_res_t res = RES_OK;
  struct pse_colors_t chunk = PSE_COLORS_INVALID;
  if( !vals || !dst )
    return RES_BAD_ARG;

  PSE_CALL_OR_RETURN(res, pseColorsChunkMap
    (vals->colors, 0, dst->as.any.count, &chunk));
  PSE_CALL_OR_RETURN(res, pseColorsConvert(&chunk, dst));
  return res;
}

enum pse_res_t
pseColorPaletteValuesDestroy
  (struct pse_color_palette_values_t* vals)
{
  if( !vals )
    return RES_BAD_ARG;
  PSE_CALL(pseConstrainedParameterSpaceValuesRefSub(vals->cps_values));
  PSE_FREE(vals->cp->alloc, vals->lock_status);
  PSE_FREE(vals->cp->alloc, vals);
  return RES_OK;
}
