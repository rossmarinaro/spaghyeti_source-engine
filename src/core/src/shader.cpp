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

//------------------------------- get shader by key


const Shader& Shader::Get(const std::string& key) {
    return System::Application::resources->shaders[
        System::Application::resources->shaders.find(key) != System::Application::resources->shaders.end() ? key : "sprite"];
}


//--------------------------- init base shaders


void Shader::InitBaseShaders()
{
    //default quad, batch rendering (sprite)

    static constexpr const char* spriteQuadShader_vertex = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
            "precision lowp float;\n"
        #endif

        "layout(location = 0) in vec3 a_Pos;\n"
        "layout(location = 1) in vec2 a_UV;\n"
        "layout(location = 2) in float a_TextureId;\n" 
        "layout(location = 3) in vec4 a_RGBA;\n"
        "layout(location = 4) in vec3 a_OutlineColor;\n"
        "layout(location = 5) in float a_OutlineWidth;\n"
        "layout(location = 6) in float a_Whiteout;\n"
        "layout(location = 7) in mat4 a_ModelViewProj;\n"

        "flat out float texID;\n"
        "out float whiteout;\n"
        "out float outlineWidth;\n"
        "out vec3 outlineColor;\n"
        "out vec2 uv;\n"
        "out vec4 rgba;\n"

        "void main()\n"
        "{\n" 
            "texID = a_TextureId;\n"
            "rgba = a_RGBA;\n"
            "uv = a_UV;\n"
            "outlineColor = a_OutlineColor;\n"
            "outlineWidth = a_OutlineWidth;\n"
            "whiteout = a_Whiteout;\n"

            "gl_Position = a_ModelViewProj * vec4(a_Pos, 1.0);\n" //must be proj * model * view per OpenGL
        "}";


    //-------------------------------------------- webgl compatible 


    static constexpr const char* spriteQuadShader_fragment = \

        "flat in float texID;\n"
        "in vec2 uv;\n"
        "in vec4 rgba;\n"
        "in vec3 outlineColor;\n"
        "in float outlineWidth;\n" 
        "in float whiteout;\n"
        "out vec4 color;\n"

        "void main()\n"
        "{\n"

            "int targetId = int(texID);\n"

            #ifdef __EMSCRIPTEN__
                "vec4 c = SPAGHYETI_WEBGL_TEXTURE_SLOT(targetId, uv);\n"
                "ivec2 texSize2d = SPAGHYETI_WEBGL_TEXTURE_SIZE(targetId);\n"
            #else
                "vec4 c = texture(SPAGHYETI_ACTIVE_TEXTURES[targetId], uv);\n"
                "ivec2 texSize2d = textureSize(SPAGHYETI_ACTIVE_TEXTURES[targetId], 0);\n"
            #endif

            "float texSize = float(texSize2d.x);\n"
            "float texelSize = 1.0 / texSize;\n"
            "vec2 size = vec2(texelSize * outlineWidth, texelSize * outlineWidth);\n"

            "if (c.a == 0.0 && outlineWidth > 0.0)\n" //outline
            "{\n"
                #ifdef __EMSCRIPTEN__
                    "vec4 baseColor = SPAGHYETI_WEBGL_TEXTURE_SLOT(targetId, uv);\n"
                    "float alphaTop = SPAGHYETI_WEBGL_TEXTURE_SLOT(targetId, uv + vec2(0.0, size.y)).a;\n"
                    "float alphaBottom = SPAGHYETI_WEBGL_TEXTURE_SLOT(targetId, uv - vec2(0.0, size.y)).a;\n"
                    "float alphaLeft = SPAGHYETI_WEBGL_TEXTURE_SLOT(targetId, uv - vec2(size.x, 0.0)).a;\n"
                    "float alphaRight = SPAGHYETI_WEBGL_TEXTURE_SLOT(targetId, uv + vec2(size.x, 0.0)).a;\n"
                #else
                    "vec4 baseColor = texture(SPAGHYETI_ACTIVE_TEXTURES[targetId], uv);\n"
                    "float alphaTop = texture(SPAGHYETI_ACTIVE_TEXTURES[targetId], uv + vec2(0.0, size.y)).a;\n"
                    "float alphaBottom = texture(SPAGHYETI_ACTIVE_TEXTURES[targetId], uv - vec2(0.0, size.y)).a;\n"
                    "float alphaLeft = texture(SPAGHYETI_ACTIVE_TEXTURES[targetId], uv - vec2(size.x, 0.0)).a;\n"
                    "float alphaRight = texture(SPAGHYETI_ACTIVE_TEXTURES[targetId], uv + vec2(size.x, 0.0)).a;\n"
                #endif

                "float maxNeighborAlpha = max(max(alphaTop, alphaBottom), max(alphaLeft, alphaRight));\n"
                // "if (c.a < 0.1 && maxNeighborAlpha > 0.1)\n"
                // "   color = vec4(outlineColor, rgba.a);\n"
                // "else\n"
                // "   color = c;\n"
                "float outlineFactor = maxNeighborAlpha * (1.0 - baseColor.a);\n"
                "vec4 mixedColor = mix(baseColor, vec4(outlineColor, rgba.a), outlineFactor);\n"
                "if (mixedColor.a < 0.01) discard;\n"
                "color = mixedColor;\n"
            "}\n"
            "else if (whiteout > 0.0) {\n" //tint fill
                "color = vec4(rgba.xyz, c.a);\n"
            "}\n"
            "else {\n" //fill
                #ifdef __EMSCRIPTEN__
                    "color = rgba * SPAGHYETI_WEBGL_TEXTURE_SLOT(targetId, uv);\n" 
                #else
                    "color = rgba * texture(SPAGHYETI_ACTIVE_TEXTURES[targetId], uv);\n" 
                #endif
                //"if (color.r > 0.9 && color.g < 0.1 && color.b > 0.9) discard;\n" //remove magenta background 
            "}\n"
        "}";


    //--------------------------------------------

    //post processing shader

    // static constexpr const char* framebufferVertex = \ 
    //     #ifdef __EMSCRIPTEN__
    //         "#version 300 es\n"
    //         "precision mediump float;\n"
    //     #else
    //         "#version 330 core\n"
    //         "precision lowp float;\n"
    //     #endif
    //     "layout(location = 0) in vec2 a_Pos;\n"
    //     "layout(location = 1) in vec2 a_UV;\n"

    //     "out vec2 uv;\n"

    //     "void main()\n"
    //     "{ \n"
    //         "uv = a_UV;\n"
    //         "gl_Position = vec4(aPos, 0.0, 1.0)\n"
    //     "}";
    
    // static constexpr const char* framebufferFragment = \
    //     #ifdef __EMSCRIPTEN__
    //         "#version 300 es\n"
    //         "precision mediump float;\n"
    //     #else
    //         "#version 330 core\n"
    //         "precision lowp float;\n"
    //     #endif
    //     "in vec2 uv;\n"
    //     "uniform sampler2D screenTexture;\n"
    //     "uniform vec2 screenRes;\n"
    //     "uniform vec2 internalRes;\n"

    //     "void main()\n"
    //     "{ \n"
    //         "float screenAspect = screenRes.x / screenRes.y;\n"
    //         "float internalAspect = internalRes.x / internalRes.y;\n"

    //         "float scale = (screenAspect > internalAspect) ? screenRes.y / internalRes.y : screenRes.x / internalRes.x;\n"
    //         "vec2 displaySize = internalRes * scale;\n"
    //         "vec2 offset = (screenRes - displaySize) / 2.0;\n"

    //         "vec2 pixelCoords = (uv * screenRes) - offset;\n"
    //         "vec2 finalUV = pixelCoords / displaySize;\n"


    //         "if (finalUV.x < 0.0 || finalUV.x > 1.0 || finalUV.y < 0.0 || finalUV.y > 1.0)\n"
    //             "color = vec4(0.0, 0.0, 0.0, 1.0);\n"
    //         "else color = texture(screenTexture, finalUV);\n"
    //     "}";

    //--------------------------------------------


    static constexpr const char* textVertex = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
           "precision lowp float;\n"
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
            "precision lowp float;\n"
        #endif

        "in vec2 uv;\n"
        "out vec4 color;\n"

        "uniform sampler2D image;\n"
        "uniform vec3 textColor;\n"
        "uniform float alphaVal;\n"

        "void main()\n"
        "{ \n"
            "float sampled = texture(image, uv).r;\n"
            "color = vec4(textColor, sampled * alphaVal); \n"
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

    Load("sprite", 
        spriteQuadShader_vertex, 
        (PreProcessorUtility(
            #ifdef __EMSCRIPTEN__
                true
            #else
                false
            #endif
        ) + TextureUtility(
            #ifdef __EMSCRIPTEN__
                true
            #else
                false
            #endif
        ) + spriteQuadShader_fragment).c_str());

    Load("text", textVertex/* spriteQuadShader_vertex */, textFragment);  

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


const std::string Shader::PreProcessorUtility(bool webgl) 
{
    std::string str; 
    
    if (webgl)
        str = \
            "#version 300 es\n"
            "precision mediump float;\n";
    else
        str = \
            "#version 330 core\n"
            "precision lowp float;\n";

    return str;
}


//---------------------------------


const std::string Shader::TextureUtility(bool webgl) 
{
    std::string textures;

    if (webgl)
    {
        textures = "#define MAX_TEXTURES 16\n\n" //8 webgl 1
        "vec4 SPAGHYETI_WEBGL_TEXTURE_SLOT(int slot, vec2 uv)\n"
        "{\n"
            "\tswitch (slot) {\n"
                "\t\tcase 0: default: return texture(SPAGHYETI_ACTIVE_TEXTURES[0], uv);\n"
                "\t\tcase 1: return texture(SPAGHYETI_ACTIVE_TEXTURES[1], uv);\n"
                "\t\tcase 2: return texture(SPAGHYETI_ACTIVE_TEXTURES[2], uv);\n"
                "\t\tcase 3: return texture(SPAGHYETI_ACTIVE_TEXTURES[3], uv);\n"
                "\t\tcase 4: return texture(SPAGHYETI_ACTIVE_TEXTURES[4], uv);\n"
                "\t\tcase 5: return texture(SPAGHYETI_ACTIVE_TEXTURES[5], uv);\n"
                "\t\tcase 6: return texture(SPAGHYETI_ACTIVE_TEXTURES[6], uv);\n"
                "\t\tcase 7: return texture(SPAGHYETI_ACTIVE_TEXTURES[7], uv);\n"
                "\t\tcase 8: return texture(SPAGHYETI_ACTIVE_TEXTURES[8], uv);\n"
                "\t\tcase 9: return texture(SPAGHYETI_ACTIVE_TEXTURES[9], uv);\n"
                "\t\tcase 10: return texture(SPAGHYETI_ACTIVE_TEXTURES[10], uv);\n"
                "\t\tcase 11: return texture(SPAGHYETI_ACTIVE_TEXTURES[11], uv);\n"
                "\t\tcase 12: return texture(SPAGHYETI_ACTIVE_TEXTURES[12], uv);\n"
                "\t\tvcase 13: return texture(SPAGHYETI_ACTIVE_TEXTURES[13], uv);\n"
                "\t\tcase 14: return texture(SPAGHYETI_ACTIVE_TEXTURES[14], uv);\n"
                "\t\tcase 15: return texture(SPAGHYETI_ACTIVE_TEXTURES[15], uv);\n"
            "\t}\n"
        "}\n"

        "ivec2 SPAGHYETI_WEBGL_TEXTURE_SIZE(int slot)\n"
        "{\n"
            "\tswitch (slot) {\n"
                "\t\tcase 0: default: return textureSize(SPAGHYETI_ACTIVE_TEXTURES[0], 0);\n"
                "\t\tcase 1: return textureSize(SPAGHYETI_ACTIVE_TEXTURES[1], 0);\n"
                "\t\tcase 2: return textureSize(SPAGHYETI_ACTIVE_TEXTURES[2], 0);\n"
                "\t\tcase 3: return textureSize(SPAGHYETI_ACTIVE_TEXTURES[3], 0);\n"
                "\t\tcase 4: return textureSize(SPAGHYETI_ACTIVE_TEXTURES[4], 0);\n"
                "\t\tcase 5: return textureSize(SPAGHYETI_ACTIVE_TEXTURES[5], 0);\n"
                "\t\tcase 6: return textureSize(SPAGHYETI_ACTIVE_TEXTURES[6], 0);\n"
                "\t\tcase 7: return textureSize(SPAGHYETI_ACTIVE_TEXTURES[7], 0);\n"
                "\t\tcase 8: return textureSize(SPAGHYETI_ACTIVE_TEXTURES[8], 0);\n"
                "\t\tcase 9: return textureSize(SPAGHYETI_ACTIVE_TEXTURES[9], 0);\n"
                "\t\tcase 10: return textureSize(SPAGHYETI_ACTIVE_TEXTURES[10], 0);\n"
                "\t\tcase 11: return textureSize(SPAGHYETI_ACTIVE_TEXTURES[11], 0);\n"
                "\t\tcase 12: return textureSize(SPAGHYETI_ACTIVE_TEXTURES[12], 0);\n"
                "\t\tcase 13: return textureSize(SPAGHYETI_ACTIVE_TEXTURES[13], 0);\n"
                "\t\tcase 14: return textureSize(SPAGHYETI_ACTIVE_TEXTURES[14], 0);\n"
                "\t\tcase 15: return textureSize(SPAGHYETI_ACTIVE_TEXTURES[15], 0);\n"
            "\t}\n"
        "}\n\n";
    }
    else
        textures = "#define MAX_TEXTURES 32\n\n";

    const std::string str = textures + "uniform sampler2D SPAGHYETI_ACTIVE_TEXTURES[MAX_TEXTURES];\n\n";

    return str;
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