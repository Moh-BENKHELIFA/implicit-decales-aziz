#ifndef PSE_SLZ_TYPES_H
#define PSE_SLZ_TYPES_H

#include "pse_slz_api.h"

PSE_API_BEGIN

/******************************************************************************
 *
 * PUBLIC TYPES
 *
 ******************************************************************************/

#define PSE_SLZ_BUILTIN_TYPES                                                  \
  PSE_SLZ_TYPE(bool, Bool)                                                     \
  PSE_SLZ_TYPE(int8_t, Int8)                                                   \
  PSE_SLZ_TYPE(int16_t, Int16)                                                 \
  PSE_SLZ_TYPE(int32_t, Int32)                                                 \
  PSE_SLZ_TYPE(int64_t, Int64)                                                 \
  PSE_SLZ_TYPE(uint8_t, UInt8)                                                 \
  PSE_SLZ_TYPE(uint16_t, UInt16)                                               \
  PSE_SLZ_TYPE(uint32_t, UInt32)                                               \
  PSE_SLZ_TYPE(uint64_t, UInt64)                                               \
  PSE_SLZ_TYPE(float, Float)                                                   \
  PSE_SLZ_TYPE(double, Double)

PSE_API_END

#endif /* PSE_SLZ_TYPES_H */
