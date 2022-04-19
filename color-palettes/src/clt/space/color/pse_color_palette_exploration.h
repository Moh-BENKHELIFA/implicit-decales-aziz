#ifndef PSE_COLOR_PALETTE_EXPLORATION_H
#define PSE_COLOR_PALETTE_EXPLORATION_H

#include "pse_color_api.h"
#include "pse_color_types.h"
#include "pse_color_space_Lab.h"

#include <pse.h>

PSE_API_BEGIN

struct pse_colors_t;
struct pse_color_palette_t;

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

struct pse_color_palette_exploration_ctxt_params_t {
  pse_color_format_t explore_in;  /*!< must be based on pse_real_t */
  struct pse_cpspace_exploration_options_t options;
};

/******************************************************************************
 *
 * CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_PALETTE_EXPLORATION_CTXT_PARAMS_DEFAULT_                     \
  { PSE_COLOR_FORMAT_LABr_, PSE_CPSPACE_EXPLORATION_OPTIONS_DEFAULT_ }

static const struct pse_color_palette_exploration_ctxt_params_t PSE_COLOR_PALETTE_EXPLORATION_CTXT_PARAMS_DEFAULT =
  PSE_COLOR_PALETTE_EXPLORATION_CTXT_PARAMS_DEFAULT_;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_COLOR_API enum pse_res_t
pseColorPaletteExplorationContextCreate
  (struct pse_color_palette_t* cp,
   struct pse_color_palette_exploration_ctxt_params_t* params,
   struct pse_cpspace_exploration_ctxt_t** ctxt);

PSE_COLOR_API enum pse_res_t
pseColorPaletteExplorationContextInitFromValues
  (struct pse_cpspace_exploration_ctxt_t* ctxt,
   struct pse_colors_t* refs);

PSE_COLOR_API enum pse_res_t
pseColorPaletteExplorationSolve
  (struct pse_cpspace_exploration_ctxt_t* ctxt,
   struct pse_colors_t* smpls,
   const size_t locked_count,
   const pse_color_idx_t* locked);

PSE_COLOR_API enum pse_res_t
pseColorPaletteExplorationResultsRetreive
  (struct pse_cpspace_exploration_ctxt_t* ctxt,
   struct pse_colors_t* optimized_clrs,
   struct pse_cpspace_exploration_extra_results_t* extra_results); /*!< may be NULL */

PSE_API_END

#endif /* PSE_COLOR_PALETTE_EXPLORATION_H */
