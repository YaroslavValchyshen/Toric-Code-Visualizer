#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h> // WebGL 2 headers for the web
#else
#include <GL/glew.h>   // GLEW headers for your desktop Mac build
#endif
#include <GLFW/glfw3.h>
#include <iostream>
#include "Shader.hpp"
#include "FileManager.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Global pointers so the rendering function can access them
GLFWwindow* window = nullptr;
unsigned int shaderProgram = 0;

// This function renders a single frame
void render_frame() {
    glUseProgram(shaderProgram);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
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
    
    
    float vertices[] = {
        -0.5f, -0.5f,
        0.5f, -0.5f,
        0.0f,  0.5f
    };
    
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    Shader shader(SHADER_DIR);
    shaderProgram = shader.initializeShader();

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
