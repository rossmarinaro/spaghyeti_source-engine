#version 330 core

 layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>


out vec2 TexCoords;


uniform mat4 model;
uniform mat4 projection;


void main()
{

    TexCoords = vertex.zw;

    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);

} 


//--------test

// layout (location = 0) in vec3 inPos;
// layout (location = 1) in vec2 inTexCoord;

// out vec2 texCoord;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;

// void main(){
//     gl_Position =  projection * view * vec4(inPos,1.0f);
//     texCoord = inTexCoord;
// }  


//------------test

// uniform mat4 uMVPMatrix;
// attribute vec4 aPosition;
// attribute vec2 aTextureCoord;
// varying vec2 vTextureCoord;
// varying vec2 vTextureCoordOffset;
// void main() {
//     // offset and texSize will be defined as attributes later.
//     // They're defined here for test purposes only
//     vec2 offset = vec2(128.0, 64.0);
//     vec2 texSize = vec2(256.0, 256.0);

//     float u = offset.x / texSize.x;
//     float v = offset.y / texSize.y;
//     vTextureCoordOffset = vec2(u, v);
//     vTextureCoord = aTextureCoord;
//     gl_Position = uMVPMatrix * aPosition;
// }


