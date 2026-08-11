#pragma once

#include <glm/glm.hpp>

// ---------------------------------------------------------------------------
// A minimal orbit camera for the torus window: yaw/pitch around the origin at
// a fixed distance. Deliberately separate from Camera (which is a 2D pan
// camera bound to the flat shader's u_MVP uniform) -- the two windows have
// genuinely different navigation needs, and forcing one class to do both would
// make each worse.
// ---------------------------------------------------------------------------
class OrbitCamera {
public:
    explicit OrbitCamera(float distance = 5.5f);

    void orbit(float deltaYaw, float deltaPitch);
    void zoom(float amount);
    void spin(float deltaTime);          // gentle idle rotation
    void setAutoSpin(bool enabled) { autoSpin = enabled; }
    bool autoSpinEnabled() const { return autoSpin; }

    glm::mat4 viewProjection(int framebufferWidth, int framebufferHeight) const;

private:
    float yaw;
    float pitch;
    float distance;
    bool  autoSpin = true;
};
