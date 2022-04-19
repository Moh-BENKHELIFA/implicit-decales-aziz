#include "pse_device_p.h"
#include "pse_cpspace_p.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "stretchy_buffer.h"

/******************************************************************************
 *
 * PRIVATE API - Device
 *
 ******************************************************************************/

/*! \todo Modify strechy buffer implementation to manage out of memory error
 * properly.
 */

enum pse_res_t
pseDeviceConstrainedParameterSpaceRegister
  (struct pse_device_t* dev,
   struct pse_cpspace_t* cps)
{
  assert(dev && cps);
  if( sb_count(dev->cpspaces_free) > 0 ) {
    dev->cpspaces[sb_last(dev->cpspaces_free)] = cps;
    sb_pop(dev->cpspaces_free);
  } else {
    sb_push(dev->cpspaces, cps);
  }
  return RES_OK;
}

enum pse_res_t
pseDeviceConstrainedParameterSpaceUnregister
  (struct pse_device_t* dev,
   struct pse_cpspace_t* cps)
{
  size_t i;
  assert(dev && cps);
  for(i = 0; i < sb_count(dev->cpspaces); ++i) {
    if( dev->cpspaces[i] == cps ) {
      dev->cpspaces[i] = NULL;
      sb_push(dev->cpspaces_free, i);
      return RES_OK;
    }
  }
  return RES_OK;
}

/******************************************************************************
 *
 * PUBLIC API - Device
 *
 ******************************************************************************/

enum pse_res_t
pseDeviceCreate
  (const struct pse_device_params_t* params,
   struct pse_device_t** out_dev)
{
  enum pse_res_t res = RES_OK;
  struct pse_device_t* dev = NULL;
  struct pse_allocator_t* alloc = NULL;
  const char* drv_filepath = NULL;
  if( !out_dev )
    return RES_BAD_ARG;

  params = params ? params : &PSE_DEVICE_PARAMS_NULL;
  if( !params->backend_drv_filepath )
    return RES_BAD_ARG;

  alloc = params->allocator ? params->allocator : &PSE_ALLOCATOR_DEFAULT;
  drv_filepath = params->backend_drv_filepath;

  dev = PSE_TYPED_ALLOC(alloc, struct pse_device_t);
  PSE_VERIFY_OR_ELSE(dev != NULL, res = RES_MEM_ERR; goto error);
  dev->allocator = alloc;
  dev->logger = params->logger;
  dev->cpspaces = NULL;
  dev->cpspaces_free = NULL;
  dev->drv = PSE_DRV_NULL;
  PSE_TRY_CALL_OR_GOTO(res,error, pseDriverLoad(dev, drv_filepath, &dev->drv));
  sb_reserve_more(dev->cpspaces, 8);
  sb_reserve_more(dev->cpspaces_free, 8);

  *out_dev = dev;

exit:
  return res;
error:
  sb_free(dev->cpspaces);
  sb_free(dev->cpspaces_free);
  PSE_CALL(pseDriverUnload(&dev->drv));
  dev->drv = PSE_DRV_NULL;
  PSE_FREE(alloc, dev);
  goto exit;
}

enum pse_res_t
pseDeviceDestroy
  (struct pse_device_t* dev)
{
  size_t i;
  if( !dev )
    return RES_BAD_ARG;

  /* Force destruction of all CPS -> we do not care about the ref counting! */
  for(i = 0; i < sb_count(dev->cpspaces); ++i) {
    if( dev->cpspaces[i] ) {
      pseConstrainedParameterSpaceDestroy(dev->cpspaces[i]);
    }
  }
  sb_free(dev->cpspaces);
  sb_free(dev->cpspaces_free);
  PSE_CALL(pseDriverUnload(&dev->drv));
  PSE_FREE(dev->allocator, dev);
  return RES_OK;
}

enum pse_res_t
pseDeviceCapacityIsManaged
  (struct pse_device_t* dev,
   enum pse_device_capacity_t cap,
   enum pse_type_t type)
{
  if( !dev )
    return RES_BAD_ARG;
  return dev->drv.is_capacity_managed(dev->drv.self, cap, type);
}
