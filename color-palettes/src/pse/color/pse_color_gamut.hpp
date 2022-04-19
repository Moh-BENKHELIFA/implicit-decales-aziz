#ifndef INGAMUTHELPER_HPP
#define INGAMUTHELPER_HPP

#include <pse/color/pse_color.hpp>
#include <Eigen/Core>

namespace Color{

namespace internal{
//! \brief Clamp vectors values
template <typename S >
struct Clamp{
  S lower, upper;
  inline Clamp(S low = 0.0, S up = 1.0) : lower(low), upper(up) {}
  inline S  operator() (S x) const { return std::max(lower, std::min(x, upper)); }
};

} // namespace internal


//! This class is in theory as efficient as InXXXspaceClampingHelper in native
//! space.
//! \FIXME Test performances against InXXXspaceClampingHelper<NativeSpace>
struct InUnitspaceClampingHelper{
    //! \brief Default implementation, assuming the ambient space to be [0:1]*
    template <typename ColorT>
    static inline ColorT process(const ColorT& v){
        using Scalar = typename ColorT::Scalar;

        static constexpr Scalar lowBound = Scalar(0.);
        static constexpr Scalar upBound  = Scalar(1.)-std::numeric_limits<Scalar>::epsilon();

        return ColorT( v.getNative().unaryExpr( std::function<Scalar(Scalar)>
                                                (internal::Clamp<Scalar>(lowBound, upBound))));
    }

    template <typename ColorT>
    static inline bool isValid(const ColorT& v){
        using Scalar = typename ColorT::Scalar;

        static constexpr Scalar lowBound = Scalar(0.);
        static constexpr Scalar upBound  = Scalar(1.)-std::numeric_limits<Scalar>::epsilon();

        const typename ColorT::CVector nat = v.getNative();

        return (nat.array() > lowBound).all() && (nat.array() < upBound).all();
    }

};

template <typename _Scalar, Color::Space _space>
struct InXXXspaceClampingHelper{
    using Scalar = _Scalar;
    static constexpr ::Color::Space space = _space;

    //! \brief Default implementation, assuming the ambient space to be [0:1]*
    template <Space cspace>
    static inline ::Color::ColorBase<Scalar, cspace> process
        (const ::Color::ColorBase<Scalar, cspace> & v){
        using MXXXCol = Color::ColorBase<Scalar, space>;

        static constexpr Scalar lowBound = Scalar(0.);
        static constexpr Scalar upBound  = Scalar(1.)-std::numeric_limits<Scalar>::epsilon();

        // MXXXCol is automatically converted by the copy operator to ColorT
        /*if (cspace == Color::HSV){
            typename MXXXCol::CVector vv (v.template getAs<space>());
            vv.template tail<2>() .unaryExpr( std::function<Scalar(Scalar)>
                                              (internal::Clamp<Scalar>(lowBound, upBound)));
            return MXXXCol(vv);
        }
        else*/
            return MXXXCol(v.template getAs<space>()
                           .unaryExpr( std::function<Scalar(Scalar)>
                                       (internal::Clamp<Scalar>(lowBound, upBound))));
    }

    //! \brief Default implementation, assuming the ambient space to be [0:1]*
    //template <typename ColorT>
    //static inline bool isValid(const ColorT& v){
    template <Space cspace>
    static inline bool isValid (const ::Color::ColorBase<Scalar, cspace> & v){
        using ColorT  = Color::ColorBase<Scalar, cspace>;
        using Scalar = typename ColorT::Scalar;

        static constexpr Scalar lowBound = Scalar(0.);
        static constexpr Scalar upBound  = Scalar(1.)-std::numeric_limits<Scalar>::epsilon();

        typename ColorT::CVector vec = v.template getAs<space>();
        //if (cspace == Color::HSV){ vec(0) = lowBound; } // ignore hue component

        return (vec.array() > lowBound).all() && (vec.array() < upBound).all();
    }
};

//template <Color::UnscaledSpace _uspace>
//struct InUnscaledXXXspaceClampingHelper{
//    static constexpr Color::UnscaledSpace uspace = _uspace;

//    //! \brief Default implementation, assuming the ambient space to be [0:1]*
//    template <typename ColorT>
//    static inline ColorT process(const ColorT& v){
//        using Scalar = typename ColorT::Scalar;
//        static const auto lowBound = Color::UnscaledSpaceProperties<uspace>::template min<Scalar>().array();
//        static const auto upBound  = Color::UnscaledSpaceProperties<uspace>::template max<Scalar>().array();

//        ColorT output;
//        output.template setFromUnscaled<uspace>(
//                     v.template getUnscaledAs<uspace>().array() // get unscaled color
//                    .max(lowBound) //clamp value below lowBound
//                    .min(upBound)  //clamp value above upbound
//                    .matrix()
//                    );

//        return output;
//    }
//};

template <typename _Scalar>
using InRGBspaceClampingHelper = InXXXspaceClampingHelper<_Scalar,Color::RGB>;
template <typename _Scalar>
using InLABspaceClampingHelper = InXXXspaceClampingHelper<_Scalar,Color::LAB>;
template <typename _Scalar>
using InHSVspaceClampingHelper = InXXXspaceClampingHelper<_Scalar,Color::HSV>;

//using InRGB_255spaceClampingHelper = InUnscaledXXXspaceClampingHelper<Color::RGB_255>;
//using InLAB_128spaceClampingHelper = InUnscaledXXXspaceClampingHelper<Color::Lab_128>;
//using InHSV_360spaceClampingHelper = InUnscaledXXXspaceClampingHelper<Color::HSV_360>;

} // namespace Color

#endif // INGAMUTHELPER_HPP
