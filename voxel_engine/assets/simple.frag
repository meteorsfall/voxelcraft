#version 330 core

in vec2 uv;

out vec3 color;

uniform sampler2D my_texture;

void main() {
    color = texture(my_texture, uv).rgb;
}
