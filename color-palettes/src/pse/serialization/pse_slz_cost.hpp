#ifndef SERIALIZATION_DISCREPANCY_HPP
#define SERIALIZATION_DISCREPANCY_HPP

#include <pse/pse_cost.hpp>

#include <pse/serialization/pse_serializer.hpp>

/******************************************************************************
 *
 *
 *
 ******************************************************************************/

// TODO: use SERIALIZE_TYPED_OBJECT_INLINE instead of duplicating code

template<typename ParametricPoint, int N>
SERIALIZER_TYPED_OBJECT_BEGIN
  (serializeNaryDiscrepancyFunctor,
   Discrepancy::NaryDiscrepancyFunctor<TEMPLATE_ARGS(ParametricPoint, N)>, d)
  using Base = Discrepancy::NaryDiscrepancyFunctor<ParametricPoint, N>;
  using DiscrOutType = typename Base::DiscrOutType;
  DiscrOutType ref = d.getRefValue();
  SERIALIZE_VALUE("StartRef", ref);
  d.setRefValue(ref);
SERIALIZER_TYPED_OBJECT_END()

template<typename ParametricPoint, int N>
SERIALIZER_TYPED_OBJECT_BEGIN
  (serializeWeightedNaryDiscrepancyFunctor,
   Discrepancy::WeightedNaryDiscrepancyFunctor<TEMPLATE_ARGS(ParametricPoint, N)>, d)
  using Base = Discrepancy::NaryDiscrepancyFunctor<ParametricPoint, N>;
  using DiscrOutType = typename Base::DiscrOutType;
  using Scalar = typename Base::Scalar;
  DiscrOutType ref = d.getRefValue();
  SERIALIZE_VALUE("StartRef", ref);
  d.setRefValue(ref);
  Scalar weight = d.getW();
  SERIALIZE_VALUE("Weight", weight);
  d.setW(weight);
SERIALIZER_TYPED_OBJECT_END()

template<typename ParametricPoint, typename BinaryFunctor>
SERIALIZER_TYPED_OBJECT_BEGIN
  (serializeDelegatingUnaryDiscrepancyFunctor,
   Discrepancy::DelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(ParametricPoint, BinaryFunctor)>, d)
  using Base = Discrepancy::DelegatingUnaryDiscrepancyFunctor<ParametricPoint, BinaryFunctor>;
  using DiscrOutType = typename Base::DiscrOutType;
  using Scalar = typename Base::Scalar;
  DiscrOutType ref = d.getRefValue();
  Scalar weight = d.getW();
  BinaryFunctor bf = d.functor();
  SERIALIZE_VALUE("StartRef", ref);
  SERIALIZE_VALUE("Weight", weight);
  SERIALIZE_TYPED_OBJECT("DelegatedFunctor", bf);
  d.setRefValue(ref);
  d.setW(weight);
  d.setFunctor(bf);
SERIALIZER_TYPED_OBJECT_END()

template<typename ParametricPoint>
SERIALIZER_TYPED_OBJECT_BEGIN
  (serializeDynamicNaryDiscrepancyFunctor,
   Discrepancy::DynamicNaryDiscrepancyFunctor<ParametricPoint>, d)
  using Base = Discrepancy::DynamicNaryDiscrepancyFunctor<ParametricPoint>;
  using Scalar = typename Base::Scalar;
  using WContainer = typename Base::WContainer;
  WContainer weights = d.weights();
  size_t weights_count = weights.size();
  SERIALIZE_ARRAY_BEGIN("Weights", weights_count);
  weights.resize(weights_count);
  for(Scalar& w: weights) {
    SERIALIZE_VALUE("Weight", w);
  }
  SERIALIZE_ARRAY_END("Weights");
  d.setWeights(weights.begin(), weights.end());
SERIALIZER_TYPED_OBJECT_END()

template<typename ParametricPoint>
SERIALIZER_TYPED_OBJECT_BEGIN
  (serializeWeightedDynamicNaryDiscrepancyFunctor,
   Discrepancy::WeightedDynamicNaryDiscrepancyFunctor<ParametricPoint>, d)
  using Base = Discrepancy::WeightedDynamicNaryDiscrepancyFunctor<ParametricPoint>;
  using Scalar = typename Base::Scalar;
  using WContainer = typename Base::WContainer;
  Scalar weight = d.getW();
  SERIALIZE_VALUE("Weight", weight);
  d.setW(weight);
  WContainer weights = d.weights();
  size_t weights_count = weights.size();
  SERIALIZE_ARRAY_BEGIN("Weights", weights_count);
  weights.resize(weights_count);
  for(Scalar& w: weights) {
    SERIALIZE_VALUE("Weight", w);
  }
  SERIALIZE_ARRAY_END("Weights");
  d.setWeights(weights.begin(), weights.end());
SERIALIZER_TYPED_OBJECT_END()

template<typename ParametricPoint, typename BinaryFunctor>
SERIALIZER_TYPED_OBJECT_BEGIN
  (serializeDelegatingDynamicNaryDiscrepancyFunctor,
   Discrepancy::DelegatingDynamicNaryDiscrepancyFunctor<TEMPLATE_ARGS(ParametricPoint, BinaryFunctor)>, d)
  using Base =
    Discrepancy::DelegatingDynamicNaryDiscrepancyFunctor<ParametricPoint, BinaryFunctor>;
  using Scalar = typename Base::Scalar;
  using WContainer = typename Base::WContainer;
  Scalar weight = d.getW();
  SERIALIZE_VALUE("Weight", weight);
  d.setW(weight);
  WContainer weights = d.weights();
  size_t weights_count = weights.size();
  SERIALIZE_ARRAY_BEGIN("Weights", weights_count);
  weights.resize(weights_count);
  for(Scalar& w: weights) {
    SERIALIZE_VALUE("Weight", w);
  }
  SERIALIZE_ARRAY_END("Weights");
  d.setWeights(weights.begin(), weights.end());
  BinaryFunctor bf = d.functor();
  SERIALIZE_TYPED_OBJECT("DelegatedFunctor", bf);
  d.setFunctor(bf);
SERIALIZER_TYPED_OBJECT_END()

template<typename ParametricPoint>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeUnaryDiscrepancyFunctor,
   Discrepancy::UnaryDiscrepancyFunctor<ParametricPoint>,
   serializeWeightedNaryDiscrepancyFunctor<TEMPLATE_ARGS(ParametricPoint, 1)>)

template<typename ParametricPoint>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeBinaryDiscrepancyFunctor,
   Discrepancy::BinaryDiscrepancyFunctor<ParametricPoint>,
   serializeWeightedNaryDiscrepancyFunctor<TEMPLATE_ARGS(ParametricPoint, 2)>)

#endif /* SERIALIZATION_DISCREPANCY_HPP */
