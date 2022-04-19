#ifndef PSE_COLOR_SPACE_RGB_H
#define PSE_COLOR_SPACE_RGB_H

#include "pse_color_api.h"
#include "pse_color_types.h"

PSE_API_BEGIN

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

#define PSE_COLOR_SPACE_RGB  ((pse_color_space_t)0x0002)

#define PSE_COLOR_SPACE_RGB_TYPES_ALL                                          \
  PSE_COLOR_TYPE(RGBr)                                                         \
  PSE_COLOR_TYPE(RGBui8)

enum pse_color_type_RGB_t {
#define PSE_COLOR_TYPE(Name)  PSE_COLOR_TYPE_##Name,
  PSE_COLOR_SPACE_RGB_TYPES_ALL
#undef PSE_COLOR_TYPE
  PSE_COLOR_TYPE_RGB_COUNT_
};

struct pse_color_RGBr_t {
  pse_real_t R, G, B;  /*!< In [0,1] */
};
struct pse_color_RGBui8_t {
  uint8_t R, G, B;  /*!< In [0,255] */
};

struct pse_color_RGB_t {
  union {
  #define PSE_COLOR_TYPE(Name)  struct pse_color_##Name##_t Name;
    PSE_COLOR_SPACE_RGB_TYPES_ALL
  #undef PSE_COLOR_TYPE
    struct pse_color_any_components_t comps;
  } as;
  pse_color_type_t type;
};
struct pse_colors_RGB_t {
  union {
  #define PSE_COLOR_TYPE(Name)  struct pse_color_##Name##_t* Name;
    PSE_COLOR_SPACE_RGB_TYPES_ALL
  #undef PSE_COLOR_TYPE
    struct pse_color_any_components_t* comps;
  } as;
  size_t count;
  pse_color_type_t type;
};

/******************************************************************************
 *
 * STATIC CHECK TO ENSURE CONSISTENCY OF MEMORY MAPPING OF COLORS
 *
 ******************************************************************************/

PSE_STATIC_ASSERT
  (sizeof(struct pse_color_any_components_t) >= sizeof(struct pse_color_RGBr_t),
   Color_Any_Components_type_do_not_allow_to_store_RGBr_color_components);
PSE_STATIC_ASSERT
  (sizeof(struct pse_color_any_components_t) >= sizeof(struct pse_color_RGBui8_t),
   Color_Any_Components_type_do_not_allow_to_store_RGBui8_color_components);

/******************************************************************************
 *
 * PUBLIC CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_RGBr_BLACK_                                                  \
  { 0, 0, 0 }
#define PSE_COLOR_RGBui8_BLACK_                                                \
  { 0, 0, 0 }
#define PSE_COLOR_RGB_BLACKr_                                                   \
  { { PSE_COLOR_RGBr_BLACK_ }, PSE_COLOR_TYPE_RGBr }
#define PSE_COLORS_RGB_INVALID_                                                \
  { { NULL }, 0, PSE_COLOR_TYPE_INVALID_ }
#define PSE_COLOR_FORMAT_RGBr_                                                 \
  PSE_COLOR_FORMAT_FROM(PSE_COLOR_SPACE_RGB, PSE_COLOR_TYPE_RGBr)
#define PSE_COLOR_FORMAT_RGBui8_                                               \
  PSE_COLOR_FORMAT_FROM(PSE_COLOR_SPACE_RGB, PSE_COLOR_TYPE_RGBui8)

static const struct pse_color_RGBr_t PSE_COLOR_RGBr_BLACK =
  PSE_COLOR_RGBr_BLACK_;
static const struct pse_color_RGBui8_t PSE_COLOR_RGBui8_BLACK =
  PSE_COLOR_RGBui8_BLACK_;
static const struct pse_color_RGB_t PSE_COLOR_RGB_BLACKr =
  PSE_COLOR_RGB_BLACKr_;
static const struct pse_colors_RGB_t PSE_COLORS_RGB_INVALID =
  PSE_COLORS_RGB_INVALID_;
static const pse_color_format_t PSE_COLOR_FORMAT_RGBr =
  PSE_COLOR_FORMAT_RGBr_;
static const pse_color_format_t PSE_COLOR_FORMAT_RGBui8 =
  PSE_COLOR_FORMAT_RGBui8_;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_COLOR_INLINE_API bool
pseColorRGBIsInGamut
  (struct pse_color_RGB_t* clr)
{
  assert(clr);
  return (clr->type == PSE_COLOR_TYPE_RGBui8)
    || ( PSE_IS_IN(clr->as.RGBr.R, (pse_real_t)0, (pse_real_t)1)
      && PSE_IS_IN(clr->as.RGBr.G, (pse_real_t)0, (pse_real_t)1)
      && PSE_IS_IN(clr->as.RGBr.B, (pse_real_t)0, (pse_real_t)1) );
}

PSE_COLOR_API void
pseColorRGBInGamutClampInPlace
  (struct pse_color_RGB_t* clr);

PSE_COLOR_API void
pseColorsRGBInGamutClampInPlace
  (struct pse_colors_RGB_t* clrs);

PSE_COLOR_INLINE_API pse_real_t
pseColorRGBDistanceSquaredFromGamut
  (struct pse_color_RGB_t* clr)
{
  struct pse_color_RGB_t tmp;
  assert(clr);
  if( clr->type == PSE_COLOR_TYPE_RGBui8 )
    return (pse_real_t)0;
  if( pseColorRGBIsInGamut(clr) )
    return (pse_real_t)0;
  /* Return the Squared norm of the clr-clamp(clr) vector */
  tmp = *clr;
  pseColorRGBInGamutClampInPlace(&tmp);
  tmp.as.RGBr.R = clr->as.RGBr.R - tmp.as.RGBr.R;
  tmp.as.RGBr.G = clr->as.RGBr.G - tmp.as.RGBr.G;
  tmp.as.RGBr.B = clr->as.RGBr.B - tmp.as.RGBr.B;
  return
      PSE_REAL_POW2(tmp.as.RGBr.R)
    + PSE_REAL_POW2(tmp.as.RGBr.G)
    + PSE_REAL_POW2(tmp.as.RGBr.B);
}

PSE_COLOR_INLINE_API pse_real_t
pseColorRGBDistanceFromGamut
  (struct pse_color_RGB_t* clr)
{
  return PSE_REAL_SQRT(pseColorRGBDistanceFromGamut(clr));
}

PSE_API_END

#endif /* PSE_COLOR_SPACE_RGB_H */
