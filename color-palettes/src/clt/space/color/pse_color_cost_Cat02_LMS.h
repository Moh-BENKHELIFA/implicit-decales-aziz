#ifndef PSE_COLOR_COST_CAT02_LMS_H
#define PSE_COLOR_COST_CAT02_LMS_H

#include "pse_color_cost_generic.h"

PSE_API_BEGIN

struct pse_cpspace_t;

/******************************************************************************
 *
 * MACROS
 *
 ******************************************************************************/

#define PSE_COLOR_COST_FUNC_Cat02LMS_ALL                                       \

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

enum pse_color_cost_func_Cat02LMS_t {
  PSE_COLOR_COST_FUNC_Cat02LMS_BEGIN = PSE_COLOR_COST_FUNC_SPECIFIC_START_,
#define PSE_COLOR_COST_FUNC(Name, _a,_b,_c)                                    \
  PSE_CONCAT(PSE_COLOR_COST_FUNC_,Name),
  PSE_COLOR_COST_FUNC_Cat02LMS_ALL
#undef PSE_COLOR_COST_FUNC
  PSE_COLOR_COST_FUNC_Cat02LMS_END,
  PSE_COLOR_COST_FUNC_Cat02LMS_COUNT =
      PSE_COLOR_COST_FUNC_Cat02LMS_END
    - PSE_COLOR_COST_FUNC_Cat02LMS_BEGIN
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
pseIsColorCat02LMSCostFunctorValid
  (const enum pse_color_cost_func_Cat02LMS_t cf)
{
  return (PSE_COLOR_COST_FUNC_Cat02LMS_BEGIN < cf)
      && (PSE_COLOR_COST_FUNC_Cat02LMS_END > cf);
}

PSE_COLOR_API enum pse_res_t
pseColorCat02LMSCostFunctorRegister
  (struct pse_cpspace_t* cps,
   const pse_color_format_t fmt,
   const enum pse_color_cost_func_Cat02LMS_t cf,
   const pse_color_components_flags_t components,
   pse_relshp_cost_func_id_t* fid);

PSE_API_END

#endif /* PSE_COLOR_COST_CAT02_LMS_H */
