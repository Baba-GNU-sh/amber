#version 330

// Line thickness in X and Y normalized screen space
uniform ivec2 viewport_res;
uniform float line_thickness;

// Lines have 2 vertices per-primitive, the geometry shader is run once per primitive
layout (lines_adjacency) in;
in vec2 minmax_out[];

// We want to output a box per line primitive which requies two trianges - with a line strip we can do this with 4 verticies
// We could probably also do this with a quad
layout (triangle_strip, max_vertices = 9) out;


// An output to the fragment shader - the flat qualifier signifies that this output won't be interpolated over the entire primitive's output
flat out vec4 fColor;

// I can't beleive GLSL doesn't have a PI definition
const float M_PI = 3.1415926535897932384626433832795;
const vec4 WHITE = vec4(0.5f, 0.5f, 0.5f, 1.0f);
const vec4 ORANGE = vec4(1.0f, 0.5f, 0.2f, 1.0f);
const vec4 GREEN = vec4(0.5f, 1.0f, 0.2f, 1.0f);
const vec4 BLUE = vec4(0.5f, 0.6f, 1.0f, 1.0f);
const float MINMAX_BOX_DEPTH = 0.5;

// Returns the normalized angle between -Pi and +Pi
float normalize_angle(float angle)
{
    float result = mod(angle + M_PI, 2.0*M_PI);
    if(result <= 0.0) return result + M_PI;
    return result - M_PI;
}

// Returns the shortest angular distance between two angles
float shortest_angular_distance(float from, float to)
{
    return normalize_angle(to-from);
}

// Gets the angle of a line defined by two segments
float angle_between(vec4 line_start, vec4 line_end)
{
    vec4 delta = line_start - line_end;
    return atan(delta.x, delta.y);
}

// Gets the vector which is normal to the line defined by two points with magnitude 1px on the viewport
vec4 get_line_normal(vec4 line_start, vec4 line_end)
{
    vec4 delta = line_start - line_end;
    float angle = atan(delta.x * viewport_res.x, delta.y * viewport_res.y);
    return vec4(cos(angle) / viewport_res.x, -sin(angle) / viewport_res.y, 0, 0);
}

void draw_minmax_box(vec4 line_start, vec4 line_end, vec2 minmax_start, vec2 minmax_end)
{
    fColor = WHITE;
    float depth = 0.1;

    gl_Position = vec4(line_start.x, minmax_start[0], MINMAX_BOX_DEPTH, 1);
    EmitVertex();
    gl_Position = vec4(line_start.x, minmax_start[1], MINMAX_BOX_DEPTH, 1);
    EmitVertex();
    gl_Position = vec4(line_end.x, minmax_end[0], MINMAX_BOX_DEPTH, 1);
    EmitVertex();
    gl_Position = vec4(line_end.x, minmax_end[1], MINMAX_BOX_DEPTH, 1);
    EmitVertex();
    EndPrimitive();
}

void draw_line_segment(vec4 line_start, vec4 line_end, vec4 next_start)
{
     // Work out the angle between this point and the next point
    vec4 normal_a = line_thickness * get_line_normal(line_end, line_start);

    // Emit the 3 corners of the triangle that defines the first half of the line segment line
    // Colour the first triangle orange
    fColor = ORANGE;
    gl_Position = line_start + normal_a;
    EmitVertex();
    gl_Position = line_start - normal_a;
    EmitVertex();
    gl_Position = line_end + normal_a;
    EmitVertex();

    // Colour the second triangle green - the "flat" qualifier on the output means the triangle is entirely this colour
    // This is the bottom half of the segment
    // fColor = green;
    gl_Position = line_end - normal_a;
    EmitVertex();

    // Colour the second triangle green - the "flat" qualifier on the output means the triangle is entirely this colour
    // fColor = blue;

    float a = angle_between(line_end, line_start);
    float b = angle_between(line_end, line_end);
    float c = shortest_angular_distance(a, b);

    vec4 normal_b = line_thickness * get_line_normal(line_end, line_end);
    if (c > 0.0)
    {
        gl_Position = line_end - normal_b;
        EmitVertex();
    }
    else
    {
        gl_Position = line_end + normal_b;
        EmitVertex();
    }

    EndPrimitive();
}

void main (void)
{
    /*
    This shader expects to be given line segments to work with, which we can use to draw lines and minmax boxes.
    We are also given the previous end line segment, and start of the next line segment because we are using lines_adjacent.
    
    Let's assume we have a line strip who's verticies are labelled A, B, C, and D like so:
    A --- B --- C --- D --- E

    This shader will be run once for each line segment (pair of verticies) in the line strip, and will be given 4 verticies at a time.
    For example in the example aboe we shall be callec twice each time with 4 verticies in the gl_in array arranges like so:
    1. gl_in = [A, B, C, D]
    2. gl_in = [B, C, D, E]

    > Note that minmax_out is arranged in the same way.
    */
    vec4 line_start = gl_in[0].gl_Position;
    vec4 line_end = gl_in[1].gl_Position;
    vec4 next_start = gl_in[2].gl_Position;

    vec2 minmax_start = minmax_out[0];
    vec2 minmax_end = minmax_out[1];

    draw_minmax_box(line_start, line_end, minmax_start, minmax_end);
    draw_line_segment(line_start, line_end, next_start);
}
