#ifndef PSE_EIGEN_VALUES_H
#define PSE_EIGEN_VALUES_H

#include "pse_eigen_api.h"

#include <pse.h>

PSE_API_BEGIN

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

PSE_EIGEN_INLINE_API const struct pse_attrib_value_accessors_t*
pseEigenValuesAccessorsGet
  (const struct pse_cpspace_values_data_t* vals,
   enum pse_point_attrib_t attrib)
{
  switch(vals->storage) {
    case PSE_CPSPACE_VALUES_STORAGE_ACCESSORS_GLOBAL: {
      return &vals->as.global.accessors;
    } break;
    case PSE_CPSPACE_VALUES_STORAGE_ACCESSORS_PER_ATTRIB: {
      return &vals->as.per_attrib.accessors[attrib];
    } break;
    default: assert(false);
  }
  return NULL;
}

PSE_API_END

#endif /* PSE_EIGEN_VALUES_H */
