#version 330

// Line thickness in X and Y normalized screen space
uniform vec2 line_thickness;

// Lines have 2 vertices per-primitive, the geometry shader is run once per primitive
layout (lines) in;

// We want to output a box per line primitive which requies two trianges - with a line strip we can do this with 4 verticies
// We could probably also do this with a quad
layout (triangle_strip, max_vertices = 4) out;

flat out vec3 fColor;

vec2 get_line_normal(vec4 line_start, vec4 line_end)
{
    // Work out the angle between this point and the next point
    vec4 delta = line_start - line_end;
    float angle = atan(delta.x, delta.y);
    return vec2(line_thickness.x * cos(angle), line_thickness.y * -sin(angle));
}

void main (void)
{
    // Work out the angle between this point and the next point
    vec2 offset = get_line_normal(gl_in[1].gl_Position, gl_in[0].gl_Position);

    // Emit the 4 corners of the rectangle that defines the line
    // Colour the first triangle orange
    fColor = vec3(1.0f, 0.5f, 0.2f);
    gl_Position = gl_in[0].gl_Position + vec4(offset, 0.0, 0.0);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(-offset, 0.0, 0.0);
    EmitVertex();
    gl_Position = gl_in[1].gl_Position + vec4(offset, 0.0, 0.0);
    EmitVertex();

    // Colour the second triangle green - the "flat" qualifier on the output means the triangle is entirely this colour
    fColor = vec3(0.5f, 1.0f, 0.2f);
    gl_Position = gl_in[1].gl_Position + vec4(-offset, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}
