#version 330 core

layout (location = 0) in vec2 coord2d;
layout (location = 1) in vec2 minmax;

uniform vec2 scale;
uniform vec2 offset;

out vec2 minmax_out;

void main()
{
    vec2 transformed = (coord2d * scale) + offset;
    gl_Position = vec4(transformed, 0, 1);
    minmax_out = (minmax * scale) + offset.y;
}
