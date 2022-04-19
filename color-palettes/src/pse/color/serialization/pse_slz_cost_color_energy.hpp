#ifndef DISCREPANCY_SERIALIZATION_COLOR_ENERGY_HPP
#define DISCREPANCY_SERIALIZATION_COLOR_ENERGY_HPP

#include <pse/color/pse_cost_color_energy.hpp>

#include <pse/serialization/pse_serializer.hpp>
#include <pse/serialization/pse_slz_cost_generic.hpp>

/******************************************************************************
 *
 * Color energy discrepancy serialization functions
 *
 ******************************************************************************/

template<class Color_>
SERIALIZER_TYPED_OBJECT_SAME_AS
  (serializeRGBMaxDiscrepancyFunctor,
   EnergyDiscrepancy::RGBMaxDiscrepancyFunctor<Color_>,
   serializeUnaryDiscrepancyFunctor<Color_>)

template<class Color_>
SERIALIZER_TYPED_OBJECT_BEGIN
  (serializeRGBMaxGlobalAnchorDiscrepancyFunctor,
   EnergyDiscrepancy::RGBMaxGlobalAnchorDiscrepancyFunctor<Color_>, d)
  using Base =
    EnergyDiscrepancy::RGBMaxGlobalAnchorDiscrepancyFunctor<Color_>;
  using Scalar = typename Base::Scalar;
  using DelegatedFunctor = typename Base::DelegatedFunctor;
  SERIALIZE_TYPED_OBJECT_CUSTOM_INLINE
    (serializeWeightedDynamicNaryDiscrepancyFunctor<Color_>, &d);
  Scalar ref = d.getRef();
  DelegatedFunctor f = d.functor();
  SERIALIZE_VALUE("Ref", ref);
  SERIALIZE_TYPED_OBJECT("DelegatedFunctor", f);
  d.setRef(ref);
  d.setFunctor(f);
SERIALIZER_TYPED_OBJECT_END()

template<typename Color_>
static inline ESRet
colorEnergyDiscrepenciesTypedObjectSerializationFunctionsRegister
  (Serializer& s)
{
  ESRet ret = ESRet_OK;
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, EnergyDiscrepancy::RGBMaxDiscrepancyFunctor<Color_>,
     serializeRGBMaxDiscrepancyFunctor<Color_>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, EnergyDiscrepancy::RGBMaxGlobalAnchorDiscrepancyFunctor<Color_>,
     serializeRGBMaxGlobalAnchorDiscrepancyFunctor<Color_>));

exit:
  return ret;
}

template<typename Color_>
static inline void
colorEnergyDiscrepenciesTypesAddToFactory
  (Factory& f)
{
  FACTORY_ADD_TYPE(f, EnergyDiscrepancy::RGBMaxDiscrepancyFunctor<Color_>);
  FACTORY_ADD_TYPE(f, EnergyDiscrepancy::RGBMaxGlobalAnchorDiscrepancyFunctor<Color_>);
}

#endif // DISCREPANCY_SERIALIZATION_COLOR_ENERGY_HPP
