#version 330 core

layout(location = 0) in vec3 vertex_position;

// Notice that the "1" here equals the "1" in glVertexAttribPointer
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in float break_amount;

uniform mat4 P;
uniform mat4 V;

// Vertex {gl_Position, per_vertex_color}
centroid out vec2 uv;
out vec2 extrapolated_uv;
out float frag_break_amount;
out float dist_to_camera;

void main() {
  vec4 cameraspace_pos = V * vec4(vertex_position, 1.0);
  dist_to_camera = length(cameraspace_pos);

  frag_break_amount = break_amount;

  uv = vertexUV;
  extrapolated_uv = vertexUV;

  gl_Position = P * cameraspace_pos;
}
