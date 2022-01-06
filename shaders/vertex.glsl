#version 330 core

attribute vec2 coord2d;
uniform vec2 scale;
uniform vec2 offset;

void main()
{
    vec2 transformed = (coord2d * scale) + offset;
    gl_Position = vec4(transformed, 0, 1);
}
