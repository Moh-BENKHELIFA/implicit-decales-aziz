#ifndef PSE_COLOR_PALETTE_VARIATION_P_H
#define PSE_COLOR_PALETTE_VARIATION_P_H

#include <pse_platform.h>

PSE_API_BEGIN

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

/*! Range used by color variation IDs. Using this structure, we want to simplify
 * the work of the user to avoid IDs collisions.
 */
enum pse_color_palette_variation_range_t {
  PSE_CPV_RANGE_CVD =   0x00000100
};

PSE_API_END

#endif /* PSE_COLOR_PALETTE_VARIATION_P_H */
