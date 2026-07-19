#include "line.hpp"

namespace {

	struct LineStyle {
		glm::vec4 color;
		float width;
	};

	// The ONLY place visual appearance is associated with a state.
	// Everything else just asks "what state am I in" and looks it up here.
	// width here is a HALF-width in world units (same space as the lattice
	// vertices), not a glLineWidth pixel value -- see Line.hpp.
	const LineStyle& styleForState(LineState state) {
		static const LineStyle styles[] = {
			{ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 0.0015f },  // LineState::Normal
			{ glm::vec4(1.0f, 0.25f, 0.1f, 1.0f), 0.006f },  // LineState::Highlighted (4x thicker)
		};
		return styles[static_cast<int>(state)];
	}

}

void Line::setState(LineState newState) {
	state = newState;
}

void Line::toggleHighlight() {
	setState(state == LineState::Normal ? LineState::Highlighted : LineState::Normal);
}

LineState Line::getState() const {
	return state;
}

glm::vec4 Line::getColor() const {
	return styleForState(state).color;
}

float Line::getWidth() const {
	return styleForState(state).width;
}