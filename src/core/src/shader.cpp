#include <fstream>
#include <sstream>
#include "../../../build/sdk/include/app.h"


void Shader::Delete() {
    glDeleteProgram(ID);
}

//-------------------------------


Shader& Shader::Get(const std::string& key) {
    return System::Application::resources->shaders[key];
} 


//-------------------------------


Shader& Shader::Use() {
    glUseProgram(this->ID);
    return *this;
}


//--------------------------- init base shaders


void Shader::InitBaseShaders()
{

    //quad (sprite)

    static const char* spriteQuadShader_vertex = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "layout(location = 0) in vec2 vert;\n"
        "layout(location = 1) in vec2 UV;\n"

        "out vec2 uv;\n"

        "uniform vec2 scale;\n"
        "uniform vec2 offset;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"

        "void main()\n"
        "{\n"            
            "uv = UV;\n"
            "gl_Position = projection * model * view * vec4((vert.xy * scale) + offset.xy, 0.0, 1.0);\n" 
        "}"; 


    static const char* spriteQuadShader_fragment = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "in vec2 uv;\n"
        "out vec4 color;\n"

        "uniform sampler2D image;\n"
        "uniform vec3 tint;\n"
        "uniform float alphaVal;\n"

        #ifndef __EMSCRIPTEN__
            "uniform int repeat;\n"
        #endif

        "void main()\n"
        "{ \n"

            #ifdef __EMSCRIPTEN__
                "color = vec4(tint, alphaVal) * texture(image, uv); \n"
            #else
                "color = vec4(tint, alphaVal) * texture(image, uv * repeat); \n"
            #endif
            //"if (color.r == 1.0 && color.b == 1.0) discard;" magenta background only
        "}";         

    //debug

    static const char* debugGraphicShader_vertex = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "layout(location = 0) in vec2 pos;\n"
        
        "uniform vec2 offset;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"

        "void main()\n"
        "{\n"
            "gl_Position = projection * model * view * vec4(pos.xy + offset.xy, 0.0, 1.0);\n"
        "}";

    static const char* debugGraphicShader_fragment = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif
            
        "out vec4 FragColor;\n"
        "uniform vec3 tint;\n"
        "uniform float alphaVal;\n"

        "void main()\n"
        "{\n"
            "FragColor = vec4(tint, alphaVal);\n"
        "}";


    static const char* textVertex = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "layout(location = 0) in vec2 inPosition;\n" 
        "layout(location = 0) out vec2 texturePos;\n" 

        "uniform vec2 resolution;\n" 
        "uniform vec2 position;\n" 
        "uniform mat4 projection;\n" 
        "uniform vec2 size;\n" 

        "void main() {\n" 
            "vec2 pos = (inPosition * size / resolution);\n" 
            "pos = pos + (position / resolution);\n" 
            "gl_Position = projection * vec4(pos * 2.0 - vec2(1.0, 1.0), 0.0, 1.0);\n" 
            "texturePos = vec2(inPosition.x, (inPosition.y - 1.0) * -1.0);\n" 
        "}"; 

    static const char* textFragment = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "layout(location = 0) in vec2 texturePos;\n" 
        "layout(location = 0) out vec4 outColor;\n" 

        "uniform sampler2D fontTexture;\n" 
        "uniform vec4 charPosition;\n" 

        "void main() {\n" 
            "ivec2 texSize = textureSize(fontTexture, 0);\n" 
            "vec2 texPos = texturePos * ((charPosition.zw - charPosition.xy) / texSize.x) + charPosition.xy / texSize.y;\n"  
            "float col = texture(fontTexture, texPos).r;\n"  
            "vec3 textColor = vec3(texPos.x, texturePos.y, texPos.y);\n"  
            "outColor = vec4(textColor, col);\n"  
        "}";

    static const char* geom_vertex1 = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

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


    //geometry

    static const char* geom_vertex2 = \

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

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


    static const char* geom_fragment = \

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

    //batch

    static const char* spriteBatchShader_vertex = 
                    
        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

        "layout (location = 0) in vec2 vert;\n"
        "layout (location = 1) in vec4 UV;\n"

        "uniform vec2 offset;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"

        "out vec4 uv;\n"

        "void main()\n"
        "{\n"           
            "uv = UV;\n"
            "gl_Position = projection * model * view * vec4(vert.xy + offset.xy, 0.0, 1.0);\n"
        "}\n"; 


    static const char* spriteBatchShader_fragment =  

        #ifdef __EMSCRIPTEN__
            "#version 300 es\n"
            "precision mediump float;\n"
        #else
            "#version 330 core\n"
        #endif

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


    //shader char arrays

    Load("sprite", spriteQuadShader_vertex, spriteQuadShader_fragment); 
    Load("UI", spriteQuadShader_vertex, spriteQuadShader_fragment);
    Load("graphics", debugGraphicShader_vertex, debugGraphicShader_fragment);
    Load("cursor", debugGraphicShader_vertex, debugGraphicShader_fragment);
    //Load("batch", spriteBatchShader_vertex, spriteBatchShader_fragment);
    //Load("text", textVertex, textFragment);
     
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


void Shader::Update(Camera* camera)
{

    glClearColor(
        camera->backgroundColor.x * camera->backgroundColor.w,
        camera->backgroundColor.y * camera->backgroundColor.w,
        camera->backgroundColor.z * camera->backgroundColor.w, 
        camera->backgroundColor.w 
    ); 

    //custom shaders

    for (auto it = System::Application::resources->shaders.begin(); it != System::Application::resources->shaders.end(); ++it)
    {
        
        auto shader = *it;

        //offset

        if (shader.first == "cursor" || shader.first == "UI")             
            Get(shader.first).SetVec2f("offset", glm::vec2(0.0f));

        else     
            Get(shader.first).SetVec2f("offset", camera->position);

        //projection

        if (shader.first != "Points" && shader.first != "Lines" && shader.first != "Triangles")
        {

            if (shader.first == "cursor")                 
               Get(shader.first).SetMat4("projection", camera->GetProjectionMatrix(static_cast<float>(System::Window::s_width * 2), static_cast<float>(System::Window::s_height * 2)));  
            
            else                 
                Get(shader.first).SetMat4("projection", camera->GetProjectionMatrix(System::Window::s_scaleWidth, System::Window::s_scaleHeight));
        } 

        //view
      
        if (shader.first == "cursor" || shader.first == "UI")             
            Get(shader.first).SetMat4("view", glm::mat4(1.0f));

        else if (shader.first == "Points" || shader.first == "Lines" || shader.first == "Triangles")         
            Get(shader.first).SetMat4("view", glm::translate(glm::mat4(1.0f), glm::vec3(camera->position, 0.0f)));

    }
}


//--------------------------------- 


bool checkCompileErrors(const std::string& key, unsigned int shader, const std::string& type)
{ 

    GLint result;
    GLsizei length = 0;
    GLchar message[1024];

    if (type != "program") //vert, frag, geom
    {

        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

        if (result == GL_FALSE) {

            glGetShaderInfoLog(shader, 1024, &length, message);

            LOG("Shader: " + key + " of type " + type + " compilation error: " + message);

            return false;
        }

        else {

            LOG("Shader: " + key + " of type " + type + " compiled successfully.");  

            return true;
        }
    }

    else
    {

        glGetProgramiv(shader, GL_LINK_STATUS, &result);

        if (result == GL_FALSE) {

            glGetProgramInfoLog(shader, 1024, &length, message);

            LOG("Shader: " + key + " of type " + type + " linking error: " + message);

            return false;
        }

        else {

            LOG("Shader: " + key + " of type " + type + " linked successfully.");  

            return true;
        
        }
    }


}

//-------------------------------------------- load shader


void Shader::Load(const std::string& key, const char* vertShader, const char* fragShader, const char* geomShader)
{

    if (std::find_if(System::Application::resources->shaders.begin(), System::Application::resources->shaders.end(), [&](auto s) { return s.first == key; }) != System::Application::resources->shaders.end())
    {
        LOG("Shader: already exists.");

        return;
    } 

    Shader shader; 

    shader.key = key.c_str();

    if (
        System::Utils::str_includes(vertShader, ".vert") && System::Utils::str_includes(fragShader, ".frag") ||
        System::Utils::str_includes(vertShader, ".glsl") && System::Utils::str_includes(fragShader, ".glsl") ||
        System::Utils::str_includes(vertShader, ".shader") && System::Utils::str_includes(fragShader, ".shader")
    )
    {

        // open files

        std::ifstream vertexShaderFile(vertShader), 
                      fragmentShaderFile(fragShader),
                      geomShaderFile(geomShader);

        std::stringstream vShaderStream, 
                          fShaderStream, 
                          gShaderStream;

        //read file's buffer contents into streams

        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();
        gShaderStream << geomShaderFile.rdbuf();

        //close file handlers

        vertexShaderFile.close();
        fragmentShaderFile.close();
        geomShaderFile.close();

        //convert stream into string

        std::string vertexCode = vShaderStream.str(),
                    fragmentCode = fShaderStream.str(),
                    geometryCode = gShaderStream.str();

        //if geometry shader path is present, also load a geometry shader

        if (geomShader != nullptr)
        {
            std::ifstream geometryShaderFile(geomShader);
            std::stringstream gShaderStream;

            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }

        const char* vs = vertexCode.c_str();
        const char* fs = fragmentCode.c_str();
        const char* gs = geometryCode.c_str();

        LOG("Shader: Loading " + key + " from file.");

        shader.Generate(key, vs, fs);
 
    }

    //from raw char
    
    else
    {
        if (vertShader) {
            LOG("Shader: Loading " + key + " from string.");
            shader.Generate(key, vertShader, fragShader);
        }
        
        else 
            LOG("Shader: No vertex or fragment.");

    }

    System::Application::resources->shaders[key] = shader; 
}

//--------------------------- generate


void Shader::Generate(const std::string& key, const char* vertexPath, const char* fragmentPath, const char* geomPath)
{

    //compile shaders

    unsigned int vertex, fragment, geometry;

    //vertex shader

    vertex = glCreateShader(GL_VERTEX_SHADER);  

    glShaderSource(vertex, 1, &vertexPath, NULL);
    glCompileShader(vertex);

    if (!checkCompileErrors(key, vertex, "vertex"))
        return;

    //fragment Shader

    if (fragmentPath != nullptr)
    {
        fragment = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(fragment, 1, &fragmentPath, NULL);
        glCompileShader(fragment);
        
        if (!checkCompileErrors(key, fragment, "fragment"))
            return;
    }

    //geometry shader

    #ifndef __EMSCRIPTEN__

        if (geomPath != nullptr)
        {
            geometry = glCreateShader(GL_GEOMETRY_SHADER);

            glShaderSource(geometry, 1, &geomPath, NULL);
            glCompileShader(geometry);

            if (!checkCompileErrors(key, geometry, "geometry"))
                return;
        }

    #endif

    //shader Program

    this->ID = glCreateProgram();

    glAttachShader(this->ID, vertex);

    if (fragmentPath != nullptr)
        glAttachShader(this->ID, fragment);

    if (geomPath != nullptr)
        glAttachShader(this->ID, geometry);

    glLinkProgram(this->ID);

    if (!checkCompileErrors(key, this->ID, "program"))
        return;

    //use program

    this->Use();

    //delete the shaders

    glDeleteShader(vertex);

    if (fragmentPath != nullptr)
        glDeleteShader(fragment);

    if (geomPath != nullptr)
        glDeleteShader(geometry);  
}


// ---------------------------------------------------------- unload


void Shader::UnLoad(const std::string& key)
{ 

    auto it = System::Application::resources->shaders.find(key);

    if (it != System::Application::resources->shaders.end()) {
        (*it).second.Delete();
        System::Application::resources->shaders.erase(it);
    }

    LOG("Shader: Deleted shader " + key);
    
}


// ---------------------------------------------------------- utility uniform functions


void Shader::SetFloat(const char* name, float value, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(this->ID, name) != -1)
        glUniform1f(glGetUniformLocation(this->ID, name), value); 
}

// -----------------------------------------------------------------------

void Shader::SetInt(const char* name, int value, bool useShader)
{

    if (useShader)
        this->Use();

    if (glGetUniformLocation(this->ID, name) != -1)
        glUniform1i(glGetUniformLocation(this->ID, name), value); 
}

// -----------------------------------------------------------------------

void Shader::SetVec2f(const char* name, float x, float y, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(this->ID, name) != -1)
        glUniform2f(glGetUniformLocation(this->ID, name), x, y);
}

// -----------------------------------------------------------------------

void Shader::SetVec2f(const char* name, const glm::vec2& value, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(this->ID, name) != -1)
        glUniform2f(glGetUniformLocation(this->ID, name), value.x, value.y);
}

// -----------------------------------------------------------------------

void Shader::SetVec3f(const char* name, float x, float y, float z, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(this->ID, name) != -1)
        glUniform3f(glGetUniformLocation(this->ID, name), x, y, z);
}

// -----------------------------------------------------------------------

void Shader::SetVec3f(const char* name, const glm::vec3& value, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(this->ID, name) != -1)
        glUniform3f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z);
}

// -----------------------------------------------------------------------

void Shader::SetVec4f(const char* name, float x, float y, float z, float w, bool useShader)
{

    if (useShader)
        this->Use();

    if (glGetUniformLocation(this->ID, name) != -1)
        glUniform4f(glGetUniformLocation(this->ID, name), x, y, z, w);
}

// -----------------------------------------------------------------------

void Shader::SetVec4f(const char* name, const glm::vec4& value, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(this->ID, name) != -1)
        glUniform4f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z, value.w);
}

// -----------------------------------------------------------------------

void Shader::SetMat4(const char* name, const glm::mat4& matrix, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(this->ID, name) != -1)
        glUniformMatrix4fv(glGetUniformLocation(this->ID, name), 1, false, glm::value_ptr(matrix));
}