#pragma once

#include <glm/glm.hpp>

// The set of states a rendered line can be in. Adding a new visual state
// later (e.g. Hovered) means adding one enum value and one entry in the
// style table in Line.cpp -- nothing else about the call sites changes.
enum class LineState {
	Normal = 0,
	ZError = 1,
	XError = 2
};

// A Line only knows what state it is in. Color and width are never set
// directly -- they are always derived from the current state through a
// single lookup (see Line.cpp). This means "changing color" has exactly
// one implementation in the whole program: Line::setState().
class Line
{
private:
	LineState state = LineState::Normal;

public:
	void setState(LineState newState);
	// Convenience for the common click->toggle interaction.
	void toggleHighlight();
	void toggleXError();

	LineState getState() const;
	glm::vec4 getColor() const;
	// Half-width, in the same world units as the lattice vertices. NOT a
	// glLineWidth value, since many drivers (and WebGL2 always) clamp
	// glLineWidth to 1px regardless of what's requested -- so both views
	// build real geometry from this instead. See FlatRenderer::syncEdges()
	// for the flat quads, and TorusRenderer::syncEdges() for the curved
	// ribbons (which scale this by EDGE_WIDTH_SCALE, the torus being much
	// larger in world units than the flat lattice).
	float getWidth() const;
};