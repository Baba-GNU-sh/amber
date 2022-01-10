#version 330

/*
This geometry shader takes a lone strip (with ajacency) and draws a solid line
of a fixed width in pixels, and bevelled joins between the segments.

The thickness of the line if tunable via the uniform "line_thickness_px". This
shader also requires the width and height of the viewport passed in via the
"viewport_res_px" uniform in order to draw the line with a constant thicnkess
in px.

The shader also expects to be passed "min" and "max" values for each vertex via
the "minmax" attribute, which it displays as "error bars" above and below each
vertex in the line.

How does "lines_adjacency" work?
Let's assume we have a line strip with verticies labelled A to G like so:
A --- B --- C --- D --- E --- F --- G

This shader will be run once for each pair of verticies in the line strip, but 
the shade is given the locations of the two verticies either side of the pair as
well. In this shader we use the value of the next vertex to render the bevel 
correctly.

In the case of the example line strip above, this shader will be run twice with
the values of "gl_in" populated like so:
1. gl_in = [A, B, C, D]
2. gl_in = [B, C, D, E]
2. gl_in = [C, D, E, F]
2. gl_in = [D, E, F, G]

Note: that minmax is arranged in the same order.
*/

// We need the resolution of the viewport in order to scale the line thickness
uniform ivec2 viewport_res_px;

// This gives us the thickness of the line in pixels
uniform float line_thickness_px;

// Tell the shader we expect a line strip as primitives with adjacency info
layout (lines_adjacency) in;

// Input from the geometry shader containing minmax values
in vec2 minmax[];

// We want to output a quad (for the minmax bars) and a 5 sided shape for the 
// line segment. We can do this by drawing two triagle strip primitives, one
// containing 4 verticies, and the other containing 5.
layout (triangle_strip, max_vertices = 9) out;

// An output to the fragment shader decribing the colour
// The "flat" qualifier means the fragment shader will use flat interpolation,
// meaning the entire triangle will be the same colour (no iterpolation between
// verticies)
flat out vec4 fColor;

// Define this if you want the shader to draw the 3 triangles that make up the
// thicc line to be rendered using different colours. Otherwise it's drawn in
// one solid colour.
// #define MULTICOLOURED_LINE_SEGMENTS

// I can't beleive GLSL doesn't have a PI definition
const float PI = 3.1415926535897932384626433832795;

// Some colours, defines as const
const vec4 WHITE = vec4(0.5f, 0.5f, 0.5f, 1.0f);
const vec4 ORANGE = vec4(1.0f, 0.5f, 0.2f, 1.0f);
const vec4 GREEN = vec4(0.5f, 1.0f, 0.2f, 1.0f);
const vec4 BLUE = vec4(0.5f, 0.6f, 1.0f, 1.0f);

// We make use of z-buffering to make sure the minmax box is behind all segments
// of the line - higher values are behind lower values
// The line is rendered at a Z values of 0.0
const float MINMAX_BOX_Z = 0.5;

// Returns the normalized angle between -PI and +PI
float normalize_angle(float angle)
{
    float result = mod(angle + PI, 2.0 * PI);
    if (result <= 0.0)
        return result + PI;
    return result - PI;
}

// Returns the shortest angular distance between two angles
float shortest_angular_distance(float from, float to)
{
    return normalize_angle(to - from);
}

// Gets the angle of the line defined by two verticies
float angle_between(vec4 line_start, vec4 line_end)
{
    vec4 delta = line_start - line_end;
    return atan(delta.x, delta.y);
}

// Gets the vector which is normal to the line defined by two verticies
// The vector has a magitude of 1px on the viewport
vec4 get_line_normal(vec4 line_start, vec4 line_end)
{
    vec4 delta = line_start - line_end;
    float angle = atan(delta.x * viewport_res_px.x, delta.y * viewport_res_px.y);
    return vec4(cos(angle) / viewport_res_px.x, -sin(angle) / viewport_res_px.y, 0, 0);
}

// Draws the minmax box
void draw_minmax_box(vec4 line_start, vec4 line_end, vec2 minmax_start, vec2 minmax_end)
{
    fColor = WHITE;
    float depth = 0.1;

    gl_Position = vec4(line_start.x, minmax_start[0], MINMAX_BOX_Z, 1);
    EmitVertex();
    gl_Position = vec4(line_start.x, minmax_start[1], MINMAX_BOX_Z, 1);
    EmitVertex();
    gl_Position = vec4(line_end.x, minmax_end[0], MINMAX_BOX_Z, 1);
    EmitVertex();
    gl_Position = vec4(line_end.x, minmax_end[1], MINMAX_BOX_Z, 1);
    EmitVertex();
    EndPrimitive();
}

// Draws the a thicc boi line segment
void draw_line_segment(vec4 line_start, vec4 line_end, vec4 next_start)
{
     // Work out the angle between this point and the next point
    vec4 normal_a = line_thickness_px * get_line_normal(line_end, line_start);

    // The bulk of the line segment is drawn using two triangles to make a
    // rectangle

    // Always set the colour to orange to begin with
    fColor = ORANGE;

    // Emit the three verticies that make up the first triangle
    gl_Position = line_start + normal_a;
    EmitVertex();
    gl_Position = line_start - normal_a;
    EmitVertex();
    gl_Position = line_end + normal_a;
    EmitVertex();

#ifdef MULTICOLOURED_LINE_SEGMENTS
    // Colour the upper triangle of the line segment green
    fColor = GREEN;
#endif
    // This is the bottom half of the segment
    gl_Position = line_end - normal_a;
    EmitVertex();

#ifdef MULTICOLOURED_LINE_SEGMENTS
    // Colour the third triangle (the bevel bit) blue
    fColor = BLUE;
#endif

    float a = angle_between(line_end, line_start);
    float b = angle_between(line_end, line_end);
    float c = shortest_angular_distance(a, b);

    vec4 normal_b = line_thickness_px * get_line_normal(line_end, line_end);
    if (c > 0.0)
    {
        // The next line bends down, draw the triangle above
        gl_Position = line_end - normal_b;
        EmitVertex();
    }
    else
    {
        // The next line bends up, draw the triangle below
        gl_Position = line_end + normal_b;
        EmitVertex();
    }

    EndPrimitive();
}

void main (void)
{
    vec4 line_start = gl_in[0].gl_Position;
    vec4 line_end = gl_in[1].gl_Position;
    vec4 next_start = gl_in[2].gl_Position;

    vec2 minmax_start = minmax[0];
    vec2 minmax_end = minmax[1];

    draw_minmax_box(line_start, line_end, minmax_start, minmax_end);
    draw_line_segment(line_start, line_end, next_start);
}
