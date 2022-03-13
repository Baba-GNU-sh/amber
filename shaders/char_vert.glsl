#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat3 view_matrix;

out vec2 TexCoord;

void main()
{
    vec3 txformed_coord = view_matrix * vec3(aPos, 1.0);
    gl_Position = vec4(txformed_coord.xy, 0, 1);
    TexCoord = aTexCoord;
}
