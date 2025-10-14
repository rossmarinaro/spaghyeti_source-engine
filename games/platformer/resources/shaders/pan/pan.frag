#version 330 core
precision highp float;

in vec2 uv;
out vec4 color;

uniform sampler2D image;
uniform float time;

void main (void)
{
    color = texture(image, mod(uv + vec2(time, 0.0), 1.0));
}


