#ifndef SERIALIZATION_CPS_HPP
#define SERIALIZATION_CPS_HPP

#include <pse/pse_cps.hpp>

#include <pse/serialization/pse_serializer.hpp>

/******************************************************************************
 *
 * Generic types associated with ConstrainedParameterSpace
 *
 ******************************************************************************/

template<typename ParametricPoint_, typename Functor_>
SERIALIZER_CUSTOM_VALUE_BEGIN
  (ParametricPointParams<TEMPLATE_ARGS(ParametricPoint_, Functor_)>, ppp)
  SERIALIZE_TYPED_OBJECT("Value", ppp._value);
  SERIALIZE_VALUE("Importance", ppp._importance);
  SERIALIZE_VALUE_AS("FunctorsCount", ppp._own_constraints_functors_count, int64_t);
  if( s.isReading() ) {
    ppp._own_constraints_functors =
      new Functor_*[ppp._own_constraints_functors_count];
    SERIALIZE_INTERRUPT_IF_NOT
      (ppp._own_constraints_functors != nullptr, ESRet_OutOfMemory);
  }
  for(size_t f = 0; f < ppp._own_constraints_functors_count; ++f) {
    SERIALIZE_TYPED_OBJECT_PTR("Functor", ppp._own_constraints_functors[f]);
  }
SERIALIZER_CUSTOM_VALUE_END()

SERIALIZER_CUSTOM_VALUE_BEGIN(PairingParams, pp)
  SERIALIZE_VALUE_AS("ParametricPointSrcId", pp.first, int64_t);
  SERIALIZE_VALUE_AS("ParametricPointDstId", pp.second, int64_t);
SERIALIZER_CUSTOM_VALUE_END()

template<typename Functor_>
SERIALIZER_CUSTOM_VALUE_BEGIN(ConstraintParams<Functor_>, cp)
  SERIALIZE_VALUE_AS("ParametricPointSrcId", cp._ppoints.first, int64_t);
  SERIALIZE_VALUE_AS("ParametricPointDstId", cp._ppoints.second, int64_t);
  SERIALIZE_VALUE_AS("FunctorsCount", cp._functors_count, int64_t);
  if( s.isReading() ) {
    cp._functors = new Functor_*[cp._functors_count];
    SERIALIZE_INTERRUPT_IF_NOT(cp._functors != nullptr, ESRet_OutOfMemory);
  }
  for(size_t f = 0; f < cp._functors_count; ++f) {
    SERIALIZE_TYPED_OBJECT_PTR("Functor", cp._functors[f]);
  }
SERIALIZER_CUSTOM_VALUE_END()

template<typename Functor_>
SERIALIZER_CUSTOM_VALUE_BEGIN(GlobalConstraintParams<Functor_>, gcp)
  SERIALIZE_TYPED_OBJECT_PTR("Functor", gcp._functor);
SERIALIZER_CUSTOM_VALUE_END()

/******************************************************************************
 *
 * ConstrainedParameterSpace
 *
 ******************************************************************************/

template<typename ParametricPoint_>
SERIALIZER_TYPED_OBJECT_BEGIN
  (serializeCPS, ConstrainedParameterSpace<ParametricPoint_>, cps)
  using CPS = ConstrainedParameterSpace<ParametricPoint_>;

  // Serialize layers count
  size_t layers_count = cps.getLayersCount();
  SERIALIZE_VALUE_AS("LayersCount", layers_count, int64_t);
  CALL(cps.setLayersCount(layers_count));

  // Serialize parametric points
  // TODO: when we will split the CPS into something simpler + interpolable CPS,
  // this code will be simpler too.
  using ParametricPointParams = typename CPS::ParametricPointParams;
  std::vector<ParametricPointParams> ppparams;
  CALL(cps.getAllParametricPointsParams(ppparams));
  size_t ppcount = ppparams.size();
  SERIALIZE_ARRAY_BEGIN("ParametricPoints", ppcount);
  ppparams.resize(ppcount);
  for(ParametricPointId id = 0; id < ppcount; ++id) {
    SERIALIZE_ARRAY_ITEM_BEGIN("ParametricPoint");
    LayerId pplayer_id = cps.getParametricPointLayerId(id);
    SERIALIZE_VALUE_AS("LayerId", pplayer_id, int64_t);
    SERIALIZE_VALUE_INLINE(ppparams[id]);
    if( s.isReading() ) {
      // During deserialization, we have to add the current parametric point to
      // the CPS
      ParametricPointIdList ids;
      CALL(cps.addParametricPoints(pplayer_id, {ppparams[id]}, ids));
      SERIALIZE_INTERRUPT_IF_NOT(ids.size() == 1, ESRet_Invalid);
      SERIALIZE_INTERRUPT_IF_NOT(ids.back() == id, ESRet_Invalid);
    }
    SERIALIZE_ARRAY_ITEM_END("ParametricPoint");
  }
  SERIALIZE_ARRAY_END("ParametricPoints");

  // Serialize pairings
  using PairingParams = typename CPS::PairingParams;
  std::vector<PairingParams> pparams;
  CALL(cps.getAllPairingsParams(pparams));
  size_t pcount = pparams.size();
  SERIALIZE_ARRAY_BEGIN("Pairings", pcount);
  pparams.resize(pcount);
  for(PairingParams& pp: pparams) {
    SERIALIZE_ARRAY_ITEM_BEGIN("Pairing");
    SERIALIZE_VALUE_INLINE(pp);
    SERIALIZE_ARRAY_ITEM_END("Pairing");
  }
  if( s.isReading() && !pparams.empty() ) {
    PairingIdList ids;
    CALL(cps.addPairings(pparams, ids));
    SERIALIZE_INTERRUPT_IF_NOT(pparams.size() == ids.size(), ESRet_Invalid);
  }
  SERIALIZE_ARRAY_END("Pairings");

  // Serialize constraints
  using ConstraintParams = typename CPS::ConstraintParams;
  std::vector<ConstraintParams> cparams;
  CALL(cps.getAllConstraintsParams(cparams));
  size_t ccount = cparams.size();
  SERIALIZE_ARRAY_BEGIN("Constraints", ccount);
  cparams.resize(ccount);
  for(ConstraintParams& cp: cparams) {
    SERIALIZE_ARRAY_ITEM_BEGIN("Constraint");
    SERIALIZE_VALUE_INLINE(cp);
    SERIALIZE_ARRAY_ITEM_END("Constraint");
  }
  if( s.isReading() && !cparams.empty() ) {
    ConstraintIdList ids;
    CALL(cps.addConstraints(cparams, ids));
    SERIALIZE_INTERRUPT_IF_NOT(cparams.size() == ids.size(), ESRet_Invalid);
  }
  SERIALIZE_ARRAY_END("Constraints");

  // Serialize global constraints
  using GlobalConstraintParams = typename CPS::GlobalConstraintParams;
  std::vector<GlobalConstraintParams> gcparams;
  CALL(cps.getAllGlobalConstraintsParams(gcparams));
  size_t gccount = gcparams.size();
  SERIALIZE_ARRAY_BEGIN("GlobalConstraints", gccount);
  gcparams.resize(gccount);
  for(GlobalConstraintId id = 0; id < gccount; ++id) {
    GlobalConstraintParams& gcp = gcparams[id];
    SERIALIZE_ARRAY_ITEM_BEGIN("GlobalConstraint");
    LayerId gclayer_id = cps.getGlobalConstraintLayerId(id);
    SERIALIZE_VALUE_AS("LayerId", gclayer_id, int64_t);
    SERIALIZE_VALUE_INLINE(gcp);
    SERIALIZE_ARRAY_ITEM_END("GlobalConstraint");
    if( s.isReading() ) {
      GlobalConstraintIdList ids;
      CALL(cps.addGlobalConstraints(gclayer_id, {gcp}, ids));
      SERIALIZE_INTERRUPT_IF_NOT(ids.size() == 1, ESRet_Invalid);
      SERIALIZE_INTERRUPT_IF_NOT(ids.back() == id, ESRet_Invalid);
    }
  }
  SERIALIZE_ARRAY_END("GlobalConstraints");
SERIALIZER_TYPED_OBJECT_END()

template<typename ParametricPoint_>
static inline ESRet
cpsTypedObjectSerializationFunctionsRegister
  (Serializer& s)
{
  using CPS = ConstrainedParameterSpace<ParametricPoint_>;
  ESRet ret = ESRet_OK;
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, CPS, serializeCPS<ParametricPoint_>));

exit:
  return ret;
}

#endif // SERIALIZATION_CPS_HPP
