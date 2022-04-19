#ifndef PSE_COLOR_PALETTE_CONSTRAINTS_P_H
#define PSE_COLOR_PALETTE_CONSTRAINTS_P_H

#include "pse_color_palette_constraints.h"

PSE_API_BEGIN

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

enum pse_color_constraint_mode_t {
  PSE_COLOR_CONSTRAINT_MODE_PER_COLOR,
  PSE_COLOR_CONSTRAINT_MODE_PER_COMPONENT
};

/*! Generic configuration for constraints which are only weighted constraints,
 * without any specific data. */
struct pse_color_constraint_generic_weighted_config_t {
  pse_color_format_t format;
  pse_real_t weight;
};

/*! Specific configuration for the distance constraint. This constraint start
 * its application when the distance between involved colors becomes lesser than
 * \p distance_min.
 */
struct pse_color_constraint_distance_config_t {
  struct pse_color_constraint_generic_weighted_config_t as_weighted;
  struct pse_color_distance_params_t dist_params;
  enum pse_color_constraint_mode_t mode;
};

struct pse_color_palette_constraint_t {
  pse_color_palette_constraint_id_t key;
  struct pse_color_palette_constraint_params_t params;
};

/******************************************************************************
 *
 * PRIVATE CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_CONSTRAINT_GENERIC_WEIGHTED_CONFIG_INVALID_                  \
  { PSE_COLOR_FORMAT_INVALID_, 0.0 }
#define PSE_COLOR_CONSTRAINT_DISTANCE_PARAMS_NULL_                             \
  { PSE_COLOR_CONSTRAINT_GENERIC_WEIGHTED_CONFIG_INVALID_,                     \
    PSE_COLOR_DISTANCE_PARAMS_DEFAULT_, PSE_COLOR_CONSTRAINT_MODE_PER_COLOR }
#define PSE_COLOR_PALETTE_CONSTRAINT_NULL_                                     \
  { PSE_COLOR_PALETTE_CONSTRAINT_ID_INVALID_,                                  \
    PSE_COLOR_PALETTE_CONSTRAINT_PARAMS_NULL_ }

static const struct pse_color_constraint_generic_weighted_config_t PSE_COLOR_CONSTRAINT_GENERIC_WEIGHTED_CONFIG_INVALID =
  PSE_COLOR_CONSTRAINT_GENERIC_WEIGHTED_CONFIG_INVALID_;
static const struct pse_color_constraint_distance_config_t PSE_COLOR_CONSTRAINT_DISTANCE_PARAMS_NULL =
  PSE_COLOR_CONSTRAINT_DISTANCE_PARAMS_NULL_;
static const struct pse_color_palette_constraint_t PSE_COLOR_PALETTE_CONSTRAINT_NULL =
  PSE_COLOR_PALETTE_CONSTRAINT_NULL_;

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

LOCAL_SYMBOL enum pse_res_t
pseColorPaletteConstraintCreate
  (struct pse_color_palette_t* cp,
   const struct pse_color_palette_constraint_params_t* params,
   struct pse_color_palette_constraint_t** cnstr);

LOCAL_SYMBOL enum pse_res_t
pseColorPaletteConstraintDestroy
  (struct pse_color_palette_t* cp,
   struct pse_color_palette_constraint_t* cnstr);

PSE_API_END

#endif /* PSE_COLOR_PALETTE_CONSTRAINTS_P_H */
