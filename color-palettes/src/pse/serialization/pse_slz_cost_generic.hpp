#ifndef DISCREPANCY_SERIALIZATION_GENERIC_HPP
#define DISCREPANCY_SERIALIZATION_GENERIC_HPP

#include <pse/pse_cost_generic.hpp>

#include <pse/serialization/pse_serializer.hpp>
#include <pse/serialization/pse_slz_cost.hpp>

/******************************************************************************
 *
 * Generic discrepancy serialization functions
 *
 ******************************************************************************/

template<typename ParametricPoint>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeConstantBinaryDiscrepancyFunctor,
   Discrepancy::ConstantBinaryDiscrepancyFunctor<ParametricPoint>,
   serializeBinaryDiscrepancyFunctor<ParametricPoint>)

template<typename ParametricPoint, typename BinaryFunctor>
SERIALIZER_TYPED_OBJECT_BEGIN
  (serializeAnchorUnaryDiscrepancyFunctor,
   Discrepancy::AnchorUnaryDiscrepancyFunctor<TEMPLATE_ARGS(ParametricPoint, BinaryFunctor)>, d)
  SERIALIZE_TYPED_OBJECT_CUSTOM_INLINE
   (serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(ParametricPoint, BinaryFunctor)>, &d);
  ParametricPoint ref = d.getRef();
  SERIALIZE_TYPED_OBJECT("Ref", ref);
  d.setRef(ref);
SERIALIZER_TYPED_OBJECT_END()

template<typename ParametricPoint, typename BinaryFunctor>
SERIALIZER_TYPED_OBJECT_BEGIN
  (serializeLinearDeviationUnaryDiscrepancyFunctor,
   Discrepancy::LinearDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(ParametricPoint, BinaryFunctor)>, d)
  SERIALIZE_TYPED_OBJECT_CUSTOM_INLINE
   (serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(ParametricPoint, BinaryFunctor)>, &d);
  ParametricPoint pps = d.getStart();
  ParametricPoint ppe = d.getEnd();
  SERIALIZE_TYPED_OBJECT("Start", pps);
  SERIALIZE_TYPED_OBJECT("End", ppe);
  d.setStart(pps);
  d.setEnd(ppe);
SERIALIZER_TYPED_OBJECT_END()

template<typename ParametricPoint, typename BinaryFunctor>
SERIALIZER_TYPED_OBJECT_BEGIN
  (serializePairWiseGlobalAnchorDiscrepancyFunctor,
   Discrepancy::PairWiseGlobalAnchorDiscrepancyFunctor<TEMPLATE_ARGS(ParametricPoint, BinaryFunctor)>, d)
  SERIALIZE_TYPED_OBJECT_CUSTOM_INLINE
    (serializeDelegatingDynamicNaryDiscrepancyFunctor<TEMPLATE_ARGS(ParametricPoint, BinaryFunctor)>, &d);
  using Base =
    Discrepancy::PairWiseGlobalAnchorDiscrepancyFunctor<ParametricPoint, BinaryFunctor>;
  using Container = typename Base::Container;
  Container c = d.getRef();
  size_t count = c.size();
  SERIALIZE_ARRAY_BEGIN("Refs", count);
  c.resize(count);
  for(auto& v: c) {
    SERIALIZE_ARRAY_ITEM_BEGIN("Ref");
    SERIALIZE_TYPED_OBJECT("Value", v);
    SERIALIZE_ARRAY_ITEM_END("Ref");
  }
  SERIALIZE_ARRAY_END("Ref");
  d.setRef(c);
SERIALIZER_TYPED_OBJECT_END()

#endif // DISCREPANCY_SERIALIZATION_GENERIC_HPP
