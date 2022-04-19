#ifndef PSE_COLOR_COST_HSV_H
#define PSE_COLOR_COST_HSV_H

#include "pse_color_cost_generic.h"

PSE_API_BEGIN

struct pse_cpspace_t;

/******************************************************************************
 *
 * MACROS
 *
 ******************************************************************************/

#define PSE_COLOR_COST_FUNC_HSV_ALL                                            \
  PSE_COLOR_COST_FUNC(HSV_L1DistanceSignedPerComponent, PER_RELATIONSHIP, PER_COMPONENT, 1)\
  PSE_COLOR_COST_FUNC(HSV_L1DistanceUnsignedPerComponent, PER_RELATIONSHIP, PER_COMPONENT, 1)\
  PSE_COLOR_COST_FUNC(HSV_L1DistanceSigned, PER_RELATIONSHIP, PER_COLOR, 1)\
  PSE_COLOR_COST_FUNC(HSV_L1DistanceUnsigned, PER_RELATIONSHIP, PER_COLOR, 1)

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

enum pse_color_cost_func_HSV_t {
  PSE_COLOR_COST_FUNC_HSV_BEGIN = PSE_COLOR_COST_FUNC_SPECIFIC_START_,
#define PSE_COLOR_COST_FUNC(Name, _a,_b,_c)                                    \
  PSE_CONCAT(PSE_COLOR_COST_FUNC_,Name),
  PSE_COLOR_COST_FUNC_HSV_ALL
#undef PSE_COLOR_COST_FUNC
  PSE_COLOR_COST_FUNC_HSV_END,
  PSE_COLOR_COST_FUNC_HSV_COUNT =
      PSE_COLOR_COST_FUNC_HSV_END
    - PSE_COLOR_COST_FUNC_HSV_BEGIN
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
pseIsColorHSVCostFunctorValid
  (const enum pse_color_cost_func_HSV_t cf)
{
  return (PSE_COLOR_COST_FUNC_HSV_BEGIN < cf)
      && (PSE_COLOR_COST_FUNC_HSV_END > cf);
}

PSE_COLOR_API enum pse_res_t
pseColorHSVCostFunctorRegister
  (struct pse_cpspace_t* cps,
   const pse_color_format_t fmt,
   const enum pse_color_cost_func_HSV_t cf,
   const pse_color_components_flags_t components,
   pse_relshp_cost_func_id_t* fid);

PSE_API_END

#endif /* PSE_COLOR_COST_HSV_H */
