#ifndef DISCREPANCY_COLOR_HPP
#define DISCREPANCY_COLOR_HPP

#include <pse/color/pse_color.hpp>

#include <pse/pse_cost.hpp>
#include <pse/pse_cost_generic.hpp>

// Implementation
#include <pse/color/pse_cost_color.inl>

namespace ColorDiscrepancy{

template<class ColorT>
struct DistanceDiscrepancyFunctor;

template<class ColorT>
struct RelativeLuminanceDiscrepancyFunctor;

template<class _Color>
struct CCTDiscrepancyFunctor;

template<class _Color, class _BinaryFunctor>
struct PointSetDeviationUnaryDiscrepancyFunctor;

template<class _ParametricPoint, class _BinaryFunctor>
struct GlobalDeviationDiscrepancyFunctor;

template<class _ParametricPoint>
struct CCTGlobalAnchorDiscrepancyFunctor;

//! Compute L1 distance in Hue.
template<class _Color> using HSV_HueDiscrepancyFunctor =
internal::CWXXXDiscrepancyFunctor<_Color, Color::HSV_360, 0> ;

//! Compute L1 distance in Saturation.
template<class _Color> using HSV_SaturationDiscrepancyFunctor =
internal::CWXXXDiscrepancyFunctor<_Color, Color::HSV_360, 1> ;

//! Compute L1 distance in Value.
template<class _Color> using HSV_ValueDiscrepancyFunctor =
internal::CWXXXDiscrepancyFunctor<_Color, Color::HSV_360, 2> ;

//! Compute L1 distance in Luminance.
template<class _Color> using LAB_LuminanceDiscrepancyFunctor =
internal::CWXXXDiscrepancyFunctor<_Color, Color::Lab_128, 0> ;

//! Compute L1 distance in a* channel from Lab.
template<class _Color> using LAB_aDiscrepancyFunctor =
internal::CWXXXDiscrepancyFunctor<_Color, Color::Lab_128, 1> ;

//! Compute L1 distance in b* channel from Lab.
template<class _Color> using LAB_bDiscrepancyFunctor =
internal::CWXXXDiscrepancyFunctor<_Color, Color::Lab_128, 2> ;


//! Compute L2 distance in Hue.
template<class _Color> using HSV_HueUDiscrepancyFunctor =
internal::CWUnsignedXXXDiscrepancyFunctor<_Color, Color::HSV_360, 0> ;

//! Compute L2 distance in Saturation.
template<class _Color> using HSV_SaturationUDiscrepancyFunctor =
internal::CWUnsignedXXXDiscrepancyFunctor<_Color, Color::HSV_360, 1> ;

//! Compute L2 distance in Value.
template<class _Color> using HSV_ValueUDiscrepancyFunctor =
internal::CWUnsignedXXXDiscrepancyFunctor<_Color, Color::HSV_360, 2> ;

//! Compute L2 distance in Luminance.
template<class _Color> using LAB_LuminanceUDiscrepancyFunctor =
internal::CWUnsignedXXXDiscrepancyFunctor<_Color, Color::Lab_128, 0> ;

//! Compute L2 distance in a* channel from Lab.
template<class _Color> using LAB_aUDiscrepancyFunctor =
internal::CWUnsignedXXXDiscrepancyFunctor<_Color, Color::Lab_128, 1> ;

//! Compute L2 distance in b* channel from Lab.
template<class _Color> using LAB_bUDiscrepancyFunctor =
internal::CWUnsignedXXXDiscrepancyFunctor<_Color, Color::Lab_128, 2> ;

////////////////////////////////////////////////////////////////////////////////
/// Anchor

template<class _Color> using AnchorHSV_HueDiscrepancyFunctor =
Discrepancy::AnchorUnaryDiscrepancyFunctor<_Color, HSV_HueDiscrepancyFunctor<_Color>>;

template<class _Color> using AnchorHSV_SaturationDiscrepancyFunctor =
Discrepancy::AnchorUnaryDiscrepancyFunctor<_Color, HSV_SaturationDiscrepancyFunctor<_Color>>;

template<class _Color> using AnchorHSV_ValueDiscrepancyFunctor =
Discrepancy::AnchorUnaryDiscrepancyFunctor<_Color, HSV_ValueDiscrepancyFunctor<_Color>>;

template<class _Color> using AnchorLAB_LuminanceDiscrepancyFunctor =
Discrepancy::AnchorUnaryDiscrepancyFunctor<_Color, LAB_LuminanceDiscrepancyFunctor<_Color>>;

template<class _Color> using AnchorLAB_aDiscrepancyFunctor =
Discrepancy::AnchorUnaryDiscrepancyFunctor<_Color, LAB_aDiscrepancyFunctor<_Color>>;

template<class _Color> using AnchorLAB_bDiscrepancyFunctor =
Discrepancy::AnchorUnaryDiscrepancyFunctor<_Color, LAB_bDiscrepancyFunctor<_Color>>;

template<class _Color> using AnchorDiscrepancyFunctor =
Discrepancy::AnchorUnaryDiscrepancyFunctor<_Color, DistanceDiscrepancyFunctor<_Color>>;

template<class _Color> using AnchorHSV_HueUDiscrepancyFunctor =
Discrepancy::AnchorUnaryDiscrepancyFunctor<_Color, HSV_HueUDiscrepancyFunctor<_Color>>;

template<class _Color> using AnchorHSV_SaturationUDiscrepancyFunctor =
Discrepancy::AnchorUnaryDiscrepancyFunctor<_Color, HSV_SaturationUDiscrepancyFunctor<_Color>>;

template<class _Color> using AnchorHSV_ValueUDiscrepancyFunctor =
Discrepancy::AnchorUnaryDiscrepancyFunctor<_Color, HSV_ValueUDiscrepancyFunctor<_Color>>;

template<class _Color> using AnchorLAB_LuminanceUDiscrepancyFunctor =
Discrepancy::AnchorUnaryDiscrepancyFunctor<_Color, LAB_LuminanceUDiscrepancyFunctor<_Color>>;

template<class _Color> using AnchorLAB_aUDiscrepancyFunctor =
Discrepancy::AnchorUnaryDiscrepancyFunctor<_Color, LAB_aUDiscrepancyFunctor<_Color>>;

template<class _Color> using AnchorLAB_bUDiscrepancyFunctor =
Discrepancy::AnchorUnaryDiscrepancyFunctor<_Color, LAB_bUDiscrepancyFunctor<_Color>>;

template<class _Color> using AnchorCCTDiscrepancyFunctor =
Discrepancy::AnchorUnaryDiscrepancyFunctor<_Color, CCTDiscrepancyFunctor<_Color>>;


////////////////////////////////////////////////////////////////////////////////
/// Linear deviation

template<class _Color> using LinearDevHSV_HueDiscrepancyFunctor =
Discrepancy::LinearDeviationUnaryDiscrepancyFunctor<_Color, HSV_HueDiscrepancyFunctor<_Color>>;

template<class _Color> using LinearDevHSV_SaturationDiscrepancyFunctor =
Discrepancy::LinearDeviationUnaryDiscrepancyFunctor<_Color, HSV_SaturationDiscrepancyFunctor<_Color>>;

template<class _Color> using LinearDevHSV_ValueDiscrepancyFunctor =
Discrepancy::LinearDeviationUnaryDiscrepancyFunctor<_Color, HSV_ValueDiscrepancyFunctor<_Color>>;

template<class _Color> using LinearDevLAB_LuminanceDiscrepancyFunctor =
Discrepancy::LinearDeviationUnaryDiscrepancyFunctor<_Color, LAB_LuminanceDiscrepancyFunctor<_Color>>;

template<class _Color> using LinearDevLAB_aDiscrepancyFunctor =
Discrepancy::LinearDeviationUnaryDiscrepancyFunctor<_Color, LAB_aDiscrepancyFunctor<_Color>>;

template<class _Color> using LinearDevLAB_bDiscrepancyFunctor =
Discrepancy::LinearDeviationUnaryDiscrepancyFunctor<_Color, LAB_bDiscrepancyFunctor<_Color>>;

template<class _Color> using LinearDeviationDiscrepancyFunctor =
Discrepancy::LinearDeviationUnaryDiscrepancyFunctor<_Color, DistanceDiscrepancyFunctor<_Color>>;

template<class _Color> using LinearDevHSV_HueUDiscrepancyFunctor =
Discrepancy::LinearDeviationUnaryDiscrepancyFunctor<_Color, HSV_HueUDiscrepancyFunctor<_Color>>;

template<class _Color> using LinearDevHSV_SaturationUDiscrepancyFunctor =
Discrepancy::LinearDeviationUnaryDiscrepancyFunctor<_Color, HSV_SaturationUDiscrepancyFunctor<_Color>>;

template<class _Color> using LinearDevHSV_ValueUDiscrepancyFunctor =
Discrepancy::LinearDeviationUnaryDiscrepancyFunctor<_Color, HSV_ValueUDiscrepancyFunctor<_Color>>;

template<class _Color> using LinearDevLAB_LuminanceUDiscrepancyFunctor =
Discrepancy::LinearDeviationUnaryDiscrepancyFunctor<_Color, LAB_LuminanceUDiscrepancyFunctor<_Color>>;

template<class _Color> using LinearDevLAB_aUDiscrepancyFunctor =
Discrepancy::LinearDeviationUnaryDiscrepancyFunctor<_Color, LAB_aUDiscrepancyFunctor<_Color>>;

template<class _Color> using LinearDevLAB_bUDiscrepancyFunctor =
Discrepancy::LinearDeviationUnaryDiscrepancyFunctor<_Color, LAB_bUDiscrepancyFunctor<_Color>>;

template<class _Color> using LinearDeviationCCTDiscrepancyFunctor =
Discrepancy::LinearDeviationUnaryDiscrepancyFunctor<_Color, CCTDiscrepancyFunctor<_Color>>;


template <class Color> using PointSetDeviationDiscrepancyFunctor =
PointSetDeviationUnaryDiscrepancyFunctor<Color, DistanceDiscrepancyFunctor<Color>>;

template <class Color> using PointSetDevHSV_HueUDiscrepancyFunctor =
PointSetDeviationUnaryDiscrepancyFunctor<Color, HSV_HueUDiscrepancyFunctor<Color>>;

////////////////////////////////////////////////////////////////////////////////
/// Global
///
template<class _Color> using GlobalAnchorHSV_SaturationDiscrepancyFunctor =
ColorDiscrepancy::GlobalDeviationDiscrepancyFunctor<_Color, HSV_SaturationDiscrepancyFunctor<_Color>> ;

template<class _Color> using GlobalAnchorHSV_ValueDiscrepancyFunctor =
ColorDiscrepancy::GlobalDeviationDiscrepancyFunctor<_Color, HSV_ValueDiscrepancyFunctor<_Color>> ;

//! Compute L1 distance in Luminance.
template<class _Color> using GlobalAnchorLAB_LuminanceDiscrepancyFunctor =
ColorDiscrepancy::GlobalDeviationDiscrepancyFunctor<_Color, LAB_LuminanceDiscrepancyFunctor<_Color>> ;

} //namespace ColorDiscrepancy

#endif // DISCREPANCY_COLOR_HPP
