#ifndef LEVENBERG_UTILS_HPP
#define LEVENBERG_UTILS_HPP

#include <Eigen/Dense>                     //Eigen::Matrix
#include <unsupported/Eigen/NumericalDiff> //Eigen::NumericalDiff
#include <unsupported/Eigen/LevenbergMarquardt> //Eigen::DenseFunctor
//#include <unsupported/Eigen/AutoDiff>

namespace Utils{

namespace Levenberg{
inline void printStatus(std::ostream& stream,
                        Eigen::LevenbergMarquardtSpace::Status status) {
    switch(status){
    case Eigen::LevenbergMarquardtSpace::NotStarted:
        stream << "NotStarted";
        break;
    case Eigen::LevenbergMarquardtSpace::Running:
        stream << "Running";
        break;
    case Eigen::LevenbergMarquardtSpace::ImproperInputParameters:
        stream << "ImproperInputParameters";
        break;
    case Eigen::LevenbergMarquardtSpace::RelativeReductionTooSmall:
        stream << "RelativeReductionTooSmall";
        break;
    case Eigen::LevenbergMarquardtSpace::RelativeErrorTooSmall:
        stream << "RelativeErrorTooSmall";
        break;
    case Eigen::LevenbergMarquardtSpace::RelativeErrorAndReductionTooSmall:
        stream << "RelativeErrorAndReductionTooSmall";
        break;
    case Eigen::LevenbergMarquardtSpace::CosinusTooSmall:
        stream << "CosinusTooSmall";
        break;
    case Eigen::LevenbergMarquardtSpace::TooManyFunctionEvaluation:
        stream << "TooManyFunctionEvaluation";
        break;
    case Eigen::LevenbergMarquardtSpace::FtolTooSmall:
        stream << "FtolTooSmall";
        break;
    case Eigen::LevenbergMarquardtSpace::XtolTooSmall:
        stream << "XtolTooSmall";
        break;
    case Eigen::LevenbergMarquardtSpace::GtolTooSmall:
        stream << "GtolTooSmall";
        break;
    case Eigen::LevenbergMarquardtSpace::UserAsked:
        stream << "UserAsked";
        break;
    default:
        stream << "Unknown (" << status << ")";
    };
}

//// Generic functor
//template<typename _Scalar,           //! <\tparam
//         int NX = Eigen::Dynamic,    //! <\tparam Ambiant space size
//         int NY = Eigen::Dynamic>    //! <\tparam Number of constraints
//struct Functor: public Eigen::DenseFunctor<_Scalar, NX, NY>
//{
//    using Base   = Eigen::DenseFunctor<_Scalar, NX, NY>;
//    using Scalar = typename Base::Scalar;
//    enum {
//      InputsAtCompileTime = Base::InputsAtCompileTime,
//      ValuesAtCompileTime = Base::ValuesAtCompileTime
//    };
//    using InputType    = typename Base::InputType;
//    using ValueType    = typename Base::ValueType;
//    using JacobianType = typename Base::JacobianType;
//    using QRSolver     = typename Base::QRSolver;

//    Functor() : m_inputs(InputsAtCompileTime), m_values(ValuesAtCompileTime) {}
//    Functor(int inputs, int values) : m_inputs(inputs), m_values(values) {}

//};



//! \todo Use Eigen::Autodiff
template<typename _Base>
struct Functor_w_df : public Eigen::NumericalDiff<_Base,Eigen::NumericalDiffMode::Central> {
    typedef _Base Base;
    typedef typename _Base::Scalar Scalar;

    inline Functor_w_df(Scalar _epsfcn=0.)
        :Eigen::NumericalDiff<_Base, Eigen::NumericalDiffMode::Central>(_epsfcn){}

    virtual ~Functor_w_df() {}
};
}
}

#endif // LEVENBERG_UTILS_HPP
