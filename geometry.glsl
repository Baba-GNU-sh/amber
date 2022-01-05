#version 330

uniform vec2 line_thickness;

// 4 vertices per-primitive -- 2 for the line (1,2) and 2 for adjacency (0,3)
layout (lines) in;

// Standard fare for drawing lines
layout (triangle_strip, max_vertices = 4) out;

void main (void) {

  // Work out the angle between this point and the next point
  float dx = gl_in[1].gl_Position.x - gl_in[0].gl_Position.x;
  float dy = gl_in[1].gl_Position.y - gl_in[0].gl_Position.y;
  float angle = atan(dx, dy) + 3.14/2;

  // TL
  gl_Position = gl_in[0].gl_Position + vec4(line_thickness[0] * sin(angle), line_thickness[1] * cos(angle), 0.0, 0.0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(-line_thickness[0] * sin(angle), -line_thickness[1] * cos(angle), 0.0, 0.0);
  EmitVertex();

  gl_Position = gl_in[1].gl_Position + vec4(line_thickness[0] * sin(angle), line_thickness[1] * cos(angle), 0.0, 0.0);
  EmitVertex();

  gl_Position = gl_in[1].gl_Position + vec4(-line_thickness[0] * sin(angle), -line_thickness[1] * cos(angle), 0.0, 0.0);
  EmitVertex();

  EndPrimitive();
}
