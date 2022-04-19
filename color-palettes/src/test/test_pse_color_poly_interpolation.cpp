#include "test_pse_color_problem.hpp"

#include <iostream>
#include <Eigen/Dense>

#include <Functionnal/functionnal.h>
#include <pse/pse_levenberg_utils.hpp>

namespace Test_Splines{

typedef double Scalar;
enum{Dim=3};
enum{Degree=1};

typedef Functionnal::PolynomialMap<Scalar, Degree, 1> ComponentFunctor;
typedef Functionnal::ConstPolynomialMap<Scalar, Degree, 1> ConstComponentFunctor;

//! \brief RGB Color, right now no logic but just to avoid confusion
typedef typename Eigen::Matrix<Scalar, Dim, 1> RGBColor;

typedef Problem<RGBColor, ComponentFunctor, ConstComponentFunctor, 100>  ProblemType;
typedef Utils::Levenberg::Functor_w_df< ProblemType> FunctorType;


void initPolynomialProblem(FunctorType& p){
    // Initialize polynomials to [0.5] x + [0.5]
    p.visitPolynomials( [] ( typename FunctorType::ComponentInterFun&poly) {
        poly.coeffs = FunctorType::ComponentInterFun::CoeffType::Zero(); // set coeffs to 0
        poly.coeffs(0) = 0.0;
        poly.coeffs(1) = 0.5;
    } );


    // Initialize colors
//    p.s0[0] = RGBColor::Zero();// good solution : 0.5x
//    p.s1[0] = RGBColor::Ones() * Scalar(0.5);

//    p.s0[1] << 0.5, 0.5, 0.5; // good solution : 0.5x + 0.5
//    p.s1[1] << 1.0, 1.0, 1.0;
    // Initialize colors
    p.s0[0] = RGBColor::Zero();// good solution : 0.5x
    p.s1[0] = RGBColor::Ones() * 0.5;

    p.s0[1] << 0.1, 0.2, 0.3; // good solution : 0.5x + 0.5
    p.s1[1] << 0.6, 0.7, 0.8;
}

} // namespace Test_Splines

////////////////////////////////////////////////////////////////////////////////
/// Main program
////////////////////////////////////////////////////////////////////////////////
int main(int /*argc*/, char */*argv*/[])
{
    using std::cout;
    using std::endl;

    using namespace Test_Splines;


    FunctorType problem (0.001);
    initPolynomialProblem(problem);

    std::cout << "Input: " << std::endl;
    problem.visitPolynomials( [] ( typename FunctorType::ComponentInterFun&poly) {
        std::cout << poly << std::endl;
    } );

    int maxfev = 2000;

    Eigen::LevenbergMarquardt<FunctorType> lm(problem);
    lm.setMaxfev(maxfev);
    //lm.parameters.ftol = 1.0e-1;
    //lm.parameters.xtol = 1.0e-1;
    std::cout << "parameters.maxfev: " << maxfev << std::endl;

    Eigen::LevenbergMarquardtSpace::Status ret = lm.minimize( problem.variables );
    std::cout << "lm.iter: " << lm.iterations() << std::endl;
    Utils::Levenberg::printStatus(std::cout, ret);

    std::cout << "x that minimizes the function: " /*<< problem.variables.transpose()*/ << std::endl;
    problem.visitPolynomials( [] ( typename FunctorType::ComponentInterFun&poly) {
        std::cout << poly << std::endl;
    } );
    return EXIT_SUCCESS;
}
