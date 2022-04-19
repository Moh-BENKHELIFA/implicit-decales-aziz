#ifndef PSE_COLOR_SPACE_CAT02_LMS_H
#define PSE_COLOR_SPACE_CAT02_LMS_H

#include "pse_color_api.h"
#include "pse_color_types.h"

PSE_API_BEGIN

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

#define PSE_COLOR_SPACE_Cat02LMS    ((pse_color_space_t)0x0006)

#define PSE_COLOR_SPACE_Cat02LMS_TYPES_ALL                                     \
  PSE_COLOR_TYPE(Cat02LMSr)

enum pse_color_type_Cat02LMS_t {
#define PSE_COLOR_TYPE(Name)  PSE_COLOR_TYPE_##Name,
  PSE_COLOR_SPACE_Cat02LMS_TYPES_ALL
#undef PSE_COLOR_TYPE
  PSE_COLOR_TYPE_Cat02LMS_COUNT_
};

/*! Long, Medium, Short color space. Each value cannot be negative, but there is
 * no upper bound specified.
 * \see http://en.wikipedia.org/wiki/LMS_color_space
 */
struct pse_color_Cat02LMSr_t {
  pse_real_t L, M, S; /*!< In [0,+inf[ */
};

struct pse_color_Cat02LMS_t {
  union {
  #define PSE_COLOR_TYPE(Name)  struct pse_color_##Name##_t Name;
    PSE_COLOR_SPACE_Cat02LMS_TYPES_ALL
  #undef PSE_COLOR_TYPE
    struct pse_color_any_components_t comps;
  } as;
  pse_color_type_t type;
};
struct pse_colors_Cat02LMS_t {
  union {
  #define PSE_COLOR_TYPE(Name)  struct pse_color_##Name##_t* Name;
    PSE_COLOR_SPACE_Cat02LMS_TYPES_ALL
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
  (sizeof(struct pse_color_any_components_t) >= sizeof(struct pse_color_Cat02LMSr_t),
   Color_Any_Components_type_do_not_allow_to_store_Cat02LMSr_color_components);

/******************************************************************************
 *
 * PUBLIC CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_Cat02LMSr_BLACK_                                             \
  { 0, 0, 0 }
#define PSE_COLOR_Cat02LMS_BLACKr_                                             \
  { { PSE_COLOR_Cat02LMSr_BLACK_ }, PSE_COLOR_TYPE_Cat02LMSr }
#define PSE_COLORS_Cat02LMS_INVALID_                                           \
  { { NULL }, 0, PSE_COLOR_TYPE_INVALID_ }
#define PSE_COLOR_FORMAT_Cat02LMSr_                                            \
  PSE_COLOR_FORMAT_FROM(PSE_COLOR_SPACE_Cat02LMS, PSE_COLOR_TYPE_Cat02LMSr)

static const struct pse_color_Cat02LMSr_t PSE_COLOR_Cat02LMSr_BLACK =
  PSE_COLOR_Cat02LMSr_BLACK_;
static const struct pse_color_Cat02LMS_t PSE_COLOR_Cat02LMS_BLACKr =
  PSE_COLOR_Cat02LMS_BLACKr_;
static const struct pse_colors_Cat02LMS_t PSE_COLORS_Cat02LMS_INVALID =
  PSE_COLORS_Cat02LMS_INVALID_;
static const pse_color_format_t PSE_COLOR_FORMAT_Cat02LMSr =
  PSE_COLOR_FORMAT_Cat02LMSr_;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_COLOR_INLINE_API bool
pseColorCat02LMSIsInGamut
  (struct pse_color_Cat02LMS_t* clr)
{
  assert(clr);
  return (clr->as.Cat02LMSr.L > (pse_real_t)0)
      && (clr->as.Cat02LMSr.M > (pse_real_t)0)
      && (clr->as.Cat02LMSr.S > (pse_real_t)0);
}

PSE_COLOR_API void
pseColorCat02LMSInGamutClampInPlace
  (struct pse_color_Cat02LMS_t* clr);

PSE_COLOR_API void
pseColorsCat02LMSInGamutClampInPlace
  (struct pse_colors_Cat02LMS_t* clrs);

PSE_COLOR_INLINE_API pse_real_t
pseColorCat02LMSDistanceSquaredFromGamut
  (struct pse_color_Cat02LMS_t* clr)
{
  struct pse_color_Cat02LMS_t tmp;
  assert(clr);

  /* Return the Squared norm of the clr-clamp(clr) vector */
  tmp = *clr;
  pseColorCat02LMSInGamutClampInPlace(&tmp);
  tmp.as.Cat02LMSr.L = clr->as.Cat02LMSr.L - tmp.as.Cat02LMSr.L;
  tmp.as.Cat02LMSr.M = clr->as.Cat02LMSr.M - tmp.as.Cat02LMSr.M;
  tmp.as.Cat02LMSr.S = clr->as.Cat02LMSr.S - tmp.as.Cat02LMSr.S;
  return
      PSE_REAL_POW2(tmp.as.Cat02LMSr.L)
    + PSE_REAL_POW2(tmp.as.Cat02LMSr.M)
    + PSE_REAL_POW2(tmp.as.Cat02LMSr.S);
}

PSE_COLOR_INLINE_API pse_real_t
pseColorCat02LMSDistanceFromGamut
  (struct pse_color_Cat02LMS_t* clr)
{
  return PSE_REAL_SQRT(pseColorCat02LMSDistanceFromGamut(clr));
}

PSE_API_END

#endif /* PSE_COLOR_SPACE_CAT02_LMS_H */
