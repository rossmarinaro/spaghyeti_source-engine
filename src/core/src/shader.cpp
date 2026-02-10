#include <fstream>
#include <sstream>

#include "../../../build/sdk/include/app.h"
#include "../../vendors/glm/gtc/type_ptr.hpp"
#include "../../shared/renderer.h"
#include "debug.h"

using namespace Graphics;


void Shader::Delete() {
    glDeleteProgram(ID);
}

//-------------------------------


const Shader& Shader::Get(const std::string& key) 
{
    if (System::Application::resources->shaders.find(key) == System::Application::resources->shaders.end()) {
        LOG("Shader: shader of key: " + key + " not loaded, defaulting to sprite shader.");
    }

    return System::Application::resources->shaders[
        System::Application::resources->shaders.find(key) != System::Application::resources->shaders.end() ? key : "sprite"];
}


//-------------------------------


const Shader& Shader::Use() {
    glUseProgram(this->ID);
    return *this;
}


//--------------------------- init base shaders


void Shader::InitBaseShaders()
{

    //quad (sprite)

    static constexpr const char* spriteQuadShader_vertex = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "layout(location = 0) in vec2 vert;\n"
        "layout(location = 1) in vec2 UV;\n"
        "layout(location = 2) in float TextureId;\n" 
        "flat out float texID;\n"

        "out vec2 uv;\n"

        "uniform vec2 scale;\n"
        "uniform mat4 mvp;\n"

        "void main()\n"
        "{\n" "texID = TextureId;\n"
            "uv = UV;\n"
            "gl_Position = mvp * vec4((vert.xy * scale), 0.0, 1.0);\n"
        "}";


    //--------------------------------------------


    static constexpr const char* spriteQuadShader_fragment = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "flat in float texID;\n"
        "in vec2 uv;\n"
        "out vec4 color;\n"

        "uniform sampler2D images[3];\n"
        "uniform vec3 tint;\n"
        "uniform float alphaVal;\n"
        "uniform int whiteout;\n"
        "uniform int repeat;\n"

        "void main()\n"
        "{ \n"
            "color = vec4(tint, alphaVal) * texture(images[int(texID)], vec2(uv.x * float(repeat), uv.y * float(repeat))); \n"
            "if (whiteout == 1) {\n"
            "   color.r += 10.0;\n" 
            "   color.g += 10.0;\n"
            "   color.b += 10.0;\n"
            "}\n"
            //"if (color.r == 1.0 && color.b == 1.0) discard;" magenta background only
        "}";


    //--------------------------------------------


    static constexpr const char* debugGraphicShader_vertex = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "layout(location = 0) in vec2 pos;\n"

        "uniform mat4 mvp;\n"

        "void main()\n"
        "{\n"
            "gl_Position = mvp * vec4(pos.xy, 0.0, 1.0);\n"
        "}";


    //--------------------------------------------


    static constexpr const char* debugGraphicShader_fragment = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "out vec4 color;\n"
        "uniform vec3 tint;\n"
        "uniform float alphaVal;\n"

        "void main()\n"
        "{\n"
            "color = vec4(tint, alphaVal);\n"
        "}";


    //--------------------------------------------


    static constexpr const char* textVertex = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "layout(location = 0) in vec4 vertex;\n"
        "out vec2 uv;\n"

        "uniform mat4 mvp;\n"

        "void main(){\n"
            "gl_Position = mvp * vec4(vertex.xy, 0.0, 1.0);\n"
            "uv = vertex.zw;\n"
        "}";


    //--------------------------------------------


    static constexpr const char* textFragment = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "in vec2 uv;\n"
        "out vec4 color;\n"

        "uniform sampler2D image;\n"
        "uniform vec3 textColor;\n"
        "uniform float alphaVal;\n"

        "void main()\n"
        "{ \n"
            "vec4 sampled = vec4(1.0, 1.0, 1.0, texture(image, uv).r);\n"
            "color = vec4(textColor, alphaVal) * sampled; \n"
        "}";


    //--------------------------------------------


    static constexpr const char* textOutlineFragment = \

         #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "uniform sampler2D image;\n"  
        "uniform float outlineWidth;\n" 
        "uniform vec3 outlineColor;\n" 
        "uniform float alphaVal;\n" 
        "uniform float characterWidth;\n" 

        "out vec4 color;\n"
        "in vec2 uv;\n"

        "void main() {\n"

            "vec4 c = vec4(1.0, 1.0, 1.0, texture(image, uv));\n"

            "if (c.a == 0.0) {\n"

                "ivec2 texSize2d = textureSize(image, 0);\n"
                "float texSize = characterWidth;\n" 
                "float texelSize = 1.0 / texSize;\n"
                "vec2 size = vec2(texelSize * outlineWidth, texelSize * outlineWidth);\n"
                "float outline = vec4(1.0, 1.0, 1.0, texture(image, uv + vec2(-size.x, 0.0))).a;\n"
 
                "outline += vec4(1.0, 1.0, 1.0, texture(image, uv + vec2(0.0, size.y))).a;\n"
                "outline += vec4(1.0, 1.0, 1.0, texture(image, uv + vec2(size.x, 0.0))).a;\n"
                "outline += vec4(1.0, 1.0, 1.0, texture(image, uv + vec2(0.0, -size.y))).a;\n"
                "outline += vec4(1.0, 1.0, 1.0, texture(image, uv + vec2(-size.x, size.y))).a;\n"
                "outline += vec4(1.0, 1.0, 1.0, texture(image, uv + vec2(size.x, size.y))).a;\n"
                "outline += vec4(1.0, 1.0, 1.0, texture(image, uv + vec2(-size.x, size.y))).a;\n"
                "outline += vec4(1.0, 1.0, 1.0, texture(image, uv + vec2(size.x, -size.y))).a;\n"
                "outline = min(outline, 1.0);\n"

                "vec4 c = vec4(1.0, 1.0, 1.0, texture(image, uv));\n"
                "color = mix(c, vec4(outlineColor, alphaVal), outline - c.a);\n"

            "}\n"
            "else\n"
               "color = vec4(1.0, 1.0, 1.0, texture(image, uv));\n"
        "}";


    //--------------------------------------------

    static constexpr const char* spriteOutlineFragment = \

         #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "uniform sampler2D image;\n"  
        "uniform float outlineWidth;\n" 
        "uniform vec3 outlineColor;\n" 
        "uniform float alphaVal;\n" 
        "uniform int whiteout;\n"

        "out vec4 color;\n"
        "in vec2 uv;\n"

        "void main() {\n"

            "vec4 c = texture(image, uv);\n"

            "if (c.a == 0.0) {\n"

                "ivec2 texSize2d = textureSize(image, 0);\n"
                "float texSize = float(texSize2d.x);\n"
                "float texelSize = 1.0 / texSize;\n"
                "vec2 size = vec2(texelSize * outlineWidth, texelSize * outlineWidth);\n"
                "float outline = texture(image, uv + vec2(-size.x, 0.0)).a;\n"

                "outline += texture(image, uv + vec2(0.0, size.y)).a;\n"
                "outline += texture(image, uv + vec2(size.x, 0.0)).a;\n"
                "outline += texture(image, uv + vec2(0.0, -size.y)).a;\n"
                "outline += texture(image, uv + vec2(-size.x, size.y)).a;\n"
                "outline += texture(image, uv + vec2(size.x, size.y)).a;\n"
                "outline += texture(image, uv + vec2(-size.x, size.y)).a;\n"
                "outline += texture(image, uv + vec2(size.x, -size.y)).a;\n"
                "outline = min(outline, 1.0);\n"

                "vec4 c = texture(image, uv);\n"
                "color = mix(c, vec4(outlineColor, alphaVal), outline - c.a);\n"
                "if (whiteout == 1) {\n"
                "   color.r += 10.0;\n"
                "   color.g += 10.0;\n"
                "   color.b += 10.0;\n"
                "}\n"

            "}\n"
            "else {\n"
               "color = texture(image, uv);\n"
                "if (whiteout == 1) {\n"
                "   color.r += 10.0;\n"
                "   color.g += 10.0;\n"
                "   color.b += 10.0;\n"
                "}\n"
            "}\n"
        "}";


    
    //--------------------------------------------

    static constexpr const char* outlineGeometry = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "in VS_OUT {\n"
            "vec2 uv;\n"
        "} gs_in[];\n"

        "layout (triangles) in;\n"
        "layout (triangle_strip, max_vertices = 3) out;\n"

        "out vec2 uv;\n" 

        "uniform float time;\n"

        "vec4 explode(vec4 position, vec3 normal) {\n" 
            "float magnitude = 2.0;\n"
            "vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude;\n" 
            "return position + vec4(direction, 0.0);\n"
        "}\n"

        "vec3 GetNormal() {\n" 
            "vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);\n"
            "vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);\n"
            "return normalize(cross(a, b));\n"
        "}\n"

        "void main() {\n"    
            "vec3 normal = GetNormal();\n"

            "gl_Position = explode(gl_in[0].gl_Position, normal);\n"
            "uv = gs_in[0].uv;\n"
            "EmitVertex();\n"
            "gl_Position = explode(gl_in[1].gl_Position, normal);\n"
            "uv = gs_in[1].uv;\n"
            "EmitVertex();\n"
            "gl_Position = explode(gl_in[2].gl_Position, normal);\n"
            "uv = gs_in[2].uv;\n"
            "EmitVertex();\n"
            "EndPrimitive();\n"
        "}";  

    //--------------------------------------------

    static constexpr const char* geom_vertex1 = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "uniform mat4 vp;\n"

        "layout(location = 0) in vec2 v_position;\n"
        "layout(location = 1) in vec4 v_color;\n"
        "layout(location = 2) in float v_size;\n"

        "out vec4 f_color;\n"

        "void main(void)\n"
        "{\n"
        "	f_color = v_color;\n"
        "	gl_Position = vp * vec4(v_position, 0.0f, 1.0f);\n"
        "   gl_PointSize = v_size;\n"
        "}\n";


    //--------------------------------------------


    static constexpr const char* geom_vertex2 = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "uniform mat4 vp;\n"

        "layout(location = 0) in vec2 v_position;\n"
        "layout(location = 1) in vec4 v_color;\n"

        "out vec4 f_color;\n"

        "void main(void)\n"
        "{\n"
        "	f_color = v_color;\n"
        "	gl_Position =  vp * vec4(v_position, 0.0f, 1.0f);\n"
        "}\n";


    //--------------------------------------------


    static constexpr const char* geom_fragment = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "in vec4 f_color;\n"

        "out vec4 color;\n"

        "void main(void)\n"
        "{\n"
        "	color = f_color;\n"
        "}\n";

    
    //--------------------------------------------


    static constexpr const char* spriteInstanceShader_vertex =

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "layout (location = 0) in vec4 vert;\n"
        "layout (location = 1) in vec2 UV;\n"

        "uniform mat4 mvp;\n"
        "uniform vec2 offsets[100];\n"

        "out vec2 uv;\n"

        "void main()\n"
        "{\n"
            "uv = UV;\n"
            "vec2 offset = offsets[gl_InstanceID];\n"
            "gl_Position = mvp * vec4(vert.xy + offset, 0.0, 1.0);\n"
        "}\n";


    static constexpr const char* spriteInstanceShader_fragment =

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "in vec2 uv;\n"
        "out vec4 color;\n"

        "uniform sampler2D image;\n" //=0

        "void main() {\n"
            "color = texture(image, uv);\n"
        "}";

        // #ifdef __EMSCRIPTEN__
        //     "#version 300 es\n"
        //     "precision mediump float;\n"
        // #else
        //     "#version 330 core\n"
        // #endif

        // "in vec2 uv;\n"
        // "out vec4 color;\n"

        // "uniform sampler2D images[32];\n"
        // "uniform vec3 tint;\n"
        // "uniform float alphaVal;\n"

        // "void main()\n"
        // "{\n"
        //     "int index = int(uv);\n"
        //     "color = texture(images[index], uv) * tint * alphaVal;\n"
        // "}\n";



    //shader char arrays

    Load("sprite", spriteQuadShader_vertex, spriteQuadShader_fragment);
    Load("graphics", debugGraphicShader_vertex, debugGraphicShader_fragment);
    Load("text", textVertex, textFragment);
    Load("outline text", textVertex, textOutlineFragment);    
    Load("outline sprite", spriteQuadShader_vertex, spriteOutlineFragment); 
    Load("instance", spriteInstanceShader_vertex, spriteInstanceShader_fragment);

    #if DEVELOPMENT == 1

        Load("Points", geom_vertex1, geom_fragment);
        Load("Lines", geom_vertex2, geom_fragment);
        Load("Triangles", geom_vertex2, geom_fragment);

    #endif

    //shader files

    //...

    LOG("Shader: Base shaders initialized.");

}


//---------------------------------


const bool checkCompileErrors(const std::string& key, unsigned int shader, const std::string& type)
{

    GLint result;
    GLsizei length = 0;
    GLchar message[1024];

    //vertex, fragment

    if (type != "program")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

        if (result == GL_FALSE) {
            glGetShaderInfoLog(shader, 1024, &length, message);
            LOG("Shader: \"" + key + "\" of type " + type + " compilation error: " + message);
            return false;
        }

        else
            return true;
    }

    //link program

    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &result);

        if (result == GL_FALSE) {
            glGetProgramInfoLog(shader, 1024, &length, message);
            LOG("Shader: \"" + key + "\" linking error: " + message);
            return false;
        }

        else {
            LOG("Shader: \"" + key + "\" compiled and linked successfully.");
            return true;
        }
    }
}

//-------------------------------------------- load shader


void Shader::Load(const std::string& key, const char* vertShader, const char* fragShader, const char* geomShader)
{

    if (std::find_if(System::Application::resources->shaders.begin(), System::Application::resources->shaders.end(), [key](const std::pair<const std::string&, Shader>& s) { return s.first == key; }) != System::Application::resources->shaders.end())
    {
        LOG("Shader: \"" + key + "\" already exists.");
        return;
    } 

    Shader shader;

    shader.key = key;

    if (
        System::Utils::str_includes(vertShader, ".vert") && System::Utils::str_includes(fragShader, ".frag") ||
        System::Utils::str_includes(vertShader, ".glsl") && System::Utils::str_includes(fragShader, ".glsl") ||
        System::Utils::str_includes(vertShader, ".shader") && System::Utils::str_includes(fragShader, ".shader") 
    )
    {

        //open files

        std::ifstream vertexShaderFile(vertShader),
                      fragmentShaderFile(fragShader);

        std::stringstream vShaderStream,
                          fShaderStream;

        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();

        vertexShaderFile.close();
        fragmentShaderFile.close();

        const std::string vertexCode = vShaderStream.str(),
                          fragmentCode = fShaderStream.str();

        const char* vs = vertexCode.c_str();
        const char* fs = fragmentCode.c_str();

        std::string geometryCode;

        if (geomShader) {
            
            std::ifstream geometryShaderFile(vertShader);
            std::stringstream gShaderStream;

            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();

        }

        if (!shader.Generate(key, vs, fs, geometryCode.size() ? geometryCode.c_str() : nullptr)) {
            LOG("Shader: \"" + key + "\" failed to load. (filepath)");
            return;
        }

        LOG("Shader: \"" + key + "\" loaded. (filepath)");

    }

    //from raw char

    else 
    {
        if (!shader.Generate(key, vertShader, fragShader, geomShader ? geomShader : nullptr)) {
            LOG("Shader: \"" + key + "\" failed to load. (embedded)");
            return;
        }

        LOG("Shader: \"" + key + "\" loaded. (embedded)");
    }

    System::Application::resources->shaders[key] = shader;
}

//--------------------------- generate


const bool Shader::Generate(const std::string& key, const char* vertexPath, const char* fragmentPath, const char* geomPath)
{

    unsigned int vertex, fragment, geometry;

    //vertex

    vertex = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertex, 1, &vertexPath, NULL);
    glCompileShader(vertex);

    if (!checkCompileErrors(key, vertex, "vertex"))
        return false;

    //geometry (optional) 

    if (geomPath)
    {
        #ifndef __EMSCRIPTEN__
            geometry = glCreateShader(GL_GEOMETRY_SHADER);

            glShaderSource(geometry, 1, &geomPath, NULL);
            glCompileShader(geometry);

            if (!checkCompileErrors(key, geometry, "geometry"))
                return false;
        #endif
    }

    //fragment

    fragment = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragment, 1, &fragmentPath, NULL);
    glCompileShader(fragment);

    if (!checkCompileErrors(key, fragment, "fragment"))
        return false;


    //shader Program

    this->ID = glCreateProgram();

    glAttachShader(this->ID, vertex);
    glAttachShader(this->ID, fragment);

    if (geomPath != nullptr)
        glAttachShader(this->ID, geometry);

    glLinkProgram(this->ID);

    if (!checkCompileErrors(key, this->ID, "program"))
        return false;

    //use program

    this->Use();

    //delete the shaders

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if (geomPath != nullptr)
        glDeleteShader(geometry);

    return true;
}


// ---------------------------------------------------------- unload


void Shader::UnLoad(const std::string& key)
{
    const auto it = System::Application::resources->shaders.find(key);

    if (it != System::Application::resources->shaders.end()) {
        (*it).second.Delete();
        System::Application::resources->shaders.erase(it);
    }

    LOG("Shader: \"" + key + "\" deleted.");
}


// ---------------------------------------------------------- utility uniform functions


void Shader::SetFloat(const char* name, float value, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(ID, name) != -1)
        glUniform1f(glGetUniformLocation(ID, name), value);
}

// -----------------------------------------------------------------------

void Shader::SetInt(const char* name, int value, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(ID, name) != -1)
        glUniform1i(glGetUniformLocation(ID, name), value);
}

// -----------------------------------------------------------------------

void Shader::SetIntV(const char* name, int length, int* value, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(ID, name) != -1)
        glUniform1iv(glGetUniformLocation(ID, name), length, value);
}

// -----------------------------------------------------------------------

void Shader::SetVec2f(const char* name, float x, float y, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(ID, name) != -1)
        glUniform2f(glGetUniformLocation(ID, name), x, y);
}

// -----------------------------------------------------------------------

void Shader::SetVec2f(const char* name, const Math::Vector2& value, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(ID, name) != -1)
        glUniform2f(glGetUniformLocation(ID, name), value.x, value.y);
}

// -----------------------------------------------------------------------

void Shader::SetVec3f(const char* name, float x, float y, float z, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(ID, name) != -1)
        glUniform3f(glGetUniformLocation(ID, name), x, y, z);
}

// -----------------------------------------------------------------------

void Shader::SetVec3f(const char* name, const Math::Vector3& value, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(ID, name) != -1)
        glUniform3f(glGetUniformLocation(ID, name), value.x, value.y, value.z);
}

// -----------------------------------------------------------------------

void Shader::SetVec4f(const char* name, float x, float y, float z, float w, bool useShader)
{

    if (useShader)
        this->Use();

    if (glGetUniformLocation(ID, name) != -1)
        glUniform4f(glGetUniformLocation(ID, name), x, y, z, w);
}

// -----------------------------------------------------------------------

void Shader::SetVec4f(const char* name, const Math::Vector4& value, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(ID, name) != -1)
        glUniform4f(glGetUniformLocation(ID, name), value.x, value.y, value.z, value.w);
}

// -----------------------------------------------------------------------

void Shader::SetMat4(const char* name, const Math::Matrix4& matrix, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(ID, name) != -1)
    {
        const glm::highp_mat4 mat = {
            { matrix.a.x, matrix.a.y, matrix.a.z, matrix.a.w },
            { matrix.b.x, matrix.b.y, matrix.b.z, matrix.b.w },
            { matrix.c.x, matrix.c.y, matrix.c.z, matrix.c.w },
            { matrix.d.x, matrix.d.y, matrix.d.z, matrix.d.w }
        };

        glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, false, glm::value_ptr(mat));
    }
}