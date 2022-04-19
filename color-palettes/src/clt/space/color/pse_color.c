#include "pse_color.h"
#include "pse_color_conversion_p.h"

#include <pse_allocator.h>

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

enum pse_res_t
pseColorsAllocate
  (struct pse_allocator_t* alloc,
   const pse_color_format_t fmt,
   const size_t colors_count,
   struct pse_colors_t* colors)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_any_components_t* buffer = NULL;
  if( !alloc || !colors || (colors_count <= 0) )
    return RES_BAD_ARG;

  /* Ensure everything is correct */
  assert(sizeof(struct pse_color_any_components_t) >= pseColorFormatMemSize(fmt));

  if( colors->as.any.count > 0 ) {
    buffer = (struct pse_color_any_components_t*)PSE_REALLOC
      (alloc, colors->as.any.comps,
       colors_count * sizeof(struct pse_color_any_components_t));
  } else {
    buffer = (struct pse_color_any_components_t*)PSE_ALLOC
      (alloc,
       colors_count * sizeof(struct pse_color_any_t));
  }
  PSE_VERIFY_OR_ELSE(buffer != NULL, res = RES_MEM_ERR; goto error);
  colors->space = PSE_COLOR_SPACE_FROM(fmt);
  colors->as.any.type = PSE_COLOR_TYPE_FROM(fmt);
  colors->as.any.comps = buffer;
  colors->as.any.count = colors_count;

exit:
  return res;
error:
  PSE_FREE(alloc, buffer);
  goto exit;
}

enum pse_res_t
pseColorsFree
  (struct pse_allocator_t* alloc,
   struct pse_colors_t* colors)
{
  if( !alloc || !colors )
    return RES_BAD_ARG;
  PSE_FREE(alloc, colors->as.any.comps);
  colors->as.any.count = 0;
  return RES_OK;
}

enum pse_res_t
pseColorsChunkMap
  (const struct pse_colors_t* src,
   const pse_color_idx_t start,
   const size_t count,
   struct pse_colors_t* dst_map)
{
  if( !src || !dst_map || !count )
    return RES_BAD_ARG;
  if(  (start >= src->as.any.count)
    || ((start+count) > src->as.any.count) )
    return RES_BAD_ARG;

  dst_map->space = src->space;
  dst_map->as.any.comps = &src->as.any.comps[start];
  dst_map->as.any.count = count;
  dst_map->as.any.type = src->as.any.type;
  return RES_OK;
}

enum pse_res_t
pseColorsConvert
  (const struct pse_colors_t* src,
   struct pse_colors_t* dst)
{
  if( !src || !dst )
    return RES_BAD_ARG;
  if( src->as.any.count != dst->as.any.count )
    return RES_BAD_ARG;
  return pseNAnyToNAny(src, pseColorsFormatGet(dst), dst)
    ? RES_OK : RES_NOT_SUPPORTED;
}

enum pse_res_t
pseColorsConvertInPlace
  (struct pse_colors_t* clrs,
   const pse_color_format_t to)
{
  if( !clrs )
    return RES_BAD_ARG;
  if( pseColorsFormatGet(clrs) != to )
    return pseNAnyToNAny(clrs, to, clrs)
      ? RES_OK : RES_NOT_SUPPORTED;
  return RES_OK;
}

enum pse_res_t
pseColorsSetAt
  (struct pse_colors_t* clrs,
   const size_t idx,
   const struct pse_color_t* clr)
{
  if( !clrs || !clr || (idx >= clrs->as.any.count) )
    return RES_BAD_ARG;
  return pseAnyCompsToAnyComps
    (pseColorFormatGet(clr), &clr->as.any.comps,
     pseColorsFormatGet(clrs), &clrs->as.any.comps[idx])
    ? RES_OK : RES_NOT_SUPPORTED;
}

enum pse_res_t
pseColorsExtractAt
  (const struct pse_colors_t* clrs,
   const size_t idx,
   struct pse_color_t* clr)
{
  if( !clrs || !clr || (idx >= clrs->as.any.count) )
    return RES_BAD_ARG;
  return pseAnyCompsToAnyComps
    (pseColorsFormatGet(clrs), &clrs->as.any.comps[idx],
     pseColorFormatGet(clr), &clr->as.any.comps)
    ? RES_OK : RES_NOT_SUPPORTED;
}

enum pse_res_t
pseColorsRawAOSExtractAt
  (const struct pse_colors_raw_aos_t* raw,
   const pse_color_idx_t idx,
   struct pse_color_t* clr)
{
  struct pse_color_any_components_t* raw_clr = NULL;
  if( !raw || !clr || (idx >= raw->count) )
    return RES_BAD_ARG;

  raw_clr = pseColorsRawAOSGetPtrAt(raw, idx);
  PSE_VERIFY_OR_ELSE(raw_clr != NULL, return RES_NOT_SUPPORTED);

  return pseAnyCompsToAnyComps
    (raw->format, raw_clr, pseColorFormatGet(clr), &clr->as.any.comps)
    ? RES_OK : RES_NOT_SUPPORTED;
}

enum pse_res_t
pseColorsRawAOSSetAt
  (struct pse_colors_raw_aos_t* raw,
   const pse_color_idx_t idx,
   const struct pse_color_t* clr)
{
  struct pse_color_any_components_t* raw_clr = NULL;
  if( !raw || !clr || (idx >= raw->count) )
    return RES_BAD_ARG;

  raw_clr = pseColorsRawAOSGetPtrAt(raw, idx);
  PSE_VERIFY_OR_ELSE(raw_clr != NULL, return RES_NOT_SUPPORTED);

  return pseAnyCompsToAnyComps
    (pseColorFormatGet(clr), &clr->as.any.comps, raw->format, raw_clr)
    ? RES_OK : RES_NOT_SUPPORTED;
}

enum pse_res_t
pseColorConvert
  (const struct pse_color_t* src,
   struct pse_color_t* dst)
{
  if( !src || !dst )
    return RES_BAD_ARG;
  return pseAnyToAny(src, pseColorFormatGet(dst), dst)
    ? RES_OK : RES_NOT_SUPPORTED;
}

enum pse_res_t
pseColorConvertInPlace
  (struct pse_color_t* clr,
   const pse_color_format_t to)
{
  if( !clr )
    return RES_BAD_ARG;
  if( pseColorFormatGet(clr) != to )
    return pseAnyToAny(clr, to, clr)
      ? RES_OK : RES_NOT_SUPPORTED;
  return RES_OK;
}
