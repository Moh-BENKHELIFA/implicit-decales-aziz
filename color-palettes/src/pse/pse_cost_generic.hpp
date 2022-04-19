#ifndef DISCREPANCY_GENERIC_HPP
#define DISCREPANCY_GENERIC_HPP

#include <pse/pse_cost.hpp>

// Implementation
#include <pse/pse_cost_generic.inl>

////////////////////////////////////////////////////////////////////////////////
/// Generic Nary Functors
///

namespace Discrepancy {

template<class ParametricPoint>
struct ConstantBinaryDiscrepancyFunctor;

template<class _ParametricPoint, class _BinaryFunctor>
struct AnchorUnaryDiscrepancyFunctor;

template<class _ParametricPoint, class _BinaryFunctor>
struct LinearDeviationUnaryDiscrepancyFunctor;

template<class _ParametricPoint, class _BinaryFunctor>
struct PairWiseGlobalAnchorDiscrepancyFunctor;

} // namespace Discrepancy

#endif // DISCREPANCY_GENERIC_HPP
