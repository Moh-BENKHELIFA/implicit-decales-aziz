#ifndef PSE_COLOR_SPACE_HSV_H
#define PSE_COLOR_SPACE_HSV_H

#include "pse_color_api.h"
#include "pse_color_types.h"

PSE_API_BEGIN

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

#define PSE_COLOR_SPACE_HSV    ((pse_color_space_t)0x0004)

#define PSE_COLOR_SPACE_HSV_TYPES_ALL                                          \
  PSE_COLOR_TYPE(HSVr)

enum pse_color_type_HSV_t {
#define PSE_COLOR_TYPE(Name)  PSE_COLOR_TYPE_##Name,
  PSE_COLOR_SPACE_HSV_TYPES_ALL
#undef PSE_COLOR_TYPE
  PSE_COLOR_TYPE_HSV_COUNT_
};

/*! \ref H represents the parametric position in [0,1] on the circle. To get it
 * in degrees, just multiply it by 360°.
 */
struct pse_color_HSVr_t {
  pse_real_t H, S, V; /*!< In [0,1] */
};

struct pse_color_HSV_t {
  union {
  #define PSE_COLOR_TYPE(Name)  struct pse_color_##Name##_t Name;
    PSE_COLOR_SPACE_HSV_TYPES_ALL
  #undef PSE_COLOR_TYPE
    struct pse_color_any_components_t comps;
  } as;
  pse_color_type_t type;
};
struct pse_colors_HSV_t {
  union {
  #define PSE_COLOR_TYPE(Name)  struct pse_color_##Name##_t* Name;
    PSE_COLOR_SPACE_HSV_TYPES_ALL
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
  (sizeof(struct pse_color_any_components_t) >= sizeof(struct pse_color_HSVr_t),
   Color_Any_Components_type_do_not_allow_to_store_HSVr_color_components);

/******************************************************************************
 *
 * PUBLIC CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_HSVr_BLACK_                                                  \
  { 0, 0, 0 }
#define PSE_COLOR_HSV_BLACKr_                                                  \
  { { PSE_COLOR_HSVr_BLACK_ }, PSE_COLOR_TYPE_HSVr }
#define PSE_COLORS_HSV_INVALID_                                                \
  { { NULL }, 0, PSE_COLOR_TYPE_INVALID_ }
#define PSE_COLOR_FORMAT_HSVr_                                                 \
  PSE_COLOR_FORMAT_FROM(PSE_COLOR_SPACE_HSV, PSE_COLOR_TYPE_HSVr)

static const struct pse_color_HSVr_t PSE_COLOR_HSVr_BLACK =
  PSE_COLOR_HSVr_BLACK_;
static const struct pse_color_HSV_t PSE_COLOR_HSV_BLACKr =
  PSE_COLOR_HSV_BLACKr_;
static const struct pse_colors_HSV_t PSE_COLORS_HSV_INVALID =
  PSE_COLORS_HSV_INVALID_;
static const pse_color_format_t PSE_COLOR_FORMAT_HSVr =
  PSE_COLOR_FORMAT_HSVr_;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_API_END

#endif /* PSE_COLOR_SPACE_HSV_H */
