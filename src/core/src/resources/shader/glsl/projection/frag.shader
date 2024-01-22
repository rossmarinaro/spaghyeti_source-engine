#version 330 core

in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec3 spriteColor;
uniform int repeat = 1;

float alphaVal = 1.0;


void main()
{    
    
    vec2 uv = vec2(TexCoords.x, TexCoords.y)* repeat;
    color = vec4(spriteColor, alphaVal) * texture(image, uv);
}  


//---------test

//uniform vec2 subTexOffset;


// void main()
// {    


//     int width = 190;
//     int height = 250;

//     int posX = 1;
//     int posY = 4;

//     float scalarX = 1.0 / width;
//     float scalarY = 1.0 / height;

//     color = vec4(1.0) * texture(image, vec2((TexCoords.x + posX) * scalarX, (TexCoords.y * scalarY) + posY * scalarY));


//     // vec2 uv = vec2(TexCoords.x, TexCoords.y + subTexOffset.y);
//     // color = vec4(spriteColor, 1.0) * texture(image, uv);
    
// }  