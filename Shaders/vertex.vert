#version 330 core
layout (location = 0) in vec2 aPos;


uniform mat4 u_MVP;

void main(){
    gl_Position = u_MVP * vec4(aPos.x, aPos.y, 0.0, 1.0);

    gl_PointSize = 20.0;
}
