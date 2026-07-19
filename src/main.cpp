#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h> // WebGL 2 headers for the web
#else
#include <GL/glew.h>   // GLEW headers for your desktop Mac build
#endif
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "Shader.hpp"
#include "Camera.hpp"
#include "FileManager.hpp"
#include "Raycasting.hpp"
#include "line.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>   // Required for WebGL2 / WebAssembly compilation
#else
    #include <GL/glew.h>     // Used for your local Mac desktop build
#endif


GLFWwindow* window = nullptr;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const size_t latticeDimension = 9;
unsigned int shaderProgram = 0;
float mouseX = 0;
float mouseY = 0;
float moveSpeed = 0.005f;
std::vector<float> vertices;             // corner points only, for GL_POINTS (duplicates are harmless here)
std::vector<glm::vec2> lineEndpoints;    // 2 entries per line: start, end -- each grid line exactly once
std::vector<Line> lines;
std::vector<float> lineQuadVertices;     // 4 quad-corner vertices (8 floats) per line
unsigned int latticeVAO = 0;
unsigned int lineQuadVAO = 0;
unsigned int lineQuadVBO = 0;
Camera* camera;

// How close (in the same space as the lattice vertices) a click needs to
// be to an edge for that edge to count as clicked.
const float CLICK_THRESHOLD = 0.02f;

// Lines are rendered as filled quads rather than GL_LINES, because
// glLineWidth is clamped to 1px by many drivers (and always on WebGL2,
// which this project also targets) regardless of the value requested.
// A quad's width is real geometry, so "thicker" actually renders as
// thicker everywhere. This recomputes the 4 corners for one line from its
// endpoints and its Line::getWidth() half-width, and pushes them to the GPU.
void rebuildLineQuad(size_t lineIndex) {
    glm::vec2 A = lineEndpoints[lineIndex * 2 + 0];
    glm::vec2 B = lineEndpoints[lineIndex * 2 + 1];

    glm::vec2 direction = glm::normalize(B - A);
    glm::vec2 perp = glm::vec2(-direction.y, direction.x) * lines[lineIndex].getWidth();

    glm::vec2 quad[4] = {
        A + perp,
        A - perp,
        B - perp,
        B + perp
    };

    size_t floatBase = lineIndex * 8;
    for (int c = 0; c < 4; c++) {
        lineQuadVertices[floatBase + c * 2 + 0] = quad[c].x;
        lineQuadVertices[floatBase + c * 2 + 1] = quad[c].y;
    }

    glBindBuffer(GL_ARRAY_BUFFER, lineQuadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, floatBase * sizeof(float), 8 * sizeof(float), &lineQuadVertices[floatBase]);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) {
        return;
    }

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    float ndcX = (2.0f * (float)xpos) / WINDOW_WIDTH - 1.0f;
    float ndcY = 1.0f - (2.0f * (float)ypos) / WINDOW_HEIGHT;
    glm::vec4 ndc(ndcX, ndcY, 0.0f, 1.0f);
    glm::vec4 worldPos4 = camera->getMVPInverse() * ndc;
    glm::vec2 worldPos(worldPos4.x / worldPos4.w, worldPos4.y / worldPos4.w);

    RaycastHit hit = Raycasting::findClosestLine(lineEndpoints, worldPos, CLICK_THRESHOLD);

    if (hit.lineIndex != -1) {
        lines[hit.lineIndex].toggleHighlight();
        rebuildLineQuad(hit.lineIndex);
    }
}

void render_frame(Camera* camera) {
    const size_t totalSquares = latticeDimension * latticeDimension;
    glUseProgram(shaderProgram);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
    #ifndef __EMSCRIPTEN__
    glEnable(GL_PROGRAM_POINT_SIZE); 
    #endif
    camera->update(window);

    int colorLocation = glGetUniformLocation(shaderProgram, "u_color");

    // Each line is 2 triangles (a quad) so its width is real geometry.
    glBindVertexArray(lineQuadVAO);
    for (size_t i = 0; i < lines.size(); i++) {
        glm::vec4 color = lines[i].getColor();
        glUniform4f(colorLocation, color.r, color.g, color.b, color.a);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,
                       (const void*)(sizeof(unsigned int) * 6 * i));
    }

    glBindVertexArray(latticeVAO);
    glUniform4f(colorLocation, 0.5, 0.5, 1.0, 1.0);
    glDrawArrays(GL_POINTS, 0, (GLsizei)(totalSquares * 4));
    glfwSwapBuffers(window);
    glfwPollEvents();
}

int main(int argc, const char * argv[]) {
    
    if (!glfwInit()) return -1;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Title", NULL, NULL);
    
    if (!window) {
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    #ifndef __EMSCRIPTEN__
    glewInit();
    #endif
    
    

    const float LATTICE_SCALE = 0.2f;
    
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    float ORIGIN_X = -0.5f;
    float ORIGIN_Y = -0.5f;

    for(int i = 0; i < latticeDimension; i++)
        for(int j = 0; j < latticeDimension; j++)
        {
            vertices.push_back(0.0f + i * LATTICE_SCALE + ORIGIN_X);
            vertices.push_back(0.0f + j * LATTICE_SCALE + ORIGIN_Y);
        
            vertices.push_back(LATTICE_SCALE + i * LATTICE_SCALE + ORIGIN_X);
            vertices.push_back(0.0f + j * LATTICE_SCALE + ORIGIN_Y);
        
            vertices.push_back(LATTICE_SCALE + i * LATTICE_SCALE + ORIGIN_X);
            vertices.push_back(LATTICE_SCALE + j * LATTICE_SCALE + ORIGIN_Y);
        
            vertices.push_back(0.0f + i * LATTICE_SCALE + ORIGIN_X);
            vertices.push_back(LATTICE_SCALE + j * LATTICE_SCALE + ORIGIN_Y);
        }
    
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    latticeVAO = VAO;

    // One Line (click/highlight state) + one quad per PHYSICAL grid line.
    // Built directly from the (latticeDimension+1) x (latticeDimension+1)
    // grid points rather than as 4 edges per square, so that a shared
    // boundary between two neighboring squares is one line, not two
    // coincident duplicates fighting over which gets drawn on top.
    for (size_t row = 0; row <= latticeDimension; row++) {
        for (size_t col = 0; col < latticeDimension; col++) {
            glm::vec2 A(col * LATTICE_SCALE + ORIGIN_X, row * LATTICE_SCALE + ORIGIN_Y);
            glm::vec2 B((col + 1) * LATTICE_SCALE + ORIGIN_X, row * LATTICE_SCALE + ORIGIN_Y);
            lineEndpoints.push_back(A);
            lineEndpoints.push_back(B);
        }
    }
    for (size_t col = 0; col <= latticeDimension; col++) {
        for (size_t row = 0; row < latticeDimension; row++) {
            glm::vec2 A(col * LATTICE_SCALE + ORIGIN_X, row * LATTICE_SCALE + ORIGIN_Y);
            glm::vec2 B(col * LATTICE_SCALE + ORIGIN_X, (row + 1) * LATTICE_SCALE + ORIGIN_Y);
            lineEndpoints.push_back(A);
            lineEndpoints.push_back(B);
        }
    }

    lines.resize(lineEndpoints.size() / 2);

    std::vector<unsigned int> quadTriangleIndices;
    quadTriangleIndices.reserve(lines.size() * 6);
    for (size_t i = 0; i < lines.size(); i++) {
        unsigned int base = static_cast<unsigned int>(i * 4);
        quadTriangleIndices.push_back(base + 0);
        quadTriangleIndices.push_back(base + 1);
        quadTriangleIndices.push_back(base + 2);
        quadTriangleIndices.push_back(base + 0);
        quadTriangleIndices.push_back(base + 2);
        quadTriangleIndices.push_back(base + 3);
    }

    glGenVertexArrays(1, &lineQuadVAO);
    glBindVertexArray(lineQuadVAO);

    glGenBuffers(1, &lineQuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lineQuadVBO);
    lineQuadVertices.resize(lines.size() * 8, 0.0f);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * lineQuadVertices.size(), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    unsigned int quadEBO;
    glGenBuffers(1, &quadEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * quadTriangleIndices.size(), quadTriangleIndices.data(), GL_STATIC_DRAW);

    for (size_t i = 0; i < lines.size(); i++) {
        rebuildLineQuad(i);
    }

    Shader shader(SHADER_DIR);
    #ifndef __EMSCRIPTEN__
    glEnable(GL_PROGRAM_POINT_SIZE); 
    #endif


    #ifdef __EMSCRIPTEN__
    shaderProgram = shader.initializeShader("300 es");
    #else 
    shaderProgram = shader.initializeShader("330 core");
    #endif
    camera = new Camera(shaderProgram, 0.01f, WINDOW_WIDTH, WINDOW_HEIGHT);
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(render_frame, 0, 1);
    #else
    while (!glfwWindowShouldClose(window)) {
        render_frame(camera);
    }
    #endif
    
    glfwTerminate();
    return 0;
}