#version 330 core

layout(location = 0) in vec3 vertex_position_straight_from_bender;

// Notice that the "1" here equals the "1" in glVertexAttribPointer
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in float break_amount;

uniform mat4 PV;
uniform vec3 M;

// Vertex {gl_Position, per_vertex_color}
centroid out vec2 uv;
out vec2 extrapolated_uv;
out float frag_break_amount;

void main(){
  frag_break_amount = break_amount;
  uv = vertexUV;
  extrapolated_uv = vertexUV;
  vec3 pos = vertex_position_straight_from_bender;
  gl_Position = PV * vec4(M + pos, 1.0);
}
