#version 330 core

in vec2 uv;

out vec3 color;

uniform float break_amount;
uniform sampler2D my_texture;

void main() {
    vec3 standard_texture = texture(my_texture, uv).rgb;
    float len = length(standard_texture);
    if (len > 0.96) {
        color = (1.0 - 0.8*break_amount) * texture(my_texture, vec2(0.5, 0.5)).rgb;
    } else {
        color = (1.0 - 0.8*break_amount) * standard_texture;
    }
}
