#version 330 core

layout(location = 0) in vec3 vertex_position_straight_from_bender;

// Notice that the "1" here equals the "1" in glVertexAttribPointer
layout(location = 1) in vec3 vertexColor;

uniform mat4 MVP;

// Vertex {gl_Position, per_vertex_color}
out vec3 per_vertex_color;

void main(){  
  per_vertex_color = 0.5 + 0.5*vertex_position_straight_from_bender;
  gl_Position = MVP * vec4(vertex_position_straight_from_bender, 1.0);
}
