#ifndef PSE_CAPACITIES_H
#define PSE_CAPACITIES_H

#include "pse_platform.h"

PSE_API_BEGIN

enum pse_device_capacity_t {
  PSEC_NONE,

  PSEC_FIRST__,

  /* Different types of parametric point attributes */
  PSEC_PPOINT_ATTRIB_COORDINATES = PSEC_FIRST__,
  PSEC_PPOINT_ATTRIB_LOCK_STATUS,

  PSEC_COUNT__
};

PSE_API_END

#endif /* PSE_CAPACITIES_H */
