#version 330 core

in vec3 per_vertex_color;

out vec3 color;

void main() {
    color = per_vertex_color;
}
