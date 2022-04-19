#ifndef SOLVER_INTERPOLATION_INL
#define SOLVER_INTERPOLATION_INL

#include <pse/pse_solver_interpolation.hpp>

#include <unordered_set>

template<class ParametricPoint,
         typename ParameterValidationHelper,
         typename ComponentInterFunc,
         typename ConstComponentInterFunc,
         int NbParametricSteps>
SolverInterpolation<ParametricPoint, ParameterValidationHelper, ComponentInterFunc, ConstComponentInterFunc, NbParametricSteps>::
SolverInterpolation()
  : Base
      (Eigen::Dynamic, // nb polynomial coeffs
       Eigen::Dynamic) // nb constraints
  , _nbBinaryConstraints(0)
  , _nbGlobalConstraints(0)
  , _laplacianOffset(0)
  , _discrepancyOffset(0)
  , _unaryOffset(0)
  , _outOfGamutOffset(0)
  , _globalConstraintsOffset(0)
  , _startLayerId(LayerId_INVALID)
  , _endLayerId(LayerId_INVALID)
{ }

template<class ParametricPoint,
         typename ParameterValidationHelper,
         typename ComponentInterFunc,
         typename ConstComponentInterFunc,
         int NbParametricSteps>
ERet
SolverInterpolation<ParametricPoint, ParameterValidationHelper, ComponentInterFunc, ConstComponentInterFunc, NbParametricSteps>::
initInternals()
{
  ERet ret = ERet_OK;
  if(  _startLayerId == LayerId_INVALID
    || _endLayerId == LayerId_INVALID
    || _startLayerId == _endLayerId)
    return ERet_BadArg;

  Base::lockValues();
  Base::_initialized = false;

  const PairingIdList& pairings =
    Base::space().getLayerPairingIdList(_startLayerId);
  ParametricPoint pp;

  _startPalette.clear();
  _endPalette.clear();

  // We have to go through all constraints, even those who are not paired. Here
  // we take all the start constraints and we keep only the constraints of the
  // end layer that are not paired. The paired ones will be managed as buddies
  // of the start layer constraints.
  _startConstraints = Base::space().getLayerConstraintIdList(_startLayerId);
  _endConstraints.clear();
  for(ConstraintId id: Base::space().getLayerConstraintIdList(_endLayerId)) {
    if( Base::space().getConstraintBuddyId(id) != ConstraintId_INVALID )
      continue;
    _endConstraints.push_back(id);
  }

  _interp.setSpace(Base::space());

  Base::_inputs_count = pairings.size() * ComponentInterFunc::NbCoeff;

  using DiscrType = Discrepancy::BinaryDiscrepancyFunctor<ParametricPoint>;

  _nbBinaryConstraints = _startConstraints.size() + _endConstraints.size();
  _nbGlobalConstraints = Base::space().getLayerGlobalConstraintsFunctorsCount(_startLayerId);
  _discrepancyOffset = _nbBinaryConstraints * DiscrType::DiscrOutSize * NbParametricSteps;
  _laplacianOffset = ParametricPoint::SpaceDim * NbParametricSteps * pairings.size();
  _unaryOffset = DiscrType::DiscrOutSize * NbParametricSteps * pairings.size();
  _outOfGamutOffset = NbParametricSteps * pairings.size();
  _globalConstraintsOffset = DiscrType::DiscrOutSize * NbParametricSteps * _nbGlobalConstraints;

  // costs component are set following the same order as below
  Base::_values_count =
      Base::nbVisionTypes() * _discrepancyOffset
    + _laplacianOffset
    + _unaryOffset
    + _outOfGamutOffset
    + _globalConstraintsOffset;

  CHECK_OR_DO(Base::_values_count >= Base::_inputs_count,
    ret = ERet_Invalid; goto error);

  _startPalette.reserve(pairings.size());
  _endPalette.reserve(pairings.size());

  for(PairingId id: pairings) {
    const ParametricPointIdPair p =
      Base::space().getPairingParametricPointIds(id);
      assert(Base::space().getParametricPointLayerId(p.first) == _startLayerId);
      assert(Base::space().getParametricPointLayerId(p.second) == _endLayerId);

      CALL_OR_GOTO(ret,error, Base::space().getParametricPointValue
        (p.first, pp));
      _startPalette.push_back(pp);
      CALL_OR_GOTO(ret,error, Base::space().getParametricPointValue
        (p.second, pp));
      _endPalette.push_back(pp);
  }

  Base::_initialized = true;
exit:
  Base::unlockValues();
  return ret;
error:
  goto exit;
}

namespace internal {
/*!
 * Compute the cost of one constraint edge
 * The algorithm is the following:
 * Input: ConstraintId 'id'
 *  - poly1 = 1st interpolation function (Pairing) leaving 'id'
 *  - poly2 = 2d  interpolation function (Pairing) leaving 'id'
 *  - compute cost between poly1 and poly2
 */
template <typename CPS,
          typename CPSInterpolator,
          typename ConstComponentInterFunc,
          typename ParameterValidationHelper,
          int NbParametricSteps>
struct ConstraintCostHelper
{
  using ParametricPoint = typename CPS::ParametricPoint;
  using Scalar = typename ParametricPoint::Scalar;
  using FVectorType = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;
  using UType = typename ConstComponentInterFunc::EmbeddedVectorType;
  using ConstraintFunctor = typename CPS::BinaryConstraintFunctor;

  static constexpr typename ParametricPoint::Space space = ParametricPoint::space;
  static constexpr typename ConstComponentInterFunc::Scalar lambda()
    { return 1e-5; }

  const FVectorType& _x;
  const CPS& _space;
  const CPSInterpolator& _interp;
  const ConstraintIdList& _start_constraints;
  const ConstraintIdList& _end_constraints;

  using DiscrType = Discrepancy::BinaryDiscrepancyFunctor<ParametricPoint>;
  using DiscrVect = typename DiscrType::DiscrOutType;

  explicit inline
  ConstraintCostHelper
    (const FVectorType& xref,
     const CPS& space,
     const CPSInterpolator& interp,
     const ConstraintIdList& start_constraints,
     const ConstraintIdList& end_constraints)
    : _x(xref)
    , _space(space)
    , _interp(interp)
    , _start_constraints(start_constraints)
    , _end_constraints(end_constraints)
  {}

  // TODO: the inner loop on NbParametricSteps could evaluate the poly1|2
  // only once for all functors. Most of the time we have only one functor,
  // but in other cases that could save lots of time! And this can be done
  // for all cases, with or without buddy.

  inline ERet
  treatPairedConstraints
    (Eigen::Ref<FVectorType> fvec,
     Color::CVD::VISION_TYPE vtype,
     const ConstraintId id,
     const ConstraintId bid,
     const bool decr_weight,
     int& nbf)
  {
    const PairingIdList& cp = _space.getConstraintInvolvedPairingIdList(id);

    // it's ok to be here, it just means that one of the vertex is not
    // connected by a pairing edge.
    // TODO: what is the meaning of having only one vertex connected??? Are we
    // sure we do not want to do anything in this case?
    if( cp.size() <= 1 )
      return ERet_OK;

    ConstComponentInterFunc poly1 = _interp.getConstInterpolationFunctor(cp[0], _x);
    ConstComponentInterFunc poly2 = _interp.getConstInterpolationFunctor(cp[1], _x);

    // We have a buddy constraint, we will have to evaluate both
    const std::vector<ConstraintFunctor*>& functors =
      _space.getConstraintFunctors(id);
    const std::vector<ConstraintFunctor*>& bfunctors =
      _space.getConstraintFunctors(bid);

    // TODO: if we want to mange the other cases, we have to implement a kind
    // of manager that explicitly says what to do, and do it for us so we
    // don't have to care about that here.
    assert(functors.size() == bfunctors.size());

    const size_t functors_count = functors.size();
    for(size_t idx = 0; idx < functors_count; ++idx, ++nbf) {
      const ConstraintFunctor* f = functors[idx];
      const ConstraintFunctor* bf = bfunctors[idx];

      // TODO: Why not a &&??? I'm quite sure it's valid if only one of them has
      // a nul weight
      if( f->getW() == Scalar(0) || bf->getW() == Scalar(0) )
        continue;

      DiscrVect refDiscrFunctor = f->getRefValue();
      DiscrVect refDiscrBuddy   = bf->getRefValue();

      const int currentFuncOffset =
        nbf * DiscrType::DiscrOutSize * NbParametricSteps;

      // Estimate cost for different parametric step, and for each
      // component we loop over the parametric space first so we can
      // estimate directly the colors and thus the associated cost
      ParametricPoint p1, p2;
      for( int s = 0; s != NbParametricSteps; ++s) {
        UType u;
        Scalar upos = Scalar(s+1) / Scalar(NbParametricSteps+2);
        u << upos;

        p1 = poly1.eval(u);
        p2 = poly2.eval(u);
        p1 = Color::CVD::Rasche2005::process(vtype, p1);
        p2 = Color::CVD::Rasche2005::process(vtype, p2);

        DiscrVect discrFunctor = f->eval({p1,p2},upos);
        DiscrVect discrBuddy   = bf->eval({p1,p2},upos);

        const Scalar w = (decr_weight ? (Scalar(1)-upos) : upos);
        // Cannot use this as it will introduce an error:
        //  const Scalar one_minus_w = (decr_weight ? upos : (Scalar(1)-upos));
        const Scalar one_minus_w = Scalar(1) - w;
        const int distIndex = s * DiscrType::DiscrOutSize + currentFuncOffset;

        // we do NOT want to compute the abs value here, because we want to
        // know the derivatives oriented in space.
        fvec(distIndex) =
            (w           * f->getW()  * (discrFunctor - refDiscrFunctor))
          + (one_minus_w * bf->getW() * (discrBuddy   - refDiscrBuddy));
      }
    }
    return ERet_OK;
  }

  inline ERet
  treatIsolatedConstraint
    (Eigen::Ref<FVectorType> fvec,
     Color::CVD::VISION_TYPE vtype,
     const ConstraintId id,
     const bool decr_weight,
     int& nbf)
  {
    const PairingIdList& cp = _space.getConstraintInvolvedPairingIdList(id);

    // it's ok to be here, it just means that one of the vertex is not
    // connected by a pairing edge.
    // TODO: what is the meaning of having only one vertex connected??? Are we
    // sure we do not want to do anything in this case?
    if( cp.size() <= 1 )
      return ERet_OK;

    ConstComponentInterFunc poly1 = _interp.getConstInterpolationFunctor(cp[0], _x);
    ConstComponentInterFunc poly2 = _interp.getConstInterpolationFunctor(cp[1], _x);

    const std::vector<ConstraintFunctor*>& functors =
      _space.getConstraintFunctors(id);

    const size_t functors_count = functors.size();
    for(size_t idx = 0; idx < functors_count; ++idx, ++nbf) {
      const ConstraintFunctor* f = functors[idx];
      if( f->getW() == Scalar(0) )
        continue;

      DiscrVect refDiscrFunctor = f->getRefValue();

      const int currentFuncOffset =
        nbf * DiscrType::DiscrOutSize * NbParametricSteps;

      // Estimate cost for different parametric step, and for each
      // component we loop over the parametric space first so we can
      // estimate directly the colors and thus the associated cost
      ParametricPoint p1, p2;
      for( int s = 0; s != NbParametricSteps; ++s) {
        UType u;
        Scalar upos = Scalar(s+1) / Scalar(NbParametricSteps+2);
        u << upos;

        p1 = poly1.eval(u);
        p2 = poly2.eval(u);
        p1 = Color::CVD::Rasche2005::process(vtype, p1);
        p2 = Color::CVD::Rasche2005::process(vtype, p2);

        DiscrVect discrFunctor = f->eval({p1,p2},upos);

        const Scalar w = (decr_weight ? (Scalar(1)-upos) : upos);
        const int distIndex = s * DiscrType::DiscrOutSize + currentFuncOffset;

        // we do NOT want to compute the abs value here, because we want to
        // know the derivatives oriented in space.
        fvec(distIndex) = w * f->getW() * (discrFunctor - refDiscrFunctor);
      }
    }
    return ERet_OK;
  }

  inline ERet
  operator()
    (Eigen::Ref<FVectorType> fvec,
     Color::CVD::VISION_TYPE vtype)
  {
    // TODO: manage return code
    int nbProcessedFunctors = 0;

    // Is the cost weight increasing (from 0 to 1) or decreasing during
    // the interpolation along the curves parametrizations
    bool decreasingW = true;

    //#pragma omp parallel
    for(ConstraintId id: _start_constraints) {
      // TODO: is there a way to avoid such 'if' and let another mechanism
      // decide what to do and let us simplify things here?
      const ConstraintId bid = _space.getConstraintBuddyId(id);
      if( bid != ConstraintId_INVALID ) {
        CALL(treatPairedConstraints
          (fvec, vtype, id, bid, decreasingW, nbProcessedFunctors));
      } else {
        // Only the current constraint, no buddy to evaluated
        CALL(treatIsolatedConstraint
          (fvec, vtype, id, decreasingW, nbProcessedFunctors));
      }
    }

    decreasingW = false;
    //#pragma omp parallel
    for(ConstraintId id: _end_constraints) {
      assert(_space.getConstraintBuddyId(id) == ConstraintId_INVALID);
      CALL(treatIsolatedConstraint
        (fvec, vtype, id, decreasingW, nbProcessedFunctors));
    }
    return ERet_OK;
  }
};

} // namespace internal



template<class ParametricPoint,
         typename ParameterValidationHelper,
         typename ComponentInterFunc,
         typename ConstComponentInterFunc,
         int NbParametricSteps>
int
SolverInterpolation<ParametricPoint, ParameterValidationHelper, ComponentInterFunc, ConstComponentInterFunc, NbParametricSteps>::
operator()(const InputType &x, ValueType &fvec) const
{
    // allow to skip computation for constraints with w=0
    fvec.setConstant(Scalar(0));

    if( !Base::isReady() )
      return 0;

    using HelperType = internal::ConstraintCostHelper<
      CPS,
      CPSInterpolator,
      ConstComponentInterFunc,
      ParameterValidationHelper,
      NbParametricSteps
    >;
    using UType = typename ConstComponentInterFunc::EmbeddedVectorType;
    constexpr size_t dsize =
      Discrepancy::BinaryDiscrepancyFunctor<ParametricPoint>::DiscrOutSize;

    int nbVisionTypes = Base::nbVisionTypes();

    ////////////////////////////////////////////////////////////////////////////
    ///
    /// Compute Binary constraints
    ///
    /// When considering multiple vision type, the normal vision is always
    /// considered as reference
    HelperType helper(x, Base::space(), _interp, _startConstraints, _endConstraints);
    for(int visId = 0; visId < nbVisionTypes; ++visId){
      Eigen::VectorBlock<ValueType> discrCosts = discrepancyBlock(fvec, visId);
      const Color::CVD::VISION_TYPE vtype = Base::getVisionType(visId);
      CALL(helper(discrCosts, vtype));
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Compute laplacian, unary costs and gamut
    int idPairing = 0;

    Eigen::VectorBlock<ValueType> unaryCosts = unaryBlock(fvec);
    Eigen::VectorBlock<ValueType> laplacianCosts = laplacianBlock(fvec);
    Eigen::VectorBlock<ValueType> gamutCosts = outOfGamutBlock(fvec);

    // TODO: useless as we are already doing a setConstant at start, right?
    // unary costs are optionnals
    unaryCosts.setZero();
    gamutCosts.setZero();

    // TODO: this loop may be optimized as we do lots of redundent computations
    // with the parametric steps.
    for(PairingId id: Base::space().getLayerPairingIdList(_startLayerId)) {
      ParametricPointIdPair pppids =
        Base::space().getPairingParametricPointIds(id);
      ConstComponentInterFunc poly =
        _interp.getConstInterpolationFunctor(id, x);

      // pairing edge laplacian
      UType up, u, un;
      ParametricPoint ppp, pp, ppn;

      up << Scalar(0) / Scalar(NbParametricSteps+2);
      u  << Scalar(1) / Scalar(NbParametricSteps+2);
      ppp = ParametricPoint(poly.eval(up));
      pp  = ParametricPoint(poly.eval(u));

      for(int s = 0; s < NbParametricSteps; ++s) {
        un << Scalar(s+2) / Scalar(NbParametricSteps+2);
        ppn = ParametricPoint(poly.eval(un));

        const int id = idPairing * NbParametricSteps + s;
        gamutCosts(id) = ParameterValidationHelper::isValid(ppn)
          ? Scalar(0)
          :   Base::outOfGamutExplorationFactor
            * (ppn - ParameterValidationHelper::process(ppn))
                .getNative().squaredNorm();

        laplacianCosts.template segment<ParametricPoint::SpaceDim>
          (ParametricPoint::SpaceDim * id) =
            HelperType::lambda()
          * ( ppp.getUnscaledNative()
            + ppn.getUnscaledNative()
            - pp.getUnscaledNative() * Scalar(2))
          * Scalar(NbParametricSteps * NbParametricSteps);

        // copy computed color and curve coordinate for the next step
        up = u; ppp = pp;
        u = un; pp = ppn;
      }

      //! \FIXME v0 and v1 could be factorized:
      //!  - using a single loop for performances (check nullptr)
      //!  - using a lambda/function called twice
      assert(Base::space().getParametricPointLayerId(pppids.first) == _startLayerId);
      assert(Base::space().getParametricPointLayerId(pppids.second) == _endLayerId);
      const int off = idPairing * dsize * NbParametricSteps;
      bool decreasingW[2] = {true, false}; // TODO: check layer??
      const std::vector<typename CPS::UnaryConstraintFunctor*>* functors[2] = {
        &Base::space().getParametricPointFunctors(pppids.first),
        &Base::space().getParametricPointFunctors(pppids.second)
      };
      for(int iv = 0; iv < 2; ++iv) {
        for(typename CPS::UnaryConstraintFunctor* f: *functors[iv]) {
          if( f->getW() == Scalar(0) )
            continue;
          for(int s = 0; s != NbParametricSteps; ++s) {
            UType u;
            u << Scalar(s+1) / Scalar(NbParametricSteps+2);
            const int id = off + s * dsize;
            // TODO: here, we do not do like other linear interpolation. We
            // should have 1-w to have the real complementary value. Think about
            // rounding error that may imply "1-u(0) + u(0) != 1", but
            // "w=1-u(0); w + 1-w == 1". We have to check which is the best in
            // term of floating point error.
            const Scalar w = (decreasingW[iv] ? (Scalar(1)-u(0)) : u(0));
            unaryCosts(id) +=
                w
              * f->getW()
              * f->eval( {ParametricPoint(poly.eval(u))}, u(0));
          }
        }
      }

      // next unary cost
      ++idPairing;
    }


    ////////////////////////////////////////////////////////////////////////////
    ///
    /// Global constraints
    ///
    ///
    Eigen::VectorBlock<ValueType> globalCosts = globalBlock(fvec);

    if( Base::space().getAllGlobalConstraintsCount() > 0 ) {
      for(int s = 0; s != NbParametricSteps; ++s){
        const int off = s * dsize;
        const Scalar factor = dsize * NbParametricSteps;

        UType u;
        const Scalar uScalar = Scalar(s+1) / Scalar(NbParametricSteps+2);
        u << uScalar;

        // Create palette and call evaluation
        // TODO: We create the 'poly' many times in this function, I'm quite
        // sure we could do it once and reuse it many times.
        std::vector<ParametricPoint> palette;
        for(PairingId id: Base::space().getLayerPairingIdList(_startLayerId)) {
          ConstComponentInterFunc poly =
            _interp.getConstInterpolationFunctor(id,x);
          palette.push_back(ParametricPoint(poly.eval(u)));
        }

        const GlobalConstraintIdList& gc0ids =
          Base::space().getLayerGlobalConstraintIdList(_startLayerId);
        const GlobalConstraintIdList& gc1ids =
          Base::space().getLayerGlobalConstraintIdList(_endLayerId);
        assert(gc0ids.size() == gc1ids.size());

        int globId = 0;
        for(size_t gidx = 0; gidx < gc0ids.size(); ++gidx, ++globId) {
          const int id = off + globId * factor;
          typename CPS::NaryConstraintFunctor* f0 =
            Base::space().getGlobalConstraintFunctors(gc0ids[gidx]);
          typename CPS::NaryConstraintFunctor* f1 =
            Base::space().getGlobalConstraintFunctors(gc1ids[gidx]);
          assert(f0 && f1);
          // TODO: why not using &&?
          if( f0->getW() == Scalar(0) || f1->getW() == Scalar(0) )
            continue;

          globalCosts(id) =
                (Scalar(1) - uScalar)
              * f0->getW()
              * f0->eval(palette.begin(), palette.end(), uScalar)
            +   (uScalar)
              * f1->getW()
              * f1->eval(palette.begin(), palette.end(), uScalar);
        }
      }
    }

    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Visitors

template<class ParametricPoint,
         typename ParameterValidationHelper,
         typename ComponentInterFunc,
         typename ConstComponentInterFunc,
         int NbParametricSteps>
template <typename F>
void
SolverInterpolation<ParametricPoint, ParameterValidationHelper, ComponentInterFunc, ConstComponentInterFunc, NbParametricSteps>::
visitPolynomials(
        F &f,
        Eigen::Matrix<Scalar, Eigen::Dynamic, 1> &x){
    _interp.visitInterpolationFunctor(f, x);
}

template<class ParametricPoint,
         typename ParameterValidationHelper,
         typename ComponentInterFunc,
         typename ConstComponentInterFunc,
         int NbParametricSteps>
template <typename F>
void
SolverInterpolation<ParametricPoint, ParameterValidationHelper, ComponentInterFunc, ConstComponentInterFunc, NbParametricSteps>::
visitPolynomials(
        const F &f,
        Eigen::Matrix<Scalar, Eigen::Dynamic, 1> &x){
    _interp.visitInterpolationFunctor(f, x);
}

template<class ParametricPoint,
         typename ParameterValidationHelper,
         typename ComponentInterFunc,
         typename ConstComponentInterFunc,
         int NbParametricSteps>
template <typename F>
void
SolverInterpolation<ParametricPoint, ParameterValidationHelper, ComponentInterFunc, ConstComponentInterFunc, NbParametricSteps>::
visitConstPolynomials(
        F &f,
        const Eigen::Matrix<Scalar, Eigen::Dynamic, 1> &x) const{
    _interp.visitConstInterpolationFunctor(f, x);
}

template<class ParametricPoint,
         typename ParameterValidationHelper,
         typename ComponentInterFunc,
         typename ConstComponentInterFunc,
         int NbParametricSteps>
template <typename F>
void
SolverInterpolation<ParametricPoint, ParameterValidationHelper, ComponentInterFunc, ConstComponentInterFunc, NbParametricSteps>::
visitConstPolynomials(
        const F &f,
        const Eigen::Matrix<Scalar, Eigen::Dynamic, 1> &x) const{
    _interp.visitConstInterpolationFunctor(f, x);
}

#endif // SOLVER_INTERPOLATION_INL
