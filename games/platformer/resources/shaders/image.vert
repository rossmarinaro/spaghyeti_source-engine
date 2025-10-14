#version 330 core
precision highp float;

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 UV;

out vec2 uv;
uniform mat4 mvp;
uniform vec2 scale;

void main()
{
    uv = UV;
    gl_Position = mvp * vec4((pos.xy * scale), 0.0, 1.0);
}


