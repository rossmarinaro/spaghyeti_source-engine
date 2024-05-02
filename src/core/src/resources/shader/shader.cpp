#include "../../../../../build/sdk/include/app.h"

#include "./glsl/raw/quad.h"
#include "./glsl/raw/batch.h"
#include "./glsl/raw/particles.h"
#include "./glsl/raw/physics_debug.h"
#include "./glsl/raw/text.h"

//---------------------------------


Shader& Shader::GetShader(const std::string &key) {
    return System::Application::resources->shaders[key];
} 


//-------------------------------


Shader &Shader::Use() {
    
    glUseProgram(this->ID);
    return *this;
}


//--------------------------- init base shaders


void Shader::InitBaseShaders()
{

    //raw char array

    Load("sprite", Shaders::spriteQuadShader_vertex, Shaders::spriteQuadShader_fragment, nullptr);
    Load("UI", Shaders::spriteQuadShader_vertex, Shaders::spriteQuadShader_fragment, nullptr);
    Load("batch", Shaders::spriteBatchShader_vertex, Shaders::spriteBatchShader_fragment, nullptr);
    Load("graphics", Shaders::debugGraphicShader_vertex, Shaders::debugGraphicShader_fragment, nullptr);
    Load("cursor", Shaders::debugGraphicShader_vertex, Shaders::debugGraphicShader_fragment, nullptr);
    Load("text", Shaders::textVertex, Shaders::textFragment, nullptr);
     
    #if DEVELOPMENT == 1

        Load("Points", Shaders::point_vertex, Shaders::point_fragment, nullptr);
        Load("Lines", Shaders::line_vertex, Shaders::line_fragment, nullptr);
        Load("Triangles", Shaders::triangle_vertex, Shaders::triangle_fragment, nullptr);

    #endif

    //files

    //...

    #if DEVELOPMENT == 1
        std::cout << "Shader: Base shaders initialized.\n";
    #endif
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
            GetShader(shader.first).SetVec2f("offset", glm::vec2(0.0f), true);

        else     
            GetShader(shader.first).SetVec2f("offset", camera->position, true);

        //projection

        if (shader.first != "Points" && shader.first != "Lines" && shader.first != "Triangles")
        {

            if (shader.first == "cursor" || shader.first == "UI")                 
                GetShader(shader.first).SetMat4("projection", camera->GetProjectionMatrix(static_cast<float>(System::Window::s_width * 2), static_cast<float>(System::Window::s_height * 2)), true);  

            else                 
                GetShader(shader.first).SetMat4("projection", camera->GetProjectionMatrix(System::Window::s_scaleWidth, System::Window::s_scaleHeight), true);
       
        }

        //view
      
        if (shader.first == "cursor" || shader.first == "UI")             
            GetShader(shader.first).SetMat4("view", glm::mat4(1.0f), true);

        else             
            GetShader(shader.first).SetMat4("view", glm::translate(glm::mat4(1.0f), glm::vec3(camera->position, 0.0f)), true);

    }
}


//--------------------------------- 


bool checkCompileErrors(unsigned int shader, const std::string& type)
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
                return false;
            }
            else {
                #if DEVELOPMENT == 1
                     std::cout << "Shader: " + type + " compiled successfully.\n";  
                #endif
                return true;
            }
        }

    #else


        int success;
        char infoLog[1024];

        if (type != "PROGRAM")
        {
            if (!success) {
                std::cout << "ERROR1::SHADER_COMPILATION_ERROR of type: " + type + "\nINFO System::Log::" + infoLog + "\n";
                return false;

            }
            else {
                #if DEVELOPMENT == 1
                    std::cout << "Shader: " + type + " compiled successfully.\n";  
                #endif
                return true;
            }
                
        }
        else
        {
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR2::SHADER_COMPILATION_ERROR of type: " + type + "\nINFO System::Log::" + infoLog + "\n";
                return false;
            }
            else {
                #if DEVELOPMENT == 1
                    std::cout << "Shader: " + type + " linked successfully.\n";  
                #endif
                return true;
            }
        };

    #endif


}

//-------------------------------------------- load shader


void Shader::Load(const std::string& key, const char* vertShader, const char* fragShader, const char* geomShader)
{

    if (std::find_if(System::Application::resources->shaders.begin(), System::Application::resources->shaders.end(), [&](auto s) { return s.first == key; }) != System::Application::resources->shaders.end())
    {
        #if DEVELOPMENT == 1
            std::cout << "Shader: already exists.\n";
        #endif

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

        #if DEVELOPMENT == 1
            std::cout << "Shader: Loading " << key << " from file.\n";
        #endif

        shader.Generate(vs, fs,/*  gShaderStream ? gs : */ nullptr);
 
    }

    //from raw char
    
    else
    {
        if (vertShader)
        {
            #if DEVELOPMENT == 1
                std::cout << "Shader: Loading " << key << " from string.\n";
            #endif
        
            shader.Generate(vertShader, fragShader, nullptr);
        }
        
        else {
            #if DEVELOPMENT == 1
                std::cout << "ERROR::SHADER: NO VERTEX OR FRAGMENT.\n";
            #endif
        }
      
    
    }

    System::Application::resources->shaders[key] = shader; 
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

    if (!checkCompileErrors(vertex, "VERTEX"))
        return;

    // fragment Shader

    if (fragmentPath != nullptr)
    {
        fragment = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(fragment, 1, &fragmentPath, NULL);
        glCompileShader(fragment);
        
        if (!checkCompileErrors(fragment, "FRAGMENT"))
            return;
    }

    // if geometry shader

    #ifndef __EMSCRIPTEN__
        if (geomPath != nullptr)
        {
            geometry = glCreateShader(GL_GEOMETRY_SHADER);

            glShaderSource(geometry, 1, &geomPath, NULL);
            glCompileShader(geometry);

            if (!checkCompileErrors(geometry, "GEOMETRY"))
                return;
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

    if (!checkCompileErrors(this->ID, "PROGRAM"))
        return;

    // use program

    this->Use();

    // delete the shaders

    glDeleteShader(vertex);

    if (fragmentPath != nullptr)
        glDeleteShader(fragment);

    if (geomPath != nullptr)
        glDeleteShader(geometry);  
}


// ---------------------------------------------------------- unload


void Shader::UnLoad(const std::string& key)
{ 
    std::map<std::string, Shader>::iterator it = System::Application::resources->shaders.find(key);
    
    if (it != System::Application::resources->shaders.end())
        System::Application::resources->shaders.erase(it);
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