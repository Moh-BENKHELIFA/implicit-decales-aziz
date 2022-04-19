#ifndef DISCREPANCY_COLOR_ENERGY_HPP
#define DISCREPANCY_COLOR_ENERGY_HPP

#include <pse/pse_cost.hpp>

// Implementation
#include <pse/color/pse_cost_color_energy.inl>

namespace EnergyDiscrepancy{

/*!
 * \brief cost used in
 * Energy Aware Color sets, J. Chuang, D. Weiskopf, T. Möller, Eurographics 2009
 * Equation (3)
 */
template<class _Color>
struct RGBMaxDiscrepancyFunctor;

/*!
 * \brief
 */
template<class _Color>
struct RGBMaxGlobalAnchorDiscrepancyFunctor;

} // namespace EneryDiscrepancy

#endif // DISCREPANCY_COLOR_ENERGY_HPP
