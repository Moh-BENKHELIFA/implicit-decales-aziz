#ifndef PSE_COLOR_TYPES_H
#define PSE_COLOR_TYPES_H

#include "pse_color_api.h"

#include <pse_types.h>

PSE_API_BEGIN

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

/*!< Right now, no more than 3 components. */
#define PSE_COLOR_SPACE_COMPS_COUNT_MAX 3

typedef pse_clt_ppoint_variation_uid_t pse_color_variation_uid_t;

typedef uintptr_t pse_color_palette_constraint_id_t;

/*! Will receive the enum values of the cost functors for each space. */
typedef uint16_t pse_color_cost_func_id_t;

typedef uint8_t pse_color_space_t;
typedef uint8_t pse_color_type_t;

/*! A color format is the combination of a color space and a color type:
 *    format = (space << 16) & type
 *
 * \see pse_color_space_t
 * \see pse_color_type_t
 */
typedef pse_clt_pspace_uid_t pse_color_format_t;
PSE_STATIC_ASSERT(sizeof(pse_color_format_t) >= 4, Insufficient_type_size);

/*! Used to have a memory mapping that do not depends on the format.
 *
 * \warning The size of this type must be at least the size of the biggest
 * specific color structure that represents the components. Many static asserts
 * in the code ensure this assertion. */
struct pse_color_any_components_t {
  pse_real_t mem[PSE_COLOR_SPACE_COMPS_COUNT_MAX];
};

typedef size_t pse_color_idx_t;

enum pse_color_component_flag_t {
  PSE_COLOR_COMPONENT_0 = 1 << 0,
  PSE_COLOR_COMPONENT_1 = 1 << 1,
  PSE_COLOR_COMPONENT_2 = 1 << 2,

  PSE_COLOR_COMPONENTS_NONE = 0x00,
  PSE_COLOR_COMPONENTS_ALL = 0xFF,
  PSE_COLOR_COMPONENTS_COUNT_MAX = 3
};
typedef uint8_t pse_color_components_flags_t;

/******************************************************************************
 *
 * PUBLIC CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_SPACE_SHIFT_          8
#define PSE_COLOR_TYPE_SHIFT_           0
#define PSE_COLOR_SPACE_MASK_           0xFF
#define PSE_COLOR_TYPE_MASK_            0xFF
#define PSE_COLOR_SPACE_SHIFTED_MASK_   (PSE_COLOR_SPACE_MASK_ << PSE_COLOR_SPACE_SHIFT_)
#define PSE_COLOR_TYPE_SHIFTED_MASK_    (PSE_COLOR_TYPE_MASK_ << PSE_COLOR_TYPE_SHIFT_)

#define PSE_COLOR_VARIATION_UID_INVALID_                                       \
  ((pse_color_variation_uid_t)PSE_CLT_PPOINT_VARIATION_UID_INVALID_)
#define PSE_COLOR_PALETTE_CONSTRAINT_ID_INVALID_                               \
  ((pse_color_palette_constraint_id_t)-1)
#define PSE_COLOR_SPACE_INVALID_                                               \
  ((pse_color_space_t)0xFF)
#define PSE_COLOR_TYPE_INVALID_                                                \
  ((pse_color_type_t)0xFF)
#define PSE_COLOR_FORMAT_INVALID_                                              \
  ((pse_color_format_t)0xFFFF)
#define PSE_COLOR_ANY_COMPONENTS_ZERO_                                         \
  { { 0, 0, 0 } }
#define PSE_COLOR_IDX_INVALID_                                                 \
  ((pse_color_idx_t)-1)

static const pse_color_variation_uid_t PSE_COLOR_VARIATION_UID_INVALID =
  PSE_COLOR_VARIATION_UID_INVALID_;
static const pse_color_palette_constraint_id_t PSE_COLOR_PALETTE_CONSTRAINT_ID_INVALID =
  PSE_COLOR_PALETTE_CONSTRAINT_ID_INVALID_;
static const pse_color_space_t PSE_COLOR_SPACE_INVALID =
  PSE_COLOR_SPACE_INVALID_;
static const pse_color_type_t PSE_COLOR_TYPE_INVALID =
  PSE_COLOR_TYPE_INVALID_;
static const pse_color_format_t PSE_COLOR_FORMAT_INVALID =
  PSE_COLOR_FORMAT_INVALID_;
static const struct pse_color_any_components_t PSE_COLOR_ANY_COMPONENTS_ZERO =
  PSE_COLOR_ANY_COMPONENTS_ZERO_;
static const pse_color_idx_t PSE_COLOR_IDX_INVALID =
  PSE_COLOR_IDX_INVALID_;

/******************************************************************************
 *
 * PUBLIC MACROS
 *
 ******************************************************************************/

#define PSE_COLOR_FORMAT_FROM(Space, Type)                                     \
  ((pse_color_format_t)                                                        \
    ( (((Space)&PSE_COLOR_SPACE_MASK_)<<PSE_COLOR_SPACE_SHIFT_)                \
    | (((Type)&PSE_COLOR_TYPE_MASK_)<<PSE_COLOR_TYPE_SHIFT_)))
#define PSE_COLOR_SPACE_FROM(Format)                                           \
  ((pse_color_space_t)(((Format)&PSE_COLOR_SPACE_SHIFTED_MASK_)>>PSE_COLOR_SPACE_SHIFT_))
#define PSE_COLOR_TYPE_FROM(Format)                                            \
  ((pse_color_type_t)((Format)&PSE_COLOR_TYPE_SHIFTED_MASK_)>>PSE_COLOR_TYPE_SHIFT_)

#define PSE_HAS_COLOR_COMPONENT(comps,i)  (((1<<(i))&(comps))!=0)

PSE_API_END

#endif /* PSE_COLOR_TYPES_H */
