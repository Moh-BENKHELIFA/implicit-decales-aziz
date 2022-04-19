#ifndef GRAPH_UTILS_INL
#define GRAPH_UTILS_INL

#include "pse_graph_utils.hpp"

#include <algorithm>
#include <numeric>

static inline ERet
gphCreateVertices
  (GraphVertices& final_vertices,
   const size_t wanted_count,
   GraphVertexIdList& added_vertices)
{
  const size_t start_idx = final_vertices.size();
  const size_t end_idx = start_idx + wanted_count;
  const size_t av_start_idx = added_vertices.size();
  final_vertices.resize(end_idx);
  added_vertices.resize(av_start_idx + wanted_count);
  std::iota(added_vertices.begin() + av_start_idx, added_vertices.end(), start_idx);
  return ERet_OK;
}

static inline ERet
gphCreateEdges
  (GraphEdges& final_edges,
   const std::vector<GraphBinaryEdgeVerticesId>& wanted_edges,
   GraphEdgeIdList& added_edges)
{
  ERet ret = ERet_OK;
  const size_t fe_initial_size = final_edges.size();
  const size_t ae_initial_size = added_edges.size();

  // We have to check the existence of given edges
  for(const GraphBinaryEdgeVerticesId& e: wanted_edges) {
    // This search could be expensive on big graphs. We could have/use another
    // structure to accelerate this search.
    auto it = std::find(final_edges.begin(), final_edges.end(), e);
    CHECK_OR_DO(it == final_edges.end(), ret = ERet_AlreadyExists; goto error);
    // A new edge
    added_edges.push_back(final_edges.size());
    final_edges.push_back(e);
  }

exit:
  return ret;
error:
  // Restore buffers to the initial states
  final_edges.resize(fe_initial_size);
  added_edges.resize(ae_initial_size);
  goto exit;
}

static inline ERet
gphCreateOrGetEdges
  (GraphEdges& final_edges,
   const std::vector<GraphBinaryEdgeVerticesId>& wanted_edges,
   std::vector<std::pair<GraphEdgeId, bool>>& added_edges)
{
  // We have to check the existence of given edges
  for(const GraphBinaryEdgeVerticesId& e: wanted_edges) {
    // This search could be expensive on big graphs. We could have/use another
    // structure to accelerate this search.
    auto it = std::find(final_edges.begin(), final_edges.end(), e);
    if( it == final_edges.end() ) {
      // A new edge
      added_edges.push_back({final_edges.size(), false});
      final_edges.push_back(e);
    } else {
      // An existing edge. This code will not compile if we do not work with
      // vectors. That's exactly what we want to do.
      added_edges.push_back({it - final_edges.begin(), true});
    }
  }
  return ERet_OK;
}

#endif // GRAPH_UTILS_INL
