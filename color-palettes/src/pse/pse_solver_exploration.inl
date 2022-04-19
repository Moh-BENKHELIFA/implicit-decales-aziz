#ifndef SOLVER_EXPLORATION_INL
#define SOLVER_EXPLORATION_INL

#include <pse/pse_solver_exploration.hpp>

// FIXME: really bad dependency! We want to be agnostic of the type of the
// parametric point!!!!
#include <pse/color/pse_cost_color.hpp>

#include <pse/pse_types.hpp>

template<class ParametricPoint, class ParameterValidationHelper>
SolverExploration<ParametricPoint, ParameterValidationHelper>::SolverExploration()
  : Base
      (Eigen::Dynamic, // nb polynomial coeffs
       Eigen::Dynamic) // nb constraints
  , _optSpaceDim(0)
  , _nbOptimizablePoints(0)
  , _nbUnaryConstraints(0)
  , _nbBinaryConstraints(0)
  , _nbGlobalConstraints(0)
  , _binaryOffset(0)
  , _unaryOffset(0)
  , _outOfGamutOffset(0)
  , _globalConstraintsOffset(0)
  , _optimizedLayer(LayerId_INVALID)
{ }

template<class ParametricPoint, class ParameterValidationHelper>
ERet
SolverExploration<ParametricPoint, ParameterValidationHelper>::lockParametricPoints
  (const std::vector<ParametricPointId>& ids)
{
  // TODO: do we want to raise error in case the points doesn't exists or the
  // points are already locked?
  for(ParametricPointId id: ids) {
    const LayerId lid = Base::space().getParametricPointLayerId(id);
    _lockedPoints.resize(std::max(_lockedPoints.size(), lid+1));
    std::set<ParametricPointId>& locked_ppoints = _lockedPoints[lid];
    (void)locked_ppoints.insert(id);
  }
  return ERet_OK;
}

template<class ParametricPoint, class ParameterValidationHelper>
ERet
SolverExploration<ParametricPoint, ParameterValidationHelper>::unlockParametricPoints
  (const std::vector<ParametricPointId>& ids)
{
  // TODO: do we want to raise error in case the points doesn't exists or the
  // points are already unlocked?
  for(ParametricPointId id: ids) {
    const LayerId lid = Base::space().getParametricPointLayerId(id);
    _lockedPoints.resize(std::max(_lockedPoints.size(), lid));
    std::set<ParametricPointId>& locked_ppoints = _lockedPoints[lid];
    (void)locked_ppoints.erase(id);
  }
  return ERet_OK;
}

template<class ParametricPoint, class ParameterValidationHelper>
size_t
SolverExploration<ParametricPoint, ParameterValidationHelper>::getLockedParametricPointsCount
  (const LayerId layer_id) const
{
  return _lockedPoints.size() > layer_id
    ? _lockedPoints[layer_id].size()
    : 0;
}

template<class ParametricPoint, class ParameterValidationHelper>
FORCE_INLINE bool
SolverExploration<ParametricPoint, ParameterValidationHelper>::isParametricPointLocked
  (const ParametricPointId id) const
{
  const LayerId lid = Base::space().getParametricPointLayerId(id);
  return _lockedPoints.size() > lid
    ? _lockedPoints[lid].count(id) > 0
    : false;
}

template<class ParametricPoint, class ParameterValidationHelper>
ERet
SolverExploration<ParametricPoint, ParameterValidationHelper>::setup()
{
  ERet ret = ERet_OK;

  Base::lockValues();
  Base::_initialized = false;

  const size_t locked_ppoints = getLockedParametricPointsCount(_optimizedLayer);

  // count the number of optimizable points, i.e. the number of parametric
  // points in the optimized layers, minus the locked ones.
  _nbOptimizablePoints = std::max(0,
      (int)Base::space().getLayerParametricPointsCount(_optimizedLayer)
    - (int)locked_ppoints);
  assert(_nbOptimizablePoints >= 0
      && size_t(_nbOptimizablePoints) <= Base::space().getAllParametricPointsCount());

  // count the number of constraints of src/dst parametric points, removing the
  // locked ones.
  _nbUnaryConstraints = std::max(0,
      (int)Base::space().getLayerParametricPointsFunctorsCount(_optimizedLayer)
    - (int)locked_ppoints);
  assert(_nbUnaryConstraints >= 0
      && size_t(_nbUnaryConstraints) <= Base::space().getAllParametricPointsFunctorsCount());

  _optSpaceDim = Base::subspaceEnabledCount();

  Base::_inputs_count = _nbOptimizablePoints * _optSpaceDim;

  using DiscrType = typename Base::CPS::BinaryConstraintFunctor;

  _nbBinaryConstraints = Base::space().getLayerConstraintsFunctorsCount(_optimizedLayer);
  _nbGlobalConstraints = Base::space().getLayerGlobalConstraintsCount(_optimizedLayer);
  _binaryOffset        = int(_nbBinaryConstraints * DiscrType::DiscrOutSize);
  _unaryOffset         = int(_nbUnaryConstraints  * DiscrType::DiscrOutSize);
  _outOfGamutOffset    = _nbOptimizablePoints;
  _globalConstraintsOffset = int(_nbGlobalConstraints * DiscrType::DiscrOutSize);

  // costs component are set following the same order as below
  Base::_values_count =
          Base::nbVisionTypes() * _binaryOffset
        + _unaryOffset
        + _outOfGamutOffset
        + _globalConstraintsOffset;

//#ifdef __DEBUG__
  std::cout << "Nb Movables:     " << _nbOptimizablePoints << std::endl;
  std::cout << "Nb Unary:        " << _nbUnaryConstraints << std::endl;
  std::cout << "Nb Binary:       " << _nbBinaryConstraints << std::endl;
  std::cout << "Nb Global:       " << _nbGlobalConstraints << std::endl;
  std::cout << "Nb Vision types: " << Base::nbVisionTypes() << std::endl;
  std::cout << "Nb Constraints:  " << Base::_values_count << std::endl;
  std::cout << "Nb Variables:    " << Base::_inputs_count << std::endl;
//#endif

  CHECK_OR_DO(Base::_values_count >= Base::_inputs_count,
    ret = ERet_Invalid; goto exit);

  Base::_initialized = true;
exit:
  Base::unlockValues();
  return ret;
}

template<class ParametricPoint, class ParameterValidationHelper>
ERet
SolverExploration<ParametricPoint, ParameterValidationHelper>::updateVariablesFromGraph
  (InputType& vars)
{
  // TODO: some computation here are not related to variables and should be done
  // elsewhere. In fact, some members must be attached to vars instead of the
  // solver: _movableVertIndirect, _constrainedPoints and lock status.

  ERet ret = ERet_OK;
  Base::lockValues();

  vars.resize(Base::_inputs_count);
  _movableVertIndirect.clear();
  _constrainedPoints.clear();

  // init variables using color points
  const ParametricPointIdList& ppoints =
    Base::space().getLayerParametricPointIdList(_optimizedLayer);
  int idx = 0;
  for(ParametricPointId id: ppoints) {
    ParametricPoint value;
    CALL_OR_GOTO(ret, exit, Base::space().getParametricPointValue(id, value));
    if( isParametricPointLocked(id) ) {
      _constrainedPoints[id] = value;
    } else {
      assert(idx < _nbOptimizablePoints);
      vars.segment(idx*_optSpaceDim, _optSpaceDim) =
        ParametricPoint(ParameterValidationHelper::process(value)).getNative();
      _movableVertIndirect[id] = idx;
      ++idx;
    }
  }
exit:
    Base::unlockValues();
    return ret;
}


template<class ParametricPoint, class ParameterValidationHelper>
ERet
SolverExploration<ParametricPoint, ParameterValidationHelper>::updateGraphFromVariables
  (const InputType& vars)
{
  ERet ret = ERet_OK;
  Base::lockValues();

  // update color points using variables
  const ParametricPointIdList& ppoints =
    Base::space().getLayerParametricPointIdList(_optimizedLayer);
  for(ParametricPointId id: ppoints) {
    if( isParametricPointLocked(id) )
      continue;
    const int idx = _movableVertIndirect[id];
    assert(idx < _nbOptimizablePoints);
    const ParametricPoint value = ParameterValidationHelper::process
      (ParametricPoint(vars.segment(idx*_optSpaceDim, _optSpaceDim)));
    CALL_OR_GOTO(ret, exit, Base::space().setParametricPointValue(id, value));
  }
exit:
  Base::unlockValues();
  return ret;
}


template<class ParametricPoint, class ParameterValidationHelper>
inline int
SolverExploration<ParametricPoint, ParameterValidationHelper>::operator()
  (const InputType& x,
   ValueType &fvec,
   bool ignoreBinaryWeights) const
{
  // The caller must ensure we are ready before calling this function!
  assert(Base::isReady());
  if (! Base::isReady()){
      fvec.setConstant(0);
      return 0;
  }
  Base::lockValues();

  // cache a palette from the nodes, to compute unary and global constraints
  // TODO: we MUST avoid that step! Quite sure we can by doing a good separation
  std::vector<ParametricPoint> palette;
  palette.reserve(_nbOptimizablePoints);
  for(int i = 0; i < _nbOptimizablePoints; ++i){
    palette.push_back(ParametricPoint(x.segment(i*_optSpaceDim, _optSpaceDim)));
  }

  if (_nbUnaryConstraints != 0){
    Eigen::VectorBlock<ValueType> unaryCosts = unaryBlock(fvec);
    assert(unaryCosts.rows() == _nbUnaryConstraints);

    int unaryId = 0;
    for(std::pair<ParametricPointId, int> mv: _movableVertIndirect) {
      const ParametricPointId id = mv.first;
      if( Base::space().getParametricPointFunctorsCount(id) <= 0 )
        continue;

      // TODO: If we could get all the functors ordered by parametric point id,
      // knowing the number of functors per parametric point, we could do only
      // one loop and even better, probably save some function calls, allowing
      // in addition, a better vectorization in each remaining call. On most
      // devices, a function call could be very expensive (e.g. on GPU, we
      // prefer to prepare buffers with all the data and then, do only one
      // function call that will do the stuff at once).

      // TODO: we do it dumbly, but we could avoid to do that each time
      using ParametricPointFunctor = typename Base::CPS::UnaryConstraintFunctor;
      const std::vector<ParametricPointFunctor*>& functors =
        Base::space().getParametricPointFunctors(id);

      const int idx = mv.second;
      assert(unaryId < _nbUnaryConstraints);
      for(ParametricPointFunctor* f: functors) {
        unaryCosts(unaryId) = f->getW() * f->eval({palette[idx]}, 0);
        ++unaryId;
      }
    }
  }

  // FIXME: as we will transform vision types in different color space, we
  // should rework everything taking into account many spaces in wich we want
  // to optimize, with a reference color space (here, seen as the normal
  // vision).

  ////////////////////////////////////////////////////////////////////////////
  /// When considering multiple vision type, the normal vision is always
  /// considered as reference
  if( _nbBinaryConstraints != 0 ) {
    const ConstraintIdList& constraints =
      Base::space().getLayerConstraintIdList(_optimizedLayer);

    int nbVisionTypes = Base::nbVisionTypes();
    for(int visId = 0; visId < nbVisionTypes; ++visId){
      Eigen::VectorBlock<ValueType> binaryCosts = binaryBlock(fvec, visId);
      assert(binaryCosts.rows() == _nbBinaryConstraints);

      Color::CVD::VISION_TYPE vtype = Base::getVisionType(visId);

      // compute binary costs
      int binaryId = 0;
      for(ConstraintId id: constraints) {
        assert(binaryId < _nbBinaryConstraints);

        const ParametricPointIdPair& cppoints =
          Base::space().getConstraintParametricPoints(id);
        assert
          (  Base::space().getParametricPointLayerId(cppoints.first)
          == Base::space().getParametricPointLayerId(cppoints.second));

        using ConstraintFunctor = typename Base::CPS::BinaryConstraintFunctor;
        const std::vector<ConstraintFunctor*>& functors =
          Base::space().getConstraintFunctors(id);

        ParametricPoint cppv0, cppv1;

        auto it = _movableVertIndirect.find(cppoints.first);
        if( it == _movableVertIndirect.end() ) {
          // this guy is not movable, use vertex color directly
          cppv0 = _constrainedPoints.at(cppoints.first);
        } else {
          cppv0 = palette[it->second];
        }

        it = _movableVertIndirect.find(cppoints.second);
        if( it == _movableVertIndirect.end() ) {
          // this guy is not movable, use vertex color directly
          cppv1 = _constrainedPoints.at(cppoints.second);
        } else {
          cppv1 = palette[it->second];
        }

        // TODO: this is related to colors and should be moved elsewhere if we
        // want to manage more than colors
        cppv0 = Color::CVD::Rasche2005::process(vtype, cppv0);
        cppv1 = Color::CVD::Rasche2005::process(vtype, cppv1);

        // TODO: WTF???! Seems to be an hack function... should not be done
        // like this!
        using DistFunctor = ColorDiscrepancy::DistanceDiscrepancyFunctor<ParametricPoint>;

        for(ConstraintFunctor* f: functors){
          if(  (vtype == Color::CVD::VISION_NORMAL)
            || Utils::isOfType<DistFunctor>(f) ) {
            Scalar refDiscr = f->getRefValue();
            Scalar discr = f->eval({cppv0, cppv1}, Scalar(0));
            binaryCosts(binaryId) =
                (ignoreBinaryWeights ? Scalar(1.) : f->getW())
              * (discr - refDiscr);
          } else {
            binaryCosts(binaryId) = 0;
          }
          ++binaryId;
        }
      }
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  ///
  /// Unary constraints
  ///
  ///
  // TODO: how to do that the generic way? This is a kind of parametric point
  // constraint. But is it exactly the case ?
  if( _outOfGamutOffset != 0 ) {
    Eigen::VectorBlock<ValueType> outOfGamutCosts = outOfGamutBlock(fvec);
    assert(outOfGamutCosts.rows() == _outOfGamutOffset);

    int vertId = 0;
    for(int idx = 0; idx < _nbOptimizablePoints; ++idx) {
      assert(vertId < _outOfGamutOffset);

      const ParametricPoint& tmp = palette[idx];
      if( ParameterValidationHelper::isValid(tmp) ) {
        outOfGamutCosts(vertId) = Scalar(0);
      } else {
        const ParametricPoint diff =
          tmp - ParameterValidationHelper::process(tmp);
        outOfGamutCosts(vertId) =
            Base::outOfGamutExplorationFactor
          * diff.getNative().squaredNorm();
      }
      ++vertId;
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  ///
  /// Global constraints
  ///
  ///
  if( _globalConstraintsOffset != 0 ) {
    Eigen::VectorBlock<ValueType>  globalCosts = globalBlock(fvec);
    assert(globalCosts.rows() == _globalConstraintsOffset);

    const GlobalConstraintIdList& gconstraints =
      Base::space().getLayerGlobalConstraintIdList(_optimizedLayer);

    int globId = 0;
    for(GlobalConstraintId id: gconstraints) {
      using GlobalConstraintFunctor = typename Base::CPS::NaryConstraintFunctor;
      GlobalConstraintFunctor* f =
        Base::space().getGlobalConstraintFunctors(id);

      globalCosts(globId) =
          f->getW() * f->eval(palette.cbegin(), palette.cend(), 0);
      ++globId;
    }
  }

  Base::unlockValues();
  return 0;
}

// TODO: this is very color specific! We should not have such function here or
// we will have to specialize the solver for colors only.
template<class ParametricPoint, class ParameterValidationHelper>
void
SolverExploration<ParametricPoint, ParameterValidationHelper>::getHarmonyAndReadilityIdsInBlock
  (std::vector<int>& harmony,
   std::vector<int>& readability,
   std::vector<int>& readabilityCVD) const
{
  harmony.clear();
  readability.clear();
  readabilityCVD.clear();

  if( _nbBinaryConstraints != 0 ) {
    const int nbVisionTypes = Base::nbVisionTypes();
    for( int visId = 0; visId < nbVisionTypes; ++visId ) {
      Color::CVD::VISION_TYPE vtype = Base::getVisionType(visId);

      // compute binary costs
      int binaryId = 0;
      for(ConstraintId id: Base::space().getLayerConstraintIdList(_optimizedLayer)) {
        assert (binaryId < _nbBinaryConstraints);

        const ParametricPointIdPair& cppoints =
          Base::space().getConstraintParametricPoints(id);
        (void)cppoints;
        assert
          (  Base::space().getParametricPointLayerId(cppoints.first) == _optimizedLayer
          && Base::space().getParametricPointLayerId(cppoints.second) == _optimizedLayer);

        using DistFunctor        = ColorDiscrepancy::DistanceDiscrepancyFunctor<ParametricPoint>;
        using HueFunctor         = ColorDiscrepancy::HSV_HueDiscrepancyFunctor<ParametricPoint>;
        using SaturationFunctor  = ColorDiscrepancy::HSV_SaturationDiscrepancyFunctor<ParametricPoint>;
        using ValueFunctor       = ColorDiscrepancy::HSV_ValueDiscrepancyFunctor<ParametricPoint>;
        using UHueFunctor        = ColorDiscrepancy::HSV_HueUDiscrepancyFunctor<ParametricPoint>;
        using USaturationFunctor = ColorDiscrepancy::HSV_SaturationUDiscrepancyFunctor<ParametricPoint>;
        using UValueFunctor      = ColorDiscrepancy::HSV_ValueUDiscrepancyFunctor<ParametricPoint>;

        using ConstraintFunctor = typename Base::CPS::BinaryConstraintFunctor;
        const std::vector<ConstraintFunctor*>& functors =
          Base::space().getConstraintFunctors(id);

        for(ConstraintFunctor* f: functors) {
          if( vtype == Color::CVD::VISION_NORMAL ) {
            if( Utils::isOfType<DistFunctor>(f) ) {
              readability.push_back(binaryId);
            } else if( Utils::isOfType<HueFunctor>(f)
                    || Utils::isOfType<SaturationFunctor>(f)
                    || Utils::isOfType<ValueFunctor>(f)
                    || Utils::isOfType<UHueFunctor>(f)
                    || Utils::isOfType<USaturationFunctor>(f)
                    || Utils::isOfType<UValueFunctor>(f)) {
              harmony.push_back(binaryId);
            }
          } else {
            if( Utils::isOfType<DistFunctor>(f) ) {
              readabilityCVD.push_back(binaryId);
            }
          }
          ++binaryId;
        }
      }
    }
  }
}

#endif // SOLVER_EXPLORATION_INL
