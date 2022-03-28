#version 330 core

layout (location = 0) in vec2 coord2d;

uniform mat3 view_matrix;

void main()
{
    vec3 txformed_coord = view_matrix * vec3(coord2d, 1.0);
    gl_Position = vec4(txformed_coord.xy, -0.5, 1);
}
