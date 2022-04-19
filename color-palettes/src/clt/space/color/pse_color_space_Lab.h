#ifndef PSE_COLOR_SPACE_LAB_H
#define PSE_COLOR_SPACE_LAB_H

#include "pse_color_api.h"
#include "pse_color_types.h"

PSE_API_BEGIN

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

#define PSE_COLOR_SPACE_LAB  ((pse_color_space_t)0x0001)

#define PSE_COLOR_SPACE_LAB_TYPES_ALL                                          \
  PSE_COLOR_TYPE(LABr)                                                         \
  PSE_COLOR_TYPE(Lui8ABi8)

enum pse_color_type_LAB_t {
#define PSE_COLOR_TYPE(Name)  PSE_COLOR_TYPE_##Name,
  PSE_COLOR_SPACE_LAB_TYPES_ALL
#undef PSE_COLOR_TYPE
  PSE_COLOR_TYPE_LAB_COUNT_
};

struct pse_color_LABr_t {
  pse_real_t L, a, b;  /*!< In [0,1] */
};

struct pse_color_Lui8ABi8_t {
  uint8_t L;    /*!< In [0,100] */
  int8_t a, b;  /*!< In [-128,127] */
};

struct pse_color_LAB_t {
  union {
  #define PSE_COLOR_TYPE(Name)  struct pse_color_##Name##_t Name;
    PSE_COLOR_SPACE_LAB_TYPES_ALL
  #undef PSE_COLOR_TYPE
    struct pse_color_any_components_t comps;
  } as;
  pse_color_type_t type;
};
struct pse_colors_LAB_t {
  union {
  #define PSE_COLOR_TYPE(Name)  struct pse_color_##Name##_t* Name;
    PSE_COLOR_SPACE_LAB_TYPES_ALL
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
  (sizeof(struct pse_color_any_components_t) >= sizeof(struct pse_color_LABr_t),
   Color_Any_Components_type_do_not_allow_to_store_LABr_color_components);
PSE_STATIC_ASSERT
  (sizeof(struct pse_color_any_components_t) >= sizeof(struct pse_color_Lui8ABi8_t),
   Color_Any_Components_type_do_not_allow_to_store_Lui8ABi8_color_components);

/******************************************************************************
 *
 * PUBLIC CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_LABr_BLACK_                                                  \
  { 0, 0, 0 }
#define PSE_COLOR_Lui8ABi8_BLACK_                                             \
  { 0, 0, 0 }
#define PSE_COLOR_LAB_BLACKr_                                                   \
  { { PSE_COLOR_LABr_BLACK_ }, PSE_COLOR_TYPE_LABr }
#define PSE_COLORS_LAB_INVALID_                                                \
  { { NULL }, 0, PSE_COLOR_TYPE_INVALID_ }
#define PSE_COLOR_FORMAT_LABr_                                                 \
  PSE_COLOR_FORMAT_FROM(PSE_COLOR_SPACE_LAB, PSE_COLOR_TYPE_LABr)
#define PSE_COLOR_FORMAT_Lui8ABi8_                                            \
  PSE_COLOR_FORMAT_FROM(PSE_COLOR_SPACE_LAB, PSE_COLOR_TYPE_Lui8ABi8)

static const struct pse_color_LABr_t PSE_COLOR_LABr_BLACK =
  PSE_COLOR_LABr_BLACK_;
static const struct pse_color_Lui8ABi8_t PSE_COLOR_Lui8ABi8_BLACK =
  PSE_COLOR_Lui8ABi8_BLACK_;
static const struct pse_color_LAB_t PSE_COLOR_LAB_BLACKr =
  PSE_COLOR_LAB_BLACKr_;
static const struct pse_colors_LAB_t PSE_COLORS_LAB_INVALID =
  PSE_COLORS_LAB_INVALID_;
static const pse_color_format_t PSE_COLOR_FORMAT_LABr =
  PSE_COLOR_FORMAT_LABr_;
static const pse_color_format_t PSE_COLOR_FORMAT_Lui8ABi8 =
  PSE_COLOR_FORMAT_Lui8ABi8_;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_API_END

#endif /* PSE_COLOR_SPACE_LAB_H */
