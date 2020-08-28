#version 330 core

in vec2 uv;
in float frag_break_amount;

out vec3 color;

uniform sampler2D my_texture;

void main() {
    vec3 texture_color = texture(my_texture, uv).rgb;
    color = (1.0 - 0.8*frag_break_amount) * texture_color;
}
