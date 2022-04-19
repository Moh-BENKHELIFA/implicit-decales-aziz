#ifndef GRAPH_UTILS_HPP
#define GRAPH_UTILS_HPP

#include "pse_types.hpp"
#include "pse_graph_types.hpp"

#include <vector>

//! \brief Create \c wanted_count new vertices in \c final_vertices and return
//! their ids in \c added_vertices.
static inline ERet
gphCreateVertices
  (GraphVertices& final_vertices,
   const size_t wanted_count,
   GraphVertexIdList& added_vertices);

//! \brief Create new given \c wanted_edges in \c final_edges and return their
//! ids in \c added_edges. This function fails with ERet_AlreadyExists if at
//! least one edge already exist.
static inline ERet
gphCreateEdges
  (GraphEdges& final_edges,
   const std::vector<GraphBinaryEdgeVerticesId>& wanted_edges,
   GraphEdgeIdList& added_edges); //!< same order than \c edges

//! \brief Create new given \c wanted_edges in \c final_edges and return their
//! ids in \c added_edges. Already existing edges are still returned but not
//! created twice. The boolean in the returned std::pair says true if it was
//! already existing.
static inline ERet
gphCreateOrGetEdges
  (GraphEdges& final_edges,
   const std::vector<GraphBinaryEdgeVerticesId>& wanted_edges,
   std::vector<std::pair<GraphEdgeId, bool>>& added_edges); //!< same order than \c edges

#include "pse_graph_utils.inl"

#endif // GRAPH_UTILS_HPP
