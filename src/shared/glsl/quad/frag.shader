#version 330 core

in vec2 uv;
out vec4 color;

uniform sampler2D image;
uniform vec3 spriteColor;
uniform float alphaVal;
uniform int repeat;

void main()
{

   color = vec4(spriteColor, alphaVal) * texture(image, uv * repeat);  
};  