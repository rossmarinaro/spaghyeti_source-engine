#include <fstream>
#include <sstream>

#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/window.h"
#include "../../vendors/glm/gtc/type_ptr.hpp"
#include "../../shared/renderer.h"
#include "debug.h"

using namespace Graphics;


void Shader::Delete() {
    glDeleteProgram(ID);
    LOG("Shader: shader of key: " + key + " deleted.");
}

//-------------------------------


const Shader& Shader::Get(const std::string& key) {
    return System::Application::resources->shaders[
        System::Application::resources->shaders.find(key) != System::Application::resources->shaders.end() ? key : "sprite"];
}



//--------------------------- init base shaders


void Shader::InitBaseShaders()
{
    //default quad (sprite)

    static constexpr const char* spriteQuadShader_vertex = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "layout(location = 0) in vec3 a_Pos;\n"
        "layout(location = 1) in vec2 a_Scale;\n"
        "layout(location = 2) in float a_Rotation;\n"
        "layout(location = 3) in vec2 a_UV;\n"
        "layout(location = 4) in float a_TextureId;\n" 
        "layout(location = 5) in vec4 a_RGBA;\n"
        "layout(location = 6) in vec3 a_OutlineColor;\n"
        "layout(location = 7) in float a_OutlineWidth;\n"
        "layout(location = 8) in mat4 a_ModelView;\n"

        "flat out float texID;\n"
        "out float outlineWidth;\n"
        "out vec3 outlineColor;\n"
        "out vec2 uv;\n"
        "out vec4 rgba;\n"

        "uniform mat4 proj;\n"

        "void main()\n"
        "{\n" 
            "texID = a_TextureId;\n"
            "rgba = a_RGBA;\n"
            "uv = a_UV;\n"
            "outlineColor = a_OutlineColor;\n"
            "outlineWidth = a_OutlineWidth;\n"

            "vec3 scaledPosition = vec3(a_Pos.xy * a_Scale, a_Pos.z);\n"
            "float c = cos(a_Rotation);\n"
            "float s = sin(a_Rotation);\n"
            "vec3 position = vec3(scaledPosition.x * c - scaledPosition.y * s, scaledPosition.x * s + scaledPosition.y * c, scaledPosition.z);\n"
            
            "gl_Position = proj * a_ModelView * vec4(position, 1.0);\n" //must be proj * modelView per OpenGL
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
        "in vec4 rgba;\n"
        "in vec3 outlineColor;\n"
        "in float outlineWidth;\n" 
        "out vec4 color;\n"

        "uniform sampler2D images[32];\n"

        "void main()\n"
        "{ \n"
            "vec4 c = texture(images[int(texID)], uv);\n"

            "if (c.a == 0.0 && outlineWidth > 0.0)\n" //outline
            "{\n" 

                "ivec2 texSize2d = textureSize(images[int(texID)], 0);\n"
                "float texSize = float(texSize2d.x);\n"
                "float texelSize = 1.0 / texSize;\n"
                "vec2 size = vec2(texelSize * outlineWidth, texelSize * outlineWidth);\n"
                "float outline = texture(images[int(texID)], uv + vec2(-size.x, 0.0)).a;\n"

                "outline += texture(images[int(texID)], uv + vec2(0.0, size.y)).a;\n"
                "outline += texture(images[int(texID)], uv + vec2(size.x, 0.0)).a;\n"
                "outline += texture(images[int(texID)], uv + vec2(0.0, -size.y)).a;\n"
                "outline += texture(images[int(texID)], uv + vec2(-size.x, size.y)).a;\n"
                "outline += texture(images[int(texID)], uv + vec2(size.x, size.y)).a;\n"
                "outline += texture(images[int(texID)], uv + vec2(-size.x, size.y)).a;\n"
                "outline += texture(images[int(texID)], uv + vec2(size.x, -size.y)).a;\n"
                "outline = min(outline, 1.0);\n"

                "vec4 c = texture(images[int(texID)], uv);\n"
                "color = mix(c, vec4(outlineColor, rgba.w), outline - c.a);\n"

            "}\n"
            "else {\n" //fill
               "color = rgba * texture(images[int(texID)], uv);\n"
                "if (color.r > 0.9 && color.g < 0.1 && color.b > 0.9) discard;\n" //remove magenta background 
            "}\n"
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


    // static constexpr const char* textOutlineFragment = \

    //      #ifdef __EMSCRIPTEN__
    //         "#version 300 es\n"
    //         "precision mediump float;\n"
    //     #else
    //         "#version 330 core\n"
    //     #endif

    //     "uniform sampler2D image;\n"  
    //     "uniform float outlineWidth;\n" 
    //     "uniform vec3 outlineColor;\n" 
    //     "uniform float alphaVal;\n" 
    //     "uniform float characterWidth;\n" 

    //     "out vec4 color;\n"
    //     "in vec2 uv;\n"

    //     "void main() {\n"

    //         "vec4 c = vec4(1.0, 1.0, 1.0, texture(image, uv));\n"

    //         "if (c.a == 0.0) {\n"

    //             "ivec2 texSize2d = textureSize(image, 0);\n"
    //             "float texSize = characterWidth;\n" 
    //             "float texelSize = 1.0 / texSize;\n"
    //             "vec2 size = vec2(texelSize * outlineWidth, texelSize * outlineWidth);\n"
    //             "float outline = vec4(1.0, 1.0, 1.0, texture(image, uv + vec2(-size.x, 0.0))).a;\n"
 
    //             "outline += vec4(1.0, 1.0, 1.0, texture(image, uv + vec2(0.0, size.y))).a;\n"
    //             "outline += vec4(1.0, 1.0, 1.0, texture(image, uv + vec2(size.x, 0.0))).a;\n"
    //             "outline += vec4(1.0, 1.0, 1.0, texture(image, uv + vec2(0.0, -size.y))).a;\n"
    //             "outline += vec4(1.0, 1.0, 1.0, texture(image, uv + vec2(-size.x, size.y))).a;\n"
    //             "outline += vec4(1.0, 1.0, 1.0, texture(image, uv + vec2(size.x, size.y))).a;\n"
    //             "outline += vec4(1.0, 1.0, 1.0, texture(image, uv + vec2(-size.x, size.y))).a;\n"
    //             "outline += vec4(1.0, 1.0, 1.0, texture(image, uv + vec2(size.x, -size.y))).a;\n"
    //             "outline = min(outline, 1.0);\n"

    //             "vec4 c = vec4(1.0, 1.0, 1.0, texture(image, uv));\n"
    //             "color = mix(c, vec4(outlineColor, alphaVal), outline - c.a);\n"

    //         "}\n"
    //         "else\n"
    //            "color = vec4(1.0, 1.0, 1.0, texture(image, uv));\n"
    //     "}";

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


    // static constexpr const char* spriteInstanceShader_vertex =

    //     #ifdef __EMSCRIPTEN__
    //         "#version 300 es\n"
    //         "precision mediump float;\n"
    //     #else
    //         "#version 330 core\n"
    //     #endif

    //     "layout (location = 0) in vec4 vert;\n"
    //     "layout (location = 1) in vec2 UV;\n"

    //     "out vec4 rgba;\n"

    //     "uniform mat4 mvp;\n"
    //     "uniform vec2 offsets[100];\n"

    //     "out vec2 uv;\n"

    //     "void main()\n"
    //     "{\n"
    //         "uv = UV;\n"
    //         "vec2 offset = offsets[gl_InstanceID];\n"
    //         "gl_Position = mvp * vec4(vert.xy + offset, 0.0, 1.0);\n"
    //     "}\n";


    //shader char arrays

    Load("sprite", spriteQuadShader_vertex, spriteQuadShader_fragment);
    Load("text", textVertex, textFragment);  
    //Load("instance", spriteInstanceShader_vertex, spriteQuadShader_fragment);

    #if DEVELOPMENT == 1
        Load("Points", geom_vertex1, geom_fragment);
        Load("Lines", geom_vertex2, geom_fragment);
        Load("Triangles", geom_vertex2, geom_fragment);
    #endif

    //shader files

    //...

    LOG("Shader: Base shaders initialized.");

    //set uniforms

    auto shader = Get("sprite");
    int samplers[System::Renderer::MAX_TEXTURES];

    for (int i = 0; i < System::Renderer::MAX_TEXTURES; i++) 
        samplers[i] = i;

    shader.SetIntV("images", System::Renderer::MAX_TEXTURES, samplers);

    const auto camera = System::Application::game->camera;

    const Math::Vector4& pm = camera->GetProjectionMatrix(System::Window::s_scaleWidth, System::Window::s_scaleHeight);
    const glm::mat4 ortho = (glm::highp_mat4)glm::ortho(pm.r, pm.g, pm.b, pm.a, -1.0f, 1.0f);

    const Math::Matrix4 proj = { 
        { ortho[0][0], ortho[0][1], ortho[0][2], ortho[0][3] }, 
        { ortho[1][0], ortho[1][1], ortho[1][2], ortho[1][3] },   
        { ortho[2][0], ortho[2][1], ortho[2][2], ortho[2][3] },  
        { ortho[3][0], ortho[3][1], ortho[3][2], ortho[3][3] }
    };

    shader.SetMat4("proj", proj);  
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

    ID = glCreateProgram();

    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);

    if (geomPath != nullptr)
        glAttachShader(ID, geometry);

    glLinkProgram(ID);

    if (!checkCompileErrors(key, ID, "program"))
        return false;

    //use program

    glUseProgram(ID);

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
        glUseProgram(ID);

    if (glGetUniformLocation(ID, name) != -1)
        glUniform1f(glGetUniformLocation(ID, name), value);
}

// -----------------------------------------------------------------------

void Shader::SetInt(const char* name, int value, bool useShader)
{
    if (useShader)
        glUseProgram(ID);

    if (glGetUniformLocation(ID, name) != -1)
        glUniform1i(glGetUniformLocation(ID, name), value);
}

// -----------------------------------------------------------------------

void Shader::SetIntV(const char* name, int length, int* value, bool useShader)
{
    if (useShader)
        glUseProgram(ID);

    if (glGetUniformLocation(ID, name) != -1)
        glUniform1iv(glGetUniformLocation(ID, name), length, value);
}

// -----------------------------------------------------------------------

void Shader::SetVec2f(const char* name, float x, float y, bool useShader)
{
    if (useShader)
        glUseProgram(ID);

    if (glGetUniformLocation(ID, name) != -1)
        glUniform2f(glGetUniformLocation(ID, name), x, y);
}

// -----------------------------------------------------------------------

void Shader::SetVec2f(const char* name, const Math::Vector2& value, bool useShader)
{
    if (useShader)
        glUseProgram(ID);

    if (glGetUniformLocation(ID, name) != -1)
        glUniform2f(glGetUniformLocation(ID, name), value.x, value.y);
}

// -----------------------------------------------------------------------

void Shader::SetVec3f(const char* name, float x, float y, float z, bool useShader)
{
    if (useShader)
        glUseProgram(ID);

    if (glGetUniformLocation(ID, name) != -1)
        glUniform3f(glGetUniformLocation(ID, name), x, y, z);
}

// -----------------------------------------------------------------------

void Shader::SetVec3f(const char* name, const Math::Vector3& value, bool useShader)
{
    if (useShader)
        glUseProgram(ID);

    if (glGetUniformLocation(ID, name) != -1)
        glUniform3f(glGetUniformLocation(ID, name), value.x, value.y, value.z);
}

// -----------------------------------------------------------------------

void Shader::SetVec4f(const char* name, float r, float g, float b, float a, bool useShader)
{

    if (useShader)
        glUseProgram(ID);

    if (glGetUniformLocation(ID, name) != -1)
        glUniform4f(glGetUniformLocation(ID, name), r, g, b, a);
}

// -----------------------------------------------------------------------

void Shader::SetVec4f(const char* name, const Math::Vector4& value, bool useShader)
{
    if (useShader)
        glUseProgram(ID);

    if (glGetUniformLocation(ID, name) != -1)
        glUniform4f(glGetUniformLocation(ID, name), value.r, value.g, value.b, value.a);
}

// -----------------------------------------------------------------------

void Shader::SetMat4(const char* name, const Math::Matrix4& matrix, bool useShader)
{
    if (useShader)
        glUseProgram(ID);

    if (glGetUniformLocation(ID, name) != -1)
    {
        const glm::highp_mat4 mat = {
            { matrix.a.r, matrix.a.g, matrix.a.b, matrix.a.a },
            { matrix.b.r, matrix.b.g, matrix.b.b, matrix.b.a },
            { matrix.c.r, matrix.c.g, matrix.c.b, matrix.c.a },
            { matrix.d.r, matrix.d.g, matrix.d.b, matrix.d.a }
        };

        glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, false, glm::value_ptr(mat));
    }
}