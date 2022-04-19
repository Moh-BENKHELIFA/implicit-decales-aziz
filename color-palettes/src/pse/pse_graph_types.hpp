#ifndef GRAPH2_TYPES_HPP
#define GRAPH2_TYPES_HPP

#include <vector>

typedef size_t GraphVertexId;
typedef size_t GraphEdgeId;
typedef std::vector<GraphVertexId> GraphVertexIdList;
typedef std::vector<GraphEdgeId> GraphEdgeIdList;
typedef std::pair<GraphVertexId, GraphVertexId> GraphVertexIdPair;
typedef GraphVertexIdPair GraphBinaryEdgeVerticesId;

typedef std::vector<GraphEdgeIdList> GraphVertices;
typedef std::vector<GraphBinaryEdgeVerticesId> GraphEdges;

static constexpr GraphVertexId GraphVertexId_INVALID = GraphVertexId(-1);
static constexpr GraphEdgeId GraphEdgeId_INVALID = GraphEdgeId(-1);
static constexpr GraphVertexIdPair GraphVertexIdPair_INVALID =
  { GraphVertexId_INVALID, GraphVertexId_INVALID };

#endif // GRAPH2_TYPES_HPP
