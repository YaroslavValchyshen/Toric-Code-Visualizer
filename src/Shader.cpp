#include "Shader.hpp"
#include "FileManager.hpp"
#include <iostream>

unsigned int Shader::initializeShader(std::string version){
    std::string vertexContent   = changeVersion(FileManager::readFile(shaderPath + "vertex.vert"), version);
    std::string fragmentContent = changeVersion(FileManager::readFile(shaderPath + "fragment.frag"), version);

    unsigned int vertexShader   = Shader::createShader(vertexContent.c_str(), GL_VERTEX_SHADER);
    unsigned int fragmentShader = Shader::createShader(fragmentContent.c_str(), GL_FRAGMENT_SHADER);

    return Shader::linkProgram(vertexShader, fragmentShader);
}

std::string Shader::changeVersion(std::string shaderContext, std::string version){
    const size_t versionPosition = 9;
    size_t first_newline = shaderContext.find('\n');

    if(first_newline != std::string::npos){
        shaderContext.replace(versionPosition, first_newline - versionPosition, version + "\n");
    }

    return shaderContext;
}

unsigned int Shader::createShader(const char* source, GLuint shaderType){
    unsigned int shader = glCreateShader(shaderType);

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(success == GL_FALSE){
        GLchar info[1024];
        glGetShaderInfoLog(shader, sizeof(info), NULL, info);
        std::cerr << "Shader compile error: " << info << std::endl;
    }

    return shader;
}

unsigned int Shader::linkProgram(unsigned int vertexShader, unsigned int fragmentShader){
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if(linked == GL_FALSE){
        GLchar info[1024];
        glGetProgramInfoLog(program, sizeof(info), NULL, info);
        std::cerr << "Shader link error: " << info << std::endl;
    }

    // The program holds its own reference once linked.
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

unsigned int Shader::createProgramFromSource(const char* vertexSource, const char* fragmentSource){
    return Shader::linkProgram(Shader::createShader(vertexSource, GL_VERTEX_SHADER),
                               Shader::createShader(fragmentSource, GL_FRAGMENT_SHADER));
}
