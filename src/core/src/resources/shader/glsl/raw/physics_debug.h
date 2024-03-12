#pragma once


namespace Shaders {

    //points

    static const char* point_vertex = \

        "#version 330\n"

        "uniform mat4 projection;\n"
        "uniform vec2 offset;\n"
        "uniform mat4 view;\n"

        "layout(location = 0) in vec2 v_position;\n"
        "layout(location = 1) in vec4 v_color;\n"
        "layout(location = 2) in float v_size;\n"

        "out vec4 f_color;\n"

        "void main(void)\n"  
        "{\n"
        "	f_color = v_color;\n"
        "	gl_Position = projection * view * vec4(v_position + offset.xy, 0.0f, 1.0f);\n" 
        "   gl_PointSize = v_size;\n"
        "}\n";

    static const char* point_fragment = \

        "#version 330\n"

        "in vec4 f_color;\n"

        "out vec4 color;\n"

        "void main(void)\n" 
        "{\n"
        "	color = f_color;\n"
        "}\n";

    //lines

    static const char* line_vertex = \

        "#version 330\n"

        "uniform mat4 projection;\n"
        "uniform vec2 offset;\n"
        "uniform mat4 view;\n"

        "layout(location = 0) in vec2 v_position;\n"
        "layout(location = 1) in vec4 v_color;\n"

        "out vec4 f_color;\n"

        "void main(void)\n"
        "{\n"
        "	f_color = v_color;\n"
        "	gl_Position = projection * view * vec4(v_position + offset.xy, 0.0f, 1.0f);\n"  
        "}\n";

    static const char* line_fragment = \

        "#version 330\n"

        "in vec4 f_color;\n"

        "out vec4 color;\n"

        "void main(void)\n"
        "{\n"
        "	color = f_color;\n"
        "}\n";

    //triangle

    static const char* triangle_vertex = \

        "#version 330\n"

        "uniform mat4 projection;\n" 
        "uniform vec2 offset;\n"
        "uniform mat4 view;\n"
        
        "layout(location = 0) in vec2 v_position;\n"
        "layout(location = 1) in vec4 v_color;\n"

        "out vec4 f_color;\n"

        "void main(void)\n"
        "{\n"
        "	f_color = v_color;\n"         
        "	gl_Position = projection * view * vec4(v_position + offset.xy, 0.0f, 1.0f);\n"
        "}\n";

    static const char* triangle_fragment = \

        "#version 330\n"

        "in vec4 f_color;\n"

        "out vec4 color;\n"

        "void main(void)\n"
        "{\n"
        "	color = f_color;\n"
        "}\n";

}