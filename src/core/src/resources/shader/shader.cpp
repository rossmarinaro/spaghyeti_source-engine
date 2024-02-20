#include "../../app/app.h"
#include "./shader.h"

#include "./glsl/raw/quad.h"
#include "./glsl/raw/batch.h"
#include "./glsl/raw/particles.h"
#include "./glsl/raw/physics_debug.h"

//--------------------------- init base shaders


void Shader::InitBaseShaders()
{

    //raw char array

    Load("player", Shaders::spriteQuadShader_vertex, Shaders::spriteQuadShader_fragment, nullptr);
    Load("sprite", Shaders::spriteQuadShader_vertex, Shaders::spriteQuadShader_fragment, nullptr);
    Load("UI", Shaders::spriteQuadShader_vertex, Shaders::spriteQuadShader_fragment, nullptr);
    Load("graphics", Shaders::debugGraphicShader_vertex, Shaders::debugGraphicShader_fragment, nullptr);
    Load("cursor", Shaders::debugGraphicShader_vertex, Shaders::debugGraphicShader_fragment, nullptr);
    
    #if DEVELOPMENT == 1

        Load("Points", Shaders::point_vertex, Shaders::point_fragment, nullptr);
        Load("Lines", Shaders::line_vertex, Shaders::line_fragment, nullptr);
        Load("Triangles", Shaders::triangle_vertex, Shaders::triangle_fragment, nullptr);

    #endif

    //files

    //Load("test from file","./shader/glsl/quad/vert.shader", "./shader/glsl/quad/frag.shader" , nullptr);

    std::cout << "Base shaders initialized.\n";
}


//---------------------------------


void Shader::Update(Camera* camera)
{

    glClearColor(
        camera->m_backgroundColor.x * camera->m_backgroundColor.w,
        camera->m_backgroundColor.y * camera->m_backgroundColor.w,
        camera->m_backgroundColor.z * camera->m_backgroundColor.w, 
        camera->m_backgroundColor.w 
    ); 

    //camera offset

    GetShader("player").SetVec2f("offset", camera->m_position, true);
    GetShader("sprite").SetVec2f("offset", camera->m_position, true);
    GetShader("graphics").SetVec2f("offset", camera->m_position, true);

   // #if DEVELOPMENT == 1

        GetShader("Points").SetVec2f("offset", camera->m_position, true);
        GetShader("Lines").SetVec2f("offset", camera->m_position, true);
        GetShader("Triangles").SetVec2f("offset", camera->m_position, true);

        GetShader("Points").SetMat4("view", camera->GetViewMatrix(camera), true);
        GetShader("Lines").SetMat4("view", camera->GetViewMatrix(camera), true);
        GetShader("Triangles").SetMat4("view", camera->GetViewMatrix(camera), true);
        
    //#endif

    //projection matrix

    GetShader("player").SetMat4("projection", camera->GetProjectionMatrix(System::Window::m_scaleWidth, System::Window::m_scaleHeight), true);
    GetShader("sprite").SetMat4("projection", camera->GetProjectionMatrix(System::Window::m_scaleWidth, System::Window::m_scaleHeight), true);
    GetShader("graphics").SetMat4("projection", camera->GetProjectionMatrix(System::Window::m_scaleWidth, System::Window::m_scaleHeight), true);
    GetShader("cursor").SetMat4("projection", camera->GetProjectionMatrix(static_cast<float>(System::Window::m_width * 2), static_cast<float>(System::Window::m_height * 2)), true);
    GetShader("UI").SetMat4("projection", camera->GetProjectionMatrix(static_cast<float>(System::Window::m_width * 2), static_cast<float>(System::Window::m_height * 2)), true);

    //view matrix

    GetShader("player").SetMat4("view", glm::mat4(1.0f), true);
    GetShader("sprite").SetMat4("view", camera->GetViewMatrix(camera), true);
    GetShader("graphics").SetMat4("view", camera->GetViewMatrix(camera), true);
}


//---------------------------------


Shader& Shader::GetShader(const std::string &key) {
    return System::Resources::Manager::shaders[key];
} 


//-------------------------------


Shader &Shader::Use() {
    
    glUseProgram(this->ID);
    return *this;
}


//--------------------------------- 


void checkCompileErrors(unsigned int shader, const std::string &type)
{

    #ifdef __EMSCRIPTEN__

    GLint result;
    GLint log_length;
    GLsizei length;

    char infoLog[1024];
    int success; 

    if (type != "PROGRAM") //vert, frag, geom
    {

        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        std::vector<GLchar> log(log_length); 

        if (result == GL_FALSE)
        {

            glGetShaderInfoLog(shader, log.size(), &length, log.data());

            std::cout << log.data() << "\n";
        }
        else
            std::cout << "SHADER: " + type + " COMPILED SUCESSFULLY.\n";  
    }

    #else


        int success;
        char infoLog[1024];

        if (type != "PROGRAM")
        {
            if (!success)
                std::cout << "ERROR1::SHADER_COMPILATION_ERROR of type: " + type + "\nINFO System::Log::" + infoLog + "\n";
            else
                std::cout << "SHADER: " + type + " COMPILED SUCESSFULLY.\n";  
        }
        else
        {
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR2::SHADER_COMPILATION_ERROR of type: " + type + "\nINFO System::Log::" + infoLog + "\n";
            }
            else
                std::cout << "SHADER: " + type + " LINKED SUCESSFULLY.\n";
        };

    #endif


}

//-------------------------------------------- load shader


void Shader::Load(const std::string &key, const char* vertShader, const char* fragShader, const char* geomShader)
{

    Shader shader; 

    shader.m_key = key.c_str();

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

        // read file's buffer contents into streams

        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();
        gShaderStream << geomShaderFile.rdbuf();

        // close file handlers

        vertexShaderFile.close();
        fragmentShaderFile.close();
        geomShaderFile.close();

        // convert stream into string

        std::string vertexCode = vShaderStream.str(),
                    fragmentCode = fShaderStream.str(),
                    geometryCode = gShaderStream.str();

        // if geometry shader path is present, also load a geometry shader

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

        std::cout << "Loading shader from file.\n";

        shader.Generate(vs, fs,/*  gShaderStream ? gs : */ nullptr);
 
    }

    //from raw char
    
    else
    {
        if (vertShader)
        {
            std::cout << "Loading raw shader.\n";
        
            shader.Generate(vertShader, fragShader, nullptr);
        }
        else 
            std::cout << "ERROR::SHADER: NO VERTEX OR FRAGMENT.\n";
    
    }


    System::Resources::Manager::shaders[key] = shader; 
}

//--------------------------- generate


void Shader::Generate(const char* vertexPath, const char* fragmentPath, const char* geomPath)
{

    // compile shaders

    unsigned int vertex, fragment, geometry;

    // vertex shader

    vertex = glCreateShader(GL_VERTEX_SHADER);  

    glShaderSource(vertex, 1, &vertexPath, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    // fragment Shader

    if (fragmentPath != nullptr)
    {
        fragment = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(fragment, 1, &fragmentPath, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
    }

    // if geometry shader

    #ifndef __EMSCRIPTEN__
        if (geomPath != nullptr)
        {
            geometry = glCreateShader(GL_GEOMETRY_SHADER);

            glShaderSource(geometry, 1, &geomPath, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }
    #endif

    // shader Program

    this->ID = glCreateProgram();

    glAttachShader(this->ID, vertex);

    if (fragmentPath != nullptr)
        glAttachShader(this->ID, fragment);

    if (geomPath != nullptr)
        glAttachShader(this->ID, geometry);

    glLinkProgram(this->ID);
    checkCompileErrors(this->ID, "PROGRAM");

    // use program

    this->Use();

    // delete the shaders

    glDeleteShader(vertex);

    if (fragmentPath != nullptr)
        glDeleteShader(fragment);

    if (geomPath != nullptr)
        glDeleteShader(geometry);  
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

void Shader::SetVec2f(const char* name, const glm::vec2 &value, bool useShader)
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

void Shader::SetVec3f(const char* name, const glm::vec3 &value, bool useShader)
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

void Shader::SetVec4f(const char* name, const glm::vec4 &value, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(this->ID, name) != -1)
        glUniform4f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z, value.w);
}

// -----------------------------------------------------------------------

void Shader::SetMat4(const char* name, const glm::mat4 &matrix, bool useShader)
{
    if (useShader)
        this->Use();

    if (glGetUniformLocation(this->ID, name) != -1)
        glUniformMatrix4fv(glGetUniformLocation(this->ID, name), 1, false, glm::value_ptr(matrix));
}