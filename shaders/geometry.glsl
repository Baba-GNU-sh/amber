#version 330

// Line thickness in X and Y normalized screen space
uniform ivec2 viewport_res;
uniform float line_thickness;

// Lines have 2 vertices per-primitive, the geometry shader is run once per primitive
layout (lines_adjacency) in;

// We want to output a box per line primitive which requies two trianges - with a line strip we can do this with 4 verticies
// We could probably also do this with a quad
layout (triangle_strip, max_vertices = 5) out;

// An output to the fragment shader - the flat qualifier signifies that this output won't be interpolated over the entire primitive's output
flat out vec3 fColor;

// I can't beleive GLSL doesn't have a PI definition
#define M_PI 3.1415926535897932384626433832795

#define JOIN_NONE 0
#define JOIN_BEVEL 1

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

void main (void)
{
    /*
    Lines look like this:
    o --- A === B --- C
    Where === signifies the line we are actually drawing.
    o (unused) and C are part of the previous and next lines respectively
    */
    vec4 point_a = gl_in[1].gl_Position;
    vec4 point_b = gl_in[2].gl_Position;
    vec4 point_c = gl_in[3].gl_Position;

    // Some colours for the triangles
    const vec3 orange = vec3(1.0f, 0.5f, 0.2f);
    const vec3 green = vec3(0.5f, 1.0f, 0.2f);
    const vec3 blue = vec3(0.5f, 0.6f, 1.0f);

    // Work out the angle between this point and the next point
    vec4 normal_a = line_thickness * get_line_normal(point_b, point_a);

    // Emit the 3 corners of the triangle that defines the first half of the line segment line
    // Colour the first triangle orange
    fColor = orange;
    gl_Position = point_a + normal_a;
    EmitVertex();
    gl_Position = point_a - normal_a;
    EmitVertex();
    gl_Position = point_b + normal_a;
    EmitVertex();

    // Colour the second triangle green - the "flat" qualifier on the output means the triangle is entirely this colour
    // This is the bottom half of the segment
    // fColor = green;
    gl_Position = point_b - normal_a;
    EmitVertex();

    // Colour the second triangle green - the "flat" qualifier on the output means the triangle is entirely this colour
    // fColor = blue;

    float a = angle_between(point_b, point_a);
    float b = angle_between(point_c, point_b);
    float c = shortest_angular_distance(a, b);

    vec4 normal_b = line_thickness * get_line_normal(point_c, point_b);
    if (c > 0.0)
    {
        gl_Position = point_b - normal_b;
        EmitVertex();
    }
    else
    {
        gl_Position = point_b + normal_b;
        EmitVertex();
    }

    EndPrimitive();
}
