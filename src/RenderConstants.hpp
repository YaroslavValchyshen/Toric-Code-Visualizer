#pragma once

#include <glm/glm.hpp>

// ---------------------------------------------------------------------------
// Values shared by BOTH views. Defect colors live here rather than in each
// renderer so the flat window and the torus window can't drift apart -- if a
// plaquette defect is blue in one, it is the same blue in the other.
//
// Edge colors are deliberately NOT here: those belong to LineState and stay in
// line.cpp, which remains the single place appearance is tied to state.
// ---------------------------------------------------------------------------
namespace RenderConstants {

    constexpr float TWO_PI = 6.28318530718f;

    // Flat-view circle tessellation.
    constexpr int   CIRCLE_SEGMENTS = 20;
    constexpr float PLAQUETTE_MARKER_RADIUS = 0.05f;   // world units
    constexpr float VERTEX_MARKER_RADIUS    = 0.028f;  // world units

    // Torus-view marker sizes, in pixels (drawn as round point sprites).
    constexpr float TORUS_PLAQUETTE_POINT_SIZE = 17.0f;
    constexpr float TORUS_VERTEX_POINT_SIZE    = 12.0f;
    constexpr float TORUS_LATTICE_POINT_SIZE   = 5.0f;

    // Plaquettes are Z-stabilizers and fire on X errors, so they take the
    // X-error hue; vertices are X-stabilizers and fire on Z errors, so they
    // take the Z-error hue. The color that causes a defect matches the color
    // of the defect it produces.
    const glm::vec4 PLAQUETTE_DEFECT_COLOR(0.10f, 0.35f, 1.00f, 1.0f);
    const glm::vec4 VERTEX_DEFECT_COLOR   (1.00f, 0.25f, 0.10f, 1.0f);

    const glm::vec4 TORUS_SURFACE_COLOR   (0.88f, 0.89f, 0.93f, 1.0f);
    const glm::vec4 TORUS_LATTICE_DOT_COLOR(0.50f, 0.50f, 1.00f, 1.0f);

} // namespace RenderConstants
