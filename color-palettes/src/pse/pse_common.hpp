#ifndef COMMON_HPP
#define COMMON_HPP

#include <array>
#include <cmath>

#include "pse_literal_string.hpp"

#define CONCAT_STR(str1,str2) str1 "_" str2
#define CONCAT_VAR(var1,var2) #var1 "_" #var2
#define VAR_TO_STR(x) #x

namespace Utils{

static inline std::size_t
StdString_fnv_1a_hash
  (const std::string& str)
{
    // This hashing gives the same result than Utils::LiteralString::hash
    static constexpr std::size_t fnv_prime =
        (sizeof(std::size_t) == 8
      ? 1099511628211u
      : 16777619u);
    static constexpr std::size_t fnv_offset =
        (sizeof(std::size_t) == 8
      ? 14695981039346656037u
      : 2166136261u);
    std::size_t hash = fnv_offset;
    for(size_t i = 1; i < str.length(); ++i) {
      hash ^= str[i];
      hash *= fnv_prime;
    }
    return hash;
    //return std::hash<std::string>()(str);
}

template <typename T> std::string type_name();
template <typename T> std::size_t type_hash();
#define DEFINE_TYPE_NAME_BUILT_IN(Type)                                        \
  template<> inline std::string type_name<Type>()                              \
    { return #Type; }                                                          \
  template<> inline std::size_t type_hash<Type>()                              \
    { return StdString_fnv_1a_hash(type_name<Type>()); }

DEFINE_TYPE_NAME_BUILT_IN(float)
DEFINE_TYPE_NAME_BUILT_IN(double)

//! \brief Allow to declare a static type name in classes/structs
#define INTERPOLIB_TYPENAME_DECLARE(Name)                                      \
  static const std::string& getClassTypeName() {                               \
    static const std::string this_name =                                       \
      std::string("") + Name;                                                  \
    return this_name;                                                          \
  }                                                                            \
  static std::size_t getClassTypeHash() {                                      \
    static const std::size_t this_hash =                                       \
      ::Utils::StdString_fnv_1a_hash(getClassTypeName());                      \
    return this_hash;                                                          \
  }                                                                            \
  inline virtual const std::string& getTypeName() const                        \
    { return getClassTypeName(); }                                             \
  inline virtual std::size_t getTypeHash() const                               \
    { return getClassTypeHash(); }

//! Check type by using the type name
template <typename T1, typename T2>
inline bool isOfType(const T2* p)
  { return p->getTypeHash() == T1::getClassTypeHash(); }

//! Old school ptr
template <typename T>
inline bool isnull(T*t){ return t == nullptr; }
//! sharedptr style
template <typename T>
inline bool isnull(const T& t){ return t == nullptr; }
//! Old school ptr
template <typename T>
inline bool notnull(T*t){ return !isnull(t); }
//! sharedptr style
template <typename T>
inline bool notnull(const T& t){ return !isnull(t); }

//! \brief Compile time pow
template<typename baseT, typename expoT>
constexpr baseT POW(baseT base, expoT expo)
{
    return (expo != 0 )? base * POW(base, expo -1) : 1;
}

//! \brief Return the number of element in an array at compile time
template <typename T, size_t N>
constexpr size_t countof(T(&)[N])
{
    return N;
}

/*!
 *
 * \brief A weighted object with implicit constructor from it's base class
 *
 * \note The base class must define a Scalar type, used to store the weight
 */
template <typename _Base>
class Weighted : public _Base{
public:

    using Base   = _Base;
    using Scalar = typename Base::Scalar;

    inline const Scalar& getW() const { return _w; }
    inline void setW(const Scalar& w) { _w = w; }

    Weighted(const Scalar &w = Scalar(1) ) : Base(), _w(w) {}
    Weighted(const Weighted<Base> &other ) : Base(other), _w(other._w) {}

    // avoid implicit cast and copy from base class
    explicit Weighted(const Base & otherBase,
                      const Scalar &w = Scalar(1))
        : Base(otherBase), _w(w){}

    virtual ~Weighted() {}
    Weighted& operator=(const Base&) =delete;

protected:
    Scalar _w;
};

/*!
 * \brief Utility namespace for c++ enums
 *
 * Provides ++ and -- operators on enums. Enums must contain the element INVALID
 * at the end
 */
namespace Enum{
template <typename E>
inline E& operator++( E &d ) {
    using IntType = typename std::underlying_type<E>::type;
    d = static_cast<E>( static_cast<IntType>(d) + 1 );
    if ( d == E::INVALID )
        d = static_cast<E>(0);
    return d;
}

template <typename E>
inline E& operator--( E &d ) {
    using IntType = typename std::underlying_type<E>::type;
    if ( d == static_cast<E>(0) )
        d = static_cast<E>(E::INVALID);
    return d =  static_cast<E>( static_cast<IntType>(d) - 1 );
}

}


}

namespace Functors{
template<bool val>
struct ConstantBoolean {
    bool operator()(...) const { return val; }
};
using ConstantTrue  = ConstantBoolean<true>;
using ConstantFalse = ConstantBoolean<false>;
} // namespace Functors

namespace Indexing{

namespace internal{

/*!
 * \brief Helper structure used to (conditionnaly) validate indices.
 * The template parameter enable allows to control the validation, without
 * overload when the validation is disabled.
 *
 * Throw std::out_of_range exception when the validation is enabled and an
 * invalid case is detected
 */
template <bool enable>
struct IndexValidator{
    /*!
     * \brief Check if n is within [0:gsize] if enable=true
     * \throw std::out_of_range
     */
    template <class IndexT, class SizeT>
    static inline
    constexpr IndexT validate(const IndexT& n,
                              const SizeT& gsize);

    /*!
     * \brief Test used internally to validate the input index
     */
    template <class IndexT, class SizeT>
    static inline
    constexpr bool _test(const IndexT& n, const SizeT& gsize){
        return n >= 0 && n < IndexT(gsize);
    }
};

template <>
template <class IndexT, class SizeT>
constexpr IndexT
IndexValidator<true>::validate(const IndexT& n,
                               const SizeT&  gsize)
{
    return IndexValidator<true>::_test(n, gsize) ?
                n :
                throw std::out_of_range(
                    std::string("[IndexValidator] ")    +
                    std::to_string(n)                   +
                    std::string(" is out of range [0:") +
                    std::to_string(gsize)               +
                    std::string("]") );
}

template <>
template <class IndexT, class SizeT>
constexpr IndexT
IndexValidator<false>::validate(const IndexT& n,
                                const SizeT& /*gsize*/)
{
    return n;
}

} // namespace internal




/*!
 * \brief Convert a normalized n-d vector to a linear index in a uniform regular grid
 * This function is recursive, and unrolled at compile time (loop over n).
 *
 * \param coord Input coordinates defined in the normalized n-hypercube.
 * \param cdim  Working dimension, must be called with n.
 * \param gsize Dimension of the grid, must be consistent in all dimensions
 *
 * \tparam validate Enable or disable the range validation
 * \tparam PointT Type of the input points (deduced)
 * \tparam IndexT Index type (deduced)
 * \tparam IndexT Size type (deduced)
 *
 * \see internal::IndexValidator for the validation procedure
 */
template<bool validate, class ndIndexT, class IndexT, class SizeT>
constexpr inline IndexT
UnrollIndexLoop(const ndIndexT& coord,
                IndexT        cdim,
                SizeT         gsize){
  return (cdim != 0)
    ? ( internal::IndexValidator<validate>::validate
        (IndexT(coord[cdim]), gsize)
        *Utils::POW(gsize, cdim) +
        UnrollIndexLoop<validate>(coord, cdim-1, gsize) )
    : internal::IndexValidator<validate>::validate
      (IndexT(coord[cdim]), gsize);
}


//template<bool validate, class ndIndexT, class IndexT, class SizeT>
//constexpr inline ndIndexT
//RollIndexLoop(IndexT index,
//              IndexT cdim,
//              SizeT  gsize){
//    return (cdim != 0)
//            ? (index / Utils::POW(gsize, cdim))  +
//              RollIndexLoop<validate, ndIndexT>(Utils::POW(gsize, cdim), cdim - 1, gsize )
//            : index / gsize // get int part of the /
//                ;
//    }


template<bool validate, class ndIndexT, class IndexT, class SizeT>
inline ndIndexT
RollIndexLoop(IndexT index,
    IndexT cdim,
    SizeT  gsize){
    ndIndexT out;

    for(IndexT d = 0; d < cdim; ++d){
        IndexT csize = Utils::POW(gsize, cdim-d);
        out(cdim-d) = index / csize;
        index  = index % csize;
    }

    out(0) = index;

    return out;
}


/*!
 * \brief Convert a normalized n-d vector to a linear index in a uniform regular
 * grid, moved by moved by an offset defined as a integer move in the n-d grid.
 * This function is recursive, and unrolled at compile time (loop over n). In
 * addition, it allows to offset the input coordinates.
 *
 * \param coord Input coordinates defined in the normalized n-hypercube.
 * \param cdim  Working dimension, must be called with n.
 * \param gsize Dimension of the grid, must be consistent in all dimensions
 *
 * \see UnrollIndexLoop<class PointT>
 */
template<bool validate, class ndIndexT, class IndexT, class SizeT>
constexpr inline IndexT
UnrollIndexLoop(const ndIndexT& coord,
                const ndIndexT& offset,
                IndexT          cdim,
                SizeT           gsize){
  return (cdim != 0)
    ? ( internal::IndexValidator<validate>::validate
        (IndexT(offset[cdim]+std::floor(coord[cdim])), gsize)
        *Utils::POW(gsize, cdim) +
        UnrollIndexLoop<validate>(coord, offset, cdim-1, gsize) )
    : internal::IndexValidator<validate>::validate
      (IndexT(offset[cdim]+std::floor(coord[cdim])), gsize);
}
} // namespace Indexing

#endif // COMMON_HPP
