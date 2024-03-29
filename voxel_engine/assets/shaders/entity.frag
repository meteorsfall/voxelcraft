#version 330 core

uniform sampler2D my_texture;

in vec2 uv;

out vec4 color;

// Source: https://stackoverflow.com/questions/24388346/how-to-access-automatic-mipmap-level-in-glsl-fragment-shader-texture
// Does not take into account GL_TEXTURE_MIN_LOD/GL_TEXTURE_MAX_LOD/GL_TEXTURE_LOD_BIAS,
// nor implementation-specific flexibility allowed by OpenGL spec
float mip_map_level(in vec2 texture_coordinate) // in texel units
{
    vec2  dx_vtc        = dFdx(texture_coordinate);
    vec2  dy_vtc        = dFdy(texture_coordinate);
    float delta_max_sqr = sqrt(max(dot(dx_vtc, dx_vtc), dot(dy_vtc, dy_vtc)));
    float lod = max( 0, 0.5 * log2(delta_max_sqr) - 1.0 );
    return lod; // Thanks @Nims
}

// Source: https://vegard.wiki/w/Texture_magnification_antialiasing
//         https://www.shadertoy.com/view/ldlSzS
vec4 texturePixelAA(sampler2D tex, vec2 uv, vec2 w, vec2 texsize, float mm) {    
    return texture(tex, (floor(uv)+0.5+clamp((fract(uv)-0.5+w)/w,0.,1.)) / texsize, mm);    
}

vec4 get_texture_color() {
    float mm = mip_map_level(uv * textureSize(my_texture, 0));
    float lower_mm = min(floor(mm), 2.0);
    float higher_mm = min(ceil(mm), 2.0);

    vec2 lower_texture_size = textureSize(my_texture, int(lower_mm + 0.5));
    vec2 higher_texture_size = textureSize(my_texture, int(higher_mm + 0.5));
    vec2 lower_adjusted_uv = uv * lower_texture_size.x + vec2(-0.5, -0.5);
    vec2 higher_adjusted_uv = uv * higher_texture_size.x + vec2(-0.5, -0.5);

    vec2 lower_w = fwidth(lower_adjusted_uv);
    vec2 higher_w = fwidth(higher_adjusted_uv);

    vec4 lower_texture_color = texturePixelAA(my_texture, lower_adjusted_uv, lower_w, lower_texture_size, lower_mm);
    vec4 higher_texture_color = texturePixelAA(my_texture, higher_adjusted_uv, higher_w, higher_texture_size, higher_mm);
    
    vec4 texture_color = mix(lower_texture_color, higher_texture_color, fract(mm));
    return texture_color;
}

void main() {
    color = get_texture_color();
}
