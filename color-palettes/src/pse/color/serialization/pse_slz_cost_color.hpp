#ifndef DISCREPANCY_SERIALIZATION_COLOR_HPP
#define DISCREPANCY_SERIALIZATION_COLOR_HPP

#include <pse/color/pse_cost_color.hpp>
#include <pse/pse_cost_generic.hpp>

#include <pse/serialization/pse_slz_cost.hpp>
#include <pse/serialization/pse_serializer.hpp>

/******************************************************************************
 *
 * Internal color discrepancy serialization functions
 *
 ******************************************************************************/

namespace internal {

template<typename Color_, Color::UnscaledSpace Space_, int ColorSpaceComponentIdx_>
SERIALIZER_TYPED_OBJECT_BEGIN
  (serializeCWXXXDiscrepancyFunctor,
   ColorDiscrepancy::internal::CWXXXDiscrepancyFunctor<TEMPLATE_ARGS(Color_, Space_, ColorSpaceComponentIdx_)>, d)
  using Base =
    ColorDiscrepancy::internal::CWXXXDiscrepancyFunctor
      <Color_, Space_, ColorSpaceComponentIdx_>;
  using DiscrOutType = typename Base::DiscrOutType;
  SERIALIZE_TYPED_OBJECT_CUSTOM_INLINE
    (serializeBinaryDiscrepancyFunctor<Color_>, &d);
  DiscrOutType st = d.safeThreshold();
  DiscrOutType lc = d.limitedContrast();
  SERIALIZE_VALUE("SafeThreshold", st);
  SERIALIZE_VALUE("LimitedContrast", lc);
  d.setSafeThreshold(st);
  d.setLimitedContrast(lc);
SERIALIZER_TYPED_OBJECT_END()

template<typename Color_, Color::UnscaledSpace Space_, int ColorSpaceComponentIdx_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeCWUnsignedXXXDiscrepancyFunctor,
   ColorDiscrepancy::internal::CWUnsignedXXXDiscrepancyFunctor<TEMPLATE_ARGS(Color_, Space_, ColorSpaceComponentIdx_)>,
   serializeBinaryDiscrepancyFunctor<Color_>)

}

/******************************************************************************
 *
 * Color discrepancy serialization functions
 *
 ******************************************************************************/

template<typename Color_>
SERIALIZER_TYPED_OBJECT_BEGIN
  (serializeDistanceDiscrepancyFunctor,
   ColorDiscrepancy::DistanceDiscrepancyFunctor<Color_>, d)
  using Base = ColorDiscrepancy::DistanceDiscrepancyFunctor<Color_>;
  using DiscrOutType = typename Base::DiscrOutType;
  SERIALIZE_TYPED_OBJECT_CUSTOM_INLINE
    (serializeBinaryDiscrepancyFunctor<Color_>, &d);
  DiscrOutType st = d.safeThreshold();
  DiscrOutType lc = d.limitedContrast();
  SERIALIZE_VALUE("SafeThreshold", st);
  SERIALIZE_VALUE("LimitedContrast", lc);
  d.setSafeThreshold(st);
  d.setLimitedContrast(lc);
SERIALIZER_TYPED_OBJECT_END()

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeRelativeLuminanceDiscrepancyFunctor,
   ColorDiscrepancy::RelativeLuminanceDiscrepancyFunctor<Color_>,
   serializeBinaryDiscrepancyFunctor<Color_>)

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeCCTDiscrepancyFunctor,
   ColorDiscrepancy::CCTDiscrepancyFunctor<Color_>,
   serializeBinaryDiscrepancyFunctor<Color_>)

template<typename Color_, typename BinaryFunctor_>
SERIALIZER_TYPED_OBJECT_BEGIN
  (serializePointSetDeviationUnaryDiscrepancyFunctor,
   ColorDiscrepancy::PointSetDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, BinaryFunctor_)>, d)
  SERIALIZE_TYPED_OBJECT_CUSTOM_INLINE
    (serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, BinaryFunctor_)>, &d);
  size_t count = d.pointsCount();
  Color_ p;
  SERIALIZE_ARRAY_BEGIN("Points", count);
  if( s.isReading() )
    d.clearAndReservePointsCount(count);
  for(size_t i = 0; i < count; ++i) {
    if( s.isWriting() )
      p = d.getPoint(i);
    SERIALIZE_ARRAY_ITEM_BEGIN("Point");
    SERIALIZE_TYPED_OBJECT("Color", p);
    SERIALIZE_ARRAY_ITEM_END("Point");
    if( s.isReading() )
      d.addPoint(p);
  }
  SERIALIZE_ARRAY_END("Points");
  if( s.isReading() )
    d.finalizePoints();
SERIALIZER_TYPED_OBJECT_END()

template<typename Color_, typename BinaryFunctor_>
SERIALIZER_TYPED_OBJECT_BEGIN
  (serializeGlobalDeviationDiscrepancyFunctor,
   ColorDiscrepancy::GlobalDeviationDiscrepancyFunctor<TEMPLATE_ARGS(Color_, BinaryFunctor_)>, d)
  using Base =
    ColorDiscrepancy::GlobalDeviationDiscrepancyFunctor<Color_, BinaryFunctor_>;
  using RefPointType = typename Base::RefPointType;
  SERIALIZE_TYPED_OBJECT_CUSTOM_INLINE
    (serializeDelegatingDynamicNaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, BinaryFunctor_)>, &d);
  RefPointType ref = d.getRef();
  SERIALIZE_TYPED_OBJECT("Ref", ref);
  d.setRef(ref);
SERIALIZER_TYPED_OBJECT_END()

template<typename Color_>
SERIALIZER_TYPED_OBJECT_BEGIN
  (serializeCCTGlobalAnchorDiscrepancyFunctor,
   ColorDiscrepancy::CCTGlobalAnchorDiscrepancyFunctor<Color_>, d)
  using Base = ColorDiscrepancy::CCTGlobalAnchorDiscrepancyFunctor<Color_>;
  using Scalar = typename Base::Scalar;
  SERIALIZE_TYPED_OBJECT_CUSTOM_INLINE
    (serializeWeightedDynamicNaryDiscrepancyFunctor<Color_>, &d);
  Scalar ref = d.getRef();
  SERIALIZE_VALUE("Ref", ref);
  d.setRef(ref);
SERIALIZER_TYPED_OBJECT_END()

// Distance L1

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeHSVHueDiscrepancyFunctor,
   ColorDiscrepancy::HSV_HueDiscrepancyFunctor<Color_>,
   internal::serializeCWXXXDiscrepancyFunctor<TEMPLATE_ARGS(Color_, Color::HSV_360, 0)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeHSVSaturationDiscrepancyFunctor,
   ColorDiscrepancy::HSV_SaturationDiscrepancyFunctor<Color_>,
   internal::serializeCWXXXDiscrepancyFunctor<TEMPLATE_ARGS(Color_, Color::HSV_360, 1)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeHSVValueDiscrepancyFunctor,
   ColorDiscrepancy::HSV_ValueDiscrepancyFunctor<Color_>,
   internal::serializeCWXXXDiscrepancyFunctor<TEMPLATE_ARGS(Color_, Color::HSV_360, 3)>)

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLABLuminanceDiscrepancyFunctor,
   ColorDiscrepancy::LAB_LuminanceDiscrepancyFunctor<Color_>,
   internal::serializeCWXXXDiscrepancyFunctor<TEMPLATE_ARGS(Color_, Color::Lab_128, 0)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLABaDiscrepancyFunctor,
   ColorDiscrepancy::LAB_aDiscrepancyFunctor<Color_>,
   internal::serializeCWXXXDiscrepancyFunctor<TEMPLATE_ARGS(Color_, Color::Lab_128, 1)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLABbDiscrepancyFunctor,
   ColorDiscrepancy::LAB_bDiscrepancyFunctor<Color_>,
   internal::serializeCWXXXDiscrepancyFunctor<TEMPLATE_ARGS(Color_, Color::Lab_128, 3)>)

// Distance L2

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeHSVHueUDiscrepancyFunctor,
   ColorDiscrepancy::HSV_HueUDiscrepancyFunctor<Color_>,
   internal::serializeCWXXXDiscrepancyFunctor<TEMPLATE_ARGS(Color_, Color::HSV_360, 0)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeHSVSaturationUDiscrepancyFunctor,
   ColorDiscrepancy::HSV_SaturationUDiscrepancyFunctor<Color_>,
   internal::serializeCWXXXDiscrepancyFunctor<TEMPLATE_ARGS(Color_, Color::HSV_360, 1)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeHSVValueUDiscrepancyFunctor,
   ColorDiscrepancy::HSV_ValueUDiscrepancyFunctor<Color_>,
   internal::serializeCWXXXDiscrepancyFunctor<TEMPLATE_ARGS(Color_, Color::HSV_360, 3)>)

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLABLuminanceUDiscrepancyFunctor,
   ColorDiscrepancy::LAB_LuminanceUDiscrepancyFunctor<Color_>,
   internal::serializeCWXXXDiscrepancyFunctor<TEMPLATE_ARGS(Color_, Color::Lab_128, 0)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLABaUDiscrepancyFunctor,
   ColorDiscrepancy::LAB_aUDiscrepancyFunctor<Color_>,
   internal::serializeCWXXXDiscrepancyFunctor<TEMPLATE_ARGS(Color_, Color::Lab_128, 1)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLABbUDiscrepancyFunctor,
   ColorDiscrepancy::LAB_bUDiscrepancyFunctor<Color_>,
   internal::serializeCWXXXDiscrepancyFunctor<TEMPLATE_ARGS(Color_, Color::Lab_128, 3)>)

// Anchor L1

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeAnchorHSVHueDiscrepancyFunctor,
   ColorDiscrepancy::AnchorHSV_HueDiscrepancyFunctor<Color_>,
   serializeAnchorUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_HueDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeAnchorHSVSaturationDiscrepancyFunctor,
   ColorDiscrepancy::AnchorHSV_SaturationDiscrepancyFunctor<Color_>,
   serializeAnchorUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_SaturationDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeAnchorHSVValueDiscrepancyFunctor,
   ColorDiscrepancy::AnchorHSV_ValueDiscrepancyFunctor<Color_>,
   serializeAnchorUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_ValueDiscrepancyFunctor<Color_>)>)

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeAnchorLABLuminanceDiscrepancyFunctor,
   ColorDiscrepancy::AnchorLAB_LuminanceDiscrepancyFunctor<Color_>,
   serializeAnchorUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_LuminanceDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeAnchorLABaDiscrepancyFunctor,
   ColorDiscrepancy::AnchorLAB_aDiscrepancyFunctor<Color_>,
   serializeAnchorUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_aDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeAnchorLABbDiscrepancyFunctor,
   ColorDiscrepancy::AnchorLAB_bDiscrepancyFunctor<Color_>,
   serializeAnchorUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_bDiscrepancyFunctor<Color_>)>)

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeAnchorDiscrepancyFunctor,
   ColorDiscrepancy::AnchorDiscrepancyFunctor<Color_>,
   serializeAnchorUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::DistanceDiscrepancyFunctor<Color_>)>)

// Anchor L2

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeAnchorHSVHueUDiscrepancyFunctor,
   ColorDiscrepancy::AnchorHSV_HueUDiscrepancyFunctor<Color_>,
   serializeAnchorUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_HueUDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeAnchorHSVSaturationUDiscrepancyFunctor,
   ColorDiscrepancy::AnchorHSV_SaturationUDiscrepancyFunctor<Color_>,
   serializeAnchorUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_SaturationUDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeAnchorHSVValueUDiscrepancyFunctor,
   ColorDiscrepancy::AnchorHSV_ValueUDiscrepancyFunctor<Color_>,
   serializeAnchorUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_ValueUDiscrepancyFunctor<Color_>)>)

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeAnchorLABLuminanceUDiscrepancyFunctor,
   ColorDiscrepancy::AnchorLAB_LuminanceUDiscrepancyFunctor<Color_>,
   serializeAnchorUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_LuminanceUDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeAnchorLABaUDiscrepancyFunctor,
   ColorDiscrepancy::AnchorLAB_aUDiscrepancyFunctor<Color_>,
   serializeAnchorUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_aUDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeAnchorLABbUDiscrepancyFunctor,
   ColorDiscrepancy::AnchorLAB_bUDiscrepancyFunctor<Color_>,
   serializeAnchorUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_bUDiscrepancyFunctor<Color_>)>)

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeAnchorCCTDiscrepancyFunctor,
   ColorDiscrepancy::AnchorCCTDiscrepancyFunctor<Color_>,
   serializeAnchorUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::CCTDiscrepancyFunctor<Color_>)>)

// Linear deviation L1

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLinearDeviationHSVHueDiscrepancyFunctor,
   ColorDiscrepancy::LinearDevHSV_HueDiscrepancyFunctor<Color_>,
   serializeLinearDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_HueDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLinearDeviationHSVSaturationDiscrepancyFunctor,
   ColorDiscrepancy::LinearDevHSV_SaturationDiscrepancyFunctor<Color_>,
   serializeLinearDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_SaturationDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLinearDeviationHSVValueDiscrepancyFunctor,
   ColorDiscrepancy::LinearDevHSV_ValueDiscrepancyFunctor<Color_>,
   serializeLinearDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_ValueDiscrepancyFunctor<Color_>)>)

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLinearDeviationLABLuminanceDiscrepancyFunctor,
   ColorDiscrepancy::LinearDevLAB_LuminanceDiscrepancyFunctor<Color_>,
   serializeLinearDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_LuminanceDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLinearDeviationLABaDiscrepancyFunctor,
   ColorDiscrepancy::LinearDevLAB_aDiscrepancyFunctor<Color_>,
   serializeLinearDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_aDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLinearDeviationLABbDiscrepancyFunctor,
   ColorDiscrepancy::LinearDevLAB_bDiscrepancyFunctor<Color_>,
   serializeLinearDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_bDiscrepancyFunctor<Color_>)>)

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLinearDeviationDiscrepancyFunctor,
   ColorDiscrepancy::LinearDeviationDiscrepancyFunctor<Color_>,
   serializeLinearDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::DistanceDiscrepancyFunctor<Color_>)>)

// Linear deviation L2

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLinearDeviationHSVHueUDiscrepancyFunctor,
   ColorDiscrepancy::LinearDevHSV_HueUDiscrepancyFunctor<Color_>,
   serializeLinearDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_HueUDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLinearDeviationHSVSaturationUDiscrepancyFunctor,
   ColorDiscrepancy::LinearDevHSV_SaturationUDiscrepancyFunctor<Color_>,
   serializeLinearDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_SaturationUDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLinearDeviationHSVValueUDiscrepancyFunctor,
   ColorDiscrepancy::LinearDevHSV_ValueUDiscrepancyFunctor<Color_>,
   serializeLinearDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_ValueUDiscrepancyFunctor<Color_>)>)

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLinearDeviationLABLuminanceUDiscrepancyFunctor,
   ColorDiscrepancy::LinearDevLAB_LuminanceUDiscrepancyFunctor<Color_>,
   serializeLinearDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_LuminanceUDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLinearDeviationLABaUDiscrepancyFunctor,
   ColorDiscrepancy::LinearDevLAB_aUDiscrepancyFunctor<Color_>,
   serializeLinearDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_aUDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLinearDeviationLABbUDiscrepancyFunctor,
   ColorDiscrepancy::LinearDevLAB_bUDiscrepancyFunctor<Color_>,
   serializeLinearDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_bUDiscrepancyFunctor<Color_>)>)

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeLinearDeviationCCTDiscrepancyFunctor,
   ColorDiscrepancy::LinearDeviationCCTDiscrepancyFunctor<Color_>,
   serializeLinearDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::CCTDiscrepancyFunctor<Color_>)>)

// Point set deviation L1

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializePointSetDeviationDiscrepancyFunctor,
   ColorDiscrepancy::PointSetDeviationDiscrepancyFunctor<Color_>,
   serializePointSetDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::DistanceDiscrepancyFunctor<Color_>)>)

// Point set deviation L2

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializePointSetDeviationHSVHueUDiscrepancyFunctor,
   ColorDiscrepancy::PointSetDevHSV_HueUDiscrepancyFunctor<Color_>,
   serializePointSetDeviationUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_HueUDiscrepancyFunctor<Color_>)>)

// Global anchor L1

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeGlobalAnchorHSVSaturationDiscrepancyFunctor,
   ColorDiscrepancy::GlobalAnchorHSV_SaturationDiscrepancyFunctor<Color_>,
   serializeGlobalDeviationDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_SaturationDiscrepancyFunctor<Color_>)>)
template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeGlobalAnchorHSVValueDiscrepancyFunctor,
   ColorDiscrepancy::GlobalAnchorHSV_ValueDiscrepancyFunctor<Color_>,
   serializeGlobalDeviationDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_ValueDiscrepancyFunctor<Color_>)>)

template<typename Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeGlobalAnchorLABLuminanceDiscrepancyFunctor,
   ColorDiscrepancy::GlobalAnchorLAB_LuminanceDiscrepancyFunctor<Color_>,
   serializeGlobalDeviationDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_LuminanceDiscrepancyFunctor<Color_>)>)

/******************************************************************************
 *
 * Functions to register color discrepancy serialization functions to serializers
 *
 ******************************************************************************/

//! \brief Used to register all color discrepancy serialization functions.
template<typename Color_>
static inline ESRet
colorDiscrepenciesTypedObjectSerializationFunctionsRegister
  (Serializer& s)
{
  ESRet ret = ESRet_OK;

  // Dependencies: generic functors instanciated for color types

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, Discrepancy::DelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_HueDiscrepancyFunctor<Color_>)>,
     serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_HueDiscrepancyFunctor<Color_>)>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, Discrepancy::DelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_SaturationDiscrepancyFunctor<Color_>)>,
     serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_SaturationDiscrepancyFunctor<Color_>)>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, Discrepancy::DelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_ValueDiscrepancyFunctor<Color_>)>,
     serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_ValueDiscrepancyFunctor<Color_>)>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, Discrepancy::DelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_LuminanceDiscrepancyFunctor<Color_>)>,
     serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_LuminanceDiscrepancyFunctor<Color_>)>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, Discrepancy::DelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_aDiscrepancyFunctor<Color_>)>,
     serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_aDiscrepancyFunctor<Color_>)>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, Discrepancy::DelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_bDiscrepancyFunctor<Color_>)>,
     serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_bDiscrepancyFunctor<Color_>)>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, Discrepancy::DelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::DistanceDiscrepancyFunctor<Color_>)>,
     serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::DistanceDiscrepancyFunctor<Color_>)>));

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, Discrepancy::DelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_HueUDiscrepancyFunctor<Color_>)>,
     serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_HueUDiscrepancyFunctor<Color_>)>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, Discrepancy::DelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_SaturationUDiscrepancyFunctor<Color_>)>,
     serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_SaturationUDiscrepancyFunctor<Color_>)>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, Discrepancy::DelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_ValueUDiscrepancyFunctor<Color_>)>,
     serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::HSV_ValueUDiscrepancyFunctor<Color_>)>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, Discrepancy::DelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_LuminanceUDiscrepancyFunctor<Color_>)>,
     serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_LuminanceUDiscrepancyFunctor<Color_>)>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, Discrepancy::DelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_aUDiscrepancyFunctor<Color_>)>,
     serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_aUDiscrepancyFunctor<Color_>)>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, Discrepancy::DelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_bUDiscrepancyFunctor<Color_>)>,
     serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::LAB_bUDiscrepancyFunctor<Color_>)>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, Discrepancy::DelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::CCTDiscrepancyFunctor<Color_>)>,
     serializeDelegatingUnaryDiscrepancyFunctor<TEMPLATE_ARGS(Color_, ColorDiscrepancy::CCTDiscrepancyFunctor<Color_>)>));

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::DistanceDiscrepancyFunctor<Color_>,
     serializeDistanceDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::CCTDiscrepancyFunctor<Color_>,
     serializeCCTDiscrepancyFunctor<Color_>));

  // Distance L1

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::HSV_HueDiscrepancyFunctor<Color_>,
     serializeHSVHueDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::HSV_SaturationDiscrepancyFunctor<Color_>,
     serializeHSVSaturationDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::HSV_ValueDiscrepancyFunctor<Color_>,
     serializeHSVValueDiscrepancyFunctor<Color_>));

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LAB_LuminanceDiscrepancyFunctor<Color_>,
     serializeLABLuminanceDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LAB_aDiscrepancyFunctor<Color_>,
     serializeLABaDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LAB_bDiscrepancyFunctor<Color_>,
     serializeLABbDiscrepancyFunctor<Color_>));

  // Distance L2

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::HSV_HueUDiscrepancyFunctor<Color_>,
     serializeHSVHueUDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::HSV_SaturationUDiscrepancyFunctor<Color_>,
     serializeHSVSaturationUDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::HSV_ValueUDiscrepancyFunctor<Color_>,
     serializeHSVValueUDiscrepancyFunctor<Color_>));

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LAB_LuminanceUDiscrepancyFunctor<Color_>,
     serializeLABLuminanceUDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LAB_aUDiscrepancyFunctor<Color_>,
     serializeLABaUDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LAB_bUDiscrepancyFunctor<Color_>,
     serializeLABbUDiscrepancyFunctor<Color_>));

  // Anchor L1

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::AnchorHSV_HueDiscrepancyFunctor<Color_>,
     serializeAnchorHSVHueDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::AnchorHSV_SaturationDiscrepancyFunctor<Color_>,
     serializeAnchorHSVSaturationDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::AnchorHSV_ValueDiscrepancyFunctor<Color_>,
     serializeAnchorHSVValueDiscrepancyFunctor<Color_>));

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::AnchorLAB_LuminanceDiscrepancyFunctor<Color_>,
     serializeAnchorLABLuminanceDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::AnchorLAB_aDiscrepancyFunctor<Color_>,
     serializeAnchorLABaDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::AnchorLAB_bDiscrepancyFunctor<Color_>,
     serializeAnchorLABbDiscrepancyFunctor<Color_>));

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::AnchorDiscrepancyFunctor<Color_>,
     serializeAnchorDiscrepancyFunctor<Color_>));

  // Anchor L2

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::AnchorHSV_HueUDiscrepancyFunctor<Color_>,
     serializeAnchorHSVHueUDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::AnchorHSV_SaturationUDiscrepancyFunctor<Color_>,
     serializeAnchorHSVSaturationUDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::AnchorHSV_ValueUDiscrepancyFunctor<Color_>,
     serializeAnchorHSVValueUDiscrepancyFunctor<Color_>));

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::AnchorLAB_LuminanceUDiscrepancyFunctor<Color_>,
     serializeAnchorLABLuminanceUDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::AnchorLAB_aUDiscrepancyFunctor<Color_>,
     serializeAnchorLABaUDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::AnchorLAB_bUDiscrepancyFunctor<Color_>,
     serializeAnchorLABbUDiscrepancyFunctor<Color_>));

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::AnchorCCTDiscrepancyFunctor<Color_>,
     serializeAnchorCCTDiscrepancyFunctor<Color_>));

  // Linear deviation L1

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LinearDevHSV_HueDiscrepancyFunctor<Color_>,
     serializeLinearDeviationHSVHueDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LinearDevHSV_SaturationDiscrepancyFunctor<Color_>,
     serializeLinearDeviationHSVSaturationDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LinearDevHSV_ValueDiscrepancyFunctor<Color_>,
     serializeLinearDeviationHSVValueDiscrepancyFunctor<Color_>));

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LinearDevLAB_LuminanceDiscrepancyFunctor<Color_>,
     serializeLinearDeviationLABLuminanceDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LinearDevLAB_aDiscrepancyFunctor<Color_>,
     serializeLinearDeviationLABaDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LinearDevLAB_bDiscrepancyFunctor<Color_>,
     serializeLinearDeviationLABbDiscrepancyFunctor<Color_>));

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LinearDeviationDiscrepancyFunctor<Color_>,
     serializeLinearDeviationDiscrepancyFunctor<Color_>));

  // Linear deviation L2

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LinearDevHSV_HueUDiscrepancyFunctor<Color_>,
     serializeLinearDeviationHSVHueUDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LinearDevHSV_SaturationUDiscrepancyFunctor<Color_>,
     serializeLinearDeviationHSVSaturationUDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LinearDevHSV_ValueUDiscrepancyFunctor<Color_>,
     serializeLinearDeviationHSVValueUDiscrepancyFunctor<Color_>));

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LinearDevLAB_LuminanceUDiscrepancyFunctor<Color_>,
     serializeLinearDeviationLABLuminanceUDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LinearDevLAB_aUDiscrepancyFunctor<Color_>,
     serializeLinearDeviationLABaUDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LinearDevLAB_bUDiscrepancyFunctor<Color_>,
     serializeLinearDeviationLABbUDiscrepancyFunctor<Color_>));

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::LinearDeviationCCTDiscrepancyFunctor<Color_>,
     serializeLinearDeviationCCTDiscrepancyFunctor<Color_>));

  // Point set deviation L1

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::PointSetDeviationDiscrepancyFunctor<Color_>,
     serializePointSetDeviationDiscrepancyFunctor<Color_>));

  // Point set deviation L2

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::PointSetDevHSV_HueUDiscrepancyFunctor<Color_>,
     serializePointSetDeviationHSVHueUDiscrepancyFunctor<Color_>));

  // Global anchor L1

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::GlobalAnchorHSV_SaturationDiscrepancyFunctor<Color_>,
     serializeGlobalAnchorHSVSaturationDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::GlobalAnchorHSV_ValueDiscrepancyFunctor<Color_>,
     serializeGlobalAnchorHSVValueDiscrepancyFunctor<Color_>));

  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, ColorDiscrepancy::GlobalAnchorLAB_LuminanceDiscrepancyFunctor<Color_>,
     serializeGlobalAnchorLABLuminanceDiscrepancyFunctor<Color_>));

exit:
  return ret;
}

//! \brief Used to add all color discrepancy types to the given factory.
template<typename Color_>
static inline void
colorDiscrepenciesTypesAddToFactory
  (Factory& f)
{
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::DistanceDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::CCTDiscrepancyFunctor<Color_>);

  // Distance L1
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::HSV_HueDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::HSV_SaturationDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::HSV_ValueDiscrepancyFunctor<Color_>);

  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LAB_LuminanceDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LAB_aDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LAB_bDiscrepancyFunctor<Color_>);

  // Distance L2
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::HSV_HueUDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::HSV_SaturationUDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::HSV_ValueUDiscrepancyFunctor<Color_>);

  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LAB_LuminanceUDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LAB_aUDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LAB_bUDiscrepancyFunctor<Color_>);

  // Anchor L1
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::AnchorHSV_HueDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::AnchorHSV_SaturationDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::AnchorHSV_ValueDiscrepancyFunctor<Color_>);

  FACTORY_ADD_TYPE(f, ColorDiscrepancy::AnchorLAB_LuminanceDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::AnchorLAB_aDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::AnchorLAB_bDiscrepancyFunctor<Color_>);

  FACTORY_ADD_TYPE(f, ColorDiscrepancy::AnchorDiscrepancyFunctor<Color_>);

  // Anchor L2
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::AnchorHSV_HueUDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::AnchorHSV_SaturationUDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::AnchorHSV_ValueUDiscrepancyFunctor<Color_>);

  FACTORY_ADD_TYPE(f, ColorDiscrepancy::AnchorLAB_LuminanceUDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::AnchorLAB_aUDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::AnchorLAB_bUDiscrepancyFunctor<Color_>);

  FACTORY_ADD_TYPE(f, ColorDiscrepancy::AnchorCCTDiscrepancyFunctor<Color_>);

  // Linear deviation L1
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LinearDevHSV_HueDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LinearDevHSV_SaturationDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LinearDevHSV_ValueDiscrepancyFunctor<Color_>);

  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LinearDevLAB_LuminanceDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LinearDevLAB_aDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LinearDevLAB_bDiscrepancyFunctor<Color_>);

  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LinearDeviationDiscrepancyFunctor<Color_>);

  // Linear deviation L2
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LinearDevHSV_HueUDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LinearDevHSV_SaturationUDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LinearDevHSV_ValueUDiscrepancyFunctor<Color_>);

  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LinearDevLAB_LuminanceUDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LinearDevLAB_aUDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LinearDevLAB_bUDiscrepancyFunctor<Color_>);

  FACTORY_ADD_TYPE(f, ColorDiscrepancy::LinearDeviationCCTDiscrepancyFunctor<Color_>);

  // Point set deviation L1
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::PointSetDeviationDiscrepancyFunctor<Color_>);

  // Point set deviation L2
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::PointSetDevHSV_HueUDiscrepancyFunctor<Color_>);

  // Global anchor L1
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::GlobalAnchorHSV_SaturationDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, ColorDiscrepancy::GlobalAnchorHSV_ValueDiscrepancyFunctor<Color_>);

  FACTORY_ADD_TYPE(f, ColorDiscrepancy::GlobalAnchorLAB_LuminanceDiscrepancyFunctor<Color_>);
}

#endif /* DISCREPANCY_SERIALIZATION_COLOR_HPP */
