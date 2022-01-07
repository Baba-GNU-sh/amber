#version 330 core

out vec4 FragColor;
flat in vec3 fColor;

void main()
{
    FragColor = vec4(fColor, 1.0);
};
