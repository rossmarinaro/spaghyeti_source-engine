#version 330 core

layout (location = 0) in vec2 vert;
layout (location = 1) in vec2 UV;

out vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 offset;


void main()
{
uv = UV;
gl_Position = projection * model * view * vec4(vert.xy + offset.xy, 0.0, 1.0);
};

