#ifndef PSE_COLOR_COST_GENERIC_H
#define PSE_COLOR_COST_GENERIC_H

#include "pse_color_types.h"

PSE_API_BEGIN

struct pse_cpspace_t;

/******************************************************************************
 *
 * MACROS
 *
 ******************************************************************************/

#define PSE_COLOR_COST_FUNC_GENERIC_ALL                                        \
  PSE_COLOR_COST_FUNC(Generic_L1DistanceSignedPerComponent, PER_RELATIONSHIP, PER_COMPONENT, 1)\
  PSE_COLOR_COST_FUNC(Generic_L1DistanceUnsignedPerComponent, PER_RELATIONSHIP, PER_COMPONENT, 1)\
  PSE_COLOR_COST_FUNC(Generic_L1DistanceSigned, PER_RELATIONSHIP, PER_COLOR, 1)\
  PSE_COLOR_COST_FUNC(Generic_L1DistanceUnsigned, PER_RELATIONSHIP, PER_COLOR, 1)

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

enum pse_color_cost_func_generic_t {
#define PSE_COLOR_COST_FUNC(Name, _a,_b,_c)                                    \
  PSE_CONCAT(PSE_COLOR_COST_FUNC_,Name),
  PSE_COLOR_COST_FUNC_GENERIC_ALL
#undef PSE_COLOR_COST_FUNC

  PSE_COLOR_COST_FUNC_GENERIC_COUNT,
  PSE_COLOR_COST_FUNC_SPECIFIC_START_
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

PSE_COLOR_API enum pse_res_t
pseColorGenericCostFunctorRegister
  (struct pse_cpspace_t* cps,
   const pse_color_format_t fmt,
   const enum pse_color_cost_func_generic_t cf,
   const pse_color_components_flags_t components,
   pse_relshp_cost_func_id_t* fid);

PSE_API_END

#endif /* PSE_COLOR_COST_GENERIC_H */
