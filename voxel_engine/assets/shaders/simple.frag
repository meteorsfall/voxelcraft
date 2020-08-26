#version 330 core

in vec2 uv;

out vec3 color;

uniform float break_amount;
uniform sampler2D my_texture;

void main() {
    color = (1.0 - 0.8*break_amount) * texture(my_texture, uv).rgb;
}
