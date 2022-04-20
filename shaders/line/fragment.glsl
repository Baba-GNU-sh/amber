#version 330 core

out vec4 FragColor;
uniform vec3 line_colour;

void main()
{
    FragColor = vec4(line_colour, 1.0);
};
