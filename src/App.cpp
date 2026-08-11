#include "App.hpp"
#include "Shader.hpp"
#include "Raycasting.hpp"
#include "RenderConstants.hpp"

#include <iostream>

#ifdef __EMSCRIPTEN__
  #include <emscripten.h>
#endif

bool App::initialize() {
    if (!glfwInit()) return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    flatWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                  "Toric Code -- Lattice", NULL, NULL);
    if (!flatWindow) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(flatWindow);
#ifndef __EMSCRIPTEN__
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    glfwSetWindowUserPointer(flatWindow, this);
    glfwSetMouseButtonCallback(flatWindow, onFlatMouseButton);
    glfwSetKeyCallback(flatWindow, onKey);

    Shader shader(SHADER_DIR);
#ifdef __EMSCRIPTEN__
    flatShaderProgram = shader.initializeShader("300 es");
#else
    flatShaderProgram = shader.initializeShader("330 core");
    glEnable(GL_PROGRAM_POINT_SIZE);
#endif

    flatCamera = std::make_unique<Camera>(flatShaderProgram, 0.01f, WINDOW_WIDTH, WINDOW_HEIGHT);
    flatRenderer.initialize(lattice);

#ifndef __EMSCRIPTEN__
    if (!initializeTorusWindow()) {
        std::cerr << "Torus window unavailable; continuing with the flat view only."
                  << std::endl;
    }
    // Leave the flat context current so the first frame starts in a known state.
    glfwMakeContextCurrent(flatWindow);
    lastFrameTime = glfwGetTime();
#endif

    return true;
}

#ifndef __EMSCRIPTEN__
bool App::initializeTorusWindow() {
    // Share the flat window's context so buffer/VAO creation is legal here and
    // so both windows can be driven from one thread without reloading GLEW.
    torusWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                   "Toric Code -- Torus", NULL, flatWindow);
    if (!torusWindow) return false;

    glfwSetWindowPos(torusWindow, 60 + WINDOW_WIDTH, 60);
    glfwMakeContextCurrent(torusWindow);
    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glfwSetWindowUserPointer(torusWindow, this);
    glfwSetMouseButtonCallback(torusWindow, onTorusMouseButton);
    glfwSetCursorPosCallback(torusWindow, onTorusCursorMove);
    glfwSetScrollCallback(torusWindow, onTorusScroll);
    glfwSetKeyCallback(torusWindow, onKey);

    torusRenderer.initialize(lattice);
    return true;
}
#endif

void App::run() {
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(frameTrampoline, this, 0, 1);
#else
    while (!glfwWindowShouldClose(flatWindow)) {
        if (torusWindow && glfwWindowShouldClose(torusWindow)) {
            // Closing the torus view shouldn't kill the app -- drop it and
            // keep the lattice running.
            glfwMakeContextCurrent(torusWindow);
            torusRenderer.shutdown();
            glfwDestroyWindow(torusWindow);
            torusWindow = nullptr;
            glfwMakeContextCurrent(flatWindow);
        }
        frame();
    }
#endif
}

void App::frameTrampoline(void* self) {
    static_cast<App*>(self)->frame();
}

void App::frame() {
    // --- flat window -------------------------------------------------------
    glfwMakeContextCurrent(flatWindow);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(flatShaderProgram);
#ifndef __EMSCRIPTEN__
    glEnable(GL_PROGRAM_POINT_SIZE);
#endif

    flatCamera->update(flatWindow);
    flatRenderer.draw(lattice, flatShaderProgram);
    glfwSwapBuffers(flatWindow);

#ifndef __EMSCRIPTEN__
    // --- torus window ------------------------------------------------------
    drawTorusWindow();
#endif

    glfwPollEvents();
}

#ifndef __EMSCRIPTEN__
void App::drawTorusWindow() {
    if (!torusWindow) return;

    const double now = glfwGetTime();
    const float deltaTime = (float)(now - lastFrameTime);
    lastFrameTime = now;

    glfwMakeContextCurrent(torusWindow);

    int width = 0, height = 0;
    glfwGetFramebufferSize(torusWindow, &width, &height);
    glViewport(0, 0, width, height);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    torusCamera.spin(deltaTime);
    torusRenderer.draw(lattice, torusCamera, width, height);

    glfwSwapBuffers(torusWindow);
}
#endif

void App::applyToggle(size_t logicalEdge) {
    lattice.toggle(logicalEdge, placingXError);

    // Every view re-reads the lattice. Each renderer's objects live in its own
    // window's context, so make that context current before touching it.
    glfwMakeContextCurrent(flatWindow);
    flatRenderer.sync(lattice);

#ifndef __EMSCRIPTEN__
    if (torusWindow) {
        glfwMakeContextCurrent(torusWindow);
        torusRenderer.sync(lattice);
        glfwMakeContextCurrent(flatWindow);
    }
#endif
}

void App::handleFlatClick(double xpos, double ypos) {
    const float ndcX = (2.0f * (float)xpos) / WINDOW_WIDTH - 1.0f;
    const float ndcY = 1.0f - (2.0f * (float)ypos) / WINDOW_HEIGHT;

    glm::vec4 ndc(ndcX, ndcY, 0.0f, 1.0f);
    glm::vec4 world4 = flatCamera->getMVPInverse() * ndc;
    glm::vec2 world(world4.x / world4.w, world4.y / world4.w);

    RaycastHit hit = Raycasting::findClosestLine(lattice.drawnEndpoints(), world, CLICK_THRESHOLD);
    if (hit.lineIndex < 0) return;

    // The hit is against a DRAWN edge; the state lives on the logical one.
    // This is what makes clicking either side of the seam light up both.
    applyToggle(lattice.logicalForDrawn((size_t)hit.lineIndex));
}

void App::onFlatMouseButton(GLFWwindow* window, int button, int action, int mods) {
    (void)mods;
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;

    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app) return;

    double xpos = 0.0, ypos = 0.0;
    glfwGetCursorPos(window, &xpos, &ypos);
    app->handleFlatClick(xpos, ypos);
}

void App::onKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode; (void)mods;
    if (action != GLFW_PRESS) return;

    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app) return;

    // Z / X pick which error type a click places. The original code used the
    // arrow keys, but Camera already consumes those for panning -- one key
    // press was doing two unrelated things.
    if (key == GLFW_KEY_Z) {
        app->placingXError = false;
        std::cout << "Placing Z errors" << std::endl;
    } else if (key == GLFW_KEY_X) {
        app->placingXError = true;
        std::cout << "Placing X errors" << std::endl;
    }
#ifndef __EMSCRIPTEN__
    else if (key == GLFW_KEY_SPACE) {
        app->torusCamera.setAutoSpin(!app->torusCamera.autoSpinEnabled());
    }
#endif
}

#ifndef __EMSCRIPTEN__
void App::onTorusMouseButton(GLFWwindow* window, int button, int action, int mods) {
    (void)mods;
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app) return;

    if (action == GLFW_PRESS) {
        app->dragging = true;
        app->torusCamera.setAutoSpin(false);
        glfwGetCursorPos(window, &app->lastCursorX, &app->lastCursorY);
    } else if (action == GLFW_RELEASE) {
        app->dragging = false;
    }
}

void App::onTorusCursorMove(GLFWwindow* window, double xpos, double ypos) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app || !app->dragging) return;

    const float dx = (float)(xpos - app->lastCursorX);
    const float dy = (float)(ypos - app->lastCursorY);
    app->lastCursorX = xpos;
    app->lastCursorY = ypos;

    app->torusCamera.orbit(dx * 0.008f, -dy * 0.008f);
}

void App::onTorusScroll(GLFWwindow* window, double xoffset, double yoffset) {
    (void)xoffset;
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app) return;
    app->torusCamera.zoom((float)yoffset * 0.4f);
}
#endif

void App::shutdown() {
    glfwMakeContextCurrent(flatWindow);
    flatRenderer.shutdown();

#ifndef __EMSCRIPTEN__
    if (torusWindow) {
        glfwMakeContextCurrent(torusWindow);
        torusRenderer.shutdown();
    }
#endif

    glfwTerminate();
}
