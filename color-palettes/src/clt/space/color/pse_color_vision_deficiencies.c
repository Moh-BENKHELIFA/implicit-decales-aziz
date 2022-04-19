#include "pse_color_vision_deficiencies.h"
#include "pse_color_palette_variation_p.h"
#include "pse_color_conversion_p.h"
#include "pse_color_space_XYZ.h"

#include "pse_color_palette_p.h"

#include <stretchy_buffer.h>

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

enum pse_cvd_variation_uid_t {
  PSE_CVDV_UID_DEUTERANOPIA_Rasche2005  = PSE_CPV_RANGE_CVD | PSE_CVD_DEUTERANOPIA_Rasche2005,
  PSE_CVDV_UID_PROTANOPIA_Rasche2005    = PSE_CPV_RANGE_CVD | PSE_CVD_PROTANOPIA_Rasche2005,
  PSE_CVDV_UID_DEUTERANOPIA_Troiano2008 = PSE_CPV_RANGE_CVD | PSE_CVD_DEUTERANOPIA_Troiano2008,
  PSE_CVDV_UID_PROTANOPIA_Troiano2008   = PSE_CPV_RANGE_CVD | PSE_CVD_PROTANOPIA_Troiano2008
};

struct pse_line_2d_t {
  pse_real_t a, b;
};

struct pse_rasche2005_config_t {
  pse_real_t u, v;
  struct pse_line_2d_t line;
};

/******************************************************************************
 *
 * PRIVATE CONSTANTS
 *
 ******************************************************************************/

#define PSE_xy2u(x,y)                                                          \
  ( ((pse_real_t)4 * (x))                                                      \
  / ((pse_real_t)12 * (y) - (pse_real_t)2 * (x) + (pse_real_t)3) )
#define PSE_xy2v(x,y)                                                          \
  ( ((pse_real_t)9 * (y))                                                      \
  / ((pse_real_t)12 * (y) - (pse_real_t)2 * (x) + (pse_real_t)3) )

#define PSE_uv2x(u,v)                                                          \
  ( ((pse_real_t)9 * (u))                                                      \
  / ((pse_real_t)2 * ((pse_real_t)3 * (u) - (pse_real_t)8 * (v) + (pse_real_t)6)) )
#define PSE_uv2y(u,v)                                                          \
  ( ((pse_real_t)2 * (v))                                                      \
  / ((pse_real_t)3 * (u) - (pse_real_t)8 * (v) + (pse_real_t)6) )

#define PSE_LINE_2D_A_FROM_POINTS(x0,y0,x1,y1)                                 \
  (((y1)-(y0))/((x1)-(x0)))
#define PSE_LINE_2D_B_FROM_POINTS(x0,y0,x1,y1)                                 \
  ((y0) - PSE_LINE_2D_A_FROM_POINTS(x0,y0,x1,y1) * (x0))
#define PSE_LINE_2D_FROM_POINTS(x0,y0,x1,y1)                                   \
  { PSE_LINE_2D_A_FROM_POINTS(x0,y0,x1,y1),                                    \
    PSE_LINE_2D_B_FROM_POINTS(x0,y0,x1,y1) }
#define PSE_RASCHE2005_CONFIG_DEUTERANOPIA_                                    \
  { -4.75, 1.31, PSE_LINE_2D_FROM_POINTS                                       \
    (PSE_xy2u(0.102775863, 0.102863739),                                       \
     PSE_xy2v(0.102775863, 0.102863739),                                       \
     PSE_xy2u(0.505845283, 0.493211177),                                       \
     PSE_xy2v(0.505845283, 0.493211177)) }
#define PSE_RASCHE2005_CONFIG_PROTANOPIA_                                      \
  { 0.61, 0.51, PSE_LINE_2D_FROM_POINTS                                        \
    (PSE_xy2u(0.115807359, 0.073580708),                                       \
     PSE_xy2v(0.115807359, 0.073580708),                                       \
     PSE_xy2u(0.471898745, 0.527050570),                                       \
     PSE_xy2v(0.471898745, 0.527050570)) }

static const struct pse_rasche2005_config_t PSE_RASCHE2005_CONFIG_DEUTERANOPIA =
  PSE_RASCHE2005_CONFIG_DEUTERANOPIA_;
static const struct pse_rasche2005_config_t PSE_RASCHE2005_CONFIG_PROTANOPIA =
  PSE_RASCHE2005_CONFIG_PROTANOPIA_;

/******************************************************************************
 *
 * HELPER FUNCTIONS
 *
 ******************************************************************************/

static PSE_FINLINE bool
pseLinesIntersectionPointGet
  (const struct pse_line_2d_t* l1,
   const struct pse_line_2d_t* l2,
   pse_real_t* px,
   pse_real_t* py)
{
  assert(l1 && l2 && px && py);
  if( l1->a == l2->a )
    return false;
  *px = (l2->b - l1->b) / (l1->a - l2->a);
  *py = l1->a * *px + l1->b;
  return true;
}

static PSE_FINLINE enum pse_cvd_variation_uid_t
pseCVDVariationToCVDVariationUID
  (const enum pse_color_vision_deficiency_variation_t cvdv)
{
  return (PSE_CPV_RANGE_CVD | cvdv);
}

/*!< \warning We must ensure that from and to can be the same object */

static PSE_FINLINE enum pse_res_t
pseCVDRasche2005TransformFromConfig
  (void* user_data,
   const struct pse_rasche2005_config_t* config,
   const struct pse_colors_ref_t* from,
   const pse_color_format_t to_fmt,
   const pse_color_variation_uid_t to_ppv,
   struct pse_colors_ref_t* to)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_t c = PSE_COLOR_INVALID;
  struct pse_color_t rgb = PSE_COLOR_INVALID;
  struct pse_line_2d_t line_uv;
  pse_real_t x,y,z, u,v, pu,pv;
  size_t i, count;
  assert(!user_data && config && from && to);
  (void)user_data, (void)to_ppv;

  PSE_VERIFY_OR_ELSE(NULL != pseNRefToNRef
    (from, PSE_COLOR_FORMAT_XYZr, to),
    return RES_BAD_ARG);

  pseColorFormatSet(&c, PSE_COLOR_FORMAT_XYZr);
  pseColorFormatSet(&rgb, PSE_COLOR_FORMAT_RGBr);
  count = pseColorsRefCountGet(to);
  for(i = 0; i < count; ++i) {
    PSE_CALL_OR_RETURN(res, pseColorsRefExtractAt(to, i, &c));
    pseXYZ2xyz(&c.as.XYZ, &x,&y,&z);
    u = PSE_xy2u(x,y);
    v = PSE_xy2v(x,y);
    line_uv.a = PSE_LINE_2D_A_FROM_POINTS(config->u, config->v, u, v);
    line_uv.b = PSE_LINE_2D_B_FROM_POINTS(config->u, config->v, u, v);

    if( pseLinesIntersectionPointGet(&config->line, &line_uv, &pu,&pv) ) {
      x = PSE_uv2x(pu,pv);
      y = PSE_uv2y(pu,pv);
      psexyY2XYZ(x,y,c.as.XYZ.as.XYZr.Y, &c.as.XYZ);
    } else {
      /* Previous implementation was using the black color when no intersaction
       * was found. */
      assert(false);
      c.as.XYZ = PSE_COLOR_XYZ_BLACKr;
    }
    /* TODO: here, we do lots of color conversion, perhaps to go in the end to
     * RGB color space... We should do the gamut clamping only at the end, after
     * the loop, to do it on all colors at once. */
    /*PSE_CALL_OR_RETURN(res, pseColorConvert(&c, &rgb));
    pseColorRGBInGamutClampInPlace(&rgb.as.RGB);*/
    PSE_CALL_OR_RETURN(res, pseColorsRefSetAt(to, i, &c/*&rgb*/));
  }
  PSE_VERIFY_OR_ELSE(NULL != pseNRefToNRef(to, to_fmt, to), return RES_BAD_ARG);
  return res;
}

static enum pse_res_t
pseCVDRasche2005DeuteranopiaTransform
  (void* user_data,
   const struct pse_colors_ref_t* from,
   const pse_color_format_t to_fmt,
   const pse_color_variation_uid_t to_ppv,
   struct pse_colors_ref_t* to)
{
  assert(to_ppv == PSE_CVDV_UID_DEUTERANOPIA_Rasche2005);
  return pseCVDRasche2005TransformFromConfig
    (user_data, &PSE_RASCHE2005_CONFIG_DEUTERANOPIA, from, to_fmt, to_ppv, to);
}

static enum pse_res_t
pseCVDRasche2005ProtanopiaTransform
  (void* user_data,
   const struct pse_colors_ref_t* from,
   const pse_color_format_t to_fmt,
   const pse_color_variation_uid_t to_ppv,
   struct pse_colors_ref_t* to)
{
  assert(to_ppv == PSE_CVDV_UID_PROTANOPIA_Rasche2005);
  return pseCVDRasche2005TransformFromConfig
    (user_data, &PSE_RASCHE2005_CONFIG_PROTANOPIA, from, to_fmt, to_ppv, to);
}

static enum pse_res_t
pseCVDTroiano2008DeuteranopiaTransform
  (void* user_data,
   const struct pse_colors_ref_t* from,
   const pse_color_format_t to_fmt,
   const pse_color_variation_uid_t to_ppv,
   struct pse_colors_ref_t* to)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_t c = PSE_COLOR_INVALID;
  size_t i, count;
  assert(!user_data && from && to);
  assert(to_ppv == PSE_CVDV_UID_DEUTERANOPIA_Troiano2008);
  (void)user_data, (void)to_ppv;

  PSE_VERIFY_OR_ELSE(NULL != pseNRefToNRef
    (from, PSE_COLOR_FORMAT_Cat02LMSr, to),
    return RES_BAD_ARG);

  pseColorFormatSet(&c, PSE_COLOR_FORMAT_Cat02LMSr);
  count = pseColorsRefCountGet(to);
  for(i = 0; i < count; ++i) {
    PSE_CALL_OR_RETURN(res, pseColorsRefExtractAt(to, i, &c));
    c.as.Cat02LMS.as.Cat02LMSr.M =
        c.as.Cat02LMS.as.Cat02LMSr.L * 0.494207
      + c.as.Cat02LMS.as.Cat02LMSr.S * 1.24827;
    PSE_CALL_OR_RETURN(res, pseColorsRefSetAt(to, i, &c));
  }
  PSE_VERIFY_OR_ELSE(NULL != pseNRefToNRef(to, to_fmt, to), return RES_BAD_ARG);
  return res;
}

static enum pse_res_t
pseCVDTroiano2008ProtanopiaTransform
  (void* user_data,
   const struct pse_colors_ref_t* from,
   const pse_color_format_t to_fmt,
   const pse_color_variation_uid_t to_ppv,
   struct pse_colors_ref_t* to)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_t c = PSE_COLOR_INVALID;
  size_t i, count;
  assert(!user_data && from && to);
  assert(to_ppv == PSE_CVDV_UID_DEUTERANOPIA_Troiano2008);
  (void)user_data, (void)to_ppv;

  PSE_VERIFY_OR_ELSE(NULL != pseNRefToNRef
    (from, PSE_COLOR_FORMAT_Cat02LMSr, to),
    return RES_BAD_ARG);

  pseColorFormatSet(&c, PSE_COLOR_FORMAT_Cat02LMSr);
  count = pseColorsRefCountGet(to);
  for(i = 0; i < count; ++i) {
    PSE_CALL_OR_RETURN(res, pseColorsRefExtractAt(to, i, &c));
    c.as.Cat02LMS.as.Cat02LMSr.L =
        c.as.Cat02LMS.as.Cat02LMSr.M * 2.02344
      - c.as.Cat02LMS.as.Cat02LMSr.S * 2.52582;
    PSE_CALL_OR_RETURN(res, pseColorsRefSetAt(to, i, &c));
  }
  PSE_VERIFY_OR_ELSE(NULL != pseNRefToNRef(to, to_fmt, to), return RES_BAD_ARG);
  return res;
}

/******************************************************************************
 *
 * MAPPING
 *
 ******************************************************************************/

static pse_color_variation_apply_cb PSE_CVD_APPLY_CBS[PSE_CVD_VARIATIONS_COUNT_] = {
  pseCVDRasche2005DeuteranopiaTransform,
  pseCVDRasche2005ProtanopiaTransform,
  pseCVDTroiano2008DeuteranopiaTransform,
  pseCVDTroiano2008ProtanopiaTransform
};

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

enum pse_res_t
pseColorsCVDVariationApply
  (const enum pse_color_vision_deficiency_variation_t cvdv,
   const struct pse_colors_ref_t* src,
   struct pse_colors_ref_t* dst)
{
  enum pse_res_t res = RES_OK;
  const enum pse_cvd_variation_uid_t uid = pseCVDVariationToCVDVariationUID(cvdv);
  pse_color_format_t fmt = PSE_COLOR_FORMAT_INVALID;
  if( !src || !dst )
    return RES_BAD_ARG;
  if( cvdv >= PSE_CVD_VARIATIONS_COUNT_ )
    return RES_BAD_ARG;
  fmt = pseColorsRefFormatGet(dst);
  PSE_CALL_OR_RETURN(res, PSE_CVD_APPLY_CBS[cvdv](NULL, src, fmt, uid, dst));
  return res;
}

enum pse_res_t
pseColorsCVDVariationApplyInPlace
  (const enum pse_color_vision_deficiency_variation_t cvdv,
   struct pse_colors_ref_t* clrs)
{
  enum pse_res_t res = RES_OK;
  const enum pse_cvd_variation_uid_t uid = pseCVDVariationToCVDVariationUID(cvdv);
  pse_color_format_t fmt = PSE_COLOR_FORMAT_INVALID;
  if( !clrs )
    return RES_BAD_ARG;
  if( cvdv >= PSE_CVD_VARIATIONS_COUNT_ )
    return RES_BAD_ARG;
  fmt = pseColorsRefFormatGet(clrs);
  PSE_CALL_OR_RETURN(res, PSE_CVD_APPLY_CBS[cvdv](NULL, clrs, fmt, uid, clrs));
  return res;
}

enum pse_res_t
pseColorPaletteVariationsAddForCVD
  (struct pse_color_palette_t* cp,
   const size_t count,
   const enum pse_color_vision_deficiency_variation_t* cvdvs,
   pse_color_variation_uid_t* uids)
{
  enum pse_res_t res = RES_OK;
  struct pse_color_variation_params_t* cpvps = NULL;
  size_t i;
  if( !cp || (count && (!cvdvs || !uids)) )
    return RES_BAD_ARG;

  sb_setn(cpvps, count);
  for(i = 0; i < count; ++i) {
    cpvps[i] = PSE_COLOR_VARIATION_PARAMS_NULL;
    cpvps[i].apply = PSE_CVD_APPLY_CBS[cvdvs[i]];
    uids[i] = pseCVDVariationToCVDVariationUID(cvdvs[i]);
  }
  PSE_CALL_OR_GOTO(res,error, pseColorPaletteVariationsDeclare
    (cp, count, uids, cpvps));

exit:
  sb_free(cpvps);
  return res;
error:
  /* Erase uids, just in case */
  for(i = 0; i < count; ++i) {
    uids[i] = PSE_CLT_PPOINT_VARIATION_UID_INVALID;
  }
  goto exit;
}

pse_color_variation_uid_t
pseColorsCVDVariationUID
  (const enum pse_color_vision_deficiency_variation_t cvdv)
{
  return pseCVDVariationToCVDVariationUID(cvdv);
}
