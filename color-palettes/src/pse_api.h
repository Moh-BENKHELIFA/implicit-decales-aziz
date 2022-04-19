#ifndef PSE_API_H
#define PSE_API_H

#include "pse_platform.h"

/*******************************************************************************
 * API visibility
 ******************************************************************************/

#if defined(PSE_BUILD_SHARED)
  #define PSE_API extern EXPORT_SYMBOL
#elif defined(PSE_BUILD_STATIC)
  #define PSE_API extern LOCAL_SYMBOL
#else
  #define PSE_API extern IMPORT_SYMBOL
#endif
#define PSE_INLINE_API static PSE_INLINE

#endif /* PSE_API_H */
