#version 330 core

in vec2 uv;
out vec4 color;
uniform sampler2D image;


void main()
{
    color = vec4(0., 1., 0., 1.) * texture(image, uv);  
};