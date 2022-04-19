#ifndef PSE_EIGEN_API_H
#define PSE_EIGEN_API_H

#include <pse_platform.h>

/*******************************************************************************
 * API visibility
 ******************************************************************************/

#if defined(PSE_EIGEN_BUILD_SHARED)
  #define PSE_EIGEN_API extern EXPORT_SYMBOL
#elif defined(PSE_EIGEN_BUILD_STATIC)
  #define PSE_EIGEN_API extern LOCAL_SYMBOL
#else
  #define PSE_EIGEN_API extern IMPORT_SYMBOL
#endif
#define PSE_EIGEN_INLINE_API static PSE_INLINE

#endif /* PSE_EIGEN_API_H */
