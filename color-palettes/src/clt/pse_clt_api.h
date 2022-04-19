#ifndef PSE_CLT_API_H
#define PSE_CLT_API_H

#include <pse_platform.h>

/*******************************************************************************
 * API visibility
 ******************************************************************************/

#if defined(PSE_CLT_BUILD_SHARED)
  #define PSE_CLT_API extern EXPORT_SYMBOL
#elif defined(PSE_CLT_BUILD_STATIC)
  #define PSE_CLT_API extern LOCAL_SYMBOL
#else
  #define PSE_CLT_API extern IMPORT_SYMBOL
#endif
#define PSE_CLT_INLINE_API static PSE_INLINE

#endif /* PSE_CLT_API_H */
