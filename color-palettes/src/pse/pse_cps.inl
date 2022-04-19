#ifndef CONSTRAINED_PARAMETER_SPACE_INL
#define CONSTRAINED_PARAMETER_SPACE_INL

#include <pse/pse_cps.hpp>

template<typename ParametricPoint>
const typename ConstrainedParameterSpace<ParametricPoint>::ParametricPointParams
ConstrainedParameterSpace<ParametricPoint>::PARAM_POINT_PARAMS_NULL =
  ::PARAM_POINT_PARAMS_NULL<ParametricPoint, UnaryConstraintFunctor>;

template<typename ParametricPoint>
const typename ConstrainedParameterSpace<ParametricPoint>::PairingParams
ConstrainedParameterSpace<ParametricPoint>::PAIRING_PARAMS_NULL =
  ::PAIRING_PARAMS_NULL;

template<typename ParametricPoint>
const typename ConstrainedParameterSpace<ParametricPoint>::ConstraintParams
ConstrainedParameterSpace<ParametricPoint>::CONSTRAINT_PARAMS_NULL =
  ::CONSTRAINT_PARAMS_NULL<BinaryConstraintFunctor>;

template<typename ParametricPoint>
const typename ConstrainedParameterSpace<ParametricPoint>::GlobalConstraintParams
ConstrainedParameterSpace<ParametricPoint>::GLOBAL_CONSTRAINT_PARAMS_NULL =
  ::GLOBAL_CONSTRAINT_PARAMS_NULL<NaryConstraintFunctor>;

template<typename ParametricPoint>
const typename ConstrainedParameterSpace<ParametricPoint>::ParametricPointContent
ConstrainedParameterSpace<ParametricPoint>::PARAM_POINT_CONTENT_EMPTY =
  { ParametricPoint{}, 0.0f, {} };

template<typename ParametricPoint>
const typename ConstrainedParameterSpace<ParametricPoint>::ConstraintContent
ConstrainedParameterSpace<ParametricPoint>::CONSTRAINT_CONTENT_EMPTY =
  { {}, ConstraintBuddyId_INVALID, {} };

template<typename ParametricPoint>
const typename ConstrainedParameterSpace<ParametricPoint>::GlobalConstraintContent
ConstrainedParameterSpace<ParametricPoint>::GLOBAL_CONSTRAINT_CONTENT_EMPTY =
  { ConstrainedParameterSpace<ParametricPoint>::GLOBAL_CONSTRAINT_PARAMS_NULL };

template<typename ParametricPoint>
const typename ConstrainedParameterSpace<ParametricPoint>::Layer
ConstrainedParameterSpace<ParametricPoint>::LAYER_EMPTY =
  { {}, {}, {}, {}, 0, 0, 0 };

template<typename ParametricPoint>
inline void
ConstrainedParameterSpace<ParametricPoint>::clear()
{
  _vertices_ppoints.clear();
  _edges_pairings.clear();
  _edges_constraints.clear();

  _content_ppoints.clear();
  _content_constraints.clear();
  _content_global_constraints.clear();
  for(Layer& l: _layers) {
    l._ppoints.clear();
    l._pairings.clear();
    l._constraints.clear();
    l._gconstraints.clear();
    l._count_ppoints_constraints_functors = 0;
    l._count_constraints_functors = 0;
    l._count_global_constraints_functors = 0;
  }
  // Keep the number of layers!

  _count_ppoints_constraints_functors = 0;
  _count_constraints_functors = 0;
  _count_global_constraints_functors = 0;
}

template<typename ParametricPoint>
template<class F>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::visitConstAllParametricPoints
  (const F& f) const
{
  const size_t count = getAllParametricPointsCount();
  for(ParametricPointId ppid = 0; ppid < count; ++ppid) {
    f(ppid);
  }
  return ERet_OK;
}

template<typename ParametricPoint>
template<class F>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::visitAllParametricPoints
  (const F& f)
{
  const size_t count = getAllParametricPointsCount();
  for(ParametricPointId ppid = 0; ppid < count; ++ppid) {
    f(ppid);
  }
  return ERet_OK;
}

template<typename ParametricPoint>
template<class F>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::visitAllPairings
  (const F& f)
{
  const size_t count = getAllPairingsCount();
  for(PairingId pid = 0; pid < count; ++pid) {
    f(pid);
  }
  return ERet_OK;
}

template<typename ParametricPoint>
template<class F>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::visitAllConstraints
  (const F& f)
{
  const size_t count = getAllConstraintsCount();
  for(ConstraintId cid = 0; cid < count; ++cid) {
    f(cid);
  }
  return ERet_OK;
}

template<typename ParametricPoint>
template<class F>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::visitAllGlobalConstraints
  (const F& f)
{
  const size_t count = getAllGlobalConstraintsCount();
  for(GlobalConstraintId gcid = 0; gcid < count; ++gcid) {
    f(gcid);
  }
  return ERet_OK;
}

template<typename ParametricPoint>
template<class F>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::visitLayerParametricPoints
  (const LayerId layer, const F& f)
{
  if( !hasLayer(layer) )
    return ERet_BadArg;
  for(ParametricPointId ppid: _layers[layer]._ppoints) {
    f(ppid);
  }
  return ERet_OK;
}

template<typename ParametricPoint>
template<class F>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::visitLayerPairings
  (const LayerId layer, const F& f)
{
  if( !hasLayer(layer) )
    return ERet_BadArg;
  for(PairingId pid: _layers[layer]._pairings) {
    f(pid);
  }
  return ERet_OK;
}

template<typename ParametricPoint>
template<class F>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::visitLayerConstraints
  (const LayerId layer, const F& f)
{
  if( !hasLayer(layer) )
    return ERet_BadArg;
  for(ConstraintId cid: _layers[layer]._constraints) {
    f(cid);
  }
  return ERet_OK;
}

template<typename ParametricPoint>
template<class F>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::visitLayerGlobalConstraints
  (const LayerId layer, const F& f)
{
  if( !hasLayer(layer) )
    return ERet_BadArg;
  for(GlobalConstraintId gcid: _layers[layer]._gconstraints) {
    f(gcid);
  }
  return ERet_OK;
}

template<typename ParametricPoint>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::getLayerParametricPointsValue
  (const LayerId layer,
   std::vector<ParametricPoint>& values)
{
  if( !hasLayer(layer) )
    return ERet_BadArg;

  for(ParametricPointId ppid: _layers[layer]._ppoints) {
    values.push_back(_content_ppoints[ppid]._value);
  }
  return ERet_OK;
}

template<typename ParametricPoint>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::setLayerParametricPointsValue
  (const LayerId layer,
   const std::vector<ParametricPoint>& values)
{
  if( !hasLayer(layer) )
    return ERet_BadArg;
  Layer& l = _layers[layer];
  if( l._ppoints.size() != values.size() )
    return ERet_BadArg;

  const size_t count = values.size();
  for(size_t i = 0; i < count; ++i) {
    _content_ppoints[l._ppoints[i]]._value = values[i];
  }
  return ERet_OK;
}

template<typename ParametricPoint>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::getAllParametricPointsParams
  (std::vector<ParametricPointParams>& params) const
{
  params.resize(_content_ppoints.size());
  for(ParametricPointId id = 0; id < params.size(); ++id) {
    const ParametricPointContent& ppc = _content_ppoints[id];
    ParametricPointParams& pp = params[id];
    pp._value = ppc._value;
    pp._importance = ppc._importance;
    pp._own_constraints_functors_count = ppc._own_constraint_functors.size();
    // HACK: we do not want to allocate memory here and we do not want to make
    // this function non-const, so we remove the const.
    pp._own_constraints_functors =
      const_cast<UnaryConstraintFunctor**>(ppc._own_constraint_functors.data());
  }
  return ERet_OK;
}

template<typename ParametricPoint>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::getParametricPointParams
  (const ParametricPointId id,
   ParametricPointParams& params) const
{
  if( !hasParametricPoint(id) )
    return ERet_BadArg;
  const ParametricPointContent& ppc = _content_ppoints[id];
  params._value = ppc._value;
  params._importance = ppc._importance;
  params._own_constraints_functors_count = ppc._own_constraint_functors.size();
  params._own_constraints_functors = ppc._own_constraint_functors.data();
  return ERet_OK;
}

template<typename ParametricPoint>
inline LayerId
ConstrainedParameterSpace<ParametricPoint>::getParametricPointLayerId
  (const ParametricPointId id) const
{
  if( !hasParametricPoint(id) )
    return LayerId_INVALID;

  // As we can ensure that an id is in one layer, we can avoid to check the last
  // one and deduce that if we didn't found the id in other layers, it will be
  // in the last.
  const LayerId last_lid = _layers.size() - 1;
  for(LayerId lid = 0; lid < last_lid; ++lid) {
    const Layer& l = _layers[lid];
    if( std::count(l._ppoints.begin(), l._ppoints.end(), id) >= 1 )
      return lid;
  }
  return last_lid;
}

template<typename ParametricPoint>
inline ParametricPointId
ConstrainedParameterSpace<ParametricPoint>::getParametricPointPairedBuddy
  (const LayerId in_layer_id,
   const ParametricPointId ref) const
{
  if( !hasLayer(in_layer_id)
   || !hasParametricPoint(ref) )
   return ParametricPointId_INVALID;

  const LayerId src_lid = getParametricPointLayerId(ref);
  if( src_lid == in_layer_id )
    return ParametricPointId_INVALID;

  for(PairingId pid: _layers[src_lid]._pairings) {
    const ParametricPointIdPair& p = _edges_pairings[pid];
    if( p.first != ref && p.second != ref )
      continue; // The reference ppoint is not used in this pairing

    if( p.first == ref && getParametricPointLayerId(p.second) == in_layer_id )
      return p.second;
    if( p.second == ref && getParametricPointLayerId(p.first) == in_layer_id )
      return p.first;
  }

  return ParametricPointId_INVALID;
}

template<typename ParametricPoint>
ERet
ConstrainedParameterSpace<ParametricPoint>::getParametricPointValue
  (const ParametricPointId id,
   ParametricPoint& value) const
{
  if( !hasParametricPoint(id) )
    return ERet_BadArg;
  value = _content_ppoints[id]._value;
  return ERet_OK;
}

template<typename ParametricPoint>
ERet
ConstrainedParameterSpace<ParametricPoint>::setParametricPointValue
  (const ParametricPointId id,
   const ParametricPoint& value)
{
  if( !hasParametricPoint(id) )
    return ERet_BadArg;
  _content_ppoints[id]._value = value;
  return ERet_OK;
}

template<typename ParametricPoint>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::setParametricPointImportance
  (const ParametricPointId id,
   const Scalar& importance)
{
  if( !hasParametricPoint(id) )
    return ERet_BadArg;
  _content_ppoints[id]._importance = importance;
  return ERet_OK;
}

template<typename ParametricPoint>
ERet
ConstrainedParameterSpace<ParametricPoint>::addParametricPointFunctors
  (const ParametricPointId id,
   const std::vector<UnaryConstraintFunctor*>& functors)
{
  if( !hasParametricPoint(id) )
    return ERet_BadArg;

  _content_ppoints[id]._own_constraint_functors.insert
    (_content_ppoints[id]._own_constraint_functors.end(),
     functors.begin(), functors.end());
  _count_ppoints_constraints_functors += functors.size();
  const LayerId lid = getParametricPointLayerId(id);
  _layers[lid]._count_ppoints_constraints_functors += functors.size();
  return ERet_OK;
}

template<typename ParametricPoint>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::getParametricPointConstraints
  (const ParametricPointId id,
   ConstraintIdList& constraints) const
{
  if( !hasParametricPoint(id) )
    return ERet_BadArg;

  const LayerId lid = getParametricPointLayerId(id);
  for(ConstraintId cid: _layers[lid]._constraints) {
    const ParametricPointIdPair& cppids = _edges_constraints[cid];
    if( cppids.first == id || cppids.second == id )
      constraints.push_back(cid);
  }
  return ERet_OK;
}

template<typename ParametricPoint>
ERet
ConstrainedParameterSpace<ParametricPoint>::getAllPairingsParams
  (std::vector<PairingParams>& params) const
{
  // No complex params, just do a copy
  params = _edges_pairings;
  return ERet_OK;
}

template<typename ParametricPoint>
ERet
ConstrainedParameterSpace<ParametricPoint>::getAllConstraintsParams
  (std::vector<ConstraintParams>& params) const
{
  params.resize(_content_constraints.size());
  for(ConstraintId id = 0; id < params.size(); ++id) {
    const ConstraintContent& cc = _content_constraints[id];
    ConstraintParams& cp = params[id];
    cp._ppoints = _edges_constraints[id];
    cp._functors_count = cc._functors.size();
    cp._functors = const_cast<BinaryConstraintFunctor**>(cc._functors.data());
  }
  return ERet_OK;
}

template<typename ParametricPoint>
inline ParametricPointId
ConstrainedParameterSpace<ParametricPoint>::getConstraintOtherParametricPoint
  (const ConstraintId id,
   const ParametricPointId known) const
{
  if( !hasConstraint(id)
   || !hasParametricPoint(known) )
    return ParametricPointId_INVALID;

  const ParametricPointIdPair& cppids = getConstraintParametricPoints(id);
  ParametricPointId other = ParametricPointId_INVALID;
  if( cppids.first == known )
    other = cppids.second;
  if( cppids.second == known )
    other = cppids.first;
  assert(other != ParametricPointId_INVALID); // Must not be an option
  return other;
}

template<typename ParametricPoint>
ERet
ConstrainedParameterSpace<ParametricPoint>::getAllGlobalConstraintsParams
  (std::vector<GlobalConstraintParams>& params) const
{
  params.resize(_content_global_constraints.size());
  for(GlobalConstraintId id = 0; id < params.size(); ++id) {
    const GlobalConstraintContent& gcc = _content_global_constraints[id];
    GlobalConstraintParams& gcp = params[id];
    gcp._functor = gcc._functor;
  }
  return ERet_OK;
}

template<typename ParametricPoint>
LayerId
ConstrainedParameterSpace<ParametricPoint>::getGlobalConstraintLayerId
  (const GlobalConstraintId id) const
{
  if( !hasGlobalConstraint(id) )
    return LayerId_INVALID;

  // As we can ensure that an id is in one layer, we can avoid to check the last
  // one and deduce that if we didn't found the id in other layers, it will be
  // in the last.
  const LayerId last_lid = _layers.size() - 1;
  for(LayerId lid = 0; lid < last_lid; ++lid) {
    const Layer& l = _layers[lid];
    if( std::count(l._gconstraints.begin(), l._gconstraints.end(), id) >= 1 )
      return lid;
  }
  return last_lid;
}

template<typename ParametricPoint>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::findAssociatedPairings
  (const ParametricPointId ppid,
   PairingIdList& pids) const
{
  // FIXME: this is a linear search! If this function is called often we should
  // find another way to do that, probably by storing the information somewhere.
  const size_t initial_size = pids.size();
  for(size_t i = 0; i < _edges_pairings.size(); ++i) {
    const PairingParams& pparams = _edges_pairings[i];
    if( pparams.first == ppid || pparams.second == ppid )
      pids.push_back(i);
  }
  return pids.size() == initial_size
    ? ERet_NotFound
    : ERet_OK;
}

template<typename ParametricPoint>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::findBuddyConstraints
  (const PairingId src_pairing,
   const PairingId dst_pairing,
   ConstraintIdList& buddies) const
{
  // TODO: this algorithm is broken as it does not check if the parametric
  // points are in another layer.
  for(ConstraintId id = 0; id < _content_constraints.size(); ++id) {
    const ConstraintContent& cc = _content_constraints[id];
    if( cc._involved_pairings.size() != 2 )
      continue;
    if(  ( cc._involved_pairings[0] == src_pairing
        && cc._involved_pairings[1] == dst_pairing)
      || ( cc._involved_pairings[0] == dst_pairing
        && cc._involved_pairings[1] == src_pairing) ) {
      // We have found a buddy constraint
      buddies.push_back(id);
    }
  }
  assert(buddies.size() <= 1);
  return ERet_OK;
}

template<typename ParametricPoint>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::setLayersCount
  (const size_t count)
{
  if( count > _layers.size() ) {
    _layers.resize(count, LAYER_EMPTY);
  } else if( count < _layers.size() ) {
    return ERet_NotImplemented;
  }
  return ERet_OK;
}

template<typename ParametricPoint>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::addParametricPoints
  (const LayerId layer_id,
   const std::vector<ParametricPointParams>& params,
   ParametricPointIdList& ids)
{
  if(  (layer_id >= _layers.size())
    || params.empty() )
    return ERet_BadArg;

  ERet ret = ERet_OK;
  Layer& layer = _layers[layer_id];
  const size_t initial_size = ids.size();

  CALL_OR_GOTO(ret, error, gphCreateVertices
    (_vertices_ppoints, params.size(), ids));

  for(size_t i = 0; i < params.size(); ++i) {
    const ParametricPointId ppid = ids[initial_size + i];
    assert(_content_ppoints.size() == ppid); // Ensure ids consistency

    const ParametricPointParams& ppparams = params[i];

    ParametricPointContent ppc = PARAM_POINT_CONTENT_EMPTY;

    ppc._value = ppparams._value;
    ppc._importance = ppparams._importance;

    // Copy functors
    ppc._own_constraint_functors.reserve
      (ppparams._own_constraints_functors_count);
    for(size_t f = 0; f < ppparams._own_constraints_functors_count; ++f) {
      ppc._own_constraint_functors.push_back
        (ppparams._own_constraints_functors[f]);
    }
    _content_ppoints.push_back(ppc);

    layer._ppoints.push_back(ppid);
    layer._count_ppoints_constraints_functors +=
      ppparams._own_constraints_functors_count;
    _count_ppoints_constraints_functors +=
      ppparams._own_constraints_functors_count;
  }

exit:
  return ret;
error:
  ids.resize(initial_size);
  goto exit;
}

template<typename ParametricPoint>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::addPairings
  (const std::vector<PairingParams>& params,
   PairingIdList& ids)
{
  if( params.empty() )
    return ERet_BadArg;

  ERet ret = ERet_OK;
  const size_t initial_size = ids.size();
  CALL_OR_GOTO(ret, error, gphCreateEdges(_edges_pairings, params, ids));

  // Keep pairings per layer
  LayerId lid;
  for(size_t i = 0; i < params.size(); ++i) {
    const PairingParams& pparams = params[i];
    lid = getParametricPointLayerId(pparams.first);
    assert(lid != LayerId_INVALID);
    _layers[lid]._pairings.push_back(ids[i]);
    lid = getParametricPointLayerId(pparams.second);
    assert(lid != LayerId_INVALID);
    _layers[lid]._pairings.push_back(ids[i]);
  }

exit:
  return ret;
error:
  ids.resize(initial_size);
  goto exit;
}

template<typename ParametricPoint>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::addConstraints
  (const std::vector<ConstraintParams>& params,
   ConstraintIdList& ids)
{
  if( params.empty() )
    return ERet_BadArg;

  ERet ret = ERet_OK;
  std::vector<ParametricPointIdPair> relationships;
  std::vector<ConstraintIdList> new_constraints(_layers.size());
  std::vector<size_t> counters_constraints_functors(_layers.size(), 0);
  ConstraintIdList buddies;
  const size_t initial_size = ids.size();
  const size_t cc_initial_size = _content_constraints.size();

  // Create the relationships between parametric points
  relationships.reserve(params.size());
  for(const ConstraintParams& cparams: params) {
    relationships.push_back(cparams._ppoints);
  }
  CALL_OR_GOTO(ret, error, gphCreateEdges
    (_edges_constraints, relationships, ids));

  buddies.reserve(1);
  for(size_t i = 0; i < params.size(); ++i) {
    const ConstraintId cid = ids[initial_size + i];
    assert(_content_constraints.size() == cid);

    const ConstraintParams& cparams = params[i];
    const LayerId layer_id = getParametricPointLayerId(cparams._ppoints.first);
    CHECK_OR_DO(layer_id != LayerId_INVALID,
      ret = ERet_Invalid; goto error);
    CHECK_OR_DO(layer_id == getParametricPointLayerId(cparams._ppoints.second),
      ret = ERet_BadArg; goto error);

    ConstraintContent cc = CONSTRAINT_CONTENT_EMPTY;

    // Copy functors
    cc._functors.reserve(cparams._functors_count);
    for(size_t f = 0; f < cparams._functors_count; ++f) {
      cc._functors.push_back(cparams._functors[f]);
    }

    // Find the involved pairing by getting thoses defined on the parametric
    // points associated with this constraint. Accept that they could not be
    // present.
    // TODO: what if we add pairings after ther constraints!???
    ret = findAssociatedPairings
      (cparams._ppoints.first, cc._involved_pairings);
    if( ret == ERet_OK ) {
      ret = findAssociatedPairings
        (cparams._ppoints.second, cc._involved_pairings);
    } else {
      // We accept to not find the associated pairing
      CHECK_OR_DO(ret == ERet_NotFound, goto error);
    }
    if( ret == ERet_OK ) {
      // TODO: relax this constraint to manage more than one pairing on each
      // parametric point, but in this case, we will have to manage buddies
      // differently.
      assert(cc._involved_pairings.size() == 2);

      // Find all existing buddy constraints
      buddies.clear();
      CALL_OR_GOTO(ret, error, findBuddyConstraints
        (cc._involved_pairings[0],
         cc._involved_pairings[1],
         buddies));
      assert(buddies.size() <= 1);
      if( buddies.size() > 0 ) {
        // Make them buddy
        cc._buddy = buddies[0];
        _content_constraints[cc._buddy]._buddy = cid;
      }
    } else {
      // We accept to not find the associated pairing
      CHECK_OR_DO(ret == ERet_NotFound, goto error);
      ret = ERet_OK;
    }

    counters_constraints_functors[layer_id] += cparams._functors_count;
    new_constraints[layer_id].push_back(cid);
    assert(_content_constraints.size() == cid);
    _content_constraints.push_back(cc);
  }

  // Everything went OK, we can update the layer and the counters
  for(LayerId id = 0; id < _layers.size(); ++id) {
    _layers[id]._constraints.insert
      (_layers[id]._constraints.end(),
       new_constraints[id].begin(),
       new_constraints[id].end());
    _layers[id]._count_constraints_functors +=
      counters_constraints_functors[id];
    _count_constraints_functors +=
      counters_constraints_functors[id];
  }

exit:
  return ret;
error:
  ids.resize(initial_size);
  _content_constraints.resize(cc_initial_size);
  goto exit;
}

template<typename ParametricPoint>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::addGlobalConstraints
  (const LayerId layer_id,
   const std::vector<GlobalConstraintParams>& params,
   GlobalConstraintIdList& ids)
{
  if(  (   (layer_id >= _layers.size())
        && (layer_id != LayerId_INVALID))
    ||  params.empty())
    return ERet_BadArg;

  ERet ret = ERet_OK;
  const GlobalConstraintId start_id = _content_global_constraints.size();
  size_t counter_global_constraints_functors = 0;

  // First, create the global constraints
  _content_global_constraints.reserve(start_id + params.size());
  for(const GlobalConstraintParams& gcparams: params) {
    const GlobalConstraintId gcid = _content_global_constraints.size();
    ids.push_back(gcid);
    _content_global_constraints.push_back({gcparams._functor});
    counter_global_constraints_functors += gcparams._functor != nullptr ? 1 : 0;
  }
  _count_global_constraints_functors += counter_global_constraints_functors;

  // Then, attach them to the requested layer(s)
  if( layer_id == LayerId_INVALID ) {
    // Add the global constraint to all layers
    for(Layer& l: _layers) {
      const size_t initial_size = l._gconstraints.size();
      l._gconstraints.resize(initial_size + params.size());
      std::iota
        (l._gconstraints.begin() + initial_size,
         l._gconstraints.end(),
         start_id);
      l._count_global_constraints_functors +=
        counter_global_constraints_functors;
    }
  } else {
    // Add the global constraint to the given layer
    Layer& layer = _layers[layer_id];
    const size_t initial_size = layer._gconstraints.size();
    layer._gconstraints.resize(initial_size + params.size());
    std::iota
      (layer._gconstraints.begin() + initial_size,
       layer._gconstraints.end(),
       start_id);
    layer._count_global_constraints_functors +=
      counter_global_constraints_functors;
  }

  return ret;
}

template<typename ParametricPoint>
inline ERet
ConstrainedParameterSpace<ParametricPoint>::clearAllGlobalConstraints()
{
  _content_global_constraints.clear();
  _count_global_constraints_functors = 0;
  for(Layer& l: _layers) {
    l._count_global_constraints_functors = 0;
    l._gconstraints.clear();
  }
  return ERet_OK;
}

template<typename ParametricPoint>
ERet
ConstrainedParameterSpace<ParametricPoint>::initConstraintsInternals
  (LayerId start_layer_id)
{
  std::vector<ParametricPoint> startPalette, endPalette;
  std::vector<Scalar> weights;

  // unary constraints
  ParametricPoint start, end;
  if( _layers.size() > 1 ) {
    for(ParametricPointIdPair p: _edges_pairings) {
      const ParametricPointContent& ppc0 = _content_ppoints[p.first];
      const ParametricPointContent& ppc1 = _content_ppoints[p.second];

      if( getParametricPointLayerId(p.first) == start_layer_id ) {
        start = ppc0._value;
        end = ppc1._value;
      } else {
        start = ppc1._value;
        end = ppc0._value;
      }

      // Here, we do like explain in the paper, we use the initial value of
      // parametric point and their importance to initialize the functors. We
      // could do something else.
      startPalette.push_back(start);
      endPalette.push_back(end);
      weights.push_back(ppc0._importance);

      for(UnaryConstraintFunctor* f: ppc0._own_constraint_functors) {
        f->evalStart( {start} );
        f->evalEnd( {end} );
      }
      for(UnaryConstraintFunctor* f: ppc1._own_constraint_functors) {
        f->evalStart( {end} );
        f->evalEnd( {start} );
      }
    }
  } else {
    assert(!_layers.empty());
    for(const ParametricPointContent& ppc: _content_ppoints) {
      startPalette.push_back(ppc._value);
      endPalette.push_back(ppc._value);
      weights.push_back(ppc._importance);
      for(UnaryConstraintFunctor* f: ppc._own_constraint_functors) {
        f->evalStart( {ppc._value} );
        f->evalEnd( {ppc._value} );
      }
    }
  }

  // binary constraints
  for(ConstraintId id = 0; id < getAllConstraintsCount(); ++id) {
    const ParametricPointIdPair c = _edges_constraints[id];
    const ParametricPointContent& ppc0 = _content_ppoints[c.first];
    const ParametricPointContent& ppc1 = _content_ppoints[c.second];

    // TODO: hacked, we have to do something better or remove the whole function
    const ParametricPointId pp0buddy = getParametricPointPairedBuddy
      ( start_layer_id == 0 ? 1 : 0, c.first );
    const ParametricPointId pp1buddy = getParametricPointPairedBuddy
      ( start_layer_id == 0 ? 1 : 0, c.second );

    if(  pp0buddy != ParametricPointId_INVALID
      && pp1buddy != ParametricPointId_INVALID ) {
      const ParametricPointContent& ppbc0 = _content_ppoints[pp0buddy];
      const ParametricPointContent& ppbc1 = _content_ppoints[pp1buddy];

      for(BinaryConstraintFunctor* f: _content_constraints[id]._functors) {
        f->evalStart( {ppc0._value, ppc1._value} );
        f->evalEnd( {ppbc0._value, ppbc1._value} );
      }
    } else {
      for(BinaryConstraintFunctor* f: _content_constraints[id]._functors) {
        f->evalStart( {ppc0._value, ppc1._value} );
      }
    }
  }

  // global constraints
  // TODO: hacked, we have to do something better or remove the whole function
  for(GlobalConstraintId id: getLayerGlobalConstraintIdList(start_layer_id)) {
    const GlobalConstraintContent& gcc = _content_global_constraints[id];
    gcc._functor->setWeights(weights.cbegin(), weights.cend());
    gcc._functor->evalStart(startPalette.cbegin(), startPalette.cend());
    gcc._functor->evalEnd(endPalette.cbegin(), endPalette.cend());
  }
  if( hasLayer(start_layer_id==0?1:0) ) {
    for(GlobalConstraintId id: getLayerGlobalConstraintIdList(start_layer_id==0?1:0)) {
      const GlobalConstraintContent& gcc = _content_global_constraints[id];
      gcc._functor->setWeights(weights.cbegin(), weights.cend());
      gcc._functor->evalStart(endPalette.cbegin(), endPalette.cend());
      gcc._functor->evalEnd(startPalette.cbegin(), startPalette.cend());
    }
  }

  return ERet_OK;
}
#endif // CONSTRAINED_PARAMETER_SPACE_INL
