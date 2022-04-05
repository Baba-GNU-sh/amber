#version 330 core

layout (location = 0) in vec2 coord2d;
layout (location = 1) in float minim;
layout (location = 2) in float maxim;

uniform mat3 view_matrix;

out vec2 minmax;

void main()
{
    vec3 coord_tx = view_matrix * vec3(coord2d, 1.0);
    gl_Position = vec4(coord_tx.xy, 0, 1);

    vec3 minim_tx = view_matrix * vec3(0.0, minim, 1.0);
    vec3 maxim_tx = view_matrix * vec3(0.0, maxim, 1.0);
    minmax = vec2(minim_tx.y, maxim_tx.y);
}
