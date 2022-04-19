#ifndef PROBLEM_HPP
#define PROBLEM_HPP

// The following directive is used only to get code completion and doesn't pass
// the pre-compilation stage.
#ifndef PROBLEM_H
#include "test_pse_color_problem.hpp"
#endif


namespace Test_Splines{
template<class _Color,
         typename _ComponentInterFun,
         typename _ConstComponentInterFun,
         int NbParametricSteps>
Problem<_Color, _ComponentInterFun, _ConstComponentInterFun, NbParametricSteps>::
Problem():
    Base(
        //! right now we have one polynomial per color dimension
        NbCol * ParametricSpaceDim * ComponentInterFun::NbCoeff,

        //! here should be Constraints, however this looks unconstrained for
        //! the solver since we manually combine costs in the eval function.
        NbCol * ParametricSpaceDim * ComponentInterFun::NbCoeff
          ) {
    //static_assert(ComponentInterFun::Degree > 0,
    //              "You need at least linear polynomial to define constraints");

    variables.resize(Base::m_inputs);

    //mapMemory(variables, interFunArray);
}

template<class _Color,
         typename _ComponentInterFun,
         typename _ConstComponentInterFun,
         int NbParametricSteps>
int
Problem<_Color, _ComponentInterFun, _ConstComponentInterFun, NbParametricSteps>::
operator()(const InputType &x, ValueType &fvec) const
{
    //const int funcOffset = ParametricSpaceDim*ComponentInterFun::NbCoeff;

    // interpolated colors
    Color c1, c2;

    // cumulated cost
    Scalar cost (0);

    Scalar discrRef0 = discrepancy.eval(s0[0], s1[0]);
    Scalar discrRef1 = discrepancy.eval(s0[1], s1[1]);


    int paramOffset = 0; //! Offset to retrieve current functor parameter
    //! We build it incrementally to support non-uniform
    //! degrees between the different polynomials

    // for now work only on the two first colors
    int col2Idx = ParametricSpaceDim * ComponentInterFun::NbCoeff;

    // estimate cost for different parametric step, and for each component
    // we loop over the parametric space first so we can estimate directly
    // the colors and thus the associated cost
    for( int s = 0; s != NbParametricSteps; ++s){
        Scalar u = Scalar(s) / Scalar(NbParametricSteps);
        typename ComponentInterFun::EmbeddedVectorType uvec;
        uvec << u;

        paramOffset = 0;

        for(int i = 0; i != ParametricSpaceDim; i++){
//                // we could use mapped functors here, but this is not
//                // compatible with autodiffs, which call this function with
//                // a different x parameter to evaluate the Jacobian matrix
//                //const ComponentInterFun& func1 = interFunArray[i];
//                //const ComponentInterFun& func2 = interFunArray[i + funcOffset];
//                //
//                // \FIXME loops should be reverted now

            const ConstComponentInterFun poly1 (
                        x.segment(paramOffset,
                                  ComponentInterFun::NbCoeff).data());
            const ConstComponentInterFun poly2 (
                        x.segment(paramOffset + col2Idx,
                                  ComponentInterFun::NbCoeff).data());

                paramOffset += ComponentInterFun::NbCoeff;

                c1(i) = poly1.eval(uvec)(0);
                c2(i) = poly2.eval(uvec)(0);

            //std::cout << i << "A :" << poly1 << std::endl;
            //std::cout << i << "B :" << poly2 << std::endl;
        }

        const Scalar discr  = discrepancy.eval(c1, c2);
        const Scalar discr0 = discr-discrRef0;
        const Scalar discr1 = discr-discrRef1;

//            std::cout << "###############################" << std::endl;
//            std::cout << "u         = " << u << std::endl;
//            std::cout << "C1        = " << c1.transpose() << std::endl;
//            std::cout << "C2        = " << c2.transpose() << std::endl;
//            std::cout << "discrRefA = " << discrRefA << std::endl;
//            std::cout << "discrRefB = " << discrRefB << std::endl;
//            std::cout << "discr     = " << discr << std::endl;
//            std::cout << "cost      = "
//                 << (Scalar(1)-u) * discr0*discr0 + u* discr1*discr1
//                 << std::endl;

        cost += (Scalar(1)-u) * discr0*discr0 + u* discr1*discr1;
    }

    fvec.setConstant(cost);
    fvec(0) = 0;

    //std::cout << cost << std::endl;

    // estimate a color using x as polynomial parameters
    //m.coeffs = x;
    //m.

    //fvec = poly.eval(x);
    // Implement y = 10*(x0+3)^2 + (x1-5)^2
    //fvec(0) = 10.0*pow(x(0)+3.0,2) ;
    //fvec(1) = x(0)*x(0);
    //fvec(2) = pow(x(1)-5.0,2);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Visitors

template<class _Color,
         typename _ComponentInterFun,
         typename _ConstComponentInterFun,
         int NbParametricSteps>
template <typename F>
void
Problem<_Color, _ComponentInterFun, _ConstComponentInterFun, NbParametricSteps>::
visitPolynomials(
        F &f,
        Eigen::Matrix<Scalar, Eigen::Dynamic, 1> &x){

    int paramOffset = 0; //! Offset to retrieve current functor parameter
    //! We build it incrementally to support non-uniform
    //! degrees between the different polynomials

    for(int c = 0; c != NbCol; ++c){
        for(int i = 0; i != ParametricSpaceDim; ++i){
            // we need to create this object here to avoid const lvalue
            // assignment errors.
            ComponentInterFun poly (
                        x.segment(paramOffset,
                                  ComponentInterFun::NbCoeff).data());
            f( poly );
            paramOffset += ComponentInterFun::NbCoeff;
        }
    }
}

template<class _Color,
         typename _ComponentInterFun,
         typename _ConstComponentInterFun,
         int NbParametricSteps>
template <typename F>
void
Problem<_Color, _ComponentInterFun, _ConstComponentInterFun, NbParametricSteps>::
visitPolynomials(
        const F &f,
        Eigen::Matrix<Scalar, Eigen::Dynamic, 1> &x){

    int paramOffset = 0; //! Offset to retrieve current functor parameter
    //! We build it incrementally to support non-uniform
    //! degrees between the different polynomials

    for(int c = 0; c != NbCol; ++c){ //could be done using 1 single loop
        for(int i = 0; i != ParametricSpaceDim; ++i){
            // we need to create this object here to avoid const lvalue
            // assignment errors.
            ComponentInterFun poly (
                        x.segment(paramOffset,
                                  ComponentInterFun::NbCoeff).data());
            f( poly );
            paramOffset += ComponentInterFun::NbCoeff;
        }
    }
}


} // namespace Test_Splines

#endif // PROBLEM_HPP
