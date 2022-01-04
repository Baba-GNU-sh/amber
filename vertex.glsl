#version 330 core
attribute vec2 coord2d;
uniform float scale_x;

void main()
{
    gl_Position = vec4((coord2d.x * scale_x) - 1.0, coord2d.y, 0, 1);
}
