#ifndef PSE_COLOR_H
#define PSE_COLOR_H

#include "pse_color_api.h"
#include "pse_color_space_Cat02_LMS.h"
#include "pse_color_space_HSV.h"
#include "pse_color_space_Lab.h"
#include "pse_color_space_RGB.h"
#include "pse_color_space_XYZ.h"

/*! \file
 * This file provide the structures and associated functions allowing the
 * manipulation of colors, in any format supported by the library.
 *
 * \todo A simplification pass must be done on all these types. We probably can
 *    do simpler and thus avoid some repeating functions doing the same things
 *    but for different types.
 */

PSE_API_BEGIN

struct pse_allocator_t;

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

struct pse_color_any_t {
  struct pse_color_any_components_t comps;
  pse_color_type_t type;
};

struct pse_color_t {
  union {
    struct pse_color_any_t any;
    struct pse_color_Cat02LMS_t Cat02LMS;
    struct pse_color_HSV_t HSV;
    struct pse_color_LAB_t Lab;
    struct pse_color_RGB_t RGB;
    struct pse_color_XYZ_t XYZ;
  } as;
  pse_color_space_t space;
};

struct pse_colors_any_t {
  struct pse_color_any_components_t* comps;
  size_t count;
  pse_color_type_t type;
};

struct pse_colors_t {
  union {
    struct pse_colors_any_t any;
    struct pse_colors_Cat02LMS_t Cat02LMS;
    struct pse_colors_HSV_t HSV;
    struct pse_colors_LAB_t Lab;
    struct pse_colors_RGB_t RGB;
    struct pse_colors_XYZ_t XYZ;
  } as;
  pse_color_space_t space;
};

enum pse_color_ref_kind_t {
  PSE_COLOR_REF_KIND_COLORS,    /* struct pse_colors_t */
  PSE_COLOR_REF_KIND_RAW_AOS    /* AOS = Array Of Strucutre */
};

/*! Colors stored as raw data using an Array Of Structure memory layout. The
 * \p format says how to interpret data pointed by \p data.
 * \param format Color format.
 * \param count Number of colors.
 * \param data Color data.
 * \param color_memsize Size in bytes of a color with all its components.
 */
struct pse_colors_raw_aos_t {
  pse_color_format_t format;
  size_t count;
  void* data;
  size_t color_memsize;
};

struct pse_colors_ref_t {
  enum pse_color_ref_kind_t kind;
  union {
    struct pse_colors_t colors;
    struct pse_colors_raw_aos_t raw_aos;
  } as;
};

/******************************************************************************
 *
 * STATIC CHECK TO ENSURE CONSISTENCY OF TYPES ON ANY PLATFORM
 *
 ******************************************************************************/

PSE_STATIC_ASSERT
  (sizeof(struct pse_color_any_t) >= sizeof(struct pse_color_Cat02LMS_t),
   Color_Any_type_do_not_allow_to_store_Cat02_LMS_color);
PSE_STATIC_ASSERT
  (sizeof(struct pse_color_any_t) >= sizeof(struct pse_color_LAB_t),
   Color_Any_type_do_not_allow_to_store_LAB_color);
PSE_STATIC_ASSERT
  (sizeof(struct pse_color_any_t) >= sizeof(struct pse_color_RGB_t),
   Color_Any_type_do_not_allow_to_store_RGB_color);
PSE_STATIC_ASSERT
  (sizeof(struct pse_color_any_t) >= sizeof(struct pse_color_XYZ_t),
   Color_Any_type_do_not_allow_to_store_XYZ_color);

PSE_STATIC_ASSERT
  (offsetof(struct pse_color_any_t, type) == offsetof(struct pse_color_Cat02LMS_t, type),
   Inconsistency_between_types_Color_Any_and_Color_Cat02_LMS);
PSE_STATIC_ASSERT
  (offsetof(struct pse_color_any_t, comps) == offsetof(struct pse_color_Cat02LMS_t, as),
   Inconsistency_between_types_Color_Any_and_Color_Cat02_LMS);
PSE_STATIC_ASSERT
  (offsetof(struct pse_color_any_t, type) == offsetof(struct pse_color_HSV_t, type),
   Inconsistency_between_types_Color_Any_and_Color_HSV);
PSE_STATIC_ASSERT
  (offsetof(struct pse_color_any_t, comps) == offsetof(struct pse_color_HSV_t, as),
   Inconsistency_between_types_Color_Any_and_Color_HSV);
PSE_STATIC_ASSERT
  (offsetof(struct pse_color_any_t, type) == offsetof(struct pse_color_LAB_t, type),
   Inconsistency_between_types_Color_Any_and_Color_LAB);
PSE_STATIC_ASSERT
  (offsetof(struct pse_color_any_t, comps) == offsetof(struct pse_color_LAB_t, as),
   Inconsistency_between_types_Color_Any_and_Color_LAB);
PSE_STATIC_ASSERT
  (offsetof(struct pse_color_any_t, type) == offsetof(struct pse_color_RGB_t, type),
   Inconsistency_between_types_Color_Any_and_Color_RGB);
PSE_STATIC_ASSERT
  (offsetof(struct pse_color_any_t, comps) == offsetof(struct pse_color_RGB_t, as),
   Inconsistency_between_types_Color_Any_and_Color_RGB);
PSE_STATIC_ASSERT
  (offsetof(struct pse_color_any_t, type) == offsetof(struct pse_color_XYZ_t, type),
   Inconsistency_between_types_Color_Any_and_Color_XYZ);
PSE_STATIC_ASSERT
  (offsetof(struct pse_color_any_t, comps) == offsetof(struct pse_color_XYZ_t, as),
   Inconsistency_between_types_Color_Any_and_Color_XYZ);

PSE_STATIC_ASSERT
  (offsetof(struct pse_colors_any_t, type) == offsetof(struct pse_colors_Cat02LMS_t, type),
   Inconsistency_between_types_Color_Any_and_Color_Cat02_LMS);
PSE_STATIC_ASSERT
  (offsetof(struct pse_colors_any_t, count) == offsetof(struct pse_colors_Cat02LMS_t, count),
   Inconsistency_between_types_Color_Any_and_Color_Cat02_LMS);
PSE_STATIC_ASSERT
  (offsetof(struct pse_colors_any_t, comps) == offsetof(struct pse_colors_Cat02LMS_t, as),
   Inconsistency_between_types_Color_Any_and_Color_Cat02_LMS);
PSE_STATIC_ASSERT
  (offsetof(struct pse_colors_any_t, type) == offsetof(struct pse_colors_HSV_t, type),
   Inconsistency_between_types_Color_Any_and_Color_HSV);
PSE_STATIC_ASSERT
  (offsetof(struct pse_colors_any_t, count) == offsetof(struct pse_colors_HSV_t, count),
   Inconsistency_between_types_Color_Any_and_Color_HSV);
PSE_STATIC_ASSERT
  (offsetof(struct pse_colors_any_t, comps) == offsetof(struct pse_colors_HSV_t, as),
   Inconsistency_between_types_Color_Any_and_Color_HSV);
PSE_STATIC_ASSERT
  (offsetof(struct pse_colors_any_t, type) == offsetof(struct pse_colors_LAB_t, type),
   Inconsistency_between_types_Color_Any_and_Color_LAB);
PSE_STATIC_ASSERT
  (offsetof(struct pse_colors_any_t, count) == offsetof(struct pse_colors_LAB_t, count),
   Inconsistency_between_types_Color_Any_and_Color_LAB);
PSE_STATIC_ASSERT
  (offsetof(struct pse_colors_any_t, comps) == offsetof(struct pse_colors_LAB_t, as),
   Inconsistency_between_types_Color_Any_and_Color_LAB);
PSE_STATIC_ASSERT
  (offsetof(struct pse_colors_any_t, type) == offsetof(struct pse_colors_RGB_t, type),
   Inconsistency_between_types_Color_Any_and_Color_RGB);
PSE_STATIC_ASSERT
  (offsetof(struct pse_colors_any_t, count) == offsetof(struct pse_colors_RGB_t, count),
   Inconsistency_between_types_Color_Any_and_Color_RGB);
PSE_STATIC_ASSERT
  (offsetof(struct pse_colors_any_t, comps) == offsetof(struct pse_colors_RGB_t, as),
   Inconsistency_between_types_Color_Any_and_Color_RGB);
PSE_STATIC_ASSERT
  (offsetof(struct pse_colors_any_t, type) == offsetof(struct pse_colors_XYZ_t, type),
   Inconsistency_between_types_Color_Any_and_Color_XYZ);
PSE_STATIC_ASSERT
  (offsetof(struct pse_colors_any_t, count) == offsetof(struct pse_colors_XYZ_t, count),
   Inconsistency_between_types_Color_Any_and_Color_XYZ);
PSE_STATIC_ASSERT
  (offsetof(struct pse_colors_any_t, comps) == offsetof(struct pse_colors_XYZ_t, as),
   Inconsistency_between_types_Color_Any_and_Color_XYZ);

/******************************************************************************
 *
 * PUBLIC CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_ANY_INVALID_                                                 \
  { PSE_COLOR_ANY_COMPONENTS_ZERO_, PSE_COLOR_TYPE_INVALID_ }
#define PSE_COLORS_ANY_INVALID_                                                \
  { NULL, 0, PSE_COLOR_TYPE_INVALID_ }
#define PSE_COLOR_INVALID_                                                     \
  { { PSE_COLOR_ANY_INVALID_ }, PSE_COLOR_SPACE_INVALID_ }
#define PSE_COLORS_INVALID_                                                    \
  { { PSE_COLORS_ANY_INVALID_ }, PSE_COLOR_SPACE_INVALID_ }
#define PSE_COLORS_RAW_AOS_INVALID_                                            \
  { PSE_COLOR_FORMAT_INVALID_, 0, NULL, 0 }
#define PSE_COLORS_REF_INVALID_                                                \
  { PSE_COLOR_REF_KIND_COLORS, { PSE_COLORS_INVALID_ } }

static const struct pse_color_any_t PSE_COLOR_ANY_INVALID =
  PSE_COLOR_ANY_INVALID_;
static const struct pse_colors_any_t PSE_COLORS_ANY_INVALID =
  PSE_COLORS_ANY_INVALID_;
static const struct pse_color_t PSE_COLOR_INVALID =
  PSE_COLOR_INVALID_;
static const struct pse_colors_t PSE_COLORS_INVALID =
  PSE_COLORS_INVALID_;
static const struct pse_colors_raw_aos_t PSE_COLORS_RAW_AOS_INVALID =
  PSE_COLORS_RAW_AOS_INVALID_;
static const struct pse_colors_ref_t PSE_COLORS_REF_INVALID =
  PSE_COLORS_REF_INVALID_;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_COLOR_INLINE_API size_t
pseColorFormatMemSize
  (const pse_color_format_t fmt)
{
  switch(fmt) {
    case PSE_COLOR_FORMAT_Cat02LMSr_:
    case PSE_COLOR_FORMAT_HSVr_:
    case PSE_COLOR_FORMAT_LABr_:
    case PSE_COLOR_FORMAT_RGBr_:
    case PSE_COLOR_FORMAT_XYZr_:      return sizeof(pse_real_t)*3;
    case PSE_COLOR_FORMAT_Lui8ABi8_:  return sizeof(uint8_t)+sizeof(int8_t)*2;
    case PSE_COLOR_FORMAT_RGBui8_:    return sizeof(uint8_t)*3;
    default: assert(false);
  }
  return 0;
}

PSE_COLOR_INLINE_API pse_color_format_t
pseColorFormatGet
  (const struct pse_color_t* clr)
{
  assert(clr);
  return PSE_COLOR_FORMAT_FROM(clr->space, clr->as.any.type);
}

/*! Allocate/reallocate memory that allows to store \p colors_count colors of
 * any format. */
PSE_COLOR_API enum pse_res_t
pseColorsAllocate
  (struct pse_allocator_t* alloc,
   const pse_color_format_t fmt,
   const size_t colors_count,
   struct pse_colors_t* clrs);

PSE_COLOR_API enum pse_res_t
pseColorsFree
  (struct pse_allocator_t* alloc,
   struct pse_colors_t* clrs);

PSE_COLOR_INLINE_API pse_color_format_t
pseColorsFormatGet
  (const struct pse_colors_t* clrs)
{
  assert(clrs);
  return PSE_COLOR_FORMAT_FROM(clrs->space, clrs->as.any.type);
}

/*! Map a chunk of \p src colors.
 * \param[in] src Source colors to be mapped.
 * \param[in] start Start color index in \p src.
 * \param[in] count Number of colors of \p src to be mapped, starting at index
 *    \p start.
 * \param[out] dst_map Mapping of \p src, providing access to \p count colors,
 *    obviously in the same format than \p src.
 *
 * \warning The resulting colors object is only a memory mapping of the
 *    provided \p src colors! Any change to \p src may break the mapping!
 */
PSE_COLOR_API enum pse_res_t
pseColorsChunkMap
  (const struct pse_colors_t* src,
   const pse_color_idx_t start,
   const size_t count,
   struct pse_colors_t* dst_map);

/*! Convert \p src colors to \p dst colors. \p dst format is used to know
 * the output format.
 *
 * \warning \p src and \p dst must have the same number of colors.
 */
PSE_COLOR_API enum pse_res_t
pseColorsConvert
  (const struct pse_colors_t* src,
   struct pse_colors_t* dst);

PSE_COLOR_API enum pse_res_t
pseColorsConvertInPlace
  (struct pse_colors_t* clrs,
   const pse_color_format_t to);

PSE_COLOR_API enum pse_res_t
pseColorsSetAt
  (struct pse_colors_t* clrs,
   const pse_color_idx_t idx,
   const struct pse_color_t* clr);

/*! Extract the color of \p clrs at index \p idx, in the format of \p clr. */
PSE_COLOR_API enum pse_res_t
pseColorsExtractAt
  (const struct pse_colors_t* clrs,
   const pse_color_idx_t idx,
   struct pse_color_t* clr);

PSE_COLOR_INLINE_API pse_color_format_t
pseColorsRawAOSFormatGet
  (const struct pse_colors_raw_aos_t* raw)
{
  assert(raw);
  return raw->format;
}

/*! Return the pointer on the color of \p raw at index \p idx. */
PSE_COLOR_INLINE_API void*
pseColorsRawAOSGetPtrAt
  (const struct pse_colors_raw_aos_t* raw,
   const pse_color_idx_t idx)
{
  assert(raw && (idx <= raw->count));
  return (void*)((uintptr_t)raw->data + idx*raw->color_memsize);
}

PSE_COLOR_INLINE_API struct pse_color_any_components_t*
pseColorsRawAOSGetCompsAt
  (const struct pse_colors_raw_aos_t* raw,
   const pse_color_idx_t idx)
{
  return (struct pse_color_any_components_t*)pseColorsRawAOSGetPtrAt(raw, idx);
}

/*! Extract the color of \p raw at index \p idx, in the format of \p clr. */
PSE_COLOR_API enum pse_res_t
pseColorsRawAOSExtractAt
  (const struct pse_colors_raw_aos_t* raw,
   const pse_color_idx_t idx,
   struct pse_color_t* clr);

PSE_COLOR_API enum pse_res_t
pseColorsRawAOSSetAt
  (struct pse_colors_raw_aos_t* raw,
   const pse_color_idx_t idx,
   const struct pse_color_t* clr);

/*! Fill the colors reference \p ref from a memory buffer \p values containing
 * \p count colors in the format \p fmt.
 */
PSE_COLOR_INLINE_API enum pse_res_t
pseColorsRefMapBuffer
  (struct pse_colors_ref_t* ref,
   const pse_color_format_t fmt,
   const size_t count,
   void* values)
{
  enum pse_res_t res = RES_OK;
  assert(ref && (count > 0) && values);
  ref->kind = PSE_COLOR_REF_KIND_RAW_AOS;
  ref->as.raw_aos = PSE_COLORS_RAW_AOS_INVALID;
  ref->as.raw_aos.format = fmt;
  ref->as.raw_aos.count = count;
  ref->as.raw_aos.data = values;
  ref->as.raw_aos.color_memsize = pseColorFormatMemSize(fmt);
  return res;
}

/*! Fill the colors reference \p ref from colors \p clrs.
 * \warning It's a memory mapping, so any change to \p clrs may break the colors
 *    reference \p ref.
 */
PSE_COLOR_INLINE_API enum pse_res_t
pseColorsRefMapColors
  (struct pse_colors_ref_t* ref,
   struct pse_colors_t* clrs)
{
  enum pse_res_t res = RES_OK;
  assert(ref && clrs && (clrs->as.any.count > 0));
  ref->kind = PSE_COLOR_REF_KIND_RAW_AOS;
  ref->as.raw_aos = PSE_COLORS_RAW_AOS_INVALID;
  ref->as.raw_aos.format = pseColorsFormatGet(clrs);
  ref->as.raw_aos.count = clrs->as.any.count;
  ref->as.raw_aos.data = (void*)clrs->as.any.comps;
  ref->as.raw_aos.color_memsize = sizeof(struct pse_color_any_components_t);
  return res;
}

PSE_COLOR_INLINE_API pse_color_format_t
pseColorsRefFormatGet
  (const struct pse_colors_ref_t* ref)
{
  assert(ref);
  switch(ref->kind) {
    case PSE_COLOR_REF_KIND_COLORS:
      return pseColorsFormatGet(&ref->as.colors);
    case PSE_COLOR_REF_KIND_RAW_AOS:
      return pseColorsRawAOSFormatGet(&ref->as.raw_aos);
    default: assert(false);
  }
  return PSE_COLOR_FORMAT_INVALID;
}

PSE_COLOR_INLINE_API size_t
pseColorsRefCountGet
  (const struct pse_colors_ref_t* ref)
{
  assert(ref);
  switch(ref->kind) {
    case PSE_COLOR_REF_KIND_COLORS: return ref->as.colors.as.any.count;
    case PSE_COLOR_REF_KIND_RAW_AOS: return ref->as.raw_aos.count;
    default: assert(false);
  }
  return 0;
}

PSE_COLOR_INLINE_API enum pse_res_t
pseColorsRefExtractAt
  (const struct pse_colors_ref_t* ref,
   const pse_color_idx_t idx,
   struct pse_color_t* clr)
{
  assert(ref);
  switch(ref->kind) {
    case PSE_COLOR_REF_KIND_COLORS:
      return pseColorsExtractAt(&ref->as.colors, idx, clr);
    case PSE_COLOR_REF_KIND_RAW_AOS:
      return pseColorsRawAOSExtractAt(&ref->as.raw_aos, idx, clr);
    default: assert(false);
  }
  return RES_BAD_ARG;
}

PSE_COLOR_INLINE_API enum pse_res_t
pseColorsRefSetAt
  (struct pse_colors_ref_t* ref,
   const pse_color_idx_t idx,
   const struct pse_color_t* clr)
{
  assert(ref);
  switch(ref->kind) {
    case PSE_COLOR_REF_KIND_COLORS:
      return pseColorsSetAt(&ref->as.colors, idx, clr);
    case PSE_COLOR_REF_KIND_RAW_AOS:
      return pseColorsRawAOSSetAt(&ref->as.raw_aos, idx, clr);
    default: assert(false);
  }
  return RES_BAD_ARG;
}

/*! Convert \p src colors to \p dst colors. \p dst space is used to know the
 * output format.
 */
PSE_COLOR_API enum pse_res_t
pseColorConvert
  (const struct pse_color_t* src,
   struct pse_color_t* dst);

PSE_COLOR_API enum pse_res_t
pseColorConvertInPlace
  (struct pse_color_t* clr,
   const pse_color_format_t to);

PSE_COLOR_INLINE_API void
pseColorFormatSet
  (struct pse_color_t* clr,
   const pse_color_format_t fmt)
{
  assert(clr);
  clr->space = PSE_COLOR_SPACE_FROM(fmt);
  clr->as.any.type = PSE_COLOR_TYPE_FROM(fmt);
}

PSE_API_END

#endif /* PSE_COLOR_H */
