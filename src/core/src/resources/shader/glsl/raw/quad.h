#pragma once


namespace Shaders {

    //------------- sprite

    #ifdef __EMSCRIPTEN__

        static const char* spriteQuadShader_vertex = 
                        
            "#version 300 es\n"

            "precision mediump float;\n"

            "layout (location = 0) in vec2 vert;\n"
            "layout (location = 1) in vec2 UV;\n"

            "out vec2 uv;\n"

            "uniform mat4 model;\n"
            "uniform mat4 projection;\n"

            "void main()\n"
            "{\n"           
                "uv = UV;\n"
                "gl_Position = projection * model * vec4(vert.xy, 0.0, 1.0);\n"
            "}\n"; 


        static const char* spriteQuadShader_fragment =  

            "#version 300 es\n"

            "precision mediump float;\n"

            "in vec2 uv;\n"
            "out vec4 color;\n"

            "uniform sampler2D image;\n" 
            "uniform vec3 tint;\n"
            "uniform float alphaVal;\n"

            "void main()\n"
            "{\n"    
                "color = vec4(tint, alphaVal) * texture(image, uv);\n"   
            "}\n"; 

        //-------------- graphics

        static const char* debugGraphicShader_vertex = 

            "#version 300 es\n"
            "precision mediump float;\n"

            "layout (location = 0) in vec2 pos;\n"

            "uniform mat4 model;\n"
            "uniform mat4 projection;\n"

            "void main()\n"
            "{\n"
            "   gl_Position = projection * model * vec4(pos.xy, 0.0, 1.0);\n"
            "}\0";

        static const char* debugGraphicShader_fragment = 

            "#version 300 es\n"

            "precision mediump float;\n"
            
            "out vec4 FragColor;\n"
            "uniform vec3 tint;\n"

            "void main()\n"
            "{\n"
            "   FragColor = vec4(tint, 1.0f);\n"
            "}\n\0";

    #else

        
        static const char* spriteQuadShader_vertex = 
                        
            "#version 330 core\n"

            "layout (location = 0) in vec2 vert;\n"
            "layout (location = 1) in vec2 UV;\n"

            "out vec2 uv;\n"

            "uniform mat4 model;\n"
            "uniform mat4 projection;\n"

            "void main()\n"
            "{\n"           
                "uv = UV;\n"
                "gl_Position = projection * model * vec4(vert.xy, 0.0, 1.0);\n" 
            "}\n"; 


        static const char* spriteQuadShader_fragment =  

            "#version 330 core\n"

            "in vec2 uv;\n"
            "out vec4 color;\n"

            "uniform sampler2D image;\n" 
            "uniform vec3 tint;\n"
            "uniform float alphaVal;\n"
            "uniform int repeat;\n"

            "void main()\n"
            "{\n"    

                "color = vec4(tint, alphaVal) * texture(image, uv * repeat);\n"   
            "}\n"; 

        //-------------- graphics

        static const char* debugGraphicShader_vertex = 

            "#version 330 core\n"

            "layout (location = 0) in vec2 pos;\n"
            
            "uniform mat4 model;\n"
            "uniform mat4 projection;\n"
 
            "void main()\n"
            "{\n"
            "   gl_Position = projection * model * vec4(pos.xy, 0.0, 1.0);\n"
            "}\0";

        static const char* debugGraphicShader_fragment = 

            "#version 330 core\n"
            
            "out vec4 FragColor;\n"
            "uniform vec3 tint;\n"

            "void main()\n"
            "{\n"
            "   FragColor = vec4(tint, 1.0f);\n"
            "}\n\0";

    #endif


    
}

