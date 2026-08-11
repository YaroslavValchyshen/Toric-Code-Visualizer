#pragma once

#include <glm/glm.hpp>
#include <cmath>

// ---------------------------------------------------------------------------
// TorusMap: the ONLY place that knows how flat lattice coordinates become 3D
// points. Pure math -- no OpenGL, no state -- so it can be reasoned about (and
// unit tested) independently of any rendering.
//
// (u, v) are measured in GRID CELLS, exactly the units Lattice uses: u is the
// column coordinate, v is the row coordinate. Both are periodic with period
// `period` (= lattice dimension), which is precisely why the periodic Lattice
// maps onto the torus with no seam and no duplicated edges: u = N is literally
// the same point as u = 0.
//
//   theta (u) sweeps the long way around the ring
//   phi   (v) sweeps the short way around the tube
// ---------------------------------------------------------------------------
struct TorusMap {
    float majorRadius = 1.65f;  // ring center -> tube center
    float minorRadius = 0.62f;  // tube radius
    float period      = 9.0f;   // lattice dimension

    static constexpr float TWO_PI = 6.28318530718f;

    // radialOffset lifts a point slightly off the surface, so edge ribbons and
    // defect markers sit ON the torus instead of z-fighting with its skin.
    glm::vec3 position(float u, float v, float radialOffset = 0.0f) const {
        const float theta = TWO_PI * u / period;
        const float phi   = TWO_PI * v / period;
        const float r     = minorRadius + radialOffset;
        const float rho   = majorRadius + r * std::cos(phi);
        return glm::vec3(rho * std::cos(theta),
                         rho * std::sin(theta),
                         r * std::sin(phi));
    }

    // Outward surface normal (unit length by construction).
    glm::vec3 normal(float u, float v) const {
        const float theta = TWO_PI * u / period;
        const float phi   = TWO_PI * v / period;
        return glm::vec3(std::cos(phi) * std::cos(theta),
                         std::cos(phi) * std::sin(theta),
                         std::sin(phi));
    }
};
