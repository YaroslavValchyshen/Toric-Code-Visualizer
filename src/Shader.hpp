#ifndef Shader_hpp
#define Shader_hpp

#include <stdio.h>
#include "GraphicsAPI.hpp"
#include <iostream>
#include <fstream>
#include <string>

// Two ways to build a program:
//   - instance API: load vertex.vert / fragment.frag from a directory on disk
//     (what the flat lattice window uses -- keeps shaders editable without a
//     rebuild)
//   - static API:   compile from strings already in memory (what the torus
//     window uses -- its shader needs 3D positions and normals, which the flat
//     shader files don't provide, and it's an implementation detail of
//     TorusRenderer rather than something worth shipping as a loose file)
class Shader
{
private:
    std::string shaderPath;
    std::string changeVersion(std::string shaderContext, std::string version);

public:
    Shader(std::string shaderPath) { this->shaderPath = shaderPath; }

    unsigned int initializeShader(std::string version = "330 core");

    static unsigned int createShader(const char* source, GLuint shaderType);
    static unsigned int linkProgram(unsigned int vertexShader, unsigned int fragmentShader);
    static unsigned int createProgramFromSource(const char* vertexSource,
                                                const char* fragmentSource);
};

#endif /* Shader_hpp */
