#ifndef FACTORY_HPP
#define FACTORY_HPP

#include <map>

typedef std::size_t FactoryTypeId;
typedef void* (*FactoryAllocCb)(void);

typedef std::map<FactoryTypeId, FactoryAllocCb> Factory;

#define FARG(...) __VA_ARGS__

//! \brief Helper to simplify the addition of a new type in a factory, with a
//! custom allocation callback.
#define FACTORY_ADD_TYPE_CUSTOM(f,Type,AllocCb)                                \
  (f)[Type::getClassTypeHash()] = AllocCb

//! \brief Helper to simplify the addition of a new type in a factory that need
//! some parameters at construction time.
#define FACTORY_ADD_TYPE_SIMPLE(f,Type,ConstructorParams)                      \
  FACTORY_ADD_TYPE_CUSTOM                                                      \
    (f,FARG(Type), FARG([]()->void*{ return new Type ConstructorParams; }))

//! \brief Helper to simplify the addition of a new type that do not need any
//! parameter at construction time.
#define FACTORY_ADD_TYPE(f,Type) FACTORY_ADD_TYPE_SIMPLE(f,FARG(Type),{})

#endif // FACTORY_HPP
