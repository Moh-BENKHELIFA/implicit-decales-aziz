#ifndef PSE_COLOR_SPACE_XYZ_H
#define PSE_COLOR_SPACE_XYZ_H

#include "pse_color_api.h"
#include "pse_color_types.h"

PSE_API_BEGIN

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

#define PSE_COLOR_SPACE_XYZ    ((pse_color_space_t)0x0003)

#define PSE_COLOR_SPACE_XYZ_TYPES_ALL                                          \
  PSE_COLOR_TYPE(XYZr)

enum pse_color_type_XYZ_t {
#define PSE_COLOR_TYPE(Name)  PSE_COLOR_TYPE_##Name,
  PSE_COLOR_SPACE_XYZ_TYPES_ALL
#undef PSE_COLOR_TYPE
  PSE_COLOR_TYPE_XYZ_COUNT_
};

struct pse_color_XYZr_t {
  pse_real_t X, Y, Z; /*!< In [0,1] (is it true for Y?) */
};

struct pse_color_XYZ_t {
  union {
  #define PSE_COLOR_TYPE(Name)  struct pse_color_##Name##_t Name;
    PSE_COLOR_SPACE_XYZ_TYPES_ALL
  #undef PSE_COLOR_TYPE
    struct pse_color_any_components_t comps;
  } as;
  pse_color_type_t type;
};
struct pse_colors_XYZ_t {
  union {
  #define PSE_COLOR_TYPE(Name)  struct pse_color_##Name##_t* Name;
    PSE_COLOR_SPACE_XYZ_TYPES_ALL
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
  (sizeof(struct pse_color_any_components_t) >= sizeof(struct pse_color_XYZr_t),
   Color_Any_Components_type_do_not_allow_to_store_XYZr_color_components);

/******************************************************************************
 *
 * PUBLIC CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_XYZr_BLACK_                                                  \
  { 0, 0, 0 }
#define PSE_COLOR_XYZ_BLACKr_                                                  \
  { { PSE_COLOR_XYZr_BLACK_ }, PSE_COLOR_TYPE_XYZr }
#define PSE_COLORS_XYZ_INVALID_                                                \
  { { NULL }, 0, PSE_COLOR_TYPE_INVALID_ }
#define PSE_COLOR_FORMAT_XYZr_                                                 \
  PSE_COLOR_FORMAT_FROM(PSE_COLOR_SPACE_XYZ, PSE_COLOR_TYPE_XYZr)

static const struct pse_color_XYZr_t PSE_COLOR_XYZr_BLACK =
  PSE_COLOR_XYZr_BLACK_;
static const struct pse_color_XYZ_t PSE_COLOR_XYZ_BLACKr =
  PSE_COLOR_XYZ_BLACKr_;
static const struct pse_colors_XYZ_t PSE_COLORS_XYZ_INVALID =
  PSE_COLORS_XYZ_INVALID_;
static const pse_color_format_t PSE_COLOR_FORMAT_XYZr =
  PSE_COLOR_FORMAT_XYZr_;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_COLOR_INLINE_API void
pseXYZ2xyz
  (const struct pse_color_XYZ_t* XYZ,
   pse_real_t* x,
   pse_real_t* y,
   pse_real_t* z)
{
  const pse_real_t inv_sum =
    (pse_real_t)1 / (XYZ->as.XYZr.X + XYZ->as.XYZr.Y + XYZ->as.XYZr.Z);
  assert(XYZ && x && y && z);
  *x = XYZ->as.XYZr.X * inv_sum;
  *y = XYZ->as.XYZr.Y * inv_sum;
  *z = XYZ->as.XYZr.Z * inv_sum;
}

PSE_COLOR_INLINE_API void
psexyY2XYZ
  (const pse_real_t x,
   const pse_real_t y,
   const pse_real_t Y,
   struct pse_color_XYZ_t* XYZ)
{
  assert(XYZ);
  /* TODO: could be simplify:
   * Z = ((1-x-y)*Y)/y == (Y-x*Y-y*Y)/y
   *                   == Y/y - (x*Y)/y - (y*Y)/y
   *                   == (Y - y*Y)/y - X
   * But what about the precision of such operations? We can use 2 FMA if I'm
   * not wrong.
   */
  XYZ->as.XYZr.X = (x*Y)/y;
  XYZ->as.XYZr.Y = Y;
  XYZ->as.XYZr.Z = (((pse_real_t)1-x-y)*Y)/y;
}

PSE_API_END

#endif /* PSE_COLOR_SPACE_XYZ_H */
