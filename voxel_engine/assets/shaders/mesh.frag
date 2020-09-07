#version 330 core

uniform sampler2D my_texture;

in vec2 uv;

out vec4 color;

void main() {
    color = texture(my_texture, uv);
}
