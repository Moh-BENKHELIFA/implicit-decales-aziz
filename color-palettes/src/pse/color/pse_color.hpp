#ifndef COLOR_HPP
#define COLOR_HPP

#include <pse/pse_common.hpp>

#include <Eigen/Core>
#include <ColorSpace/colorspace.h>

#include <iostream>
#include <array>
#include <string>
#include <iomanip> // std::setw, std::setfill


#define LAB_TO_UNIT_VECTOR(l,a,b) \
    CVector(l,typename CVector::Scalar(128)+a,typename CVector::Scalar(128)+b)\
    .array() / Eigen::Array<  typename CVector::Scalar, \
                              CVector::RowsAtCompileTime, \
                              CVector::ColsAtCompileTime> (100, 256, 256)

#define UNIT_TO_LAB_LUM(l)  typename CVector::Scalar(100) * l
#define UNIT_TO_LAB_COL(ab) typename CVector::Scalar(256) \
                      * (ab-typename CVector::Scalar(0.5))

#define LUV_TO_UNIT_VECTOR(l,u,v) \
    CVector(l,typename CVector::Scalar(100)+u,typename CVector::Scalar(100)+v)\
    .array() / Eigen::Array<  typename CVector::Scalar, \
                              CVector::RowsAtCompileTime, \
                              CVector::ColsAtCompileTime> (100, 200, 200)

#define UNIT_TO_LUV_LUM(l)  typename CVector::Scalar(100) * l
#define UNIT_TO_LUV_COL(uv) typename CVector::Scalar(200) \
                      * (uv-typename CVector::Scalar(0.5))


namespace Color{

enum Space {
    RGB = 0,
    HSV = 1,
    LAB = 2,
    LUV = 3,
    INVALID = 4
};

enum UnscaledSpace {
    RGB_255   = 0, //! <\brief RGB_255: RGB, [0:255]
    HSV_360   = 1, //! <\brief H [0:360], SV [0:1]
    Lab_128   = 2, //! <\brief L [0:100], ab [-128:128]
    LMS_Cat02 = 3, //! <\brief http://en.wikipedia.org/wiki/LMS_color_space. Doesn't have unscaled space
    Luv_100   = 4, //! <\brief L [0:100], uv [-100:100] http://en.wikipedia.org/wiki/CIELUV
    INVALID_UNSCALED = 5  //! <\brief Invalid color space
};


template<UnscaledSpace space>
struct UnscaledSpaceProperties{
    template <typename Scalar> static inline Eigen::Matrix<Scalar, 3, 1> min();
    template <typename Scalar> static inline Eigen::Matrix<Scalar, 3, 1> max();
    static inline constexpr Color::Space scaledSpace();
    static inline constexpr bool isCyclic( int id );
};



//! This structure must not be modified. Requires correct mapping with Space
static constexpr std::array<Utils::LiteralString, Space::INVALID> SpaceNames{{
    Utils::LiteralString("RGB"),
    Utils::LiteralString("HSV"),
    Utils::LiteralString("Lab"),
    Utils::LiteralString("Luv"),
 }};

//! This structure must not be modified. Requires correct mapping with Space
static constexpr std::array<Utils::LiteralString, UnscaledSpace::INVALID_UNSCALED> UnscaledSpaceNames{{
    SpaceNames[Color::RGB],
    SpaceNames[Color::HSV],
    SpaceNames[Color::LAB],
    Utils::LiteralString("LMS"),
    SpaceNames[Color::LUV],
}};


inline Space& operator++( Space &d ) { return Utils::Enum::operator ++(d); }
inline Space& operator--( Space &d ) { return Utils::Enum::operator --(d); }

namespace internal{

//! \brief Helper used to compute the number of dimension for color spaces
//! (specialize for other values than default)
template <Space _space>
struct SpaceDimHelper {
    enum {Size = 3};
};


//! \brief Eigen impl. for www.opengl.org/sdk/docs/man/html/clamp.xhtml

struct ModuloOne{
  inline ModuloOne() {}
  template <typename S >
  inline S  operator() (S x) const { return x < S(0.) ? S(1.) - std::modf(x,&x) : std::modf(x,&x); }
};

/*!
 * \brief Convertion helper, to convert color vectors between color spaces
 * Implements the default behavior: return input
 *
 * Specialized behaviour are implemented using Deps::Colorspace
 */
template <Space in, Space out>
struct ConvertionHelper{
    template <typename CVector> CVector
    static inline convert(const CVector& inVec);
};


/*!
 * Helper to rescale colors to specific space ranges (see Color::UnscaledSpace).
 * Available values:
 *  - RGB_255: RGB, [0:255]
 *  - Lab_128: L [0:100], ab [-128:128]
 *  - HSV_360: H [0:360], SV [0:100]
 *  - Luv_100: L [0:100], uv [-100:100]
 */
template <Space in, UnscaledSpace out>
struct UnscalingHelper{
    template <typename CVector> CVector
    static inline convert(const CVector& inVec);

    template <typename CVector> CVector
    static inline revert(const CVector& unscaledVec);
};


} // namespace internal
} // namespace Color

#include <pse/color/pse_color_conversion.hpp>
#include <pse/color/pse_color_unscaling.hpp>
#include <pse/color/pse_color_spaces.hpp>

namespace Color{
/*!
 * \brief Store a color in a given color space, described by the template
 * parameter _space.
 *
 * \tparam _space Color space used to store the color coordinates (native space)
 *
 * Internally this classes uses template specialization to adapt conversion
 * functions wrt. to the color space.
 *
 * Colors are internally stored using normalized coordinates. They can be
 * copied and rescaled for outside use by calling getUnscaledAs function. See
 * internal::UnscalingHelper documentation for available conversions
 *
 * Color conversions are done using http://www.getreuer.info/home/colorspace
 */
template <typename _Scalar, Space _space>
class ColorBase{
public:
    INTERPOLIB_TYPENAME_DECLARE
      ( "ColorBase<"
      + Utils::type_name<_Scalar>()
      + ","
      + std::string(Color::SpaceNames[int(_space)])
      + ">")
    using Scalar = _Scalar;
    enum {
        //! \brief Native color space size
      SpaceDim = internal::SpaceDimHelper<_space>::Size
    };
    using Space = ::Color::Space;
    static constexpr Space space = _space;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    //! \brief Color vector
    using CVector = Eigen::Matrix<Scalar, SpaceDim, 1>;

private:
    /*! \brief Raw buffer storing color in the colorspace defined by _space */
    CVector _buf;
    static_assert(space != ::Color::INVALID, "Invalid Color Space");

public:
    inline ColorBase(){
        _buf.fill(0);
    }

    inline ColorBase(Scalar x, Scalar y, Scalar z)  :_buf(x,y,z) {}

    //! \brief Use Eigen constructor here, to avoid potential alignement issues
    inline ColorBase(Scalar* buf) : _buf(CVector(buf)) {}

    template <typename inVec>
    explicit
    inline ColorBase(const inVec& v) :_buf(v) {}

    //! \brief Copy constructor with automatic color space conversion
    template <Space spaceOther>
    inline ColorBase(const ColorBase<Scalar, spaceOther>& col)
        :_buf(col.template getAs<space>()) {}


    ////////////////////////////////////////////////////////////////////////////
    /// Operators
    ////////////////////////////////////////////////////////////////////////////
    ///
    ///
    //! \brief Assignement operator. Color validity is not checked
    template <Space spaceOther>
    inline ColorBase<Scalar, space>& operator= (const ColorBase<Scalar, spaceOther>& other)
    { _buf = other.template getAs<space>(); return *this;}

    //! \brief Assignement operator from a color vector, assumed to be defined
    //! in the right color space. Color validity is not checked
    inline ColorBase<Scalar, space>& operator= (const CVector& vec)
    { _buf = vec; return *this;}

    //! \brief Equality operator. Color validity is not checked
    template <Space spaceOther>
    inline bool operator== (const ColorBase<Scalar, spaceOther>& other) const
    { return _buf == other.template getAs<space>(); }

    //! \brief Equality operator from a color vector, assumed to be defined
    //! in the right color space. Color validity is not checked
    inline bool operator== (const CVector& vec) const
    { return _buf == vec; }

    //! \brief Inequality operator. Color validity is not checked
    template <Space spaceOther>
    inline bool operator!= (const ColorBase<Scalar, spaceOther>& other) const
    { return _buf != other.template getAs<space>(); }

    //! \brief Inequality operator from a color vector, assumed to be defined
    //! in the right color space. Color validity is not checked
    inline bool operator!= (const CVector& vec) const
    { return _buf != vec; }

    /*! \brief Scalar * operator */
    inline ColorBase<Scalar, space> operator* (Scalar x) const {
        return ColorBase<Scalar, space>(_buf.array()*x);
    }

    /*! \brief Scalar * operator */
    inline ColorBase<Scalar, space>& operator*= (Scalar x) {
        return *this = *this * x;
    }

    /*! \brief Scalar * operator */
    inline ColorBase<Scalar, space> operator/ (Scalar x) const {
        return ColorBase<Scalar, space>(_buf.array()/x);
    }

    /*! \brief Scalar * operator */
    inline ColorBase<Scalar, space>& operator/= (Scalar x) {
        return *this = *this / x;
    }

    /*! \brief Color add operator */
    inline ColorBase<Scalar, space> operator+ (const ColorBase<Scalar, space>& other) const {
        return ColorBase<Scalar, space>(_buf+other._buf);
    }

    /*! \brief Color substraction operator */
    inline ColorBase<Scalar, space> operator- (const ColorBase<Scalar, space>& other) const {
        return ColorBase<Scalar, space>(_buf-other._buf);
    }

    inline ColorBase<Scalar, space>& operator+= (const ColorBase<Scalar, space>& other) {
        return *this = *this + other;
    }

    inline ColorBase<Scalar, space>& operator-= (const ColorBase<Scalar, space>& other) {
        return *this = *this - other;
    }

//    /*! \brief Color add operator with space conversion */
//    template <Space spaceOther>
//    inline ColorBase<Scalar, space> operator+ (const ColorBase<Scalar, spaceOther>& other) const {
//        return ColorBase<Scalar, space>(_buf+other.template getAs<space>());
//    }

//    /*! \brief Color substraction operator with space conversion */
//    template <Space spaceOther>
//    inline ColorBase<Scalar, space> operator- (const ColorBase<Scalar, spaceOther>& other) const {
//        return ColorBase<Scalar, space>(_buf-other.template getAs<space>());
//    }

    friend std::ostream& operator <<(std::ostream& s, const ColorBase<Scalar, space>& c) {
        s << c.getTypeName()
          << "{ " << c.getNative()[0]
          << ", " << c.getNative()[1]
          << ", " << c.getNative()[2]
          << " }";
        return s;
    }

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
        case Space::RGB:
            return getAs<Space::RGB>();
        case Space::HSV:
            return getAs<Space::HSV>();
        case Space::LAB:
            return getAs<Space::LAB>();
        case Space::LUV:
            return getAs<Space::LUV>();
        case Space::INVALID:
        default:
            return CVector();
        }
    }

    inline CVector getUnscaledAs(UnscaledSpace outSpace) const {
        switch (outSpace) {
        case UnscaledSpace::RGB_255:
            return internal::UnscalingHelper<Space::RGB, UnscaledSpace::RGB_255>
                    ::convert( getAs<Space::RGB>() );
        case UnscaledSpace::HSV_360:
            return internal::UnscalingHelper<Space::HSV, UnscaledSpace::HSV_360>
                    ::convert( getAs<Space::HSV>() );
        case UnscaledSpace::Lab_128:
            return internal::UnscalingHelper<Space::LAB, UnscaledSpace::Lab_128>
                    ::convert( getAs<Space::LAB>() );
        case UnscaledSpace::Luv_100:
            return internal::UnscalingHelper<Space::LUV, UnscaledSpace::Luv_100>
                    ::convert( getAs<Space::LUV>() );
        case UnscaledSpace::LMS_Cat02:
            return internal::UnscalingHelper<Space::RGB, UnscaledSpace::LMS_Cat02>
                    ::convert( getAs<Space::RGB>() );
        default:
            return CVector();
        }
    }

    //! \brief Compile time equivalent of #getUnscaledAs(UnscaledSpace outSpace)
    template <UnscaledSpace outSpace>
    inline CVector getUnscaledAs() const {
        switch (outSpace) {
        case UnscaledSpace::RGB_255:
            return internal::UnscalingHelper<Space::RGB, UnscaledSpace::RGB_255>
                    ::convert( getAs<Space::RGB>() );
        case UnscaledSpace::HSV_360:
            return internal::UnscalingHelper<Space::HSV, UnscaledSpace::HSV_360>
                    ::convert( getAs<Space::HSV>() );
        case UnscaledSpace::Lab_128:
            return internal::UnscalingHelper<Space::LAB, UnscaledSpace::Lab_128>
                    ::convert( getAs<Space::LAB>() );
        case UnscaledSpace::Luv_100:
            return internal::UnscalingHelper<Space::LUV, UnscaledSpace::Luv_100>
                    ::convert( getAs<Space::LUV>() );
        case UnscaledSpace::LMS_Cat02:
            return internal::UnscalingHelper<Space::RGB, UnscaledSpace::LMS_Cat02>
                    ::convert( getAs<Space::RGB>() );
        default:
            return CVector();
        }
    }

    /*! \brief Convenience function: get color as a vector in its native space*/
    inline const CVector& getNative() const { return _buf; }

    inline CVector getUnscaledNative() const {
        switch (_space) {
        case Space::RGB:
            return internal::UnscalingHelper<Space::RGB, UnscaledSpace::RGB_255>
                    ::convert( getNative() );
        case Space::HSV:
            return internal::UnscalingHelper<Space::HSV, UnscaledSpace::HSV_360>
                    ::convert( getNative() );
        case Space::LAB:
            return internal::UnscalingHelper<Space::LAB, UnscaledSpace::Lab_128>
                    ::convert( getNative() );
        case Space::LUV:
            return internal::UnscalingHelper<Space::LUV, UnscaledSpace::Luv_100>
                    ::convert( getNative() );
        default:
            return CVector();
        }
    }

    static constexpr UnscaledSpace getUnscaledSpace() {
        return (_space == Space::RGB
                ? UnscaledSpace::RGB_255
                : (_space == Space::HSV
                   ? UnscaledSpace::HSV_360
                   : (_space == Space::LAB
                      ? UnscaledSpace::Lab_128
                      : (_space == Space::LUV
                         ? UnscaledSpace::Luv_100
                         : UnscaledSpace::INVALID_UNSCALED))));
    }

//    /*! \brief Get similar valid color by calling explicitely same space conversion*/
//    inline CVector getValidNative() const { getAs<space>(); }

//    /*! \brief Convenience function: get similar valid color vector */
//    inline static CVector getValidNative(const CVector& vec) {
//        return internal::ConvertionHelper<_space, _space>::convert(vec);
//    }

//    /*! \brief Set and return similar valid color by calling explicitely same space conversion*/
//    inline const CVector& validate() { return _buf = getAs<space>(); }

    /*! \brief Convenience function: get color as RGB vector */
    inline CVector getRGB() const { return getAs<Space::RGB>(); }

    /*! \brief Convenience function: get color as RGB vector */
    inline CVector getHSV() const { return getAs<Space::HSV>(); }

    /*! \brief Convenience function: get color as RGB vector */
    inline CVector getLAB() const { return getAs<Space::LAB>(); }

    /*! \brief Convenience function: get color as RGB vector */
    inline CVector getLUV() const { return getAs<Space::LUV>(); }

    /*! \brief Convert the color to RGB and output its hex. representation */
    inline std::string getRGBHex() const {
        const CVector rgb = getAs<RGB>();
        std::stringstream stream;
        stream << "#"
               << std::setfill ('0')
               << std::setw(2) << std::hex << int(std::round(Scalar(255.)*rgb(0)))
               << std::setw(2) << std::hex << int(std::round(Scalar(255.)*rgb(1)))
               << std::setw(2) << std::hex << int(std::round(Scalar(255.)*rgb(2)));

        assert(isValidRGBHex(stream.str()));
        return stream.str();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Setters
    ////////////////////////////////////////////////////////////////////////////

    /*! \brief Convert color to arbitrary color space */
    template<Space inSpace, typename CType>
    inline void setFrom( const CType& rawvec ) {
        _buf = internal::ConvertionHelper<inSpace, _space>::convert(rawvec);
    }

    /*! \brief Convert color to arbitrary color space */
    template<UnscaledSpace inSpace, typename CType>
    inline void setFromUnscaled( const CType& rawvec ) {
        constexpr Space sp = UnscaledSpaceProperties<inSpace>::scaledSpace();
        _buf =
        // convert to inSpace scaled space
        ColorBase<Scalar,sp>(internal::UnscalingHelper<sp, inSpace>::revert(rawvec))
        // convert to this scale
                .template getAs<_space>();
    }

    /*! \brief Convenience function: */
    template <typename CType>
    inline void setNative (const CType& raw) { _buf = raw; }

    /*! \brief Convenience function */
    template <typename CType>
    inline void setUnscaledNative (const CType& raw)
    { setFromUnscaled<getUnscaledSpace()>(raw); }
    //{ _buf = internal::UnscalingHelper<_space, getUnscaledSpace()>::revert(raw); }

    /*! \brief Convenience function: set color using a RGB vector */
    template <typename CType>
    inline void setRGB (const CType& rawRGB) { setFrom<Space::RGB>(rawRGB); }

    /*! \brief Convenience function: set color using a HSV vector */
    template <typename CType>
    inline void setHSV (const CType& rawHSV) { setFrom<Space::HSV>(rawHSV); }

    /*! \brief Convenience function: set color using a LaB vector */
    template <typename CType>
    inline void setLAB (const CType& rawLAB) { setFrom<Space::LAB>(rawLAB);  }

    /*! \brief Convenience function: set color using a LaB vector */
    template <typename CType>
    inline void setLUV (const CType& rawLUV) { setFrom<Space::LUV>(rawLUV);  }


    ////////////////////////////////////////////////////////////////////////////
    /// Other
    ////////////////////////////////////////////////////////////////////////////

    /*!
     * \brief compute Correlated Color Temperature using McCamy's formula
     * \return
     * Source: http://ams.com/chi/content/download/251586/993227/version/2
     *
     * Citation:
     * @article {COL:COL5080170211,
     *  author = {McCamy, C. S.},
     *  title = {Correlated color temperature as an explicit function of chromaticity coordinates},
     *  journal = {Color Research & Application},
     *  volume = {17},
     *  number = {2},
     *  publisher = {Wiley Subscription Services, Inc., A Wiley Company},
     *  issn = {1520-6378},
     *  url = {http://dx.doi.org/10.1002/col.5080170211},
     *  doi = {10.1002/col.5080170211},
     *  pages = {142--144},
     *  year = {1992},
     *  }
     */
    inline Scalar computeCCT() const {
        CVector rgb = getUnscaledAs<UnscaledSpace::RGB_255>();
        const Scalar&r = rgb.data()[0];
        const Scalar&g = rgb.data()[1];
        const Scalar&b = rgb.data()[2];
    #if 0
        // Gives better performances, but change the results.
        const Scalar n = (Scalar(0.23881)*r +Scalar( 0.25499)*g +Scalar(-0.58291)*b) /
                   (Scalar(0.11109)*r +Scalar(-0.85406)*g +Scalar( 0.52289)*b);
        const Scalar n2 = n * n;
        return  Scalar(449)    * n2 * n +
                Scalar(3525)   * n2 +
                Scalar(6823.3) * n +
                Scalar(5520.33);
    #else
        Scalar n = (Scalar(0.23881)*r +Scalar( 0.25499)*g +Scalar(-0.58291)*b) /
                   (Scalar(0.11109)*r +Scalar(-0.85406)*g +Scalar( 0.52289)*b);
        return  Scalar(449)    * std::pow(n,3) +
                Scalar(3525)   * std::pow(n,2) +
                Scalar(6823.3) * n +
                Scalar(5520.33);
    #endif
    }

    /*!
     * \brief compute color temperature using Ou et al. formula. Reference found
     * in the paper SPRWeb: Preserving Subjective Responses to Website Colour
     * Schemes Through Automatic Recolouring
     *
     * Other metrics can be implemented: activity and weight.
     *
     * Citation:
     * @article {COL:COL20010,
     * author = {Ou, Li-Chen and Luo, M. Ronnier and Woodcock, Andrée and Wright, Angela},
     * title = {A study of colour emotion and colour preference. Part I: Colour emotions for single colours},
     * journal = {Color Research & Application},
     * volume = {29},
     * number = {3},
     * publisher = {Wiley Subscription Services, Inc., A Wiley Company},
     * issn = {1520-6378},
     * url = {http://dx.doi.org/10.1002/col.20010},
     * doi = {10.1002/col.20010},
     * pages = {232--240},
     * year = {2004},
     * }
     */
    inline Scalar computeTemperature() const {
        CVector lab = getUnscaledAs<UnscaledSpace::Lab_128>();
        const Scalar&a      = lab.data()[1];
        const Scalar&b      = lab.data()[2];
        const Scalar chroma = std::sqrt(a*a+b*b);
        const Scalar hue    = std::atan2(b, a);
        return   Scalar(-0.5) + Scalar(0.02)
               * std::pow(chroma,Scalar(1.07))
               * std::cos(hue - Scalar(50));
    }


    ////////////////////////////////////////////////////////////////////////////
    /// Initializers
    ////////////////////////////////////////////////////////////////////////////

    static inline
    ColorBase<Scalar, space> Random() {
        return ColorBase<Scalar, RGB>(CVector::Random().array().abs());
    }

    static inline
    ColorBase<Scalar, space> Zero() {
        return ColorBase<Scalar, RGB>(CVector::Zero());
    }

    static inline
    ColorBase<Scalar, space> Black() {
        return ColorBase<Scalar, RGB>::Zero();
    }

    //! \param g gray level in [0:1] (black->white)
    static inline
    ColorBase<Scalar, space> Gray(Scalar g) {
        return ColorBase<Scalar, RGB>(g*CVector::Ones());
    }

    static inline
    ColorBase<Scalar, space> White() {
        return ColorBase<Scalar, RGB>(CVector::Ones());
    }

    static inline
    bool isValidRGBHex(const std::string &hex) {
        return hex.length() == 7 &&  hex.at(0) == '#';
    }

    //! \brief Return black in case of errors
    static inline
    ColorBase<Scalar, space> fromRGBHex(const std::string &hex,
                                        bool *ok = nullptr) {
        ColorBase<Scalar, space> output = Black();

        bool ret;

        if ((ret = isValidRGBHex(hex))){

            int r = std::stoi(hex.substr(1,2), nullptr, 16);
            int g = std::stoi(hex.substr(3,2), nullptr, 16);
            int b = std::stoi(hex.substr(5,2), nullptr, 16);
            output.template setFrom<RGB>(CVector(Scalar(r)/Scalar(255),
                                             Scalar(g)/Scalar(255),
                                             Scalar(b)/Scalar(255)));
        }

        if (ok != nullptr) *ok = ret;

        return output;
    }

}; // class ColorBase

template <typename Scalar, Space space>
inline ColorBase<Scalar, space> operator*(Scalar t,
                                          const ColorBase<Scalar, space>&col){
    return col*t;
}

template <typename _Scalar>
struct ColorFactory{
    using Scalar = _Scalar;

    template<typename FunctorT>
    static inline
    void generateAndProcess(Space space, FunctorT& fun){
        switch (space) {
        case Color::RGB: fun( ColorBase<Scalar, Color::RGB>() ); return;
        case Color::HSV: fun( ColorBase<Scalar, Color::HSV>() ); return;
        case Color::LAB: fun( ColorBase<Scalar, Color::LAB>() ); return;
        case Color::LUV: fun( ColorBase<Scalar, Color::LUV>() ); return;
        default: return;
        }
    }
};

} // namespace Color

template <typename Scalar>
using RGBColor = Color::ColorBase<Scalar, Color::Space::RGB>;

template <typename Scalar>
using HSVColor = Color::ColorBase<Scalar, Color::Space::HSV>;

template <typename Scalar>
using LABColor = Color::ColorBase<Scalar, Color::Space::LAB>;

template <typename Scalar>
using LUVColor = Color::ColorBase<Scalar, Color::Space::LUV>;

#endif // COLOR_HPP
