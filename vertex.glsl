#version 330 core
attribute vec2 coord2d;
uniform vec2 scale;
uniform vec2 offset;

void main()
{
    gl_Position = vec4((coord2d.x * scale.x) + offset.x, (coord2d.y * scale.y) + offset.y, 0, 1);
}
