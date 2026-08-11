#pragma once

#ifndef __EMSCRIPTEN__

#include "GraphicsAPI.hpp"
#include <vector>
#include <glm/glm.hpp>

#include "Lattice.hpp"
#include "TorusMap.hpp"
#include "OrbitCamera.hpp"

// ---------------------------------------------------------------------------
// TorusRenderer: draws the SAME Lattice wrapped onto a real torus.
//
// Reads the identical Line states the flat view reads, so errors, highlights
// and defects always match -- there is no second copy of anything to keep in
// sync. It iterates LOGICAL edges (2*N*N), not drawn ones, because on the
// torus the seam isn't duplicated: it's the same edge arriving back where it
// started.
//
// Not built for Emscripten: a second native window doesn't fit the
// single-canvas browser model that build targets.
//
// NOTE: this renderer owns its own GL context's objects. Whoever drives it
// must make the torus window's context current before calling anything here.
// ---------------------------------------------------------------------------
class TorusRenderer {
public:
    void initialize(const Lattice& lattice);
    void sync(const Lattice& lattice);
    void draw(const Lattice& lattice, const OrbitCamera& camera,
              int framebufferWidth, int framebufferHeight);
    void shutdown();

    TorusMap& map() { return torusMap; }

private:
    TorusMap torusMap;
    unsigned int program = 0;

    unsigned int surfaceVAO = 0, surfaceVBO = 0, surfaceEBO = 0;
    unsigned int edgeVAO = 0, edgeVBO = 0, edgeEBO = 0;
    unsigned int dotVAO = 0, dotVBO = 0;
    unsigned int defectVAO = 0, defectVBO = 0;

    std::vector<float> edgeVertices;   // pos(3) + normal(3) per vertex
    size_t surfaceIndexCount = 0;
    size_t dotCount = 0;
    size_t plaquetteDefectPoints = 0;
    size_t vertexDefectPoints = 0;

    void buildSurface();
    void buildDots(const Lattice& lattice);
    void syncEdges(const Lattice& lattice);
    void syncDefects(const Lattice& lattice);
};

#endif // __EMSCRIPTEN__
