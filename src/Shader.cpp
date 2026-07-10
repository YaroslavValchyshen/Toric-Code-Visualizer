
#include "Shader.hpp"
#include "FileManager.hpp"
#include <iostream>

unsigned int Shader::initializeShader(std::string version){

    std::string vertexContent = changeVersion(FileManager::readFile(shaderPath + "vertex.vert"), version);
    std::string fragmentContent = changeVersion(FileManager::readFile(shaderPath + "fragment.frag"), version);
    unsigned int vertexShader = Shader::createShader(vertexContent.data(), GL_VERTEX_SHADER);
    unsigned int fragmentShader = Shader::createShader(fragmentContent.data(), GL_FRAGMENT_SHADER);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    return shaderProgram;
}

std::string Shader::changeVersion(std::string shaderContext, std::string version){
    const size_t versionPosition = 9;
    size_t first_newline = shaderContext.find('\n');

    if(first_newline != std::string::npos){
        shaderContext.replace(versionPosition, first_newline - versionPosition, version + "\n");
    }

    return shaderContext;
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
