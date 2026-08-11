#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <cstddef>
#include "line.hpp"

// ---------------------------------------------------------------------------
// Lattice: the toric code itself. Pure data + topology, ZERO OpenGL.
//
// This is the single source of truth that every renderer reads from. The flat
// window and the torus window do not each keep their own copy of "which edges
// have errors" -- they both ask this object, which is why they can never
// disagree.
//
// TOPOLOGY -- the important design decision:
// A toric code lives on a torus, which means the lattice is PERIODIC: column
// N wraps back around to column 0, and so does row N. So the logical model
// here is an N x N periodic grid with exactly 2*N*N edges, N*N plaquettes and
// N*N vertices -- and, crucially, EVERY vertex and plaquette has exactly 4
// edges. (With open boundaries the border checks would only have 2 or 3 edges,
// which is a different code with different physics.)
//
// The flat picture, though, still wants to draw a closed square border, so it
// draws (N+1)*N + N*(N+1) = 180 edges for N=9 while there are only 162 logical
// ones. The extra 18 are the seam: the top border row is the SAME edge as the
// bottom border row, and likewise left/right. So "drawn edges" and "logical
// edges" are deliberately separate concepts here, with drawnToLogical mapping
// between them. Toggling a seam edge lights up both of its drawn copies at
// once, which is exactly the right visual: it shows you the identification
// that makes the square a torus.
// ---------------------------------------------------------------------------
class Lattice {
public:
    Lattice(size_t dimension, float cellScale, glm::vec2 origin);

    size_t    dimension() const { return dim; }
    float     cellSize()  const { return scale; }
    glm::vec2 origin()    const { return gridOrigin; }

    // --- logical edges: each physical qubit exactly once -------------------
    size_t      edgeCount() const { return edgeStates.size(); }
    Line&       edge(size_t logicalIndex)       { return edgeStates[logicalIndex]; }
    const Line& edge(size_t logicalIndex) const { return edgeStates[logicalIndex]; }

    // Index lookups. Both wrap (mod dimension), which is what makes this a
    // torus rather than a square with borders.
    size_t horizontalEdge(size_t row, size_t col) const;
    size_t verticalEdge(size_t col, size_t row) const;

    // Where a logical edge sits in grid-cell coordinates: (u0,v0) -> (u1,v1),
    // measured in columns/rows. The torus renderer feeds these straight into
    // its parametrization; the flat renderer doesn't need them because it
    // works from drawnEndpoints() instead.
    void edgeGridSpan(size_t logicalIndex, float& u0, float& v0, float& u1, float& v1) const;

    void toggle(size_t logicalIndex, bool asXError);

    // --- drawn edges: the flat picture, seam included ----------------------
    size_t drawnEdgeCount() const { return drawnToLogical.size(); }
    // Flat list, 2 entries per drawn edge (start, end) -- the layout
    // Raycasting::findClosestLine expects.
    const std::vector<glm::vec2>& drawnEndpoints() const { return drawnPoints; }
    size_t logicalForDrawn(size_t drawnIndex) const { return drawnToLogical[drawnIndex]; }

    // --- stabilizers -------------------------------------------------------
    size_t plaquetteCount() const { return dim * dim; }
    size_t vertexCount()    const { return dim * dim; }

    // The 4 edges bounding plaquette (col,row) / touching vertex (col,row).
    std::array<size_t, 4> plaquetteEdges(size_t col, size_t row) const;
    std::array<size_t, 4> vertexEdges(size_t col, size_t row) const;

    // A plaquette is a Z-type stabilizer, so it anticommutes with X errors and
    // fires on odd X parity around its 4 edges. A vertex is an X-type
    // stabilizer, so it fires on odd Z parity across the 4 edges it touches.
    bool plaquetteFires(size_t col, size_t row) const;
    bool vertexFires(size_t col, size_t row) const;

    // --- flat-space positions ---------------------------------------------
    glm::vec2 plaquetteCenter(size_t col, size_t row) const;
    glm::vec2 vertexPosition(size_t col, size_t row) const;

private:
    size_t    dim;
    float     scale;
    glm::vec2 gridOrigin;

    std::vector<Line>      edgeStates;    // 2 * dim * dim
    std::vector<glm::vec2> drawnPoints;   // 2 per drawn edge
    std::vector<size_t>    drawnToLogical;

    size_t parityCount(const std::array<size_t, 4>& edges, LineState state) const;
};
