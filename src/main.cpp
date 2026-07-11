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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif


GLFWwindow* window = nullptr;
unsigned int shaderProgram = 0;
glm::mat4 model = glm::mat4(1.0f);
float objectX = 0;
float objectY = 0;
float moveSpeed = 0.005f;



void render_frame() {
    const size_t totalSquares = 9 * 9;
    glUseProgram(shaderProgram);
    glClear(GL_COLOR_BUFFER_BIT);
    #ifndef __EMSCRIPTEN__
    glEnable(GL_PROGRAM_POINT_SIZE); 
    #endif
    objectX = 0;
    objectY = 0;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        objectX += moveSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        objectX -= moveSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        objectY -= moveSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        objectY += moveSpeed;
    }
    model = glm::translate(model, glm::vec3(objectX, objectY, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_MVP"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(glGetUniformLocation(shaderProgram, "u_color"), 0.0, 1.0, 0.0, 1.0);
    
    for (int s = 0; s < totalSquares; s++) {
        int startingVertex = s * 4;
    
        glDrawArrays(GL_LINE_LOOP, startingVertex, 4);
    }   
    glUniform4f(glGetUniformLocation(shaderProgram, "u_color"), 0.0, 0.0, 1.0, 1.0);
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
    const float LATTICE_SCALE = 0.2f;
    std::vector<float> vertices;
    
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

    Shader shader(SHADER_DIR);
    #ifndef __EMSCRIPTEN__
    glEnable(GL_PROGRAM_POINT_SIZE); 
    #endif


    #ifdef __EMSCRIPTEN__
    shaderProgram = shader.initializeShader("300 es");
    #else 
    shaderProgram = shader.initializeShader("330 core");
    #endif

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(render_frame, 0, 1);
    #else
    while (!glfwWindowShouldClose(window)) {
        render_frame();
    }
    #endif
    
    glfwTerminate();
    return 0;
}
