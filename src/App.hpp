#pragma once

#include "GraphicsAPI.hpp"
#include <memory>
#include <glm/glm.hpp>

#include "Lattice.hpp"
#include "FlatRenderer.hpp"
#include "Camera.hpp"

#ifndef __EMSCRIPTEN__
  #include "TorusRenderer.hpp"
  #include "OrbitCamera.hpp"
#endif

// ---------------------------------------------------------------------------
// App: owns the windows, the Lattice, and the renderers, and wires input to
// state changes. This is the composition root -- the only place that knows
// both "there is a flat view" and "there is a torus view".
//
// The key invariant: nothing mutates the Lattice except applyToggle(), which
// then tells every renderer to re-sync. That's what makes the two windows
// impossible to desynchronise.
// ---------------------------------------------------------------------------
class App {
public:
    bool initialize();
    void run();
    void shutdown();

private:
    static constexpr int    WINDOW_WIDTH  = 800;
    static constexpr int    WINDOW_HEIGHT = 800;
    static constexpr size_t LATTICE_DIMENSION = 9;
    static constexpr float  LATTICE_SCALE = 0.2f;
    static constexpr float  CLICK_THRESHOLD = 0.02f;

    GLFWwindow* flatWindow = nullptr;
    Lattice lattice{ LATTICE_DIMENSION, LATTICE_SCALE, glm::vec2(-0.5f, -0.5f) };
    FlatRenderer flatRenderer;
    std::unique_ptr<Camera> flatCamera;
    unsigned int flatShaderProgram = 0;

    bool placingXError = false;

#ifndef __EMSCRIPTEN__
    GLFWwindow* torusWindow = nullptr;
    TorusRenderer torusRenderer;
    OrbitCamera torusCamera;
    bool dragging = false;
    double lastCursorX = 0.0, lastCursorY = 0.0;
    double lastFrameTime = 0.0;

    bool initializeTorusWindow();
    void drawTorusWindow();
#endif

    void frame();
    static void frameTrampoline(void* self);

    // Single funnel for every state change, so no caller can forget to re-sync.
    void applyToggle(size_t logicalEdge);
    void handleFlatClick(double xpos, double ypos);

    static void onFlatMouseButton(GLFWwindow* window, int button, int action, int mods);
    static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
#ifndef __EMSCRIPTEN__
    static void onTorusMouseButton(GLFWwindow* window, int button, int action, int mods);
    static void onTorusCursorMove(GLFWwindow* window, double xpos, double ypos);
    static void onTorusScroll(GLFWwindow* window, double xoffset, double yoffset);
#endif
};
