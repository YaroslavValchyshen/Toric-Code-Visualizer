
#include "Shader.hpp"
#include "FileManager.hpp"

unsigned int Shader::initializeShader(){

    std::string vertexContent = FileManager::readFile(shaderPath + "vertex.vert");
    std::string fragmentContent = FileManager::readFile(shaderPath + "fragment.frag");
    unsigned int vertexShader = Shader::createShader(vertexContent.data(), GL_VERTEX_SHADER);
    unsigned int fragmentShader = Shader::createShader(fragmentContent.data(), GL_FRAGMENT_SHADER);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    return shaderProgram;
}

unsigned int Shader::createShader(char* source, GLuint shaderType){
    unsigned int shader = glCreateShader(shaderType);
    
    
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if(success == GL_FALSE){
        GLchar info[256];
        glGetShaderInfoLog(shader, 256, NULL, info);
        std::cout << info;
    }

    return shader;
}
