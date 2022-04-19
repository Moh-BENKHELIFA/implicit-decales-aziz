#ifndef PSE_COLOR_COST_H
#define PSE_COLOR_COST_H

#include "pse_color_types.h"
#include "pse_color_cost_Cat02_LMS.h"
#include "pse_color_cost_HSV.h"
#include "pse_color_cost_Lab.h"
#include "pse_color_cost_RGB.h"
#include "pse_color_cost_XYZ.h"

PSE_API_BEGIN

struct pse_color_palette_t;

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

/*! Macro used to define the list of all cost functors related to colors. This
 * macro can the be used to generate repetitive and dumb code related to each
 * cost functor.
 *
 * \see ::PSE_COLOR_COST_FUNC
 */
#define PSE_COLOR_COST_FUNC_ALL                                                \
  PSE_COLOR_COST_FUNC_Cat02LMS_ALL                                             \
  PSE_COLOR_COST_FUNC_HSV_ALL                                                  \
  PSE_COLOR_COST_FUNC_LAB_ALL                                                  \
  PSE_COLOR_COST_FUNC_RGB_ALL                                                  \
  PSE_COLOR_COST_FUNC_XYZ_ALL

/*! Each cost functor must be declared with a macro using this prototype.
 * With \p CostArityModeShortcut taking its value among:
 *    - \c PER_RELATIONSHIP each relationship will generate \p CostsCount costs.
 *    - \c PER_POINT each point involved in a relationship will generate
 *        \p CostsCount costs.
 *
 * \see ::pse_cost_arity_mode_t
 */
#define PSE_COLOR_COST_FUNC(Name,CostArityModeShortcut,CostsCount)
#undef PSE_COLOR_COST_FUNC

/*! Store a configuration of a cost functor, i.e. what is making it  different
 * from the others. */
struct pse_color_cost_func_config_t {
  pse_color_format_t format;
  pse_color_cost_func_id_t id;
  pse_color_components_flags_t components; /*!< those used */
};

/******************************************************************************
 *
 * PUBLIC CONSTANTS
 *
 ******************************************************************************/

#define PSE_COLOR_COST_FUNC_ID_INVALID_                                        \
  ((pse_color_cost_func_id_t)-1)
#define PSE_COLOR_COST_FUNC_CONFIG_NULL_                                       \
  { PSE_COLOR_FORMAT_INVALID_,                                                 \
    PSE_COLOR_COST_FUNC_ID_INVALID_,                                           \
    PSE_COLOR_COMPONENTS_NONE }

static const pse_color_cost_func_id_t PSE_COLOR_COST_FUNC_ID_INVALID =
  PSE_COLOR_COST_FUNC_ID_INVALID_;
static const struct pse_color_cost_func_config_t PSE_COLOR_COST_FUNC_CONFIG_NULL =
  PSE_COLOR_COST_FUNC_CONFIG_NULL_;

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

/*! This function create or get already create cost functors IDs. It also return
 * the \p fcfgs, i.e. the cost functors configurations specific to the related
 * cost functors.
 */
PSE_COLOR_API enum pse_res_t
pseColorPaletteCostFunctorsCreateOrGetFromConfig
  (struct pse_color_palette_t* cp,
   const size_t count,
   const struct pse_color_cost_func_config_t* configs,
   pse_relshp_cost_func_id_t* fids);

PSE_COLOR_API enum pse_res_t
pseColorPaletteCostFunctorsClean
  (struct pse_color_palette_t* cp,
   const size_t count,
   const struct pse_color_cost_func_config_t* configs);

PSE_API_END

#endif /* PSE_COLOR_COST_H */
