#version 330 core
out vec4 FragColor;

in vec2 tex_coord;

uniform sampler2D tex;
uniform vec3 tint_colour;

void main()
{
    // Sample the texture first
    vec4 tex = texture(tex, tex_coord);

    // Discard early if the this pixel is going to be transparent
    // This allows us to display simple transparency even when alpha blending is disabled
    if (tex.a == 0.0)
    {
        discard;
    }

    // Tint the texure with the specified tint
    vec3 tinted_tex = tex.rgb * tint_colour;

    // Build the output colour from the tinted texture and the alpha channel of the original texture
    FragColor = vec4(tinted_tex, tex.a);
}
