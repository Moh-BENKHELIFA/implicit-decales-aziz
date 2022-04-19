#ifndef COLORVISIONDEFICIENCIES_HPP
#define COLORVISIONDEFICIENCIES_HPP

#include <pse/color/pse_color.hpp>
#include <pse/color/pse_color_gamut.hpp>

namespace Color{


//! This namespace present simulations of CVDs.
namespace CVD {


enum VISION_TYPE {
    VISION_NORMAL       = 0,
    VISION_DEUTERANOPIA = 1,
    VISION_PROTANOPIA   = 2
};

namespace Utils {

template <typename Scalar>
struct Line{
    Scalar a, b;

    //constexpr Line () : a(0), b(0){}
    Line (Scalar x0, Scalar y0, Scalar x1, Scalar y1)
        : a((y1-y0)/(x1-x0)), b(y0 - a*x0) {}

    bool getIntersectionPoint(const Line& other,
                              Scalar &x,
                              Scalar &y){
        if (other.a == this->a) return false;

        x = (other.b - this->b) / (this->a - other.a);
        y = this->operator ()(x);

        return true;
    }

    inline Scalar operator() (Scalar x) const { return this->a*x + this->b; }
};

template <typename Scalar>
struct Converter{
    static constexpr Scalar xy2u(Scalar x, Scalar y){
        return Scalar(4)*x / (Scalar(12)*y-Scalar(2)*x+3);
    }
    static constexpr Scalar xy2v(Scalar x, Scalar y){
        return Scalar(9)*y / (Scalar(12)*y-Scalar(2)*x+3);
    }
    static constexpr Scalar uv2x(Scalar u, Scalar v){
        return Scalar(9)*u / (Scalar(2)*(Scalar(3)*u-Scalar(8)*v+Scalar(6)));
    }
    static constexpr Scalar uv2y(Scalar u, Scalar v){
        return Scalar(2)*v / (Scalar(3)*u-Scalar(8)*v+Scalar(6));
    }
    static constexpr Scalar XYZ2x(Scalar X, Scalar Y, Scalar Z){
        return X / (X+Y+Z);
    }
    static constexpr Scalar XYZ2y(Scalar X, Scalar Y, Scalar Z){
        return Y / (X+Y+Z);
    }
    static constexpr Scalar xyY2X(Scalar x, Scalar y, Scalar Y){
        return x*Y/y;
    }
    static constexpr Scalar xyY2Z(Scalar x, Scalar y, Scalar Y){
        return (Scalar(1)-x-y)*Y/y;
    }
};



template<typename Scalar>
struct ConfDeuteranopia {
    static constexpr Scalar u = -4.75;
    static constexpr Scalar v =  1.31;
    static const Utils::Line<Scalar> line;
};
template<typename Scalar>
const Utils::Line<Scalar> ConfDeuteranopia<Scalar>::line = Utils::Line<Scalar> (
                    Utils::Converter<Scalar>::xy2u(0.102775863, 0.102863739),
                    Utils::Converter<Scalar>::xy2v(0.102775863, 0.102863739),
                    Utils::Converter<Scalar>::xy2u(0.505845283, 0.493211177),
                    Utils::Converter<Scalar>::xy2v(0.505845283, 0.493211177));


template<typename Scalar>
struct ConfProtanopia {
    static constexpr Scalar u =  0.61;
    static constexpr Scalar v =  0.51;
    static const Utils::Line<Scalar> line;
};
template<typename Scalar>
const Utils::Line<Scalar> ConfProtanopia<Scalar>::line = Utils::Line<Scalar> (
                    Utils::Converter<Scalar>::xy2u(0.115807359, 0.073580708),
                    Utils::Converter<Scalar>::xy2v(0.115807359, 0.073580708),
                    Utils::Converter<Scalar>::xy2u(0.471898745, 0.52705057),
                    Utils::Converter<Scalar>::xy2v(0.471898745, 0.52705057));

} // namespace Utils

/*!
 * \brief The Troiano2008 class implements simulations proposed in
 *   Rasche, K.; Geist, R.; Westall, J.,
 *   Detail preserving reproduction of color images for monochromats and dichromats,
 *   in Computer Graphics and Applications, IEEE , vol.25, no.3, pp.22-30, May-June 2005
 *   doi: 10.1109/MCG.2005.54
 *   http://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=1438255&isnumber=30967
 */
class Rasche2005 {
protected:
    static constexpr Color::UnscaledSpace LMSSpace = UnscaledSpace::LMS_Cat02;
    static constexpr Color::Space LMSSpaceUnscaled = UnscaledSpaceProperties<LMSSpace>::scaledSpace();
    using GamutHelper = Color::InUnitspaceClampingHelper;

    template <template<typename> class Conf, typename Scalar, Color::Space space>
    static ColorBase<Scalar, space> _convert( const ColorBase<Scalar, space>& well){
        using Converter = Utils::Converter<Scalar>;

        typename ColorBase<Scalar, space>::CVector RGBin = well.getRGB();

        // the color need to be stored in rgb to be correctly clamped at the end
        // of the process.
        ColorBase<Scalar, Color::RGB> outColor;

        num Xin,Yin,Zin;
        Rgb2Xyz(&Xin,&Yin,&Zin,RGBin(0), RGBin(1), RGBin(2));

        // go to xy space
        Scalar x = Converter::XYZ2x(Xin, Yin, Zin);
        Scalar y = Converter::XYZ2y(Xin, Yin, Zin);

        // go to uv space
        Scalar u = Converter::xy2u(x,y);
        Scalar v = Converter::xy2v(x,y);

        Utils::Line<Scalar> Luv(Conf<Scalar>::u, Conf<Scalar>::v, u, v);

        // go to uv space
        Scalar Outu, Outv;
        if (Luv.getIntersectionPoint(Conf<Scalar>::line, Outu, Outv)) {

            Scalar outx = Converter::uv2x(Outu, Outv);
            Scalar outy = Converter::uv2y(Outu, Outv);

            Scalar Xout, Yout, Zout;
            Xout = Converter::xyY2X(outx, outy, Yin);
            Yout = Yin;
            Zout = Converter::xyY2Z(outx, outy, Yin);

            num Rout, Gout, Bout;
            Xyz2Rgb(&Rout, &Gout, &Bout, Xout, Yout, Zout);

            outColor.setRGB(typename ColorBase<Scalar, space>::
                            CVector(Rout, Gout, Bout));
        }
        return GamutHelper::process(outColor);
    }

public:
    template <typename Scalar, Color::Space space >
    static ColorBase<Scalar, space> well2protanopes(
            const ColorBase<Scalar, space>& well){
        return _convert<Utils::ConfProtanopia>(well);
    }


    template <typename Scalar, Color::Space space >
    static ColorBase<Scalar, space> well2deuteranopes(
            const ColorBase<Scalar, space>& well){
        return _convert<Utils::ConfDeuteranopia>(well);
    }

    template <VISION_TYPE vtype, typename Scalar, Color::Space space >
    static ColorBase<Scalar, space> process(
            const ColorBase<Scalar, space>& well){
        switch (vtype){
        case VISION_DEUTERANOPIA: return well2deuteranopes(well);
        case VISION_PROTANOPIA:   return well2protanopes(well);
        default:                  return well;
        }
    }

    template <typename Scalar, Color::Space space >
    static ColorBase<Scalar, space> process(
            VISION_TYPE vtype,
            const ColorBase<Scalar, space>& well){
        switch (vtype){
        case VISION_DEUTERANOPIA: return well2deuteranopes(well);
        case VISION_PROTANOPIA:   return well2protanopes(well);
        default:                  return well;
        }
    }

};

/*!
 * \brief The Troiano2008 class implements simulations proposed in
 *   Luigi Troiano, Cosimo Birtolo, and Maria Miranda. 2008.
 *   Adapting palettes to color vision deficiencies by genetic algorithm.
 *   In Proceedings of the 10th annual conference on Genetic and evolutionary
 *   computation (GECCO '08), Maarten Keijzer (Ed.).
 *   ACM, New York, NY, USA, 1065-1072.
 *   DOI=http://dx.doi.org/10.1145/1389095.1389291
 *
 * \deprecated
 */
class Troiano2008 {
protected:
    static constexpr Color::UnscaledSpace LMSSpace = UnscaledSpace::LMS_Cat02;
    static constexpr Color::Space LMSSpaceUnscaled = UnscaledSpaceProperties<LMSSpace>::scaledSpace();
    using GamutHelper = Color::InUnitspaceClampingHelper;

    template <typename Scalar, Color::Space space, typename Derived >
    static ColorBase<Scalar, space> _convert(
            ColorBase<Scalar, space>& well,
            const Eigen::MatrixBase<Derived>& M){
        using ColorT     = ColorBase<Scalar, space>;
        using LMSColorT  = ColorBase<Scalar, LMSSpaceUnscaled>;
        using CVector = typename ColorT::CVector;

        LMSColorT compatWell = well;

        // do alterations
        CVector wellVec = M * compatWell.template getUnscaledAs<LMSSpace>();
        compatWell.template setFromUnscaled<LMSSpace>(wellVec);

        // convert back to the right space
        return ColorBase<Scalar, space> (GamutHelper::process(compatWell));
    }

public:

    template <typename Scalar, Color::Space space >
    static ColorBase<Scalar, space> well2protanopes(
            ColorBase<Scalar, space>& well){
        static constexpr std::array<Scalar, 9> M
        {
            0.      ,  2.02344, -2.52582,
            0.      ,  1.     ,  0.,
            0.      ,  0.     ,  1.
        };
        return _convert(well, Eigen::Map<const Eigen::Matrix<Scalar, 3, 3>> (M.data()));
    }


    template <typename Scalar, Color::Space space >
    static ColorBase<Scalar, space> well2deuteranopes(
            ColorBase<Scalar, space>& well){
        static constexpr std::array<Scalar, 9> M
        {
            1.      ,  0.     ,  0.     ,
            0.494207,  0.     ,  1.24827,
            0.      ,  0.     ,  1.
        };
        return _convert(well, Eigen::Map<const Eigen::Matrix<Scalar, 3, 3>> (M.data()));
    }

};



} // namespace CVD
} // namespace Color


#endif // COLORVISIONDEFICIENCIES_HPP

