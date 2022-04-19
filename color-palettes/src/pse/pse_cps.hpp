#ifndef CONSTRAINED_PARAMETER_SPACE_HPP
#define CONSTRAINED_PARAMETER_SPACE_HPP

#include "pse_graph_utils.hpp"
#include "pse_cps_types.hpp"
#include "pse_cost.hpp"

#include <map>

//! \brief Constrained parameter space that use an internal graph layout.
//!
//! \note The ownership of the parametric points and the constraints is managed
//! by the user of this class! This allow to use std::shared_ptr during the
//! read-write use of this space and then use direct pointers or objects during
//! the read-only use.
//!
//! \note We could do better things here, by extracting the internal types and
//! using \c uintptr_t as constraints id, allowing the user to store a pointer
//! on something we don't care in this class.
template<typename ParametricPoint_>
class ConstrainedParameterSpace
{
public:
  INTERPOLIB_TYPENAME_DECLARE
    ( "ConstrainedParameterSpace<"
    + ParametricPoint_::getClassTypeName()
    + ">")

  using ParametricPoint = ParametricPoint_;
  using Scalar = typename ParametricPoint::Scalar; // FIXME: I really do not like that

  // TODO: check if that types could be template parameters instead of adding
  // a strong dependency on discrepancy here.
  using UnaryConstraintFunctor =
    Discrepancy::UnaryDiscrepancyFunctor<ParametricPoint>;
  using BinaryConstraintFunctor =
    Discrepancy::BinaryDiscrepancyFunctor<ParametricPoint>;
  using NaryConstraintFunctor =
    Discrepancy::WeightedDynamicNaryDiscrepancyFunctor<ParametricPoint>;

  using ParametricPointParams = ::ParametricPointParams<ParametricPoint, UnaryConstraintFunctor>;
  using PairingParams = ::PairingParams;
  using ConstraintParams = ::ConstraintParams<BinaryConstraintFunctor>;
  using GlobalConstraintParams = ::GlobalConstraintParams<NaryConstraintFunctor>;

  static const ParametricPointParams PARAM_POINT_PARAMS_NULL;
  static const PairingParams PAIRING_PARAMS_NULL;
  static const ConstraintParams CONSTRAINT_PARAMS_NULL;
  static const GlobalConstraintParams GLOBAL_CONSTRAINT_PARAMS_NULL;

public:
  ConstrainedParameterSpace
    (const LayerId layers_count = 2)
    : _layers(layers_count, LAYER_EMPTY)
    , _count_ppoints_constraints_functors(0)
    , _count_constraints_functors(0)
    , _count_global_constraints_functors(0)
  {}
  virtual ~ConstrainedParameterSpace() {}

  inline void clear();

public:
  inline bool
  hasLayer(const LayerId id) const
    { return id < _layers.size(); }
  inline bool
  hasParametricPoint(const ParametricPointId id) const
    { return id < _content_ppoints.size(); }
  inline bool
  hasPairing(const PairingId id) const
    { return id < _edges_pairings.size(); }
  inline bool
  hasConstraint(const ConstraintId id) const
    { return id < _content_constraints.size(); }
  inline bool
  hasGlobalConstraint(const GlobalConstraintId id) const
    { return id < _content_global_constraints.size(); }

public:
  inline size_t
  getLayersCount() const
    { return _layers.size(); }

  inline size_t
  getAllParametricPointsCount() const
    { return _vertices_ppoints.size(); }
  inline size_t
  getLayerParametricPointsCount(const LayerId layer) const
    { assert(hasLayer(layer)); return _layers[layer]._ppoints.size(); }

  inline size_t
  getAllPairingsCount() const
    { return _edges_pairings.size(); }
  inline size_t
  getLayerPairingsCount(const LayerId layer) const
    { assert(hasLayer(layer)); return _layers[layer]._pairings.size(); }

  inline size_t
  getAllConstraintsCount() const
    { return _content_constraints.size(); }
  inline size_t
  getLayerConstraintsCount(const LayerId layer) const
    { assert(hasLayer(layer)); return _layers[layer]._constraints.size(); }

  inline size_t
  getAllGlobalConstraintsCount() const
    { return _content_global_constraints.size(); }
  inline size_t
  getLayerGlobalConstraintsCount(const LayerId layer) const
    { assert(hasLayer(layer)); return _layers[layer]._gconstraints.size(); }

public:
  template<class F>
  inline ERet
  visitConstAllParametricPoints(const F& f) const;
  template<class F>
  inline ERet
  visitAllParametricPoints(const F& f);
  template<class F>
  inline ERet
  visitAllPairings(const F& f);
  template<class F>
  inline ERet
  visitAllConstraints(const F& f);
  template<class F>
  inline ERet
  visitAllGlobalConstraints(const F& f);

  template<class F>
  inline ERet
  visitLayerParametricPoints
    (const LayerId layer, const F& f);
  template<class F>
  inline ERet
  visitLayerPairings
    (const LayerId layer, const F& f);
  template<class F>
  inline ERet
  visitLayerConstraints
    (const LayerId layer, const F& f);
  template<class F>
  inline ERet
  visitLayerGlobalConstraints
    (const LayerId layer, const F& f);

public:
  inline size_t
  getAllParametricPointsFunctorsCount() const
    { return _count_ppoints_constraints_functors; }
  inline size_t
  getLayerParametricPointsFunctorsCount(const LayerId layer) const
    { assert(hasLayer(layer)); return _layers[layer]._count_ppoints_constraints_functors; }
  inline size_t
  getParametricPointFunctorsCount(const ParametricPointId id) const
    { return getParametricPointFunctors(id).size(); }
  inline const std::vector<UnaryConstraintFunctor*>&
  getParametricPointFunctors(const ParametricPointId id) const
    { assert(hasParametricPoint(id)); return _content_ppoints[id]._own_constraint_functors; }

  inline const ParametricPointIdPair&
  getPairingParametricPointIds
    (const PairingId id) const
    { assert(hasPairing(id)); return _edges_pairings[id]; }

  inline size_t
  getAllConstraintsFunctorsCount() const
    { return _count_constraints_functors; }
  inline size_t
  getLayerConstraintsFunctorsCount(const LayerId layer) const
    { assert(hasLayer(layer)); return _layers[layer]._count_constraints_functors; }
  inline size_t
  getConstraintFunctorsCount(const ConstraintId id) const
    { return getConstraintFunctors(id).size(); }
  inline const std::vector<BinaryConstraintFunctor*>&
  getConstraintFunctors(const ConstraintId id) const
    { assert(hasConstraint(id)); return _content_constraints[id]._functors; }
  inline const ParametricPointIdPair&
  getConstraintParametricPoints(const ConstraintId id) const
    { assert(hasConstraint(id)); return _edges_constraints[id]; }
  inline ConstraintId
  getConstraintBuddyId(const ConstraintId id) const
    { assert(hasConstraint(id)); return _content_constraints[id]._buddy; }
  inline const PairingIdList&
  getConstraintInvolvedPairingIdList(const ConstraintId id) const
    { assert(hasConstraint(id)); return _content_constraints[id]._involved_pairings; }

  inline size_t
  getAllGlobalConstraintsFunctorsCount() const
    { return _count_global_constraints_functors; }
  inline size_t
  getLayerGlobalConstraintsFunctorsCount(const LayerId layer) const
    { assert(hasLayer(layer)); return _layers[layer]._count_global_constraints_functors; }
  inline size_t
  getGlobalConstraintFunctorsCount(const GlobalConstraintId id) const
    { return getGlobalConstraintFunctors(id) ? 1 : 0; }
  inline NaryConstraintFunctor*
  getGlobalConstraintFunctors(const GlobalConstraintId id) const
    { assert(hasGlobalConstraint(id)); return _content_global_constraints[id]._functor; }

public:
  inline const ParametricPointIdList&
  getLayerParametricPointIdList
    (const LayerId layer) const
    { assert(hasLayer(layer)); return _layers[layer]._ppoints; }
  inline const PairingIdList&
  getLayerPairingIdList
    (const LayerId layer) const
    { assert(hasLayer(layer)); return _layers[layer]._pairings; }
  inline const ConstraintIdList&
  getLayerConstraintIdList
    (const LayerId layer) const
    { assert(hasLayer(layer)); return _layers[layer]._constraints; }
  inline const GlobalConstraintIdList&
  getLayerGlobalConstraintIdList
    (const LayerId layer) const
    { assert(hasLayer(layer)); return _layers[layer]._gconstraints; }
  inline ERet
  getLayerParametricPointsValue
    (const LayerId layer,
     std::vector<ParametricPoint>& values);
  inline ERet
  setLayerParametricPointsValue
    (const LayerId layer,
     const std::vector<ParametricPoint>& values);

  inline ERet
  getAllParametricPointsParams
    (std::vector<ParametricPointParams>& params) const;
  inline ERet
  getParametricPointParams
    (const ParametricPointId id,
     ParametricPointParams& params) const;
  inline LayerId
  getParametricPointLayerId
    (const ParametricPointId id) const;
  inline ParametricPointId
  getParametricPointPairedBuddy
    (const LayerId in_layer_id,
     const ParametricPointId ref) const;
  inline ERet
  getParametricPointValue
    (const ParametricPointId id,
     ParametricPoint& value) const;
  inline ERet
  setParametricPointValue
    (const ParametricPointId id,
     const ParametricPoint& value);
  inline ERet
  setParametricPointImportance
    (const ParametricPointId id,
     const Scalar& importance);
  inline ERet
  addParametricPointFunctors
    (const ParametricPointId id,
     const std::vector<UnaryConstraintFunctor*>& functors);
  inline ERet
  getParametricPointConstraints
    (const ParametricPointId id,
     ConstraintIdList& constraints) const;

  inline ERet
  getAllPairingsParams
    (std::vector<PairingParams>& params) const;

  inline ERet
  getAllConstraintsParams
    (std::vector<ConstraintParams>& params) const;
  inline ParametricPointId
  getConstraintOtherParametricPoint
    (const ConstraintId id,
     const ParametricPointId known) const;

  inline ERet
  getAllGlobalConstraintsParams
    (std::vector<GlobalConstraintParams>& params) const;

  inline LayerId
  getGlobalConstraintLayerId
    (const GlobalConstraintId id) const;


public:
  //! These functions do an heavy search and thus should be rarely called
  inline ERet
  findAssociatedPairings
    (const ParametricPointId ppid,
     PairingIdList& pids) const;
  inline ERet
  findBuddyConstraints
    (const PairingId src_pairing,
     const PairingId dst_pairing,
     ConstraintIdList& buddies) const;

public:
  inline ERet
  setLayersCount
    (const size_t count);

  inline ERet
  addParametricPoints
    (const LayerId layer_id,
     const std::vector<ParametricPointParams>& params,
     ParametricPointIdList& ids);

  inline ERet
  addPairings
    (const std::vector<PairingParams>& params,
     PairingIdList& ids);

  inline ERet
  addConstraints
    (const std::vector<ConstraintParams>& params,
     ConstraintIdList& ids);

  inline ERet
  addGlobalConstraints
    (const LayerId layer_id, //!< if INVALID, then added to all layers
     const std::vector<GlobalConstraintParams>& params,
     GlobalConstraintIdList& ids);
  inline ERet
  clearAllGlobalConstraints();

public:
  // TODO: to remove ASAP
  //! \brief Call evalStart for all the constraints of the space, mainly to
  //! initialize the reference values of constraints.
  inline ERet
  initConstraintsInternals
    (LayerId start_layer_id = 0);

private:
  //! Graph memory layout to represent the constraints/pairings between
  //! parametric points. They store the relationships and then allow to check
  //! the existence of the relationship.
  GraphVertices _vertices_ppoints;
  GraphEdges _edges_pairings;
  GraphEdges _edges_constraints;

private:

  struct ParametricPointContent {
    ParametricPoint _value;
    Scalar _importance;
    std::vector<UnaryConstraintFunctor*> _own_constraint_functors;

    //PairingIdList _attached_pairings; // TODO: optim but duplication
    //ConstraintIdList _attached_constraints; // TODO: optim but duplication
  };
  static const ParametricPointContent PARAM_POINT_CONTENT_EMPTY;

  struct ConstraintContent {
    std::vector<BinaryConstraintFunctor*> _functors;
    ConstraintId _buddy; //!< Used to known the related constraints in other layers
    PairingIdList _involved_pairings;
  };
  static const ConstraintContent CONSTRAINT_CONTENT_EMPTY;

  struct GlobalConstraintContent {
    NaryConstraintFunctor* _functor;
  };
  static const GlobalConstraintContent GLOBAL_CONSTRAINT_CONTENT_EMPTY;

  //! There is no specific pairing content. We just have to known that two
  //! points are paired.

  struct Layer {
    ParametricPointIdList _ppoints;   //!< indices of parametric points in this layer
    PairingIdList _pairings;   //!< indices of pairings attached to this layer
    ConstraintIdList _constraints;   //!< indices of constraints in this layer
    GlobalConstraintIdList _gconstraints; //!< indices of global constraints in this layer
    size_t _count_ppoints_constraints_functors; //!< sum of constraints functors of vertices of this layer
    size_t _count_constraints_functors; //!< sum of constraints functors connected to vertices of this layer
    size_t _count_global_constraints_functors; //!< sum of global constraints functors of this layer
  };
  static const Layer LAYER_EMPTY;

  std::vector<ParametricPointContent> _content_ppoints;
  std::vector<ConstraintContent> _content_constraints;
  std::vector<GlobalConstraintContent> _content_global_constraints;
  std::vector<Layer> _layers;

  size_t _count_ppoints_constraints_functors;
  size_t _count_constraints_functors;
  size_t _count_global_constraints_functors;

  //! Stores the buddy constraints in other layers related to an existing
  //! constraint. The key of the map is a shared id among all constraints that
  //! are buddy of each other.
  //std::map<ConstraintBuddyId, ConstraintIdList> _constraints_buddies;

};

// FIXME: I really don't like this class that seems generic but that is not by
// using lots of implicitly expected functions. I would prefer something more
// explicit, thus avoiding errors and hard immersion.
template<typename ConstrainedParameterSpace,
         typename ComponentInterFunc,
         typename ConstComponentInterFunc>
class ConstrainedParameterSpaceInterpolator
{
public:
  using ParametricPoint =
    typename ConstrainedParameterSpace::ParametricPoint;
  using FVectorType =
    Eigen::Matrix<typename ParametricPoint::Scalar, Eigen::Dynamic, 1>;

  ConstrainedParameterSpaceInterpolator()
    : _space(nullptr)
  {}
  ConstrainedParameterSpaceInterpolator
    (ConstrainedParameterSpace& space)
    : _space(&space)
  {}

  inline void setSpace(ConstrainedParameterSpace& space) { _space = &space; }
  inline const ConstrainedParameterSpace* space() const { return _space; }

  ////////////////////////////////////////////////////////////////////////////
  /// Interpolation functors
  ///
private:
  template<typename InterFunc, typename VecType>
  inline InterFunc
  getInterpolationFunctorImplem
    (const PairingId id,
     VecType& x) const
  {
    assert(_space != nullptr);
    const ParametricPointIdPair pppids = _space->getPairingParametricPointIds(id);
    ParametricPoint pp0val, pp1val;
    // TODO: manage return code!
    CALL(_space->getParametricPointValue(pppids.first, pp0val));
    CALL(_space->getParametricPointValue(pppids.second, pp1val));
    return InterFunc
      (x.template segment<InterFunc::NbCoeff>(id * InterFunc::NbCoeff).data(),
       pp0val.getNative(),
       pp1val.getNative());
  }

  template<typename InterFunc, typename F, typename VecType>
  inline void
  visitInterpolationFunctorImplem
    (F& f,
     VecType &x ) const
  {
    assert(_space != nullptr);
    ParametricPoint pp0val, pp1val;
    const size_t count = _space->getAllPairingsCount();
    for(PairingId id = 0; id < count; ++id) {
      const ParametricPointIdPair pppids = _space->getPairingParametricPointIds(id);
      // TODO: manage return code!
      CALL(_space->getParametricPointValue(pppids.first, pp0val));
      CALL(_space->getParametricPointValue(pppids.second, pp1val));
      InterFunc comp
        (x.template segment<InterFunc::NbCoeff>(id * InterFunc::NbCoeff).data(),
         pp0val.getNative(),
         pp1val.getNative());
      f(comp, id);
    }
  }

public:
  inline ComponentInterFunc
  getInterpolationFunctor
    (const PairingId id,
     FVectorType& x)
  { return getInterpolationFunctorImplem<ComponentInterFunc, FVectorType>(id, x); }

  inline ConstComponentInterFunc
  getConstInterpolationFunctor
    (const PairingId id,
     const FVectorType& x ) const
  { return getInterpolationFunctorImplem<ConstComponentInterFunc, const FVectorType>(id, x); }

  template <typename F>
  inline void
  visitInterpolationFunctor
    (const F& f,
     FVectorType& x)
  { return visitInterpolationFunctorImplem<ComponentInterFunc, const F, FVectorType>(f,x); }

  template <typename F>
  inline void
  visitConstInterpolationFunctor
    (const F& f,
     const FVectorType& x) const
  { return visitInterpolationFunctorImplem<ConstComponentInterFunc, const F, const FVectorType>(f,x); }

private:
  ConstrainedParameterSpace* _space;

};

#include <pse/pse_cps.inl>

#endif // CONSTRAINED_PARAMETER_SPACE_HPP
