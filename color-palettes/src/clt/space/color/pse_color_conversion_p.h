#ifndef PSE_COLOR_CONVERSION_P_H
#define PSE_COLOR_CONVERSION_P_H

#include "pse_color.h"

/* TODO: remove the use of this library to do it our-self and avoid this
 * dependency that could be a problem regarding the license. */
#include <ColorSpace/colorspace.h>

PSE_STATIC_ASSERT(sizeof(num) == sizeof(pse_real_t), Incompatible_types);

#if 0
#undef PSE_INLINE
#undef PSE_FINLINE
#define PSE_INLINE
#define PSE_FINLINE
#endif

PSE_API_BEGIN

/******************************************************************************
 *
 * HELPER FUNCTIONS
 *
 ******************************************************************************/

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

#define PSE_COLOR_HSL         ((pse_color_space_t)0x0005)

enum pse_color_type_HSL_t {
  PSE_COLOR_TYPE_HSLr
};

/*! \ref H represents the parametric position in [0,1] on the circle. To get it
 * in degrees, just multiply it by 360°.
 */
struct pse_color_HSLr_t {
  pse_real_t H, S, L; /*!< In [0,1] */
};

#define PSE_COLOR_TYPES_ALL                                                    \
  PSE_COLOR_SPACE_Cat02LMS_TYPES_ALL                                           \
  PSE_COLOR_SPACE_HSV_TYPES_ALL                                                \
  PSE_COLOR_SPACE_LAB_TYPES_ALL                                                \
  PSE_COLOR_SPACE_RGB_TYPES_ALL                                                \
  PSE_COLOR_SPACE_XYZ_TYPES_ALL

/******************************************************************************
 *
 * PRIVATE CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_FORMAT_HSLr_                                                 \
  PSE_COLOR_FORMAT_FROM(PSE_COLOR_HSL, PSE_COLOR_TYPE_HSLr)

static const pse_color_format_t PSE_COLOR_FORMAT_HSLr =
  PSE_COLOR_FORMAT_HSLr_;

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

/******************************************************************************
 * Cat02LMS=>Cat02LMS
 ******************************************************************************/

static PSE_FINLINE struct pse_color_Cat02LMSr_t*
pseCat02LMSrToCat02LMSr
  (const struct pse_color_Cat02LMSr_t* src,
   struct pse_color_Cat02LMSr_t* dst)
{
  assert(src && dst);
  *dst = *src;
  return dst;
}

/******************************************************************************
 * HSV=>HSV
 ******************************************************************************/

static PSE_FINLINE struct pse_color_HSVr_t*
pseHSVrToHSVr
  (const struct pse_color_HSVr_t* src,
   struct pse_color_HSVr_t* dst)
{
  assert(src && dst);
  *dst = *src;
  return dst;
}

/******************************************************************************
 * XYZ=>XYZ
 ******************************************************************************/

static PSE_FINLINE struct pse_color_XYZr_t*
pseXYZrToXYZr
  (const struct pse_color_XYZr_t* src,
   struct pse_color_XYZr_t* dst)
{
  assert(src && dst);
  *dst = *src;
  return dst;
}

/******************************************************************************
 * LAB=>LAB
 ******************************************************************************/

static PSE_FINLINE struct pse_color_Lui8ABi8_t*
pseLABrToLui8ABi8
  (const struct pse_color_LABr_t* src,
   struct pse_color_Lui8ABi8_t* dst)
{
  /* TODO: force the rounding mode? */
  const int32_t L = (int32_t)(src->L * 100.0);
  const int32_t a = (int32_t)(src->a * 255.0 - 128.0);
  const int32_t b = (int32_t)(src->b * 255.0 - 128.0);
  /* TODO: Right now, we force the gamut as we cannot store the values outside
   * it. Is it a good way to do this or do we want to be informed? */
  dst->L = (uint8_t)PSE_CLAMP(L,    0, 100);
  dst->a =  (int8_t)PSE_CLAMP(a, -128, 127);
  dst->b =  (int8_t)PSE_CLAMP(b, -128, 127);
  return dst;
}

static PSE_FINLINE struct pse_color_LABr_t*
pseLui8ABi8ToLABr
  (const struct pse_color_Lui8ABi8_t* src,
   struct pse_color_LABr_t* dst)
{
  dst->L = (pse_real_t)src->L / 100.0;
  dst->a = ((pse_real_t)src->a + 128.0) / 255.0;
  dst->b = ((pse_real_t)src->b + 128.0) / 255.0;
  return dst;
}

static PSE_FINLINE struct pse_color_LABr_t*
pseLABrToLABr
  (const struct pse_color_LABr_t* src,
   struct pse_color_LABr_t* dst)
{
  *dst = *src;
  return dst;
}

static PSE_FINLINE struct pse_color_Lui8ABi8_t*
pseLui8ABi8ToLui8ABi8
  (const struct pse_color_Lui8ABi8_t* src,
   struct pse_color_Lui8ABi8_t* dst)
{
  *dst = *src;
  return dst;
}

/******************************************************************************
 * RGB=>RGB
 ******************************************************************************/

static PSE_FINLINE struct pse_color_RGBui8_t*
pseRGBrToRGBui8
  (const struct pse_color_RGBr_t* src,
   struct pse_color_RGBui8_t* dst)
{
  /* TODO: force the rounding mode? */
  const int32_t R = (int32_t)(src->R * 255.0);
  const int32_t G = (int32_t)(src->G * 255.0);
  const int32_t B = (int32_t)(src->B * 255.0);
  /* TODO: Right now, we force the gamut as we cannot store the values outside
   * it. Is it a good way to do this or do we want to be informed? */
  dst->R = (uint8_t)PSE_CLAMP(R, 0, 255);
  dst->G = (uint8_t)PSE_CLAMP(G, 0, 255);
  dst->B = (uint8_t)PSE_CLAMP(B, 0, 255);
  return dst;
}

static PSE_FINLINE struct pse_color_RGBr_t*
pseRGBui8ToRGBr
  (const struct pse_color_RGBui8_t* src,
   struct pse_color_RGBr_t* dst)
{
  dst->R = (pse_real_t)src->R / 255.0;
  dst->G = (pse_real_t)src->G / 255.0;
  dst->B = (pse_real_t)src->B / 255.0;
  return dst;
}

static PSE_FINLINE struct pse_color_RGBr_t*
pseRGBrToRGBr
  (const struct pse_color_RGBr_t* src,
   struct pse_color_RGBr_t* dst)
{
  *dst = *src;
  return dst;
}

static PSE_FINLINE struct pse_color_RGBui8_t*
pseRGBui8ToRGBui8
  (const struct pse_color_RGBui8_t* src,
   struct pse_color_RGBui8_t* dst)
{
  *dst = *src;
  return dst;
}

/******************************************************************************
 * HSV=>RGB
 ******************************************************************************/

static PSE_FINLINE struct pse_color_RGBr_t*
pseHSVrToRGBr
  (const struct pse_color_HSVr_t* src,
   struct pse_color_RGBr_t* dst)
{
  Hsv2Rgb(&dst->R,&dst->G,&dst->B, src->H*(pse_real_t)360,src->S,src->V);
  return dst;
}

static PSE_FINLINE struct pse_color_RGBui8_t*
pseHSVrToRGBui8
  (const struct pse_color_HSVr_t* src,
   struct pse_color_RGBui8_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToRGBui8(pseHSVrToRGBr(src,&rgb),dst);
}

/******************************************************************************
 * XYZ=>RGB
 ******************************************************************************/

static PSE_FINLINE struct pse_color_RGBr_t*
pseXYZrToRGBr
  (const struct pse_color_XYZr_t* src,
   struct pse_color_RGBr_t* dst)
{
  Xyz2Rgb(&dst->R,&dst->G,&dst->B, src->X,src->Y,src->Z);
  return dst;
}

static PSE_FINLINE struct pse_color_RGBui8_t*
pseXYZrToRGBui8
  (const struct pse_color_XYZr_t* src,
   struct pse_color_RGBui8_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToRGBui8(pseXYZrToRGBr(src,&rgb),dst);
}

/******************************************************************************
 * LAB=>XYZ
 ******************************************************************************/

static PSE_FINLINE struct pse_color_XYZr_t*
pseLABrToXYZr
  (const struct pse_color_LABr_t* src,
   struct pse_color_XYZr_t* dst)
{
  Lab2Xyz
    (&dst->X,&dst->Y,&dst->Z,
     src->L * 100.0,
     (src->a * 255.0) - 128.0,
     (src->b * 255.0) - 128.0);
  return dst;
}

static PSE_FINLINE struct pse_color_XYZr_t*
pseLui8ABi8ToXYZr
  (const struct pse_color_Lui8ABi8_t* src,
   struct pse_color_XYZr_t* dst)
{
  struct pse_color_LABr_t rgb;
  return pseLABrToXYZr(pseLui8ABi8ToLABr(src,&rgb),dst);
}

/******************************************************************************
 * RGB=>XYZ
 ******************************************************************************/

static PSE_FINLINE struct pse_color_XYZr_t*
pseRGBrToXYZr
  (const struct pse_color_RGBr_t* src,
   struct pse_color_XYZr_t* dst)
{
  Rgb2Xyz(&dst->X,&dst->Y,&dst->Z, src->R,src->G,src->B);
  return dst;
}

static PSE_FINLINE struct pse_color_XYZr_t*
pseRGBui8ToXYZr
  (const struct pse_color_RGBui8_t* src,
   struct pse_color_XYZr_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToXYZr(pseRGBui8ToRGBr(src,&rgb), dst);
}

/******************************************************************************
 * XYZ=>LAB
 ******************************************************************************/

static PSE_FINLINE struct pse_color_LABr_t*
pseXYZrToLABr
  (const struct pse_color_XYZr_t* src,
   struct pse_color_LABr_t* dst)
{
  Xyz2Lab(&dst->L,&dst->a,&dst->b, src->X,src->Y,src->Z);
  dst->L /= 100.0;
  dst->a = (dst->a + 128.0) / 255.0;
  dst->b = (dst->b + 128.0) / 255.0;
  return dst;
}

static PSE_FINLINE struct pse_color_Lui8ABi8_t*
pseXYZrToLui8ABi8
  (const struct pse_color_XYZr_t* src,
   struct pse_color_Lui8ABi8_t* dst)
{
  struct pse_color_LABr_t rgb;
  return pseLABrToLui8ABi8(pseXYZrToLABr(src,&rgb),dst);
}

/******************************************************************************
 * RGB=>Cat02LMS
 ******************************************************************************/

static PSE_FINLINE struct pse_color_Cat02LMSr_t*
pseRGBrToCat02LMSr
  (const struct pse_color_RGBr_t* src,
   struct pse_color_Cat02LMSr_t* dst)
{
  Rgb2Cat02lms(&dst->L,&dst->M,&dst->S, src->R,src->G,src->B);
  return dst;
}

static PSE_FINLINE struct pse_color_Cat02LMSr_t*
pseRGBui8ToCat02LMSr
  (const struct pse_color_RGBui8_t* src,
   struct pse_color_Cat02LMSr_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToCat02LMSr(pseRGBui8ToRGBr(src,&rgb),dst);
}

/******************************************************************************
 * Cat02LMS=>RGB
 ******************************************************************************/

static PSE_FINLINE struct pse_color_RGBr_t*
pseCat02LMSrToRGBr
  (const struct pse_color_Cat02LMSr_t* src,
   struct pse_color_RGBr_t* dst)
{
  Cat02lms2Rgb(&dst->R,&dst->G,&dst->B, src->L,src->M,src->S);
  return dst;
}

static PSE_FINLINE struct pse_color_RGBui8_t*
pseCat02LMSrToRGBui8
  (const struct pse_color_Cat02LMSr_t* src,
   struct pse_color_RGBui8_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToRGBui8(pseCat02LMSrToRGBr(src,&rgb),dst);
}

/******************************************************************************
 * Cat02LMS=>XYZ
 ******************************************************************************/

static PSE_FINLINE struct pse_color_XYZr_t*
pseCat02LMSrToXYZr
  (const struct pse_color_Cat02LMSr_t* src,
   struct pse_color_XYZr_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToXYZr(pseCat02LMSrToRGBr(src,&rgb),dst);
}

/******************************************************************************
 * XYZ=>Cat02LMS
 ******************************************************************************/

static PSE_FINLINE struct pse_color_Cat02LMSr_t*
pseXYZrToCat02LMSr
  (const struct pse_color_XYZr_t* src,
   struct pse_color_Cat02LMSr_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToCat02LMSr(pseXYZrToRGBr(src,&rgb),dst);
}

/******************************************************************************
 * LAB=>Cat02LMS
 ******************************************************************************/

static PSE_FINLINE struct pse_color_Cat02LMSr_t*
pseLABrToCat02LMSr
  (const struct pse_color_LABr_t* src,
   struct pse_color_Cat02LMSr_t* dst)
{
  struct pse_color_XYZr_t xyz;
  return pseXYZrToCat02LMSr(pseLABrToXYZr(src, &xyz), dst);
}

static PSE_FINLINE struct pse_color_Cat02LMSr_t*
pseLui8ABi8ToCat02LMSr
  (const struct pse_color_Lui8ABi8_t* src,
   struct pse_color_Cat02LMSr_t* dst)
{
  struct pse_color_LABr_t lab;
  return pseLABrToCat02LMSr(pseLui8ABi8ToLABr(src,&lab),dst);
}

/******************************************************************************
 * RGB=>LAB
 ******************************************************************************/

static PSE_FINLINE struct pse_color_LABr_t*
pseRGBrToLABr
  (const struct pse_color_RGBr_t* src,
   struct pse_color_LABr_t* dst)
{
  Rgb2Lab(&dst->L,&dst->a,&dst->b, src->R,src->G,src->B);
  dst->L /= 100.0;
  dst->a = (dst->a + 128.0) / 255.0;
  dst->b = (dst->b + 128.0) / 255.0;
  return dst;
}

static PSE_FINLINE struct pse_color_Lui8ABi8_t*
pseRGBrToLui8ABi8
  (const struct pse_color_RGBr_t* src,
   struct pse_color_Lui8ABi8_t* dst)
{
  struct pse_color_LABr_t lab;
  return pseLABrToLui8ABi8(pseRGBrToLABr(src,&lab),dst);
}

static PSE_FINLINE struct pse_color_LABr_t*
pseRGBui8ToLABr
  (const struct pse_color_RGBui8_t* src,
   struct pse_color_LABr_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToLABr(pseRGBui8ToRGBr(src,&rgb), dst);
}

static PSE_FINLINE struct pse_color_Lui8ABi8_t*
pseRGBui8ToLui8ABi8
  (const struct pse_color_RGBui8_t* src,
   struct pse_color_Lui8ABi8_t* dst)
{
  struct pse_color_LABr_t lab;
  return pseLABrToLui8ABi8(pseRGBui8ToLABr(src,&lab),dst);
}

/******************************************************************************
 * Cat02LMS=>LAB
 ******************************************************************************/

static PSE_FINLINE struct pse_color_LABr_t*
pseCat02LMSrToLABr
  (const struct pse_color_Cat02LMSr_t* src,
   struct pse_color_LABr_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToLABr(pseCat02LMSrToRGBr(src,&rgb),dst);
}

static PSE_FINLINE struct pse_color_Lui8ABi8_t*
pseCat02LMSrToLui8ABi8
  (const struct pse_color_Cat02LMSr_t* src,
   struct pse_color_Lui8ABi8_t* dst)
{
  struct pse_color_LABr_t lab;
  return pseLABrToLui8ABi8(pseCat02LMSrToLABr(src,&lab),dst);
}

/******************************************************************************
 * HSV=>Cat02LMS
 ******************************************************************************/

static PSE_FINLINE struct pse_color_Cat02LMSr_t*
pseHSVrToCat02LMSr
  (const struct pse_color_HSVr_t* src,
   struct pse_color_Cat02LMSr_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToCat02LMSr(pseHSVrToRGBr(src,&rgb),dst);
}

/******************************************************************************
 * HSV=>XYZ
 ******************************************************************************/

static PSE_FINLINE struct pse_color_XYZr_t*
pseHSVrToXYZr
  (const struct pse_color_HSVr_t* src,
   struct pse_color_XYZr_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToXYZr(pseHSVrToRGBr(src,&rgb),dst);
}

/******************************************************************************
 * HSV=>LAB
 ******************************************************************************/

static PSE_FINLINE struct pse_color_LABr_t*
pseHSVrToLABr
  (const struct pse_color_HSVr_t* src,
   struct pse_color_LABr_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToLABr(pseHSVrToRGBr(src,&rgb),dst);
}

static PSE_FINLINE struct pse_color_Lui8ABi8_t*
pseHSVrToLui8ABi8
  (const struct pse_color_HSVr_t* src,
   struct pse_color_Lui8ABi8_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToLui8ABi8(pseHSVrToRGBr(src,&rgb),dst);
}

/******************************************************************************
 * RGB=>HSV
 ******************************************************************************/

static PSE_FINLINE struct pse_color_HSVr_t*
pseRGBrToHSVr
  (const struct pse_color_RGBr_t* src,
   struct pse_color_HSVr_t* dst)
{
  Rgb2Hsv(&dst->H,&dst->S,&dst->V, src->R,src->G,src->B);
  dst->H /= (pse_real_t)360;
  return dst;
}

static PSE_FINLINE struct pse_color_HSVr_t*
pseRGBui8ToHSVr
  (const struct pse_color_RGBui8_t* src,
   struct pse_color_HSVr_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToHSVr(pseRGBui8ToRGBr(src,&rgb), dst);
}

/******************************************************************************
 * XYZ=>HSV
 ******************************************************************************/

static PSE_FINLINE struct pse_color_HSVr_t*
pseXYZrToHSVr
  (const struct pse_color_XYZr_t* src,
   struct pse_color_HSVr_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToHSVr(pseXYZrToRGBr(src,&rgb),dst);
}

/******************************************************************************
 * LAB=>RGB
 ******************************************************************************/

static PSE_FINLINE struct pse_color_RGBr_t*
pseLABrToRGBr
  (const struct pse_color_LABr_t* src,
   struct pse_color_RGBr_t* dst)
{
  Lab2Rgb
    (&dst->R,&dst->G,&dst->B,
     src->L * 100.0,
     src->a * 255.0 - 128.0,
     src->b * 255.0 - 128.0);
  return dst;
}

static PSE_FINLINE struct pse_color_RGBui8_t*
pseLABrToRGBui8
  (const struct pse_color_LABr_t* src,
   struct pse_color_RGBui8_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToRGBui8(pseLABrToRGBr(src,&rgb), dst);
}

static PSE_FINLINE struct pse_color_RGBr_t*
pseLui8ABi8ToRGBr
  (const struct pse_color_Lui8ABi8_t* src,
   struct pse_color_RGBr_t* dst)
{
  struct pse_color_LABr_t lab;
  return pseLABrToRGBr(pseLui8ABi8ToLABr(src,&lab),dst);
}

static PSE_FINLINE struct pse_color_RGBui8_t*
pseLui8ABi8ToRGBui8
  (const struct pse_color_Lui8ABi8_t* src,
   struct pse_color_RGBui8_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToRGBui8(pseLui8ABi8ToRGBr(src,&rgb),dst);
}

/******************************************************************************
 * LAB=>HSV
 ******************************************************************************/

static PSE_FINLINE struct pse_color_HSVr_t*
pseLABrToHSVr
  (const struct pse_color_LABr_t* src,
   struct pse_color_HSVr_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToHSVr(pseLABrToRGBr(src,&rgb),dst);
}

static PSE_FINLINE struct pse_color_HSVr_t*
pseLui8ABi8ToHSVr
  (const struct pse_color_Lui8ABi8_t* src,
   struct pse_color_HSVr_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToHSVr(pseLui8ABi8ToRGBr(src,&rgb),dst);
}

/******************************************************************************
 * Cat02LMS=>HSV
 ******************************************************************************/

static PSE_FINLINE struct pse_color_HSVr_t*
pseCat02LMSrToHSVr
  (const struct pse_color_Cat02LMSr_t* src,
   struct pse_color_HSVr_t* dst)
{
  struct pse_color_RGBr_t rgb;
  return pseRGBrToHSVr(pseCat02LMSrToRGBr(src,&rgb),dst);
}

/******************************************************************************
 * *=>Any
 ******************************************************************************/

#define PSE_COLOR_TYPE_CURRENT Cat02LMSr
#include "pse_color_conversion_tmpl.h"

#define PSE_COLOR_TYPE_CURRENT HSVr
#include "pse_color_conversion_tmpl.h"

#define PSE_COLOR_TYPE_CURRENT XYZr
#include "pse_color_conversion_tmpl.h"

#define PSE_COLOR_TYPE_CURRENT LABr
#include "pse_color_conversion_tmpl.h"
#define PSE_COLOR_TYPE_CURRENT Lui8ABi8
#include "pse_color_conversion_tmpl.h"

#define PSE_COLOR_TYPE_CURRENT RGBr
#include "pse_color_conversion_tmpl.h"
#define PSE_COLOR_TYPE_CURRENT RGBui8
#include "pse_color_conversion_tmpl.h"

/******************************************************************************
 * Any=>Any
 ******************************************************************************/

static PSE_INLINE struct pse_color_any_components_t*
pseAnyCompsToAnyComps
  (const pse_color_format_t src_fmt,
   const struct pse_color_any_components_t* src,
   const pse_color_format_t dst_fmt,
   struct pse_color_any_components_t* dst)
{
  switch(src_fmt) {
  #define PSE_COLOR_TYPE(Name)                                                 \
    case PSE_COLOR_FORMAT_##Name##_:                                           \
      return pse##Name##ToAny((struct pse_color_##Name##_t*)src, dst_fmt, dst);
    PSE_COLOR_TYPES_ALL
  #undef PSE_COLOR_TYPE
    default: assert(false);
  }
  return NULL;
}

static PSE_FINLINE struct pse_color_t*
pseAnyToAny
  (const struct pse_color_t* src,
   const pse_color_format_t dst_fmt,
   struct pse_color_t* dst)
{
  assert(src && dst);
  PSE_TRY_VERIFY_OR_ELSE(NULL != pseAnyCompsToAnyComps
    (pseColorFormatGet(src), &src->as.any.comps,
     dst_fmt, &dst->as.any.comps),
    return NULL);
  dst->space = PSE_COLOR_SPACE_FROM(dst_fmt);
  dst->as.any.type = PSE_COLOR_TYPE_FROM(dst_fmt);
  return dst;
}

/******************************************************************************
 * NRef=>NRef
 ******************************************************************************/

static PSE_FINLINE struct pse_colors_t*
pseNAnyToNAny
  (const struct pse_colors_t* src,
   const pse_color_format_t dst_fmt,
   struct pse_colors_t* dst)
{
  const pse_color_format_t src_fmt =
    PSE_COLOR_FORMAT_FROM(src->space, src->as.any.type);
  size_t i;

  /* Ensure consistency */
  PSE_VERIFY_OR_ELSE(src->as.any.count == dst->as.any.count, return NULL);

  /* TODO: optimize by doing SIMD? */
  for(i = 0; i < dst->as.any.count; ++i) {
    PSE_TRY_VERIFY_OR_ELSE(NULL != pseAnyCompsToAnyComps
      (src_fmt, &src->as.any.comps[i],
       dst_fmt, &dst->as.any.comps[i]),
      return NULL);
  }
  dst->space = PSE_COLOR_SPACE_FROM(dst_fmt);
  dst->as.any.type = PSE_COLOR_TYPE_FROM(dst_fmt);
  return dst;
}

static PSE_FINLINE struct pse_colors_raw_aos_t*
pseNAnyToNRawAOS
  (const struct pse_colors_t* src,
   const pse_color_format_t dst_fmt,
   struct pse_colors_raw_aos_t* dst)
{
  const pse_color_format_t src_fmt =
    PSE_COLOR_FORMAT_FROM(src->space, src->as.any.type);
  size_t i, dst_fmt_memsize;
  void* dst_curr = NULL;

  /* Ensure consistency */
  PSE_VERIFY_OR_ELSE(src->as.any.count == dst->count, return NULL);

  dst_fmt_memsize = pseColorFormatMemSize(dst_fmt);
  PSE_VERIFY_OR_ELSE(dst_fmt_memsize, return NULL);

  /* TODO: optimize by doing SIMD? */
  for(i = 0; i < src->as.any.count; ++i) {
    dst_curr = pseColorsRawAOSGetPtrAt(dst, i);
    PSE_TRY_VERIFY_OR_ELSE(NULL != pseAnyCompsToAnyComps
      (src_fmt, &src->as.any.comps[i],
       dst_fmt, (struct pse_color_any_components_t*)dst_curr),
      return NULL);
  }
  dst->format = dst_fmt;
  return dst;
}

static PSE_FINLINE struct pse_colors_t*
pseNRawAOSToNAny
  (const struct pse_colors_raw_aos_t* src,
   const pse_color_format_t dst_fmt,
   struct pse_colors_t* dst)
{
  size_t i, src_fmt_memsize;
  void* src_curr = NULL;

  /* Ensure consistency */
  PSE_VERIFY_OR_ELSE(src->count == dst->as.any.count, return NULL);

  src_fmt_memsize = pseColorFormatMemSize(src->format);
  PSE_VERIFY_OR_ELSE(src_fmt_memsize, return NULL);

  /* TODO: optimize by doing SIMD? */
  for(i = 0; i < src->count; ++i) {
    src_curr = pseColorsRawAOSGetPtrAt(src, i);
    PSE_TRY_VERIFY_OR_ELSE(NULL != pseAnyCompsToAnyComps
      (src->format, (struct pse_color_any_components_t*)src_curr,
       dst_fmt, &dst->as.any.comps[i]),
      return NULL);
  }
  dst->space = PSE_COLOR_SPACE_FROM(dst_fmt);
  dst->as.any.type = PSE_COLOR_TYPE_FROM(dst_fmt);
  return dst;
}

static PSE_FINLINE struct pse_colors_raw_aos_t*
pseNRawAOSToNRawAOS
  (const struct pse_colors_raw_aos_t* src,
   const pse_color_format_t dst_fmt,
   struct pse_colors_raw_aos_t* dst)
{
  size_t i, src_fmt_memsize, dst_fmt_memsize;
  void* src_curr = NULL;
  void* dst_curr = NULL;

  /* Ensure consistency */
  PSE_VERIFY_OR_ELSE(src->count == dst->count, return NULL);

  src_fmt_memsize = pseColorFormatMemSize(src->format);
  dst_fmt_memsize = pseColorFormatMemSize(dst->format);
  PSE_VERIFY_OR_ELSE(src_fmt_memsize && dst_fmt_memsize, return NULL);

  /* TODO: optimize by doing SIMD? */
  for(i = 0; i < src->count; ++i) {
    src_curr = pseColorsRawAOSGetPtrAt(src, i);
    dst_curr = pseColorsRawAOSGetPtrAt(dst, i);
    PSE_TRY_VERIFY_OR_ELSE(NULL != pseAnyCompsToAnyComps
      (src->format, (struct pse_color_any_components_t*)src_curr,
       dst_fmt, (struct pse_color_any_components_t*)dst_curr),
      return NULL);
  }
  dst->format = dst_fmt;
  return dst;
}

static PSE_FINLINE struct pse_colors_ref_t*
pseNAnyToNRef
  (const struct pse_colors_t* src,
   const pse_color_format_t dst_fmt,
   struct pse_colors_ref_t* dst)
{
  assert(dst);
  switch(dst->kind) {
    case PSE_COLOR_REF_KIND_COLORS: {
      return pseNAnyToNAny(src, dst_fmt, &dst->as.colors) ? dst : NULL;
    } break;
    case PSE_COLOR_REF_KIND_RAW_AOS: {
      return pseNAnyToNRawAOS(src, dst_fmt, &dst->as.raw_aos) ? dst : NULL;
    } break;
    default: assert(false);
  }
  return NULL;
}

static PSE_FINLINE struct pse_colors_ref_t*
pseNRawAOSToNRef
  (const struct pse_colors_raw_aos_t* src,
   const pse_color_format_t dst_fmt,
   struct pse_colors_ref_t* dst)
{
  assert(dst);
  switch(dst->kind) {
    case PSE_COLOR_REF_KIND_COLORS: {
      return pseNRawAOSToNAny(src, dst_fmt, &dst->as.colors) ? dst : NULL;
    } break;
    case PSE_COLOR_REF_KIND_RAW_AOS: {
      return pseNRawAOSToNRawAOS(src, dst_fmt, &dst->as.raw_aos) ? dst : NULL;
    } break;
    default: assert(false);
  }
  return NULL;
}

static PSE_FINLINE struct pse_colors_ref_t*
pseNRefToNRef
  (const struct pse_colors_ref_t* src,
   const pse_color_format_t dst_fmt,
   struct pse_colors_ref_t* dst)
{
  assert(src);
  switch(src->kind) {
    case PSE_COLOR_REF_KIND_COLORS: {
      return pseNAnyToNRef(&src->as.colors, dst_fmt, dst);
    } break;
    case PSE_COLOR_REF_KIND_RAW_AOS: {
      return pseNRawAOSToNRef(&src->as.raw_aos, dst_fmt, dst);
    } break;
    default: assert(false);
  }
  return NULL;
}

PSE_API_END

#endif /* PSE_COLOR_CONVERSION_P_H */
