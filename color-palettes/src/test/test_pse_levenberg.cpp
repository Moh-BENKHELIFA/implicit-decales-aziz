#include <iostream>
#include <Eigen/Dense>

#include <pse/pse_levenberg_utils.hpp>

namespace Test_Levenberg{


template<typename _Scalar, int _Dim>
struct my_functor : Eigen::DenseFunctor<_Scalar>
{
    typedef _Scalar Scalar;
    enum {Dim=2};
    enum {Constraints=3};

    using Base = Eigen::DenseFunctor<Scalar>;
    using InputType    = typename Base::InputType;
    using ValueType    = typename Base::ValueType;
    using JacobianType = typename Base::JacobianType;
    using QRSolver     = typename Base::QRSolver;

    inline my_functor(void): Base(Dim,Constraints) {
        static_assert(Dim == _Dim, "Invalid dimension");
    }
    int operator()(const InputType &x, ValueType &fvec) const
    {
        // Implement y = 10*(x0+3)^2 + (x1-5)^2
        fvec(0) = 10.0*pow(x(0)+3.0,2) ;
        fvec(1) = x(0)*x(0);
        fvec(2) = pow(x(1)-5.0,2);

        return 0;
    }
};


}


int main(int /*argc*/, char */*argv*/[])
{
    using namespace Test_Levenberg;


    int maxfev = 2000;

    typedef double Scalar;
    enum {Dim = 2};
    typedef Utils::Levenberg::Functor_w_df< my_functor<Scalar,Dim> > FunctorType;

    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> x;
    x = Eigen::Matrix<Scalar, Eigen::Dynamic, 1> (int(Dim));
    x << Scalar(2.), Scalar(3.);
    std::cout << "x: " << x.transpose() << std::endl;

    FunctorType functor;
    Eigen::LevenbergMarquardt<FunctorType> lm(functor);
    lm.setXtol(1.0e-10);
    lm.setMaxfev(maxfev);
    std::cout << "parameters.maxfev: " << maxfev << std::endl;

    Eigen::LevenbergMarquardtSpace::Status ret = lm.minimize(x);
    std::cout << "lm.iter: " << lm.iterations() << std::endl;
    Utils::Levenberg::printStatus(std::cout, ret);

    std::cout << "x that minimizes the function: " << x.transpose() << std::endl;

    //std::cout << "press [ENTER] to continue " << std::endl;
    //std::cin.get();
    return EXIT_SUCCESS;
}
