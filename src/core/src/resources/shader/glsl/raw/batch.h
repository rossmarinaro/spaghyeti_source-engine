#ifndef SHADER_BATCH
#define SHADER_BATCH


namespace Shaders {

    static const char* batchQuadShader_vertex = 
                    
        "#version 330 core\n"

        "layout (location = 0) in vec2 vert;\n"
        "layout (location = 1) in vec4 UV;\n"

        "uniform vec2 offset;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"

        "void main()\n"
        "{\n"           
            "uv = UV;\n"
            "gl_Position = projection * model * view * vec4(vert.xy + offset.xy, 0.0, 1.0);\n"
        "}\n"; 


    static const char* batchQuadShader_fragment =  

        "#version 330 core\n"

        "in vec2 uv;\n"
        "out vec4 color;\n"

        "uniform sampler2D images[32];\n" 
        "uniform vec3 tint;\n"
        "uniform float alphaVal;\n"

        "void main()\n"
        "{\n"    
            "int index = int(uv);\n" 
            "color = texture(images[index], uv) * tint * alphaVal;\n"   
        "}\n";  

}


#endif