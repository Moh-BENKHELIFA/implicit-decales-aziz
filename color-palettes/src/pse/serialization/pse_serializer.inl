#ifndef SERIALIZATION_SERIALIZER_INL
#define SERIALIZATION_SERIALIZER_INL

#include <pse/serialization/pse_serializer.hpp>
#include <pse/serialization/pse_slz_utils.hpp>

#include <memory>
#include <algorithm>

inline ESRet
Serializer::addFactory
  (Factory* f)
{
  CHECK_OR_DO
    (std::find(_factories.begin(), _factories.end(), f) == _factories.end(),
     return ESRet_AlreadyExists);
  _factories.push_back(f);
  return ESRet_OK;
}

inline ESRet
Serializer::removeFactory
  (Factory* f)
{
  auto it = std::find(_factories.begin(), _factories.end(), f);
  CHECK_OR_DO(it != _factories.end(), return ESRet_NotFound);
  _factories.erase(it);
  return ESRet_OK;
}

inline ESRet
Serializer::registerTypedObjectSerializeFunc
  (const SerializableTypedObjectId type_id,
   SerializeFunction func)
{
  if( func == nullptr )
    return ESRet_BadArg;
  if( _typed_objects_serialize_func.count(type_id) > 0 )
    return ESRet_AlreadyExists;
  _typed_objects_serialize_func[type_id] = func;
  return ESRet_OK;
}

inline ESRet
Serializer::unregisterTypedObjectSerializeFunc
  (const std::size_t type_id)
{
  if( _typed_objects_serialize_func.erase(type_id) == 0 )
    return ESRet_NotFound;
  return ESRet_OK;
}

inline SerializeFunction
Serializer::typedObjectSerializeFuncGet
  (const SerializableTypedObjectId type_id) const
{
  if( _typed_objects_serialize_func.count(type_id) == 0 )
    return nullptr;
  return _typed_objects_serialize_func.at(type_id);
}

inline ESRet
Serializer::pushContext
  (void* ctxt)
{
  _contexts.push_back(ctxt);
  return ESRet_OK;
}

template<typename T>
inline T*
Serializer::currentContext() const
{
  if( _contexts.empty() )
    return nullptr;
  return static_cast<T*>(_contexts.back());
}

inline ESRet
Serializer::popContext()
{
  if( !_contexts.empty() )
    return ESRet_Invalid;
  _contexts.pop_back();
  return ESRet_OK;
}

ESRet
Serializer::header()
{
  if(_kind == ESKind_Unknown)
    return ESRet_Unavailable;

  ESRet ret = ESRet_OK;
  uint64_t version = (uint64_t)_version;
  SCALL_OR_RETURN(ret, beginMap("header"));
  SCALL_OR_RETURN(ret, valueUInt64("version", version));
  SCALL_OR_RETURN(ret, endLastMap());
  _version = (size_t)version;
  return ret;
}

//! \brief Allocated a typed object using the serializer factories.
template<typename T>
inline ESRet
Serializer::allocateTypedObject
  (const SerializableTypedObjectId type_id,
   T*& ptr)
{
  ESRet ret = ESRet_OK;
  T* p = nullptr;
  for(auto f: _factories) {
    auto it = f->find(type_id);
    if( it == f->end() )
      continue;
    p = static_cast<T*>(it->second());
    CHECK_OR_DO
      (p != nullptr,
       ret = ESRet_OutOfMemory; goto error);
    CHECK_OR_DO
      (p->getTypeHash() == type_id,
       ret = ESRet_Invalid; goto error);
  }
  CHECK_OR_DO
    (p != nullptr,
     ret = ESRet_NotFound; goto error);
  ptr = p;
exit:
  return ret;
error:
  delete p;
  goto exit;
}

template<typename T>
inline ESRet
serializeTypedObject
  (Serializer& s,
   const std::string& value_name,
   T& value)
{
  // NOTE: we cannot rely on the T::getClassTypeHash() as it may be a base class.
  // TODO: try to implement a isBaseClassOf to check that T is a base class of
  // the deserialized type_id
  ESRet ret = ESRet_OK;
  SerializableTypedObjectId type_id = value.getTypeHash();
  SCALL_OR_RETURN(ret, s.beginTypedObject(value_name, type_id));
  CHECK_OR_DO(type_id == value.getTypeHash(), return ESRet_Invalid);
  SerializeFunction f = s.typedObjectSerializeFuncGet(type_id);
  CHECK_OR_DO(f != nullptr, return ESRet_Unavailable);
  SCALL_OR_RETURN(ret, f(s, static_cast<void*>(&value)));
  return s.endTypedObject();
}

template<typename T>
inline ESRet
serializeTypedObjectPtr
  (Serializer& s,
   const std::string& value_name,
   T*& value)
{
  // NOTE: we cannot rely on the T::getClassTypeHash() as it may be a base class.
  // TODO: try to implement a isBaseClassOf to check that T is a base class of
  // the deserialized type_id

  ESRet ret = ESRet_OK;
  SerializableTypedObjectId type_id;
  if( s.isWriting() ) {
    type_id = value->getTypeHash();
  }
  SCALL_OR_RETURN(ret, s.beginTypedObject(value_name, type_id));
  if( s.isReading() )
    SCALL_OR_RETURN(ret, s.allocateTypedObject<T>(type_id, value));
  SerializeFunction f = s.typedObjectSerializeFuncGet(type_id);
  CHECK_OR_DO(f != nullptr, return ESRet_Unavailable);
  SCALL_OR_RETURN(ret, f(s, static_cast<void*>(value)));
  return s.endTypedObject();
}

template<typename T>
inline ESRet
serializeTypedObjectPtr
  (Serializer& s,
   const std::string& value_name,
   std::shared_ptr<T>& value)
{
  ESRet ret = ESRet_OK;
  T* p = s.isWriting() ? value.get() : nullptr;
  SCALL_OR_RETURN(ret, serializeTypedObjectPtr<T>(s, value_name, p));
  if( s.isReading() )
    value = std::shared_ptr<T>(p);
  return ret;
}

#endif /* SERIALIZATION_SERIALIZER_INL */
