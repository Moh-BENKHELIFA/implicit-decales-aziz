#ifndef PSE_SLZ_API_H
#define PSE_SLZ_API_H

#include <pse_platform.h>

/*******************************************************************************
 * API visibility
 ******************************************************************************/

#if defined(PSE_SLZ_BUILD_SHARED)
  #define PSE_SLZ_API extern EXPORT_SYMBOL
#elif defined(PSE_SLZ_BUILD_STATIC)
  #define PSE_SLZ_API extern LOCAL_SYMBOL
#else
  #define PSE_SLZ_API extern IMPORT_SYMBOL
#endif
#define PSE_SLZ_INLINE_API static PSE_INLINE

#endif /* PSE_SLZ_API_H */
