#version 330 core

in vec2 uv;

out vec3 color;

uniform sampler2D myTextureSampler;

void main() {
    color = texture(myTextureSampler, uv).rgb;
}
