#include "pse_drv.h"
#include "pse_device_p.h"

/******************************************************************************
 *
 * Helper functions
 *
 ******************************************************************************/

#if defined(PSE_OS_WINDOWS)

#include "Windows.h"

static PSE_INLINE pse_lib_handle_t
pseLibOpen
  (const char* filepath)
{
  assert(filepath);
  return (pse_lib_handle_t)LoadLibraryA(filepath);
}

static PSE_INLINE void*
pseLibSymbolGet
  (pse_lib_handle_t lib,
   const char* name)
{
  assert(lib && name);
  PSE_STATIC_ASSERT(sizeof(FARPROC) == sizeof(void*), Incompatible_type_size);
  return (void*)GetProcAddress((HMODULE)lib, name);
}

static PSE_INLINE enum pse_res_t
pseLibClose
  (pse_lib_handle_t lib)
{
  BOOL success = FALSE;
  assert(lib);
  success = FreeLibrary((HMODULE)lib);
  return success ? RES_OK : RES_INTERNAL;
}

static PSE_INLINE const char*
pseLibLastError()
{
  const DWORD errc = GetLastError();
  char* err = NULL;
  FormatMessageA
    (  FORMAT_MESSAGE_FROM_SYSTEM
     | FORMAT_MESSAGE_IGNORE_INSERTS
     | FORMAT_MESSAGE_ALLOCATE_BUFFER,
     NULL, errc,
     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
     err, 0, NULL);
  return err;
}

static PSE_FINLINE void
pseLibLastErrorFree
  (const char* err)
{
  LocalFree((char*)err);
}

#elif defined(PSE_OS_UNIX) || defined(PSE_OS_MACH)

#include <dlfcn.h>
#include <stdio.h>

static PSE_INLINE pse_lib_handle_t
pseLibOpen
  (const char* filepath)
{
  assert(filepath);
  return (pse_lib_handle_t)dlopen(filepath, RTLD_NOW|RTLD_GLOBAL);
}

static PSE_INLINE void*
pseLibSymbolGet
  (pse_lib_handle_t lib,
   const char* name)
{
  assert(lib && name);
  return dlsym((void*)lib, name);
}

static PSE_INLINE enum pse_res_t
pseLibClose
  (pse_lib_handle_t lib)
{
  assert(lib);
  return dlclose((void*)lib) != 0 ? RES_INTERNAL : RES_OK;
}

static PSE_INLINE const char*
pseLibLastError()
{
  const char* err = dlerror();
  return err;
}

static PSE_FINLINE void
pseLibLastErrorFree
  (const char* err)
{
  (void)err;
}

#endif

/******************************************************************************
 *
 * PUBLIC API
 *
 ******************************************************************************/

enum pse_res_t
pseDriverLoad
  (struct pse_device_t* dev,
   const char* filepath,
   struct pse_drv_t* drv)
{
  enum pse_res_t res = RES_OK;
  struct pse_drv_params_t drv_params = PSE_DRV_PARAMS_NULL;
  pse_drv_entrypoint_cb* entrypoint = NULL;
  if( !filepath || !drv )
    return RES_BAD_ARG;

  drv->lib = pseLibOpen(filepath);
  if( drv->lib == PSE_LIB_HANDLE_INVALID ) {
    const char* err = pseLibLastError();
    PSE_LOG(dev->logger, ERROR, "Cannot load driver: ");
    PSE_LOG(dev->logger, ERROR, filepath);
    PSE_LOG(dev->logger, ERROR, "\n");
    PSE_LOG(dev->logger, ERROR, err);
    pseLibLastErrorFree(err);
    res = RES_IO_ERR;
    goto error;
  }
  PSE_VERIFY_OR_ELSE
    (drv->lib != PSE_LIB_HANDLE_INVALID,
     res = RES_IO_ERR; goto error);

  entrypoint = pseLibSymbolGet(drv->lib, PSE_AS_CSTR(PSE_DRV_ENTRYPOINT_SYMBOL));
  PSE_VERIFY_OR_ELSE(entrypoint != NULL, res = RES_INVALID; goto error);

  drv_params.allocator = dev->allocator;
  drv_params.logger = dev->logger;
  PSE_CALL_OR_GOTO(res,error, (*entrypoint)(dev, &drv_params, drv));

exit:
  return res;
error:
  if( drv->lib ) {
    PSE_CALL(pseLibClose(drv->lib));
    drv->lib = PSE_LIB_HANDLE_INVALID;
  }
  goto exit;
}

enum pse_res_t
pseDriverUnload
  (struct pse_drv_t* drv)
{
  enum pse_res_t res = RES_OK;
  if( !drv )
    return RES_BAD_ARG;
  if( !drv->lib )
    return RES_OK;

  if( drv->clean ) {
    PSE_CALL_OR_GOTO(res,exit, drv->clean(drv->self));
  }
  PSE_CALL_OR_GOTO(res,exit, pseLibClose(drv->lib));
  drv->lib = PSE_LIB_HANDLE_INVALID;

exit:
  return res;
}
