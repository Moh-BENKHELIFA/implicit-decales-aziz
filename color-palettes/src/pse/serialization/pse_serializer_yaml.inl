#ifndef SERIALIZATION_YAML_SERIALIZER_INL
#define SERIALIZATION_YAML_SERIALIZER_INL

#include <inttypes.h>
#include <float.h>

#ifdef COMPILER_CL
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

#define YAML_THIS_TAG             "!cpp/plx"
#define YAML_TYPEID_PRINTF_SCANF_FORMAT YAML_THIS_TAG "-type:%zu"
#define YAML_COUNT_PRINTF_SCANF_FORMAT  YAML_THIS_TAG "-count:%zu"

#define YCHECK_OR_INVALID(ret,label,truth)                                     \
  CHECK_OR_DO((truth), (ret) = ESRet_Invalid; goto label)
#define YCALL_OR_INVALID(ret,label,call)                                       \
  YCHECK_OR_INVALID(ret,label,(call))
#define YPARSE(ret,label,event,expected)                                       \
  YCALL_OR_INVALID(ret,label, yaml_parser_parse(&_stream.read.parser, (event)));\
  YCHECK_OR_INVALID(ret,label, (event)->type == (expected))
#define YEMIT(ret,label,event)                                                 \
  if(!yaml_emitter_emit(&_stream.write.emitter, (event))) {                    \
    /* On failure, that may mean that we need to flush */                      \
    YCALL_OR_INVALID(ret,label, yaml_emitter_flush                             \
      (&_stream.write.emitter));                                               \
    YCALL_OR_INVALID(ret,label, yaml_emitter_emit                              \
      (&_stream.write.emitter, (event)));                                      \
  } (void)0

namespace internal {

static inline int
CStrToBool
  (const char* str, bool& value)
{
  if(  (0 != strncasecmp("true", str, 4))
    && (0 != strncasecmp("false", str, 5)) )
    return 0;
  value = (0 == strncasecmp("true", str, 4));
  return 1;
}
static inline int
BoolToCStr
  (const bool value, char* str, size_t str_max_length)
{ return snprintf(str, str_max_length, "%s", value ? "true" : "false"); }

#define CSTR_TO_FROM_INT_VALUE(TypeName, Type, ScnFmt, PriFmt)                 \
static inline int                                                              \
CStrTo ## TypeName                                                             \
  (const char* str, Type& value)                                               \
{ return sscanf(str, "%" ScnFmt, &value); }                                    \
static inline int                                                              \
TypeName ## ToCStr                                                             \
  (const Type& value, char* str, size_t str_max_length)                        \
{ return snprintf(str, str_max_length, "%" PriFmt, value); }

CSTR_TO_FROM_INT_VALUE(UInt8,  uint8_t,  SCNu8,  PRIu8)
CSTR_TO_FROM_INT_VALUE(UInt16, uint16_t, SCNu16, PRIu16)
CSTR_TO_FROM_INT_VALUE(UInt32, uint32_t, SCNu32, PRIu32)
CSTR_TO_FROM_INT_VALUE(UInt64, uint64_t, SCNu64, PRIu64)
CSTR_TO_FROM_INT_VALUE(Int8,   int8_t,   SCNd8,  PRId8)
CSTR_TO_FROM_INT_VALUE(Int16,  int16_t,  SCNd16, PRId16)
CSTR_TO_FROM_INT_VALUE(Int32,  int32_t,  SCNd32, PRId32)
CSTR_TO_FROM_INT_VALUE(Int64,  int64_t,  SCNd64, PRId64)

#define CSTR_TO_FROM_FLT_VALUE(TypeName, Type, ScnFmt, PriFmt, Precision)      \
static inline int                                                              \
CStrTo ## TypeName                                                             \
  (const char* str, Type& value)                                               \
{ return sscanf(str, "%" ScnFmt, &value); }                                    \
static inline int                                                              \
TypeName ## ToCStr                                                             \
  (const Type& value, char* str, size_t str_max_length)                        \
{ return snprintf(str, str_max_length, "%" PriFmt, Precision, value); }

CSTR_TO_FROM_FLT_VALUE(Float,  float,  "g",  ".*g", FLT_DIG)
CSTR_TO_FROM_FLT_VALUE(Double, double, "lg", ".*g", DBL_DIG)

template<typename T>
static inline ESRet
valueWriteToCString
  (const T& value, char* str, int& str_length, size_t str_max_length);

template<typename T>
static inline ESRet
valueReadFromCString
  (const char* str, const size_t str_length, T& value);

#define STYPE(Type, TypeName)                                                  \
template<> inline ESRet                                                        \
valueWriteToCString                                                            \
  (const Type& value, char* str, int& str_length, size_t str_max_length)       \
{                                                                              \
  (void)str_length;                                                            \
  str_length = TypeName ## ToCStr(value, str, str_max_length);                 \
  return (size_t)str_length < str_max_length ? ESRet_OK : ESRet_Invalid;       \
}                                                                              \
template<> inline ESRet                                                        \
valueReadFromCString                                                           \
  (const char* str, const size_t str_length, Type& value)                      \
{                                                                              \
  (void)str_length;                                                            \
  return 1 == CStrTo ## TypeName(str, value)                                   \
    ? ESRet_OK                                                                 \
    : ESRet_Invalid;                                                           \
}
SERIALIZABLE_BUILTIN_TYPES
#undef STYPE

}

ESRet
SerializerYAML::beginStream
  (const std::string& stream)
{
  if( _kind != ESKind_Unknown )
    return ESRet_Unavailable; // already in a serialization
  if( !yaml_parser_initialize(&_stream.read.parser) )
    return ESRet_OutOfMemory;

  ESRet ret = ESRet_OK;
  _kind = ESKind_Read;
  _stream.read.buf = &stream;
  _stream.read.buf_curr_pos = 0;
  yaml_parser_set_input
    (&_stream.read.parser,
     SerializerYAML::read_from_std_string,
     &_stream.read);

  YPARSE(ret,error, &_event, YAML_STREAM_START_EVENT);
  yaml_event_delete(&_event);
  YPARSE(ret,error, &_event, YAML_DOCUMENT_START_EVENT);
  yaml_event_delete(&_event);
  YPARSE(ret,error, &_event, YAML_MAPPING_START_EVENT);
  yaml_event_delete(&_event);

  SCALL_OR_GOTO(ret,error, header());

exit:
  return ret;
error:
  _kind = ESKind_Unknown;
  _stream.read.buf = NULL;
  yaml_parser_delete(&_stream.read.parser);
  goto exit;
}

ESRet
SerializerYAML::beginStream
  (std::string& stream)
{
  if( _kind != ESKind_Unknown )
    return ESRet_Unavailable; // already in a serialization
  if( !yaml_emitter_initialize(&_stream.write.emitter) )
    return ESRet_OutOfMemory;

  ESRet ret = ESRet_OK;
  _kind = ESKind_Write;
  _stream.write.buf = &stream;
  yaml_emitter_set_output
    (&_stream.write.emitter,
     SerializerYAML::write_to_std_string,
     &_stream.write);

  // Start the YAML stream
  YCALL_OR_INVALID(ret,error, yaml_stream_start_event_initialize
    (&_event, YAML_UTF8_ENCODING));
  YEMIT(ret,error, &_event);
  YCALL_OR_INVALID(ret,error, yaml_document_start_event_initialize
    (&_event, NULL, NULL, NULL, 1));
  YEMIT(ret,error, &_event);
  YCALL_OR_INVALID(ret,exit, yaml_mapping_start_event_initialize
    (&_event, NULL, (yaml_char_t*)YAML_MAP_TAG,
     1, YAML_ANY_MAPPING_STYLE));
  YEMIT(ret,exit, &_event);

  SCALL_OR_GOTO(ret,error, header());

exit:
  return ret;
error:
  _kind = ESKind_Unknown;
  _stream.write.buf = NULL;
  yaml_emitter_delete(&_stream.write.emitter);
  goto exit;
}

ESRet
SerializerYAML::endStream()
{
  if( _kind == ESKind_Unknown )
    return ESRet_Invalid; // not in a serialization

  ESRet ret = ESRet_OK;
  if( _kind == ESKind_Write ) {
    YCALL_OR_INVALID(ret,exit, yaml_mapping_end_event_initialize(&_event));
    YEMIT(ret,error, &_event);
    YCALL_OR_INVALID(ret,error, yaml_document_end_event_initialize(&_event, 1));
    YEMIT(ret,error, &_event);
    YCALL_OR_INVALID(ret,error, yaml_stream_end_event_initialize(&_event));
    YEMIT(ret,error, &_event);
    yaml_emitter_delete(&_stream.write.emitter);
  } else {
    YPARSE(ret,exit, &_event, YAML_MAPPING_END_EVENT);
    yaml_event_delete(&_event);
    YPARSE(ret,error, &_event, YAML_DOCUMENT_END_EVENT);
    yaml_event_delete(&_event);
    YPARSE(ret,error, &_event, YAML_STREAM_END_EVENT);
    yaml_event_delete(&_event);
    yaml_parser_delete(&_stream.read.parser);
  }
  _kind = ESKind_Unknown;

exit:
  return ret;
error:
  goto exit;
}

ESRet
SerializerYAML::beginMap
  (const std::string& name)
{
  if( name.empty() )
    return ESRet_BadArg;

  ESRet ret = ESRet_OK;
  if( _kind == ESKind_Write ) {
    YCALL_OR_INVALID(ret,exit, yaml_scalar_event_initialize
      (&_event, NULL, (yaml_char_t*)YAML_STR_TAG,
       (yaml_char_t*)name.c_str(), -1,
       1, 1, YAML_PLAIN_SCALAR_STYLE));
    YEMIT(ret,exit, &_event);
    YCALL_OR_INVALID(ret,exit, yaml_mapping_start_event_initialize
      (&_event, NULL, (yaml_char_t*)YAML_MAP_TAG,
       1, YAML_ANY_MAPPING_STYLE));
    YEMIT(ret,exit, &_event);
  } else {
    YPARSE(ret,exit, &_event, YAML_SCALAR_EVENT);
    YCHECK_OR_INVALID(ret,exit, strcmp
      ((const char*)_event.data.scalar.value,
       name.c_str()) == 0);
    yaml_event_delete(&_event);
    YPARSE(ret,exit, &_event, YAML_MAPPING_START_EVENT);
    yaml_event_delete(&_event);
  }

exit:
  return ret;
}

ESRet
SerializerYAML::endLastMap()
{
  ESRet ret = ESRet_OK;
  if( _kind == ESKind_Write ) {
    YCALL_OR_INVALID(ret,exit, yaml_mapping_end_event_initialize(&_event));
    YEMIT(ret,exit, &_event);
  } else {
    YPARSE(ret,exit, &_event, YAML_MAPPING_END_EVENT);
    yaml_event_delete(&_event);
  }

exit:
  return ret;
}



ESRet
SerializerYAML::beginArray
  (const std::string& name,
   size_t& count)
{
  if( name.empty() )
    return ESRet_BadArg;

  ESRet ret = ESRet_OK;
  if( _kind == ESKind_Write ) {
    static char tmp_tag[64];
    snprintf(tmp_tag, 64, YAML_COUNT_PRINTF_SCANF_FORMAT, count);
    YCALL_OR_INVALID(ret,exit, yaml_scalar_event_initialize
      (&_event, NULL, (yaml_char_t*)tmp_tag,
       (yaml_char_t*)name.c_str(), -1,
       0, 0, YAML_PLAIN_SCALAR_STYLE));
    YEMIT(ret,exit, &_event);
    YCALL_OR_INVALID(ret,exit, yaml_sequence_start_event_initialize
      (&_event, NULL, (yaml_char_t*)YAML_SEQ_TAG,
       1, YAML_ANY_SEQUENCE_STYLE));
    YEMIT(ret,exit, &_event);
  } else {
    YPARSE(ret,exit, &_event, YAML_SCALAR_EVENT);
    YCHECK_OR_INVALID(ret,exit, strcmp
      ((const char*)_event.data.scalar.value,
       name.c_str()) == 0);
    YCHECK_OR_INVALID(ret,exit, sscanf
      ((const char*)_event.data.scalar.tag,
       YAML_COUNT_PRINTF_SCANF_FORMAT, &count) == 1);
    yaml_event_delete(&_event);
    YPARSE(ret,exit, &_event, YAML_SEQUENCE_START_EVENT);
    yaml_event_delete(&_event);
  }

exit:
  return ret;
}

ESRet
SerializerYAML::beginArrayItem
  (const std::string& name)
{
  if( name.empty() )
    return ESRet_BadArg;

  //! \note We do not use the name in YAML
  ESRet ret = ESRet_OK;
  if( _kind == ESKind_Write ) {
    YCALL_OR_INVALID(ret,exit, yaml_mapping_start_event_initialize
      (&_event, NULL, (yaml_char_t*)YAML_MAP_TAG,
       1, YAML_ANY_MAPPING_STYLE));
    YEMIT(ret,exit, &_event);
  } else {
    YPARSE(ret,exit, &_event, YAML_MAPPING_START_EVENT);
    yaml_event_delete(&_event);
  }

exit:
  return ret;
}

ESRet
SerializerYAML::endLastArrayItem()
{
  ESRet ret = ESRet_OK;

  //! \note We do not use the name in YAML
  if( _kind == ESKind_Write ) {
    YCALL_OR_INVALID(ret,exit, yaml_mapping_end_event_initialize(&_event));
    YEMIT(ret,exit, &_event);
  } else {
    YPARSE(ret,exit, &_event, YAML_MAPPING_END_EVENT);
    yaml_event_delete(&_event);
  }

exit:
  return ret;
}

ESRet
SerializerYAML::endLastArray()
{
  ESRet ret = ESRet_OK;
  if( _kind == ESKind_Write ) {
    YCALL_OR_INVALID(ret,exit, yaml_sequence_end_event_initialize(&_event));
    YEMIT(ret,exit, &_event);
  } else {
    YPARSE(ret,exit, &_event, YAML_SEQUENCE_END_EVENT);
    yaml_event_delete(&_event);
  }

exit:
  return ret;
}

ESRet
SerializerYAML::beginTypedObject
  (const std::string& obj_name,
   size_t& type_id)
{
  if( obj_name.empty() )
    return ESRet_BadArg;

  ESRet ret = ESRet_OK;
  if( _kind == ESKind_Write ) {
    static char tmp_tag[64];
    snprintf(tmp_tag, 64, YAML_TYPEID_PRINTF_SCANF_FORMAT, type_id);
    YCALL_OR_INVALID(ret,exit, yaml_scalar_event_initialize
      (&_event, NULL, (yaml_char_t*)tmp_tag,
       (yaml_char_t*)obj_name.c_str(), -1,
       0, 0, YAML_PLAIN_SCALAR_STYLE));
    YEMIT(ret,exit, &_event);
    YCALL_OR_INVALID(ret,exit, yaml_mapping_start_event_initialize
      (&_event, NULL, (yaml_char_t*)YAML_MAP_TAG,
       1, YAML_ANY_MAPPING_STYLE));
    YEMIT(ret,exit, &_event);
  } else {
    YPARSE(ret,exit, &_event, YAML_SCALAR_EVENT);
    YCHECK_OR_INVALID(ret,exit, strcmp
      ((const char*)_event.data.scalar.value,
       obj_name.c_str()) == 0);
    YCHECK_OR_INVALID(ret,exit, sscanf
      ((const char*)_event.data.scalar.tag,
       YAML_TYPEID_PRINTF_SCANF_FORMAT, &type_id) == 1);
    yaml_event_delete(&_event);
    YPARSE(ret,exit, &_event, YAML_MAPPING_START_EVENT);
    yaml_event_delete(&_event);
  }

exit:
  return ret;
}

ESRet
SerializerYAML::endTypedObject()
{
  return endLastMap();
}

template<typename T>
inline ESRet
SerializerYAML::valueImplem
  (const std::string& value_name,
   T& value)
{
  if( value_name.empty() )
    return ESRet_BadArg;

  ESRet ret = ESRet_OK;
  switch(_kind) {
    case ESKind_Read: {
      YPARSE(ret,exit, &_event, YAML_SCALAR_EVENT);
      yaml_event_delete(&_event);
      YPARSE(ret,exit, &_event, YAML_SCALAR_EVENT);
      SCALL_OR_GOTO(ret,exit, internal::valueReadFromCString
        ((const char*)_event.data.scalar.value,
         _event.data.scalar.length,
         value));
      yaml_event_delete(&_event);
    } break;
    case ESKind_Write: {
      static char tmp_val[64];
      static int tmp_length;
      SCALL_OR_GOTO(ret,exit, internal::valueWriteToCString
        (value, tmp_val, tmp_length, 64));
      YCALL_OR_INVALID(ret,exit, yaml_scalar_event_initialize
        (&_event, NULL, (yaml_char_t*)YAML_STR_TAG,
         (yaml_char_t*)value_name.c_str(), value_name.length(),
         1, 1, YAML_PLAIN_SCALAR_STYLE));
      YEMIT(ret,exit, &_event);
      YCALL_OR_INVALID(ret,exit, yaml_scalar_event_initialize
        (&_event, NULL, (yaml_char_t*)YAML_DEFAULT_SCALAR_TAG,
         (yaml_char_t*)tmp_val, tmp_length,
         1, 1, YAML_ANY_SCALAR_STYLE));
      YEMIT(ret,exit, &_event);
    } break;
    default: assert(false);
  }

exit:
  return ret;
}

#define STYPE(type,name)                                                       \
ESRet                                                                          \
SerializerYAML::value##name                                                    \
  (const std::string& value_name,                                              \
   type& value)                                                                \
{ return valueImplem<type>(value_name, value); }
SERIALIZABLE_BUILTIN_TYPES
#undef STYPE

inline int
SerializerYAML::read_from_std_string
  (void* ctxt,
   unsigned char* buf,
   size_t size,
   size_t* size_read)
{
  struct SerializerYAML::SerializerYAMLParser* parser =
    (struct SerializerYAML::SerializerYAMLParser*)ctxt;
  const size_t copy_size = std::min
    (size, parser->buf->length() - parser->buf_curr_pos - 1);
  const char* ptr = parser->buf->c_str() + parser->buf_curr_pos;
  memcpy(buf, ptr, copy_size);
  *size_read = copy_size;
  parser->buf_curr_pos += copy_size;
  return 1;
}

inline int
SerializerYAML::write_to_std_string
  (void* ctxt,
   unsigned char* buffer,
   size_t size)
{
  struct SerializerYAML::SerializerYAMLEmitter* emitter =
    (struct SerializerYAML::SerializerYAMLEmitter*)ctxt;
  emitter->buf->append((const char*)buffer, size);
  return 1;
}

#undef CSTR_TO_FROM_INT_VALUE
#undef CSTR_TO_FROM_FLT_VALUE

#undef YCHECK_OR_INVALID
#undef YCALL_OR_INVALID
#undef YPARSE
#undef YEMIT

#undef YAML_THIS_TAG
#undef YAML_TYPEID_PRINTF_SCANF_FORMAT
#undef YAML_COUNT_PRINTF_SCANF_FORMAT

#endif // SERIALIZATION_YAML_SERIALIZER_INL
