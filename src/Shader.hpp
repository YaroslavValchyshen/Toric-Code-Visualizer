
#ifndef Shader_hpp
#define Shader_hpp
#include <stdio.h>
#ifdef __EMSCRIPTEN__
  #include <GLES3/gl3.h>
#else
  #include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>

#endif /* Shader_hpp */
class Shader
{
private:
    std::string shaderPath;
    std::string changeVersion(std::string shaderContext, std::string version);
public:
    Shader(std::string shaderPath){
        this->shaderPath = shaderPath;
    }
    unsigned int initializeShader(std::string version = "330 core");
    unsigned int createShader(char* source, GLuint shaderType);
};
