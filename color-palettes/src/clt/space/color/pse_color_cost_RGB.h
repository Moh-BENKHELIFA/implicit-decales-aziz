#ifndef PSE_COLOR_COST_RGB_H
#define PSE_COLOR_COST_RGB_H

#include "pse_color_cost_generic.h"

PSE_API_BEGIN

struct pse_cpspace_t;

/******************************************************************************
 *
 * MACROS
 *
 ******************************************************************************/

#define PSE_COLOR_COST_FUNC_RGB_ALL                                            \
  PSE_COLOR_COST_FUNC(RGB_InGamut, PER_POINT, PER_COLOR, 1)                    \
  PSE_COLOR_COST_FUNC(RGB_OLEDScreenEnergyUsage, PER_POINT, PER_COLOR, 1)

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

enum pse_color_cost_func_RGB_t {
  PSE_COLOR_COST_FUNC_RGB_BEGIN = PSE_COLOR_COST_FUNC_SPECIFIC_START_,
#define PSE_COLOR_COST_FUNC(Name, _a,_b,_c)                                    \
  PSE_CONCAT(PSE_COLOR_COST_FUNC_,Name),
  PSE_COLOR_COST_FUNC_RGB_ALL
#undef PSE_COLOR_COST_FUNC
  PSE_COLOR_COST_FUNC_RGB_END,
  PSE_COLOR_COST_FUNC_RGB_COUNT =
      PSE_COLOR_COST_FUNC_RGB_END
    - PSE_COLOR_COST_FUNC_RGB_BEGIN
    - 1
};

/******************************************************************************
 *
 * PUBLIC CONSTANTS
 *
 ******************************************************************************/

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_COLOR_INLINE_API bool
pseIsColorRGBCostFunctorValid
  (const enum pse_color_cost_func_RGB_t cf)
{
  return (PSE_COLOR_COST_FUNC_RGB_BEGIN < cf)
      && (PSE_COLOR_COST_FUNC_RGB_END > cf);
}

PSE_COLOR_API enum pse_res_t
pseColorRGBCostFunctorRegister
  (struct pse_cpspace_t* cps,
   const pse_color_format_t fmt,
   const enum pse_color_cost_func_RGB_t cf,
   const pse_color_components_flags_t components,
   pse_relshp_cost_func_id_t* fid);

PSE_API_END

#endif /* PSE_COLOR_COST_RGB_H */
