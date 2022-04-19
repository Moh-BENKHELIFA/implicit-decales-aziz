#ifndef PSE_CPSPACE_VALUES_P_H
#define PSE_CPSPACE_VALUES_P_H

#include "pse.h"
#include "pse_ref_count.h"
#include "pse_drv.h"

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

struct pse_cpspace_values_t {
  struct pse_cpspace_values_data_t data;
  struct pse_cpspace_t* cps;
  pse_atomic_t lock;  /*! \todo read-write mutex */
  pse_ref_t ref;
};

/******************************************************************************
 *
 * PRIVATE CONSTANTS
 *
 ******************************************************************************/

#define PSE_CPSPACE_VALUES_NULL_                                               \
  { PSE_CPSPACE_VALUES_DATA_NULL_, NULL, 0, 0 }

static const struct pse_cpspace_values_t PSE_CPSPACE_VALUES_NULL =
  PSE_CPSPACE_VALUES_NULL_;

/******************************************************************************
 *
 * PRIVATE API
 *
 ******************************************************************************/

LOCAL_SYMBOL void
pseConstrainedParameterSpaceValuesDestroy
  (struct pse_cpspace_values_t* vals);

LOCAL_SYMBOL enum pse_res_t
pseConstrainedParameterSpaceValuesDataValidate
  (struct pse_cpspace_t* cps,
   const struct pse_cpspace_values_data_t* data);

#endif /* PSE_CPSPACE_VALUES_P_H */
