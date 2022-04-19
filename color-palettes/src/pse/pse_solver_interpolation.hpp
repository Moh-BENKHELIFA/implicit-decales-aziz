#ifndef SOLVER_INTERPOLATION_HPP
#define SOLVER_INTERPOLATION_HPP

#include <pse/pse_solver.hpp>
#include <pse/pse_cps.hpp>

////////////////////////////////////////////////////////////////////////////////
/// Solver
///
/// Designed as a Levenberg Marquardt functor, with
///  -x:    parameters of the different polynoms used for interpolation, right
///         now we have 3rd order polygon per color component, so 4*3 scalar
///         values
///  -cost: distance functor, for now only one corresponding to the
///         l2Distance functor
///
/// \warning: Do not take into account the subspace optimization param yet
////////////////////////////////////////////////////////////////////////////////
template<
  class ParametricPoint_,
  typename ParameterValidationHelper,
  typename ComponentInterFunc_,
  typename ConstComponentInterFunc_,
  int NbParametricSteps_ = 10
>
struct SolverInterpolation
  : public SolverFunctor<ParametricPoint_, ParameterValidationHelper>
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  static constexpr int NbParametricSteps = NbParametricSteps_;

  using ParametricPoint = ParametricPoint_;
  using ComponentInterFunc = ComponentInterFunc_;
  using ConstComponentInterFunc = ConstComponentInterFunc_;
  using Base = SolverFunctor<ParametricPoint, ParameterValidationHelper>;
  using CPS = typename Base::CPS;
  using CPSInterpolator = ConstrainedParameterSpaceInterpolator<CPS, ComponentInterFunc, ConstComponentInterFunc>;

  using InputType = typename Base::InputType;
  using ValueType = typename Base::ValueType;
  using JacobianType = typename Base::JacobianType;
  using QRSolver = typename Base::QRSolver;
  using Scalar = typename Base::Scalar;

  using DiscrOutType = typename Discrepancy::BinaryDiscrepancyFunctor<ParametricPoint>::DiscrOutType;

public:
  /*!
   * \brief Solver
   * \param sharedVector Memory space aggregating all the variables and mapped
   *        to Polynomials
   */
  inline SolverInterpolation();
  virtual ~SolverInterpolation() {}

  //! Must be called before any computation and after any graph update
  //! \todo Add checkConsistency, that check at optimization time if the graph as been modified or not
  inline ERet initInternals();

  //! Must be called after initInternals()
  inline Eigen::VectorBlock<ValueType> discrepancyBlock(ValueType& fvec, int i = 0) const
  {
      checkBlocksSizeConsistency(fvec);
      return fvec.segment(i*_discrepancyOffset, _discrepancyOffset);
  }

  inline void setDiscrepancyValue(const DiscrOutType& value, ValueType& fvec, int index, int i = 0) const
  {
      discrepancyBlock(fvec, i)(index) = value;
  }

  //! Must be called after initInternals()
  inline Eigen::VectorBlock<ValueType> laplacianBlock(ValueType& fvec) const
  {
      checkBlocksSizeConsistency(fvec);
      return fvec.segment(Base::nbVisionTypes()*_discrepancyOffset, _laplacianOffset);
  }

  //! Must be called after initInternals()
  inline Eigen::VectorBlock<ValueType> unaryBlock(ValueType& fvec) const
  {
      checkBlocksSizeConsistency(fvec);
      return fvec.segment(Base::nbVisionTypes()*_discrepancyOffset + _laplacianOffset, _unaryOffset);
  }


  //! Must be called after initInternals()
  inline Eigen::VectorBlock<ValueType> outOfGamutBlock(ValueType& fvec) const
  {
      checkBlocksSizeConsistency(fvec);
      return fvec.segment(Base::nbVisionTypes()*_discrepancyOffset + _laplacianOffset + _unaryOffset, _outOfGamutOffset);
  }

  //! Must be called after initInternals()
  inline Eigen::VectorBlock<ValueType> globalBlock(ValueType& fvec) const
  {
      checkBlocksSizeConsistency(fvec);
      return fvec.tail(_globalConstraintsOffset);
  }

  inline int nbBinaryConstraints() const { return _nbBinaryConstraints; }
  inline int nbGlobalConstraints() const { return _nbGlobalConstraints; }

  inline void setStartLayerId(const LayerId id) { _startLayerId = id; }
  inline LayerId startLayerId() const { return _startLayerId; }

  inline void setEndLayerId(const LayerId id) { _endLayerId = id; }
  inline LayerId endLayerId() const { return _endLayerId; }

public:
  //! Variables used during the optimization, updated by the solver, and
  //! directly mapped to interFunArray
  //Eigen::Matrix<Scalar, Eigen::Dynamic, 1> variables;

private:
  int _nbBinaryConstraints;
  int _nbGlobalConstraints;
  int _laplacianOffset;
  int _discrepancyOffset;
  int _unaryOffset;
  int _outOfGamutOffset;
  int _globalConstraintsOffset;

  LayerId _startLayerId, _endLayerId;
  std::vector<ParametricPoint> _startPalette, _endPalette;
  ConstraintIdList _startConstraints, _endConstraints;

  CPSInterpolator _interp;

  inline void checkBlocksSizeConsistency(const ValueType& fvec) const {
      (void)fvec;//remove warning
      assert ((  Base::nbVisionTypes()*_discrepancyOffset +
                 _laplacianOffset +
                 _unaryOffset +
                 _outOfGamutOffset +
                 _globalConstraintsOffset ) == fvec.rows() );
  }

public:
  /*!
   * \brief Apply a non-const functor to the solver polynomials
   * \param f a functor class implementing: operator( ComponentInterFun& p)
   *
   * Can be used either with Functor class and lambda functions:
   * \code
   * int counter = 0;
   * auto f = [&counter] ( typename SolverType::ComponentInterFun& poly)
   *   {
   *     std::cout << poly << std::endl;
   *     ++counter;
   *   };
   * solver.visitPolynomials( f );
   * \endcode
   */
  template <typename F>
  inline
  void visitPolynomials
    (F &f,
     Eigen::Matrix<Scalar, Eigen::Dynamic, 1> &x);

  /*!
   * \brief Apply a const functor to the solver polynomials
   * \param f a functor class implementing: operator( ComponentInterFun& p)
   *
   * Can be used either with Functor class and lambda functions:
   * \code
   * solver.visitPolynomials(
   *   [] ( typename SolverType::ComponentInterFun& poly)
   *     {
   *       std::cout << poly << std::endl;
   *     } );
   * \endcode
   */
  template <typename F>
  inline
  void visitPolynomials
    (const F &f,
     Eigen::Matrix<Scalar, Eigen::Dynamic, 1> &x);
  //! \brief Apply a non-const functor to polynomials binded to x
  template <typename F>
  inline
  void visitConstPolynomials(F &f,
                        const Eigen::Matrix<Scalar, Eigen::Dynamic, 1> &x) const;

  //! \brief Apply a const functor to polynomials binded to x
  template <typename F>
  inline
  void visitConstPolynomials
    (const F &f,
     const Eigen::Matrix<Scalar, Eigen::Dynamic, 1> &x) const;


public:

  //! Operator called by the solver and computing the energy term in fvec
  inline
  int operator()(const InputType &x, ValueType &fvec) const;
};

template <class StreamT, class C, typename V, typename T, typename TC, int S>
inline
StreamT& operator<< (StreamT& stream, const SolverInterpolation<C, V, T, TC, S> &p){
  using std::endl;

  stream << "Interpolating between : \n"
         << "\t" << p.h0.transpose() << "\t->\t" << p.h1.transpose() << "\n"
         << "\t" << p.f0.transpose() << "\t->\t" << p.f1.transpose() << endl;

  return stream;
}

#include <pse/pse_solver_interpolation.inl>

#endif // SOLVER_INTERPOLATION_HPP
