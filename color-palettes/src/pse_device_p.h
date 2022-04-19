#ifndef PSE_DEVICE_H
#define PSE_DEVICE_H

/*! \file
 * Private header for the device concept
 */

#include "pse.h"
#include "pse_drv.h"

/******************************************************************************
 *
 * PRIVATE TYPES
 *
 ******************************************************************************/

struct pse_device_t {
  struct pse_allocator_t* allocator;
  struct pse_logger_t* logger;

  struct pse_drv_t drv;

  struct pse_cpspace_t** cpspaces;
  size_t* cpspaces_free;
};

/******************************************************************************
 *
 * PRIVATE API - Device
 *
 ******************************************************************************/

LOCAL_SYMBOL enum pse_res_t
pseDeviceConstrainedParameterSpaceRegister
  (struct pse_device_t* dev,
   struct pse_cpspace_t* cps);

LOCAL_SYMBOL enum pse_res_t
pseDeviceConstrainedParameterSpaceUnregister
  (struct pse_device_t* dev,
   struct pse_cpspace_t* cps);

#endif /* PSE_DEVICE_H */
