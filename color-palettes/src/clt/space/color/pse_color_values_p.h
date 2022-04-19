#ifndef PSE_COLOR_VALUES_P_H
#define PSE_COLOR_VALUES_P_H

#include "pse_color_types.h"

PSE_API_BEGIN

struct pse_color_palette_t;
struct pse_colors_t;
struct pse_cpspace_values_t;

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

struct pse_color_palette_values_t {
  struct pse_color_palette_t* cp;
  struct pse_colors_t* colors;
  pse_color_format_t exchange_format; /* world will communicate with this format */
  bool* lock_status;
  struct pse_cpspace_values_t* cps_values;
};

/******************************************************************************
 *
 * PRIVATE CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_PALETTE_VALUES_NULL_                                         \
  { NULL, NULL, PSE_COLOR_FORMAT_INVALID_, NULL, NULL }

static const struct pse_color_palette_values_t PSE_COLOR_PALETTE_VALUES_NULL =
  PSE_COLOR_PALETTE_VALUES_NULL_;

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

LOCAL_SYMBOL enum pse_res_t
pseColorPaletteValuesCreateFrom
  (struct pse_color_palette_t* cp,
   const pse_color_format_t target_format,
   struct pse_colors_t* clrs,
   const size_t locked_count,
   const pse_color_idx_t* locked,
   struct pse_color_palette_values_t** vals);

LOCAL_SYMBOL enum pse_res_t
pseColorPaletteValuesCreateMapping
  (struct pse_color_palette_t* cp,
   const pse_color_format_t expected_in_format,
   struct pse_colors_t* clrs,
   struct pse_color_palette_values_t** vals);

LOCAL_SYMBOL enum pse_res_t
pseColorPaletteValuesCopy
  (struct pse_color_palette_values_t* vals,
   struct pse_colors_t* dst);

LOCAL_SYMBOL enum pse_res_t
pseColorPaletteValuesDestroy
  (struct pse_color_palette_values_t* vals);

PSE_API_END

#endif /* PSE_COLOR_VALUES_P_H */
