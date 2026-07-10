#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h> // WebGL 2 headers for the web
#else
#include <GL/glew.h>   // GLEW headers for your desktop Mac build
#endif
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "Shader.hpp"
#include "FileManager.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif


GLFWwindow* window = nullptr;
unsigned int shaderProgram = 0;


void render_frame() {
    glUseProgram(shaderProgram);
    glClear(GL_COLOR_BUFFER_BIT);
    const size_t totalSquares = 9 * 9;
    for (int s = 0; s < totalSquares; s++) {
        int startingVertex = s * 4;
    
        glDrawArrays(GL_LINE_LOOP, startingVertex, 4);
    }   
    //glEnable(GL_PROGRAM_POINT_SIZE); // If setting size in shader, OR use:
    glPointSize(14.0f);               // Quick global point size setting

    // Draw all vertices at once across the entire lattice
    glDrawArrays(GL_POINTS, 0, totalSquares * 4);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}

int main(int argc, const char * argv[]) {
    
    if (!glfwInit()) return -1;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(800, 800, "Title", NULL, NULL);
    
    if (!window) {
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    #ifndef __EMSCRIPTEN__
    glewInit();
    #endif
    
    const size_t latticeDimension = 9;
    const float LATTICE_SCALE = 0.1f;
    std::vector<float> vertices;
    for(int i = 0; i < latticeDimension; i++)
        for(int j = 0; j < latticeDimension; j++)
        {
            vertices.push_back(0.0f + i * LATTICE_SCALE);
            vertices.push_back(0.0f + j * LATTICE_SCALE);
        
            vertices.push_back(LATTICE_SCALE + i * LATTICE_SCALE);
            vertices.push_back(0.0f + j * LATTICE_SCALE);
        
            vertices.push_back(LATTICE_SCALE + i * LATTICE_SCALE);
            vertices.push_back(LATTICE_SCALE + j * LATTICE_SCALE);
        
            vertices.push_back(0.0f + i * 0.1f);
            vertices.push_back(LATTICE_SCALE + j * LATTICE_SCALE);
        }
    
    float vertices1[] = {
        0.0f, 0.0f,
        0.0f, 0.1f,
        0.1f, 0.0f,
        0.1f, 0.1f
    };
    
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    Shader shader(SHADER_DIR);

    #ifdef __EMSCRIPTEN__
    shaderProgram = shader.initializeShader("300 es");
    #else 
    shaderProgram = shader.initializeShader("330 core");
    #endif

#ifdef __EMSCRIPTEN__
    // For the web: Hand control over to the browser. 
    // Passing 0 FPS defaults to the browser's requestAnimationFrame matching display refresh.
    emscripten_set_main_loop(render_frame, 0, 1);
#else
    // For your desktop: Run the traditional rendering loop.
    while (!glfwWindowShouldClose(window)) {
        render_frame();
    }
#endif
    
    glfwTerminate();
    return 0;
}
