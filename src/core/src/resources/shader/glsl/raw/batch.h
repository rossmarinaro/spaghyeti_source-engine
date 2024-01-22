#ifndef SHADER_BATCH
#define SHADER_BATCH


namespace Shaders {

//"#version 450 core\n";

    const char* batchQuadShader_vertex = 
                    
        "#version 330 core\n"

        "layout (location = 0) in vec3 a_Position;\n"
        "layout (location = 1) in vec4 a_Color;\n"
        "layout (location = 2) in vec2 a_TexCoord;\n"
        "layout (location = 3) in float a_TexIndex;\n"

        "out vec4 v_Color;\n"
        "out vec2 v_TexCoord;\n"
        "out float v_TexIndex;\n"

        "uniform mat4 u_ViewProj;\n"
        "uniform mat4 u_Transform;\n"

        "void main()\n"
        "{\n"           
            "v_Color = a_Color;\n" 
            "v_TexCoord = a_TexCoord;\n"
            "v_TexIndex = a_TexIndex;\n"
            "gl_Position = u_ViewProj * u_Trasform * vec4(a_Position, 1.0);\n"
        "}\n"; 


    const char* batchQuadShader_fragment =  

        "#version 330 core\n"

        "layout (location = 0) out vec4 o_Color;\n"

        "in vec4 v_Color;\n"
        "in vec2 v_TexCoord;\n"
        "in float TexIndex;\n"

        "uniform sampler2D u_Textures[32];\n" 

        "void main()\n"
        "{\n"    
            "int index = int(v_TexIndex);\n" 
            "o_Color = texture(u_Textures[index], v_TexCoord) * v_Color;\n"   
        "}\n";  

}


#endif