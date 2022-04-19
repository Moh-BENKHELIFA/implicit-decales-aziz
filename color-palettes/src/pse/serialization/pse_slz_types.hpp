#ifndef SERIALIZATION_TYPES_HPP
#define SERIALIZATION_TYPES_HPP

#include <pse/pse_types.hpp>
#include <pse/pse_factory.hpp>

#include <cassert>
#include <string>

//! \brief Serialization return code for error management
enum ESRet {
  ESRet_OK,
  ESRet_Unavailable,
  ESRet_OutOfMemory,
  ESRet_IOErr,
  ESRet_BadArg,
  ESRet_Invalid,
  ESRet_NotImplemented,
  ESRet_AlreadyExists,
  ESRet_NotFound
};

//! \brief Serialization
enum ESKind {
  ESKind_Unknown,

  ESKind_Write,
  ESKind_Read,

  ESKind_Serialize = ESKind_Write,
  ESKind_Deserialize = ESKind_Read
};

//! \brief All built-in types managed natively by serializers
#define SERIALIZABLE_BUILTIN_TYPES                                             \
  STYPE(bool, Bool)                                                            \
  STYPE(int8_t, Int8)                                                          \
  STYPE(int16_t, Int16)                                                        \
  STYPE(int32_t, Int32)                                                        \
  STYPE(int64_t, Int64)                                                        \
  STYPE(uint8_t, UInt8)                                                        \
  STYPE(uint16_t, UInt16)                                                      \
  STYPE(uint32_t, UInt32)                                                      \
  STYPE(uint64_t, UInt64)                                                      \
  STYPE(float, Float)                                                          \
  STYPE(double, Double)

//! \brief Serialization function prototype for typed objects serialize
//! function.
typedef ESRet
(*SerializeFunction)
  (class Serializer& s,
   void* typed_obj_ptr);

//! \note Use the same type as the factories to stay compatible
typedef FactoryTypeId SerializableTypedObjectId;

#endif /* SERIALIZATION_TYPES_HPP */
