#version 330 core
precision highp float;

in vec2 uv;
out vec4 color;

uniform float time;
uniform sampler2D image;

vec3 hsv2rgb (vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main (void)
{
    vec2 gg = gl_FragCoord.xy;
    vec2 resolution = vec2(60.0, 60.0);
    float bins = 10.0;
    vec2 pos = (gg / resolution.xy);

    float bin = floor(pos.x * bins);

    color = vec4(hsv2rgb(vec3(bin / (bins * time), bin * bins, bin / (bins * time))), 0.75) * texture(image, uv);
}
