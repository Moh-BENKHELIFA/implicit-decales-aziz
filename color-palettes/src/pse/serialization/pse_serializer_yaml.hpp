#ifndef SERIALIZATION_YAML_SERIALIZER_HPP
#define SERIALIZATION_YAML_SERIALIZER_HPP

#include <pse/serialization/pse_serializer.hpp>

#include <yaml.h>

//! /brief YAML serializer.
class SerializerYAML : public Serializer {
public:
  using Base = Serializer;
  static constexpr size_t VERSION_CURRENT = 1;

public:
  SerializerYAML()
    : Serializer(VERSION_CURRENT)
  {}
  virtual ~SerializerYAML() {}

  ESRet
  beginStream
    (const std::string& stream);
  ESRet
  beginStream
    (std::string& stream);
  ESRet
  endStream();

public:
  virtual ESRet
  header() {
    return Base::header();
  }

  virtual ESRet
  beginMap
    (const std::string& name);
  virtual ESRet
  endLastMap();

  virtual ESRet
  beginArray
    (const std::string& _name,
     size_t& count);
  virtual ESRet
  beginArrayItem
    (const std::string& _name);
  virtual ESRet
  endLastArrayItem();
  virtual ESRet
  endLastArray();

  virtual ESRet
  beginTypedObject
    (const std::string& obj_name,
     std::size_t& type_id);
  virtual ESRet
  endTypedObject();

  #define STYPE(type,name)                                                     \
  virtual ESRet                                                                \
  value##name                                                                  \
    (const std::string& value_name,                                            \
     type& value);
  SERIALIZABLE_BUILTIN_TYPES
  #undef STYPE

protected:

  template<typename T>
  inline ESRet
  valueImplem
    (const std::string& value_name,
     T& value);

  static int
  read_from_std_string
    (void* b,
     unsigned char* buffer,
     size_t size,
     size_t* size_read);

  static int
  write_to_std_string
    (void* str,
     unsigned char* buffer,
     size_t size);

  struct SerializerYAMLParser{
    yaml_parser_t parser;
    const std::string* buf;
    std::size_t buf_curr_pos;
  };
  struct SerializerYAMLEmitter{
    yaml_emitter_t emitter;
    std::string* buf;
  };

  union {
    struct SerializerYAMLParser read;
    struct SerializerYAMLEmitter write;
  } _stream;
  yaml_event_t _event;

};

#include <pse/serialization/pse_serializer_yaml.inl>

#endif /* SERIALIZATION_YAML_SERIALIZER_HPP */
