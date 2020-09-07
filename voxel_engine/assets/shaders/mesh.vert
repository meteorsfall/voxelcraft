#version 330 core

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_uv;

uniform mat4 PV;
uniform mat4 M;

// Vertex {gl_Position, per_vertex_color}
out vec2 uv;

void main(){
  uv = vertex_uv;
  gl_Position = (PV * M) * vec4(vertex_position, 1.0);
}
