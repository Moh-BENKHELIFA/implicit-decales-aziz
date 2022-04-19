#ifndef SERIALIZATION_SERIALIZER_HPP
#define SERIALIZATION_SERIALIZER_HPP

#include "pse_slz_types.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

//! /brief Root class for all serializer.
//!
//! A serializer must implement the virtual interface of this class and is
//! responsible to provide initialization and clean functionalities if needed.
//! It will be up to the user to call these functions before using the
//! serializer.
class Serializer {
public:
  Serializer
    (const std::size_t version)
    : _version(version)
    , _kind(ESKind_Unknown)
  {}
  virtual ~Serializer() {}

  inline bool isReading() const { return _kind == ESKind_Read; }
  inline bool isWriting() const { return _kind == ESKind_Write; }

public:
  //! \brief Add a factory that will be used during typed object deserialization
  //! to allocate a new object of the serializerd type id.
  inline ESRet addFactory(Factory* f);
  inline ESRet removeFactory(Factory* f);

  //! \brief Register a serialize function associated to the given typed object
  //! type id.
  inline ESRet
  registerTypedObjectSerializeFunc
    (const SerializableTypedObjectId id,
     SerializeFunction func);
  inline ESRet
  unregisterTypedObjectSerializeFunc
    (const SerializableTypedObjectId type_id);
  inline SerializeFunction
  typedObjectSerializeFuncGet
    (const SerializableTypedObjectId type_id) const;

  //! \brief Push a context during serialization/deserialization process,
  //! allowing the caller to get back this specific context until it calls the
  //! \c popContext function.
  inline ESRet pushContext(void* ctxt);
  inline ESRet popContext();

  //! \brief Get the last pushed context, casted to the given type.
  template<typename T>
  inline T*
  currentContext() const;

  //! \brief Serialize the serializer header.
  //!
  //! The header allow the serializer to store/restore its version and other
  //! internal stuff like its kind. This allow to manage retrocompatibility of
  //! the serialized data.
  virtual ESRet header();

  //! \brief Begin a map, i.e. a list of key->value association
  virtual ESRet
  beginMap
    (const std::string& name) = 0;
  virtual ESRet
  endLastMap() = 0;

  //! \brief Begin an array of value, i.e. a list of values
  virtual ESRet
  beginArray
    (const std::string& name,
     size_t& count) = 0; //!< filled on read
  virtual ESRet
  beginArrayItem
    (const std::string& name) = 0;
  virtual ESRet
  endLastArrayItem() = 0;
  virtual ESRet
  endLastArray() = 0;

  //! \brief Begin a specific kind of map, that is a typed object that can be
  //! specificaly serialized using the registered functions and that can also
  //! be allocated using the factories.
  virtual ESRet
  beginTypedObject
    (const std::string& obj_name,
     std::size_t& type_id) = 0; //!< filled on read, to be used for allocation
  virtual ESRet
  endTypedObject() = 0;

  //! \brief Declare all basic serialization functions for built-in types.
  #define STYPE(type,name)                                                     \
  virtual ESRet                                                                \
  value##name                                                                  \
    (const std::string& value_name,                                            \
     type& value) = 0;
  SERIALIZABLE_BUILTIN_TYPES
  #undef STYPE

  //! \brief Allocated a typed object using the serializer factories.
  template<typename T>
  inline ESRet
  allocateTypedObject
    (const SerializableTypedObjectId id,
     T*& ptr);

protected:
  using TypedObjectSerializeFunctionMap =
    std::map<SerializableTypedObjectId, SerializeFunction>;

  size_t _version;
  ESKind _kind;
  std::vector<Factory*> _factories;
  std::vector<void*> _contexts;
  TypedObjectSerializeFunctionMap _typed_objects_serialize_func;

};

template<typename T>
inline ESRet
serializeTypedObject
  (Serializer& s,
   const std::string& value_name,
   T& value);

template<typename T>
inline ESRet
serializeTypedObjectPtr
  (Serializer& s,
   const std::string& value_name,
   T*& value);

template<typename T>
inline ESRet
serializeTypedObjectPtr
  (Serializer& s,
   const std::string& value_name,
   std::shared_ptr<T>& value);

//! \brief Just implement the generic serialization function \c serializeValue
//! for all built-in types.
#define STYPE(type,name)                                                       \
inline ESRet                                                                   \
serializeValue                                                                 \
  (Serializer& s,                                                              \
   const std::string& value_name,                                              \
   type& value)                                                                \
{ return s.value##name(value_name, value); }
SERIALIZABLE_BUILTIN_TYPES
#undef STYPE

#include <pse/serialization/pse_serializer.inl>

#endif /* SERIALIZATION_SERIALIZER_HPP */
