#version 330 core
out vec4 FragColor;
  
in vec2 TexCoord;

uniform sampler2D tex;
uniform vec3 glyph_colour;

void main()
{
    vec4 color = texture(tex, TexCoord);
    FragColor = vec4(glyph_colour, color.r);
}
