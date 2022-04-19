#ifndef CONSTRAINED_PARAMETER_SPACE_TYPES_HPP
#define CONSTRAINED_PARAMETER_SPACE_TYPES_HPP

#include "pse_graph_types.hpp"

/******************************************************************************
 *
 * Types
 *
 ******************************************************************************/

typedef size_t LayerId;
typedef size_t ConstraintBuddyId;
typedef GraphVertexId ParametricPointId;
typedef GraphEdgeId PairingId;
typedef GraphEdgeId ConstraintId;
typedef GraphEdgeId GlobalConstraintId;

typedef GraphVertexIdPair ParametricPointIdPair;

typedef GraphVertexIdList ParametricPointIdList;
typedef GraphEdgeIdList PairingIdList;
typedef GraphEdgeIdList ConstraintIdList;
typedef GraphEdgeIdList GlobalConstraintIdList;

static constexpr LayerId LayerId_INVALID = LayerId(-1);
static constexpr ConstraintBuddyId ConstraintBuddyId_INVALID = ConstraintBuddyId(-1);
static constexpr ParametricPointIdPair ParametricPointIdPair_INVALID = GraphVertexIdPair_INVALID;
static constexpr ParametricPointId ParametricPointId_INVALID = GraphVertexId_INVALID;
static constexpr PairingId PairingId_INVALID = GraphEdgeId_INVALID;
static constexpr ConstraintId ConstraintId_INVALID = GraphEdgeId_INVALID;
static constexpr GlobalConstraintId GlobalConstraintId_INVALID = GraphEdgeId_INVALID;

template<typename ParametricPoint, typename Functor>
struct ParametricPointParams {
  ParametricPoint _value;
  typename ParametricPoint::Scalar _importance;
  size_t _own_constraints_functors_count;
  Functor** _own_constraints_functors;
};

typedef GraphBinaryEdgeVerticesId PairingParams;

template<typename Functor>
struct ConstraintParams {
  ParametricPointIdPair _ppoints;
  size_t _functors_count;
  Functor** _functors;
};

template<typename Functor>
struct GlobalConstraintParams {
  Functor* _functor;
};

/******************************************************************************
 *
 * Constants
 *
 ******************************************************************************/

template<typename ParametricPoint, typename Functor>
static const ParametricPointParams<ParametricPoint, Functor> PARAM_POINT_PARAMS_NULL =
  { ParametricPoint{}, typename ParametricPoint::Scalar(1), 0, nullptr };

static const PairingParams PAIRING_PARAMS_NULL =
  { ParametricPointId_INVALID, ParametricPointId_INVALID };

template<typename Functor>
static const ConstraintParams<Functor> CONSTRAINT_PARAMS_NULL =
  { ParametricPointIdPair_INVALID, 0, nullptr };

template<typename Functor>
static const GlobalConstraintParams<Functor> GLOBAL_CONSTRAINT_PARAMS_NULL =
  { nullptr };

#endif // CONSTRAINED_PARAMETER_SPACE_TYPES_HPP
