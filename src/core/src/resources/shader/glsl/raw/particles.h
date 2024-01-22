#pragma once


namespace Shaders {

    const char* particleShader_vertex = 
                
    "#version 330 core\n"

    "layout (location = 0) in vec4 Vertex;\n"

    "out vec2 TexCoords;\n"
    "out vec4 ParticleColor;\n"

    "uniform vec4 color;\n"
    "uniform mat4 projection;\n"
    "uniform vec2 offset;\n"

    "void main()\n"
    "{\n"           
        "float scale = 10.0f;\n"
        "TexCoords = vertex.zw;\n"
        "ParticleColor = color;\n"
        "gl_Position = projection * vec4((vertex.xy * scale) + offset, 0.0, 1.0);\n"
    "}\n"; 

    const char* particleShader_fragment = 
                
    "#version 330 core\n"

    "in vec2 TexCoords;\n"
    "in vec4 ParticleColor;\n"
    "out vec4 color;\n"

    "uniform sampler2D sprite;\n"

    "void main()\n"
    "{\n"           
        "color = (texture(sprite, TexCoords) * ParticleColor);\n"
    "}\n"; 

}