#version 330 core
precision highp float;

in vec2 uv;
out vec4 color;

uniform sampler2D image;

void main (void)
{
    color = texture(image, uv);
    
    color.r += 10.0; 
    color.g += 10.0;  
    color.b += 10.0;
}


