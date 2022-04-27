#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 tex_coord_attr;

uniform mat3 view_matrix;
uniform float depth = 0.0f;

out vec2 tex_coord;

void main()
{
    vec3 pos_transformed = view_matrix * vec3(pos, 1.0);
    gl_Position = vec4(pos_transformed.xy, depth, 1.0);
    tex_coord = tex_coord_attr;
}
