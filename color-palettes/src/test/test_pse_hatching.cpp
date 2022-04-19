#include <iostream>
#include <Eigen/Dense>

#include <Functionnal/functionnal.h>
#include <Functionnal/constrainedBezier.h>
#include <interpo-lib/solver_interpolation.hpp>
#include <interpo-lib/levenberg_utils.hpp>
#include <interpo-lib/color.hpp>
#include <interpo-lib/hatching.hpp>
#include <interpo-lib/discrepancy.hpp>

namespace HatchingDiscrepancy{

/*!
 * \brief Computes the constrast between the two hatching, without considering
 * the angle.
 *
 * The constrast is obtained by computing the difference of hatch occupancies,
 * defined as the width of the hatched line measured relatively to the spacing.
 *
 */
template<class _Hatching>
struct MeanContrast : public Discrepancy::BinaryDiscrepancyFunctor<_Hatching> {
    INTERPOLIB_TYPENAME_DECLARE("hatchingdiscr_meanContrast")
    using Base          = Discrepancy::BinaryDiscrepancyFunctor<_Hatching>;
    using HatchingT     = typename Base::ParametricPoint;
    using Scalar        = typename Base::Scalar;
    using DiscrOutType  = typename Base::DiscrOutType;
    using ParameterSet  = typename Base::ParameterSet;
    enum { DiscrOutSize = Base::DiscrOutSize };

private:
    using ExpectedHatching = Hatching::HatchingBase<Scalar, Base::ParametricPoint::space>;
    static_assert(std::is_same< HatchingT, ExpectedHatching>::value,
                  "MeanContrast requires hatching parameters");

public:
    virtual DiscrOutType eval( const ParameterSet& l, Scalar /*t*/) const{
        typename HatchingT::CVector h1vec = (l.begin()  )->
                template getUnscaledAs<Hatching::UDefault>();
        typename HatchingT::CVector h2vec = (l.begin()+1)->
                template getUnscaledAs<Hatching::UDefault>();
        Scalar spacing1 = h1vec(1);
        Scalar spacing2 = h2vec(1);
        Scalar thickness1 = h1vec(2);
        Scalar thickness2 = h2vec(2);

        // compute the width of a line + spacing
        Scalar interval1 = spacing1 + thickness1;
        Scalar interval2 = spacing2 + thickness2;

        return std::abs(thickness1 / interval1 -
                        thickness2 / interval2);
    }
    virtual ~MeanContrast() {}
};



//! Compute L1 distance in angle space, normalized in [0:1]
//! \deprecated
template<class _Hatching>
struct AngleDistance : public Discrepancy::BinaryDiscrepancyFunctor<_Hatching> {
    INTERPOLIB_TYPENAME_DECLARE("hatchingdiscr_angleDistance")
    using Base          = Discrepancy::BinaryDiscrepancyFunctor<_Hatching>;
    using HatchingT     = typename Base::ParametricPoint;
    using Scalar        = typename Base::Scalar;
    using DiscrOutType  = typename Base::DiscrOutType;
    using ParameterSet  = typename Base::ParameterSet;
    enum { DiscrOutSize = Base::DiscrOutSize };

private:
    using ExpectedHatching = Hatching::HatchingBase<Scalar, Base::ParametricPoint::space>;
    static_assert(std::is_same< HatchingT, ExpectedHatching>::value,
                  "AngleDistance requires hatching parameters");

public:
    virtual DiscrOutType eval( const ParameterSet& l, Scalar /*t*/) const{
        static constexpr Scalar half = Scalar(180);
        Scalar norm = (l.begin()+1)->template getUnscaledAs<Hatching::UDefault>()(0) -
                      (l.begin()  )->template getUnscaledAs<Hatching::UDefault>()(0);
        return DiscrOutType(std::abs((norm > half ? norm - half : norm)/half));
    }
    virtual ~AngleDistance() {}
};
} // namespace HatchingDiscrepancy


namespace Test_Hatching{

struct ClampingHelper{
    //! \brief Default implementation, assuming the ambient space to be [0:1]*
    template <typename CVector>
    static inline const CVector& process(const CVector& v){
        return v;
        //using Scalar = typename CVector::Scalar;
        //return v.unaryExpr( std::function<Scalar(Scalar)>
        //                   (internal::Clamp<Scalar>(0., 1.)));
    }
};


typedef double Scalar;

static constexpr Hatching::Space ProjSpace = Hatching::Default;
using ParametricPoint = Hatching::HatchingBase<Scalar, ProjSpace>;

enum{Dim=ParametricPoint::SpaceDim};
enum{Degree=3};

using ParameterValidationHelper = ::Color::InUnitspaceClampingHelper;

using ComponentFunctor      = Functionnal::ConstrainedBezierMap<Scalar, Degree, Dim> ;
using ConstComponentFunctor = Functionnal::ConstConstrainedBezierMap<Scalar, Degree, Dim> ;

using DiscrepancyMeasure = Discrepancy::NaryDiscrepancyFunctorAddOp<
        HatchingDiscrepancy::AngleDistance<ParametricPoint>,
        HatchingDiscrepancy::MeanContrast<ParametricPoint> >;

using ProblemType = SolverInterpolation<ParametricPoint,
                        ComponentFunctor,
                        ConstComponentFunctor,
                        ParameterValidationHelper,
                        100>  ;
using ProblemT = Utils::Levenberg::Functor_w_df<ProblemType> ;


} // namespace Test_Hatching

template <int nbRun>
bool
runTest(){
    return true;
}

int main(int /*argc*/, char */*argv*/[])
{
    using namespace Test_Hatching;

    ProblemT problem;
    Eigen::LevenbergMarquardt<ProblemT> solver( problem );

    DiscrepancyMeasure measure;
    typename DiscrepancyMeasure::FirstFunctor  submeasure1;
    typename DiscrepancyMeasure::SecondFunctor submeasure2;
    ParametricPoint h1 = ParametricPoint::Random();
    ParametricPoint h2 = ParametricPoint::Random();
    ParametricPoint h3 = ParametricPoint::Random();

    std::cout << "h1: " << h1.getUnscaledAs<Hatching::UDefault>().transpose() << std::endl;
    std::cout << "h2: " << h2.getUnscaledAs<Hatching::UDefault>().transpose() << std::endl;
    std::cout << "h3: " << h3.getUnscaledAs<Hatching::UDefault>().transpose() << std::endl;

    std::cout << "// First submeasure" << std::endl;

    std::cout << "Contrast (1,2): " << submeasure1.eval({h1, h2}, 0) << std::endl;
    std::cout << "Contrast (2,1): " << submeasure1.eval({h2, h1}, 0) << std::endl;
    std::cout << "Contrast (1,1): " << submeasure1.eval({h1, h1}, 0) << std::endl;
    std::cout << "Contrast (1,3): " << submeasure1.eval({h1, h3}, 0) << std::endl;
    std::cout << "Contrast (2,3): " << submeasure1.eval({h2, h3}, 0) << std::endl;

    std::cout << "// Second submeasure" << std::endl;

    std::cout << "Contrast (1,2): " << submeasure2.eval({h1, h2}, 0) << std::endl;
    std::cout << "Contrast (2,1): " << submeasure2.eval({h2, h1}, 0) << std::endl;
    std::cout << "Contrast (1,1): " << submeasure2.eval({h1, h1}, 0) << std::endl;
    std::cout << "Contrast (1,3): " << submeasure2.eval({h1, h3}, 0) << std::endl;
    std::cout << "Contrast (2,3): " << submeasure2.eval({h2, h3}, 0) << std::endl;

    std::cout << "// Combined measures" << std::endl;

    std::cout << "Contrast (1,2): " << measure.eval({h1, h2}, 0) << std::endl;
    std::cout << "Contrast (2,1): " << measure.eval({h2, h1}, 0) << std::endl;
    std::cout << "Contrast (1,1): " << measure.eval({h1, h1}, 0) << std::endl;
    std::cout << "Contrast (1,3): " << measure.eval({h1, h3}, 0) << std::endl;
    std::cout << "Contrast (2,3): " << measure.eval({h2, h3}, 0) << std::endl;

    //solver.minimize( problem.variables );

    return runTest<50>() ? EXIT_SUCCESS : EXIT_FAILURE;
}
