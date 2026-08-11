#pragma once

#include "GraphicsAPI.hpp"

#include <vector>
#include "Lattice.hpp"

// ---------------------------------------------------------------------------
// FlatRenderer: draws the Lattice as the flat square grid, using the project's
// file-based shader (u_MVP + u_color) and Camera.
//
// It owns GL objects and nothing else -- it holds no error state of its own.
// Every draw reads current state out of the Lattice it's handed, so it cannot
// drift out of sync with the torus view.
//
// Edges are filled quads rather than GL_LINES because glLineWidth is clamped
// to 1px by many drivers (and always on WebGL2, which this project also
// targets). A quad's width is real geometry, so "thicker" renders as thicker
// everywhere.
// ---------------------------------------------------------------------------
class FlatRenderer {
public:
    void initialize(const Lattice& lattice);
    void sync(const Lattice& lattice);   // edges + defects, after any state change
    void draw(const Lattice& lattice, unsigned int shaderProgram);
    void shutdown();

private:
    unsigned int dotVAO = 0, dotVBO = 0;
    unsigned int edgeVAO = 0, edgeVBO = 0, edgeEBO = 0;
    unsigned int defectVAO = 0, defectVBO = 0;

    std::vector<float> edgeVertices;     // 4 corners (8 floats) per DRAWN edge
    size_t dotCount = 0;
    size_t plaquetteDefectTriangles = 0; // triangles, drawn first
    size_t vertexDefectTriangles = 0;    // triangles, drawn after

    void syncEdges(const Lattice& lattice);
    void syncDefects(const Lattice& lattice);
};
