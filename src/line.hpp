#pragma once

#include <glm/glm.hpp>

// The set of states a rendered line can be in. Adding a new visual state
// later (e.g. Hovered) means adding one enum value and one entry in the
// style table in Line.cpp -- nothing else about the call sites changes.
enum class LineState {
	Normal = 0,
	Highlighted = 1
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

	LineState getState() const;
	glm::vec4 getColor() const;
	// Half-width, in the same world units as the lattice vertices. Used to
	// build the quad that renders this line (see rebuildLineQuad() in
	// main.cpp) -- NOT a glLineWidth value, since many drivers (and WebGL2
	// always) clamp glLineWidth to 1px regardless of what's requested.
	float getWidth() const;
};