#ifndef SOLVER_EXPLORATION_HPP
#define SOLVER_EXPLORATION_HPP

#include <pse/pse_solver.hpp>
#include <pse/pse_types.hpp>
#include <pse/pse_cps_types.hpp>

#include <map>
#include <set>

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
///
/// Strategy regarding variables lock:
///   This class locks the variables only when writting
///
////////////////////////////////////////////////////////////////////////////////
template<class ParametricPoint, class ParameterValidationHelper>
struct SolverExploration
  : public SolverFunctor<ParametricPoint, ParameterValidationHelper>
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  using Base = SolverFunctor<ParametricPoint, ParameterValidationHelper>;

  using InputType = typename Base::InputType;
  using ValueType = typename Base::ValueType;
  using JacobianType = typename Base::JacobianType;
  using QRSolver = typename Base::QRSolver;
  using Scalar = typename Base::Scalar;
  using CPS = typename Base::CPS;

public:
  inline SolverExploration();
  virtual ~SolverExploration() {}

  inline ERet
  lockParametricPoints
    (const std::vector<ParametricPointId>& ids);
  inline ERet
  unlockParametricPoints
    (const std::vector<ParametricPointId>& ids);
  inline size_t
  getLockedParametricPointsCount
    (const LayerId layer_id) const;
  inline bool
  isParametricPointLocked
    (const ParametricPointId id) const;

  //! Must be called before any computation and after any graph update
  //! This function doesn't need to be called iif the fonctors parameters
  //! have been modified. It should be called in any other case.
  inline ERet setup();
  inline ERet updateVariablesFromGraph(InputType& vars);
  inline ERet updateGraphFromVariables(const InputType& vars);

  inline void optimizeStartPalette(bool start)
  {
      Base::_initialized = false;
      _optimizedLayer = start ? 0 : 1;
  }

  inline bool isStartPaletteOptimized () const { return _optimizedLayer == 0; }

  inline void checkBlocksSizeConsistency(const ValueType& fvec) const {
      (void)fvec;//remove warning
      assert (    Base::nbVisionTypes()*_binaryOffset
               == fvec.rows()
                  - _unaryOffset
                  - _outOfGamutOffset
                  - _globalConstraintsOffset );
  }

  //! Must be called after initInternals()
  inline Eigen::VectorBlock<ValueType> binaryBlock(ValueType& fvec, int i = 0) const
  {
      checkBlocksSizeConsistency(fvec);
      return fvec.segment(i*_binaryOffset, _binaryOffset);
  }

  //! Must be called after initInternals()
  inline Eigen::VectorBlock<ValueType> unaryBlock(ValueType& fvec) const
  {
      checkBlocksSizeConsistency(fvec);
      return fvec.segment(Base::nbVisionTypes()*_binaryOffset, _unaryOffset);
  }

  //! Must be called after initInternals()
  inline Eigen::VectorBlock<ValueType> outOfGamutBlock(ValueType& fvec) const
  {
      checkBlocksSizeConsistency(fvec);
      return fvec.segment(Base::nbVisionTypes()*_binaryOffset + _unaryOffset, _outOfGamutOffset);
  }

  //! Must be called after initInternals()
  inline Eigen::VectorBlock<ValueType> globalBlock(ValueType& fvec) const
  {
      checkBlocksSizeConsistency(fvec);
      return fvec.tail(_globalConstraintsOffset);
  }

  inline int nbUnaryConstraints() const  { return _nbUnaryConstraints; }
  inline int nbBinaryConstraints() const { return _nbBinaryConstraints; }
  inline int nbGlobalConstraints() const { return _nbGlobalConstraints; }

  //! return the ids of the harmony and readability residuals
  inline void getHarmonyAndReadilityIdsInBlock
    (std::vector<int>& harmony,
     std::vector<int>& readability,
     std::vector<int>& readabilityCVD) const;

public:
  //! Variables used during the optimization, updated by the solver, and
  //! directly mapped to interFunArray
  //Eigen::Matrix<Scalar, Eigen::Dynamic, 1> variables;

private:
  // TODO: check these variables that are not all so generic. Can we now expect
  // to deduce some of them from the ConstrainedParameterSpace?

  std::vector<std::set<ParametricPointId>> _lockedPoints; //!< they will be not be optimized

  int _optSpaceDim;
  int _nbOptimizablePoints;
  int _nbUnaryConstraints, _nbBinaryConstraints, _nbGlobalConstraints;
  int _binaryOffset;
  int _unaryOffset;
  int _outOfGamutOffset;
  int _globalConstraintsOffset;
  std::map<ParametricPointId, int> _movableVertIndirect;
  std::map<ParametricPointId, ParametricPoint> _constrainedPoints;
  LayerId _optimizedLayer;

public:

  //! Operator called by the solver and computing the energy term in fvec
  inline
  int operator()(const InputType &x, ValueType &fvec, bool ignoreBinaryWeights = false) const;
};

template <class StreamT, class C, typename V>
inline
StreamT& operator<< (StreamT& stream, const SolverExploration<C, V> &p){
  using std::endl;

  stream << "Interpolating between : \n"
         << "\t" << p.h0.transpose() << "\t->\t" << p.h1.transpose() << "\n"
         << "\t" << p.f0.transpose() << "\t->\t" << p.f1.transpose() << endl;

  return stream;
}

#include "pse_solver_exploration.inl"

#endif // SOLVER_EXPLORATION_HPP
