#include "OrbitCamera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace {
    constexpr float PITCH_LIMIT = 1.5f;   // just under pi/2, avoids gimbal flip at the poles
    constexpr float MIN_DISTANCE = 3.0f;
    constexpr float MAX_DISTANCE = 14.0f;
    constexpr float AUTO_SPIN_RATE = 0.25f; // radians/sec
}

OrbitCamera::OrbitCamera(float distance)
    : yaw(0.7f), pitch(0.85f), distance(distance) {}

void OrbitCamera::orbit(float deltaYaw, float deltaPitch) {
    yaw += deltaYaw;
    pitch = std::clamp(pitch + deltaPitch, -PITCH_LIMIT, PITCH_LIMIT);
}

void OrbitCamera::zoom(float amount) {
    distance = std::clamp(distance - amount, MIN_DISTANCE, MAX_DISTANCE);
}

void OrbitCamera::spin(float deltaTime) {
    if (autoSpin) yaw += AUTO_SPIN_RATE * deltaTime;
}

glm::mat4 OrbitCamera::viewProjection(int framebufferWidth, int framebufferHeight) const {
    const float aspect = (framebufferHeight > 0)
        ? (float)framebufferWidth / (float)framebufferHeight
        : 1.0f;

    glm::vec3 eye(distance * std::cos(pitch) * std::cos(yaw),
                  distance * std::cos(pitch) * std::sin(yaw),
                  distance * std::sin(pitch));

    glm::mat4 projection = glm::perspective(glm::radians(42.0f), aspect, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(eye, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    return projection * view;
}
