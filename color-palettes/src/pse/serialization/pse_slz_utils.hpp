#ifndef SERIALIZE_UTILS_HPP
#define SERIALIZE_UTILS_HPP

#include <pse/serialization/pse_slz_types.hpp>

class QXmlStreamReader;
class QXmlStreamWriter;
class QString;

// FIXME: to remove and to transform into something that is external of the
// classes that want serialization mechanism.
#define INTERPOLIB_SERIALIZATION_INTERFACE()                                   \
    virtual void serialize(class QXmlStreamWriter& xml) const = 0;             \
    virtual bool deserialize(class QXmlStreamReader& xml,                      \
                             const class QString& filename) = 0;

#define SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER(s,Type,Func)           \
  s.registerTypedObjectSerializeFunc                                           \
    (Type::getClassTypeHash(), static_cast<SerializeFunction>((Func)))

//! \brief Check a predicate and go to label when it is evaluated to false.
#define CHECK_OR_DO(truth, code)                                               \
  if(!(truth)) { code; } (void)0

//! \brief Do a call to a function returning a ESRet value and go to label when
//! it is not evaluate to ESRet_OK.
#define SCALL_OR_GOTO(ret,label,call)                                          \
  CHECK_OR_DO(((ret) = (call)) == ESRet_OK, assert(false); goto label)

//! \brief Do a call to a function returning a ESRet value and return it when it
//! is not evaluate to ESRet_OK.
#define SCALL_OR_RETURN(ret,call)                                              \
  CHECK_OR_DO(((ret) = (call)) == ESRet_OK, assert(false); return (ret))

//! \brief Do a call to a function returning a ESRet and assert if it is not
//! evaluate to ESRet_OK. Dev tool to ensure that a function call will not fail.
#define SCALL(call)                                                            \
  CHECK_OR_DO((call) == ESRet_OK, assert(false))

//! \brief Macros used to simplify the declaration of a new specialization of
//! the serialize function for a typed object.
#define SERIALIZER_TYPED_OBJECT_BEGIN(FuncName, Type, VarName)                 \
inline ESRet                                                                   \
FuncName                                                                       \
  (Serializer& s,                                                              \
   void* typed_obj_voidptr__)                                                  \
{                                                                              \
  ESRet ret = ESRet_OK;                                                        \
  Type* typed_obj_typedptr__ = static_cast<Type*>(typed_obj_voidptr__);        \
  Type& VarName = *typed_obj_typedptr__;                                       \
  {

//! \brief Macros used to simplify the declaration of a new specialization of
//! the serialize function for a given value type.
#define SERIALIZER_CUSTOM_VALUE_BEGIN(Type, VarName)                           \
inline ESRet                                                                   \
serializeValue                                                                 \
  (Serializer& s,                                                              \
   const std::string& value_name,                                              \
   Type& VarName)                                                              \
{                                                                              \
  ESRet ret = ESRet_OK;                                                        \
  {                                                                            \
    if( !value_name.empty() )                                                  \
      SERIALIZE_MAP_BEGIN(value_name);
#define SERIALIZE_INTERRUPT_IF_NOT(Cond, Error)                                \
    CHECK_OR_DO((Cond), ret = (Error); goto exit)
#define SERIALIZE_VALUE(Name, Var)                                             \
    SCALL_OR_GOTO(ret,exit, PARAM__(serializeValue(s, Name, Var)))
#define SERIALIZE_VALUE_INLINE(Var)                                            \
    SERIALIZE_VALUE("", Var)
#define SERIALIZE_VALUE_AS(Name, Var, AsType)                                  \
    { AsType tmp__ = static_cast<AsType>(Var);                                 \
      SCALL_OR_GOTO(ret,exit, serializeValue(s, Name, tmp__));                 \
      Var = static_cast<decltype(Var)>(tmp__);                                 \
    } (void)0
#define SERIALIZE_TYPED_OBJECT_CUSTOM_INLINE(SerializeFunc, Var)               \
    SCALL_OR_GOTO(ret,exit, PARAM__(SerializeFunc(s, Var)))
#define SERIALIZE_TYPED_OBJECT(Name, Var)                                      \
    SCALL_OR_GOTO(ret,exit, serializeTypedObject(s, Name, Var))
#define SERIALIZE_TYPED_OBJECT_PTR(Name, Var)                                  \
    SCALL_OR_GOTO(ret,exit, serializeTypedObjectPtr(s, Name, Var))
#define SERIALIZE_MAP_BEGIN(Name)                                              \
    SCALL_OR_GOTO(ret,exit, s.beginMap(Name))
#define SERIALIZE_MAP_END(Name)                                                \
    SCALL_OR_GOTO(ret,exit, s.endLastMap())
#define SERIALIZE_ARRAY_BEGIN(Name, Count)                                     \
    SCALL_OR_GOTO(ret,exit, s.beginArray(Name, Count))
#define SERIALIZE_ARRAY_END(Name)                                              \
    SCALL_OR_GOTO(ret,exit, s.endLastArray())
#define SERIALIZE_ARRAY_ITEM_BEGIN(Name)                                       \
    SCALL_OR_GOTO(ret,exit, s.beginArrayItem(Name));
#define SERIALIZE_ARRAY_ITEM_END(Name)                                         \
    SCALL_OR_GOTO(ret,exit, s.endLastArrayItem())
#define SERIALIZER_TYPED_OBJECT_END()                                          \
  }                                                                            \
exit:                                                                          \
  return ret;                                                                  \
}
#define SERIALIZER_CUSTOM_VALUE_END()                                          \
    if( !value_name.empty() )                                                  \
      SERIALIZE_MAP_END(value_name);                                           \
  }                                                                            \
exit:                                                                          \
  return ret;                                                                  \
}

#define SERIALIZER_TYPED_OBJECT_SAME_AS(FuncName, Type, AsFuncName)            \
  SERIALIZER_TYPED_OBJECT_BEGIN(PARAM__(FuncName), PARAM__(Type), v)           \
    SERIALIZE_TYPED_OBJECT_CUSTOM_INLINE(PARAM__(AsFuncName), &v);             \
  SERIALIZER_TYPED_OBJECT_END()

//! \brief Used to ensure that macro parameters will not be splitted because of
//! comas.
#define PARAM__(...)  __VA_ARGS__
#define TEMPLATE_ARGS(...) PARAM__(__VA_ARGS__)

#endif // SERIALIZE_UTILS_HPP
