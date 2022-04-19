#ifndef PSE_COLOR_API_H
#define PSE_COLOR_API_H

#include <pse_platform.h>

/*******************************************************************************
 * API visibility
 ******************************************************************************/

#if defined(PSE_COLOR_BUILD_SHARED)
  #define PSE_COLOR_API extern EXPORT_SYMBOL
#elif defined(PSE_COLOR_BUILD_STATIC)
  #define PSE_COLOR_API extern LOCAL_SYMBOL
#else
  #define PSE_COLOR_API extern IMPORT_SYMBOL
#endif
#define PSE_COLOR_INLINE_API static PSE_INLINE

#endif /* PSE_COLOR_API_H */
