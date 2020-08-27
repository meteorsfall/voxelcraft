#version 330 core

in vec2 TexCoords;
out vec4 color;

uniform sampler2D textTexture;
uniform ivec3 textColor;

void main()
{
    color = vec4(vec3(textColor) / 255, texture(textTexture, TexCoords).r);
}
