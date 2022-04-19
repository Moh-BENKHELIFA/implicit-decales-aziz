#ifndef HATCHING_HPP
#define HATCHING_HPP

#include <Eigen/Core>
#include "common.hpp"

namespace Hatching{
enum Space {
    Default = 0,
    INVALID = 1
};

enum UnscaledSpace {
    UDefault = 0
};

//! This structure must not be modified. Requires correct mapping with Space
static const std::array<std::string, Space::INVALID> SpaceNames{
    "Default"
};

inline Space& operator++( Space &d ) { return Utils::Enum::operator ++(d); }
inline Space& operator--( Space &d ) { return Utils::Enum::operator --(d); }

namespace internal{

//! \brief Helper used to compute the number of dimension for color spaces
//! (specialize for other values than default)
template <Space _space>
struct SpaceDimHelper {
    enum {Size = 3}; //angle, spacing, thickness
};


/*!
 * \brief Convertion helper, to convert parameters vectors between color spaces
 * Implements the default behavior: return input
 *
 * Specialized behaviour are implemented using Deps::Colorspace
 */
template <Space in, Space out>
struct ConvertionHelper{
    template <typename CVector> CVector
    static inline convert(const CVector& inVec) {
        static_assert(in  != INVALID, "In space must be a valid space");
        static_assert(out != INVALID, "Out space must be a valid space");
        return inVec;
    }
};


/*!
 * Helper to rescale colors to specific space ranges (see Color::UnscaledSpace).
 * Available values:
 *  - USimple: Angle: [0:360], spacing [0:1], thickness [0:1]
 */
template <Space in, UnscaledSpace out>
struct UnscalingHelper{
    template <typename CVector> CVector
    static inline convert(CVector inVec) {
        static_assert(in  != INVALID,  "In space must be a valid space");
        static_assert(out == UDefault, "Out space must be a valid space");
        inVec(0) *= 360;
        return inVec;
    }
};

} // namespace internal

/*!
 * \brief Store a hatching parameter set.
 *
 * This class is design to follow Color's prototype
 */
template <typename _Scalar, Space _space>
class HatchingBase{
public:
    INTERPOLIB_TYPENAME_DECLARE("hatchingdiscr")
    using Scalar = _Scalar;
    enum {
        //! \brief Native color space size
      SpaceDim = internal::SpaceDimHelper<_space>::Size
    };
    using Space = ::Hatching::Space;
    static constexpr Space space = _space;

    //! \brief Color vector
    using CVector = Eigen::Matrix<Scalar, SpaceDim, 1>;

private:
    /*! \brief Raw buffer storing color in the colorspace defined by _space */
    CVector _buf;
    static_assert(space != Space::INVALID, "Invalid Parameter Space");

public:
    inline HatchingBase(){
        _buf.fill(0);
    }

    inline HatchingBase(Scalar angle, Scalar spacing, Scalar thickness)
        :_buf(angle,spacing,thickness) {}

    //! \brief Use Eigen constructor here, to avoid potential alignement issues
    inline HatchingBase(Scalar* buf) : _buf(CVector(buf)) {}

    template <typename inVec>
    explicit
    inline HatchingBase(const inVec& v) :_buf(v) {}

    //! \brief Copy constructor with automatic color space conversion
    template <Space spaceOther>
    inline HatchingBase(const HatchingBase<Scalar, spaceOther>& col)
        :_buf(col.template getAs<space>()) {}

    //! \brief Assignement operator. Color validity is not checked
    inline HatchingBase<Scalar, space>& operator= (const HatchingBase<Scalar, space>& other)
    { _buf = other._buf; return *this;}

    //! \brief Assignement operator from a parameter vector, assumed to be defined
    //! in the right color space. Color validity is not checked
    inline HatchingBase<Scalar, space>& operator= (const CVector& vec)
    { _buf = vec; return *this;}


    ////////////////////////////////////////////////////////////////////////////
    /// Getters
    ////////////////////////////////////////////////////////////////////////////

    /*! \brief Convert color to arbitrary color space, compile-time check */
    template<Space outSpace>
    inline CVector getAs() const {
        return internal::ConvertionHelper<_space, outSpace>::convert(_buf);
    }

    /*! \brief Convert color to arbitrary color space, running time check */
    inline CVector getAs(Space outSpace) const {
        switch (outSpace) {
        case Space::Default:
            return getAs<Space::Default>();
        case Space::INVALID:
        default:
            return CVector();
        }
    }

    template <UnscaledSpace outSpace>
    inline CVector getUnscaledAs() const {
        switch (outSpace) {
        case UnscaledSpace::UDefault:
            return internal::UnscalingHelper<Space::Default, UnscaledSpace::UDefault>
                    ::convert( getAs<Space::Default>() );
        default:
            return CVector();
        }
    }

    inline CVector getUnscaledAs(UnscaledSpace outSpace) const {
        switch (outSpace) {
        case UnscaledSpace::UDefault:
            return internal::UnscalingHelper<Space::Default, UnscaledSpace::UDefault>
                    ::convert( getAs<Space::Default>() );
        default:
            return CVector();
        }
    }

    /*! \brief Convenience function: get color as a vector in its native space*/
    inline const CVector& getNative() const { return _buf; }

    inline CVector getUnscaledNative() const {
        switch (_space) {
        case Space::Default:
            return internal::UnscalingHelper<Space::Default, UnscaledSpace::UDefault>
                    ::convert( getNative() );
        default:
            return CVector();
        }
    }

    /*! \brief Convenience function: get color as RGB vector */
    inline CVector getSimple() const { return getAs<Space::Default>(); }

    ////////////////////////////////////////////////////////////////////////////
    /// Setters
    ////////////////////////////////////////////////////////////////////////////

    /*! \brief Convert color to arbitrary color space */
    template<Space inSpace, typename CType>
    inline void setFrom( const CType& rawvec ) {
        _buf = internal::ConvertionHelper<inSpace, space>::convert(rawvec);
    }

    /*! \brief Convenience function: set color using a RGB vector */
    template <typename CType>
    inline void setNative (const CType& raw) { _buf = raw; }

    /*! \brief Convenience function: set parameters using a RGB vector */
    template <typename CType>
    inline void setSimple (const CType& rawDefault) { setFrom<Space::Default>(rawDefault); }


    ////////////////////////////////////////////////////////////////////////////
    /// Other
    ////////////////////////////////////////////////////////////////////////////

    static inline
    HatchingBase<Scalar, space> Random() {
        return HatchingBase<Scalar, Space::Default>(CVector::Random().array().abs());
    }

    static inline
    HatchingBase<Scalar, space> Zero() {
        return HatchingBase<Scalar, Space::Default>(CVector::Zero());
    }
}; // class Hatching Base


template <typename _Scalar>
struct HatchingFactory{
    using Scalar = _Scalar;

    template<typename FunctorT>
    static inline
    void generateAndProcess(Space space, FunctorT& fun){
        switch (space) {
        case Space::Default: fun( HatchingBase<Scalar, Space::Default>() ); return;
        default: return;
        }
    }
};

} // namespace Hatching

template <typename Scalar>
using HatchingDefault = Hatching::HatchingBase<Scalar, Hatching::Space::Default>;

#endif // HATCHING_HPP

