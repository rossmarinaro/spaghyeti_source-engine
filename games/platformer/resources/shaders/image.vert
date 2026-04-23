#version 330 core
precision lowp float;

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec2 a_Scale;
layout(location = 2) in float a_Rotation;
layout(location = 3) in vec2 a_UV;
layout(location = 4) in float a_TextureId;
layout(location = 5) in vec4 a_RGBA;
layout(location = 6) in vec3 a_OutlineColor;
layout(location = 7) in float a_OutlineWidth;
layout(location = 8) in float a_Whiteout;
layout(location = 9) in mat4 a_ModelView;

flat out float texID;
out float outlineWidth;
out float whiteout;
out vec3 outlineColor;
out vec2 uv;
out vec4 rgba;

uniform mat4 proj;

void main()
{
    texID = a_TextureId; 
    rgba = a_RGBA;
    uv = a_UV;
    outlineColor = a_OutlineColor;
    outlineWidth = a_OutlineWidth;
    whiteout = a_Whiteout;

    vec3 scaledPosition = vec3(a_Pos.xy * a_Scale, a_Pos.z);
    float c = cos(a_Rotation);
    float s = sin(a_Rotation);
    vec3 position = vec3(scaledPosition.x * c - scaledPosition.y * s, scaledPosition.x * s + scaledPosition.y * c, scaledPosition.z);
    
    gl_Position = proj * a_ModelView * vec4(position, 1.0);
}
