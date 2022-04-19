#ifndef PROBLEM_H
#define PROBLEM_H

#include <pse/pse_levenberg_utils.hpp>

namespace Test_Splines{
////////////////////////////////////////////////////////////////////////////////
/// Discrepancy functors
////////////////////////////////////////////////////////////////////////////////
namespace Discrepancy{

template<class _Element>
struct L2DistanceFunctor{
    typedef _Element Element;
    typedef typename _Element::Scalar Scalar;

    inline Scalar eval( const Element& e1, const Element& e2) const{
        // assumes norm is L2 norm
        // eigen.tuxfamily.org/dox/classEigen_1_1MatrixBase.html#a0be1b433c65ce9d92c81a4718daf54e5

        //std::cout << e1.transpose() << " VS " << e2.transpose() << std::endl;

        return (e2 - e1).norm();
    }
};

} //namespace Discrepancy

////////////////////////////////////////////////////////////////////////////////
/// Problem
///
/// Designed as a Levenberg Marquardt functor, with
///  -x:    parameters of the different polynoms used for interpolation, right
///         now we have 3rd order polygon per color component, so 4*3 scalar
///         values
///  -cost: distance functor, for now only one corresponding to the
///         l2Distance functor
////////////////////////////////////////////////////////////////////////////////
template<class _Color,
         typename _ComponentInterFun,
         typename _ConstComponentInterFun,
         int NbParametricSteps = 10>
struct Problem: public Eigen::DenseFunctor<
        typename _Color::Scalar>{       // constraint number, right now only one (L2norm)
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    typedef _Color Color;
    typedef typename Color::Scalar Scalar;
    enum{  ParametricSpaceDim = Color::RowsAtCompileTime };
    typedef _ComponentInterFun ComponentInterFun;
    typedef _ConstComponentInterFun ConstComponentInterFun;

    using Base = Eigen::DenseFunctor<typename _Color::Scalar>;
    using InputType    = typename Base::InputType;
    using ValueType    = typename Base::ValueType;
    using JacobianType = typename Base::JacobianType;
    using QRSolver     = typename Base::QRSolver;

    // Colors
    enum {NbCol = 2}; //! Number of colors. Need to be dynamically set
    typedef std::array<Color, NbCol> Style;

    Style s0, s1;

    //! Component-wise color interpolation functors
    //std::vector<ComponentInterFun> interFunArray;

    /*!
     * \brief Variable dimension number
     *
     * \warning Assumes for now that Degree is constant and applied to the
     * component of a single pair of color
     *
     */
    enum{
        PerEdgeConstraint = 1, //number of constraints per edge
        NbConstraints     = PerEdgeConstraint * NbCol*ParametricSpaceDim
    };

    //! Discrepency measure
    Discrepancy::L2DistanceFunctor<Color> discrepancy;

    //! Variables used during the optimization, updated by the solver, and
    //! directly mapped to interFunArray
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> variables;

        /*!
     * \brief Problem
     * \param sharedVector Memory space aggregating all the variables and mapped
     *        to Polynomials
     */
    inline Problem();

private:
    //! \brief Apply a non-const functor to polynomials binded to x
    template <typename F>
    void visitPolynomials(F &f,
                          Eigen::Matrix<Scalar, Eigen::Dynamic, 1> &x);

    //! \brief Apply a const functor to polynomials binded to x
    template <typename F>
    void visitPolynomials(const F &f,
                          Eigen::Matrix<Scalar, Eigen::Dynamic, 1> &x);

public:
    /*!
     * \brief Apply a non-const functor to the problem polynomials
     * \param f a functor class implementing: operator( ComponentInterFun& p)
     *
     * Can be used either with Functor class and lambda functions:
     * \code
     * int counter = 0;
     * auto f = [&counter] ( typename ProblemType::ComponentInterFun& poly)
     *   {
     *     std::cout << poly << std::endl;
     *     ++counter;
     *   };
     * problem.visitPolynomials( f );
     * \endcode
     */
    template <typename F>
    void visitPolynomials(F &f){
        visitPolynomials(f, variables);
    }

    /*!
     * \brief Apply a const functor to the problem polynomials
     * \param f a functor class implementing: operator( ComponentInterFun& p)
     *
     * Can be used either with Functor class and lambda functions:
     * \code
     * problem.visitPolynomials(
     *   [] ( typename ProblemType::ComponentInterFun& poly)
     *     {
     *       std::cout << poly << std::endl;
     *     } );
     * \endcode
     */
    template <typename F>
    void visitPolynomials(const F &f){
        visitPolynomials(f, variables);
    }

//    /*!
//     * \return the segment corresponding to the interpolation functor for a
//     * given color and component
//     *
//     * Can be used to initialize an Interpolation functor (e.g.PolynomialMap):
//     * \code
//     * ComponentInterFun poly (  )
//     * \endcode
//     */
//    inline
//    Eigen::ConstSegmentReturnType getComponentSegment(
//            int colorId,
//            int componentId) const{
//        return x.segment(ComponentInterFun::NbCoeff * colorId *, ComponentInterFun::NbCoeff).data();
//    }

public:
    //! Operator called by the solver and computing the energy term in fvec
    inline
    int operator()(const InputType &x, ValueType &fvec) const;
};



template <class StreamT, class C, typename T, typename TC, int S>
inline
StreamT& operator<< (StreamT& stream, const Problem<C, T, TC, S> &p){
    using std::endl;

    stream << "Interpolating between : \n"
           << "\t" << p.h0.transpose() << "\t->\t" << p.h1.transpose() << "\n"
           << "\t" << p.f0.transpose() << "\t->\t" << p.f1.transpose() << endl;

    //stream << "m(h0, h1) = " << p.m.eval(p.h0, p.h1) << endl;
    //stream << "m(f0, f1) = " << p.m.eval(p.f0, p.f1) << endl;

    return stream;
}
} // namespace Test_Splines

#include "test_pse_color_problem.inl"


#endif // PROBLEM_H
