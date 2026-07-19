#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstddef>

// Result of a click -> line hit test.
// lineIndex is -1 when nothing was within the click threshold.
struct RaycastHit {
	int lineIndex = -1;
	float distance = 0.0f;
};

class Raycasting{
public: 
	static void raycast(glm::vec3 origin);

	// Shortest distance from point p to the segment A-B.
	static float raycastDistance(glm::vec2 p, glm::vec2 A, glm::vec2 B);

	// All the detection infrastructure lives here: given a flat list of line
	// endpoints (2 entries per line: start, end) and a click point, find the
	// closest line. Returns lineIndex == -1 if the closest line is farther
	// than maxDistance away. This is intentionally decoupled from how the
	// lines were laid out (lattice, squares, etc.) -- it just compares a
	// point against segments.
	static RaycastHit findClosestLine(const std::vector<glm::vec2>& lineEndpoints,
	                                   glm::vec2 point,
	                                   float maxDistance);
};