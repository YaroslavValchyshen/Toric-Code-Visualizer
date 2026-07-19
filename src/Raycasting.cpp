#include "Raycasting.hpp"
#include <limits>

float Raycasting::raycastDistance(glm::vec2 p, glm::vec2 A, glm::vec2 B) {
    glm::vec2 AB = B - A;
    float lengthSquared = glm::dot(AB, AB);

    // A and B are (nearly) the same point, so the "segment" is a point.
    if (lengthSquared < 1e-10f) {
        return glm::distance(p, A);
    }

    // Project p onto the line AB, clamped to the segment itself.
    float t = glm::dot(p - A, AB) / lengthSquared;
    t = glm::clamp(t, 0.0f, 1.0f);

    glm::vec2 closestPoint = A + t * AB;
    return glm::distance(p, closestPoint);
}

RaycastHit Raycasting::findClosestLine(const std::vector<glm::vec2>& lineEndpoints,
                                        glm::vec2 point,
                                        float maxDistance) {
    RaycastHit hit;
    hit.distance = std::numeric_limits<float>::max();

    const size_t lineCount = lineEndpoints.size() / 2;

    for (size_t i = 0; i < lineCount; i++) {
        float distance = raycastDistance(point, lineEndpoints[i * 2], lineEndpoints[i * 2 + 1]);
        if (distance < hit.distance) {
            hit.distance = distance;
            hit.lineIndex = static_cast<int>(i);
        }
    }

    if (hit.distance > maxDistance) {
        hit.lineIndex = -1;
    }

    return hit;
}

void Raycasting::raycast(glm::vec3 origin) {
    // Reserved for future 3D raycasting use; the 2D lattice picking above
    // (findClosestLine) is what drives line selection today.
}