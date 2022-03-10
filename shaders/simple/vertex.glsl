#version 330 core

layout (location = 0) in vec2 coord2d;

void main()
{
    gl_Position = vec4(coord2d, 0, 1);
}
