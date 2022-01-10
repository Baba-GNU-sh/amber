#version 330 core

layout (location = 0) in vec2 coord2d;
layout (location = 1) in vec2 minmax_in;

uniform vec2 scale;
uniform vec2 offset;

out vec2 minmax;

void main()
{
    vec2 transformed = (coord2d * scale) + offset;
    gl_Position = vec4(transformed, 0, 1);
    minmax = (minmax_in * scale) + offset.y;
}
