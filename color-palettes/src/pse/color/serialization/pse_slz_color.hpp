#ifndef SERIALIZATION_COLOR_HPP
#define SERIALIZATION_COLOR_HPP

#include <pse/color/pse_color.hpp>

#include <pse/serialization/pse_serializer.hpp>

/******************************************************************************
 *
 * Color serialization functions
 *
 ******************************************************************************/

template<typename Scalar>
SERIALIZER_TYPED_OBJECT_BEGIN(serializeColorRGB, RGBColor<Scalar>, v)
  typename RGBColor<Scalar>::CVector rgb = v.getRGB();
  SERIALIZE_VALUE("R", rgb[0]);
  SERIALIZE_VALUE("G", rgb[1]);
  SERIALIZE_VALUE("B", rgb[2]);
  v.setRGB(rgb);
SERIALIZER_TYPED_OBJECT_END()

template<typename Scalar>
SERIALIZER_TYPED_OBJECT_BEGIN(serializeColorHSV, HSVColor<Scalar>, v)
  typename HSVColor<Scalar>::CVector hsv = v.getHSV();
  SERIALIZE_VALUE("H", hsv[0]);
  SERIALIZE_VALUE("S", hsv[1]);
  SERIALIZE_VALUE("V", hsv[2]);
  v.setHSV(hsv);
SERIALIZER_TYPED_OBJECT_END()

template<typename Scalar>
SERIALIZER_TYPED_OBJECT_BEGIN(serializeColorLAB, LABColor<Scalar>, v)
  typename LABColor<Scalar>::CVector lab = v.getLAB();
  SERIALIZE_VALUE("L", lab[0]);
  SERIALIZE_VALUE("a", lab[1]);
  SERIALIZE_VALUE("b", lab[2]);
  v.setLAB(lab);
SERIALIZER_TYPED_OBJECT_END()

template<typename Scalar>
SERIALIZER_TYPED_OBJECT_BEGIN(serializeColorLUV, LUVColor<Scalar>, v)
  typename LUVColor<Scalar>::CVector luv = v.getLUV();
  SERIALIZE_VALUE("L", luv[0]);
  SERIALIZE_VALUE("u", luv[1]);
  SERIALIZE_VALUE("v", luv[2]);
  v.setLUV(luv);
SERIALIZER_TYPED_OBJECT_END()

template<typename Scalar>
static inline ESRet
colorTypedObjectSerializationFunctionsRegister
  (Serializer& s)
{
  ESRet ret = ESRet_OK;
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, RGBColor<Scalar>, serializeColorRGB<Scalar>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, HSVColor<Scalar>, serializeColorHSV<Scalar>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, LABColor<Scalar>, serializeColorLAB<Scalar>));
  SCALL_OR_GOTO(ret,exit, SERIALIZER_TYPED_OBJECT_SERIALIZE_FUNC_REGISTER
    (s, LUVColor<Scalar>, serializeColorLUV<Scalar>));

exit:
  return ret;
}

#endif // SERIALIZATION_COLOR_HPP
