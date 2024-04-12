#define STB_IMAGE_IMPLEMENTATION
#include "../../vendors/stb/stb_image.h" 

#include "../../../../build/sdk/include/manager.h"
#include "../../../../build/sdk/include/app.h"
#include "../../../../build/sdk/include/entity.h"

using namespace Graphics;


//-------------------------------- base primitive


void Primitive::GenBuffer()
{ 
    glGenVertexArrays(1, &this->VAO); 
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->UVBO);
    glBindVertexArray(this->VAO);
}


//--------------------------------


void Primitive::Bind_Buffer(
    const auto &data, 
    const GLuint &buffer, 
    const GLuint &VAO,
    GLuint location, 
    GLint vecCount, 
    GLenum type, 
    GLsizei normal,  
    GLenum draw,  
    unsigned long long stride
) 
{

    glBindVertexArray(VAO); 
    glBindBuffer(GL_ARRAY_BUFFER, buffer);    
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, draw);
    glVertexAttribPointer(location, vecCount, type, normal, stride, (void*)0);
    glEnableVertexAttribArray(location);
}



//--------------------------------


void Primitive::Draw (int shape, int dimension, int slot, int vertices, int drawStyle) 
{ 
    glBindBuffer(GL_ARRAY_BUFFER, 0);    

    #ifndef __EMSCRIPTEN__
        glPolygonMode(GL_FRONT_AND_BACK, drawStyle);
    #endif
    
    glDrawArrays(shape, 0, vertices);
  
}


//------------------------------- generic shape container


//quad
Geometry::Geometry(float x, float y, float width, float height): 
    Entity("geometry", x, y),
        m_type("quad"),
        width(width),
        height(height)
{ 
    this->m_tint = glm::vec3(0.0f, 0.0f, 1.0f);
    this->m_texture = Graphics::Texture2D::GetTexture("base");
    this->m_shader = Shader::GetShader("graphics");
    
    #if DEVELOPMENT == 1
        std::cout << "Entity: quad created.\n"; 
    #endif
}



//------------------------------------- 


void Geometry::Render()
{

    this->m_model = glm::mat4(1.0f); 

    auto SetShader = [&](){
        this->m_shader.SetVec3f("tint", this->m_tint, true);
        this->m_shader.SetMat4("model", this->m_model, true);  
        this->m_shader.SetFloat("alphaVal", this->m_alpha, true);
    };

    //quad or line

    if (strcmp(this->m_type, "quad") == 0)
    {

        this->m_texture.FrameWidth = this->width;
        this->m_texture.FrameHeight = this->height;

        this->m_model = glm::translate(this->m_model, glm::vec3(this->m_position, 0.0f));  

        this->m_model = glm::translate(this->m_model, glm::vec3(0.5f * this->width + this->m_position.x, 0.5f * this->height + this->m_position.y, 0.0f)); 

        if (this->m_rotation > 0)
            this->m_model = glm::rotate(this->m_model, glm::radians(this->m_rotation), glm::vec3(0.0f, 0.0f, 1.0f)); 

        this->m_model = glm::translate(this->m_model, glm::vec3(-0.5f * this->width - this->m_position.x, -0.5f * this->height - this->m_position.y, 0.0f));
        
        SetShader();

        this->m_texture.Update(this->m_position, false, false, this->drawStyle); 

    }


}


//--------------------------------- Texture 2D


Texture2D& Texture2D::GetTexture(const std::string& key) {
    return System::Application::resources->textures[key];
}


//-----------------------------------


void Texture2D::SetChannels(Texture2D& texture, unsigned int channels) 
{
    
    texture.Channels = channels;

    if (texture.Channels == 4) { 
        texture.Internal_Format = GL_RGBA;
        texture.Image_Format = GL_RGBA;
    }
}


//-----------------------------------


void Texture2D::SetFiltering() 
{
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->Wrap_S);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->Wrap_T);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->Filter_Max); 
}


//-----------------------------------


void Texture2D::Load(const std::string& key) 
{


    bool success = false;

    //creates texture objects

    Texture2D texture;

    const char* filepath = System::Resources::Manager::GetFilePath(key);

    //file asset found in cache

    if (strcmp(filepath, "") != 0) 
    {
    
        int width, height, nrChannels;    

        stbi_set_flip_vertically_on_load(1);

        unsigned char* data = stbi_load(filepath, &width, &height, &nrChannels, 0);

        SetChannels(texture, nrChannels);

        if (data)
        {
            texture.Generate(width, height, data); 

            stbi_image_free(data); 

            success = true;
        }
    }

    //raw asset

    else
    {   

        const void* rawData = System::Resources::Manager::GetRawData(key); 

        if (rawData != 0) {        

            std::array<int, 3> dimensionsAndChannels = System::Resources::Manager::GetRawDimensionsAndChannels(key);
            
            SetChannels(texture, dimensionsAndChannels[2]);

            texture.Generate(dimensionsAndChannels[0], dimensionsAndChannels[1], rawData);

            success = true;
        }
    }

    if (!success) {

        #if DEVELOPMENT == 1
            std::cout << "Texture2D: Image of key: " + key + " not found.\n";    
        #endif
        
        return;
    }
    
    else {

        #if DEVELOPMENT == 1
            std::cout << "Texture2D: " + key + " loaded.\n";
        #endif
    }
      

    //assign texture in manager

    System::Application::resources->textures[key] = texture;  


} 


//--------------------------------------


void Texture2D::UnLoad(const std::string& key) 
{
    
    Texture2D* texture = &System::Application::resources->textures[key];

    glDeleteTextures(1, &texture->ID);   
    glBindTexture(GL_TEXTURE_2D, 0); 
    glDeleteVertexArrays(1, &texture->VAO); 
    glDeleteBuffers(1, &texture->VBO);
    glDeleteBuffers(1, &texture->UVBO);

    auto it = System::Application::resources->textures.find(key);

    if (it != System::Application::resources->textures.end())
        System::Application::resources->textures.erase(it);

    #if DEVELOPMENT == 1
        std::cout << "Texture2D: Deleted texture " + key + "\n";
    #endif
}


//--------------------------------------


void Texture2D::Generate(unsigned int width, unsigned int height, auto &data)
{

    this->Width = width;
    this->Height = height; 

    this->FrameWidth = this->Width;
    this->FrameHeight = this->Height;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->ID); 
    
    #ifndef __EMSCRIPTEN__
        glGenerateMipmap(GL_TEXTURE_2D);
    #endif

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   
 
    glTexImage2D(GL_TEXTURE_2D, 0, this->Internal_Format, this->Width, this->Height, 0, this->Image_Format, GL_UNSIGNED_BYTE, data);

    //set Texture wrap and filter modes

    SetFiltering();

    //unbind texture 

    glBindTexture(GL_TEXTURE_2D, 0);

    this->Bind();

}

//----------------------------------------


void Texture2D::Update(const glm::vec2& position, bool flipX, bool flipY, int drawStyle) 
{   

    //---------------- format texture
 
    Format offset;

    //flip X

    if (flipX && !flipY) 
        offset = { this->FrameWidth, this->FrameHeight, this->U2, this->V1, this->U1, this->V2 }; 

    //flip Y

    else if (!flipX && flipY)
        offset = { this->FrameWidth, this->FrameHeight, this->U1, this->V2, this->U2, this->V1 }; 

    //flip X, Y

    else if (flipX && flipY)
        offset = { this->FrameWidth, this->FrameHeight, this->U2, this->V2, this->U1, this->V1 }; 

    //no flip

    else
        offset = { this->FrameWidth, this->FrameHeight, this->U1, this->V1, this->U2, this->V2 }; 

    
    Renderable texture = { position.x, position.y, offset }; //posX, posY, UV


    //----------------- vertices 
    
    short vertices[12];

    //top right

        vertices[0] = texture.x + texture.format.width; 
        vertices[1] = texture.y;

    //bottom right

        vertices[2] = texture.x + texture.format.width;
        vertices[3] = texture.y + texture.format.height;

    //top left

        vertices[4] = texture.x;
        vertices[5] = texture.y;

    //bottom right

        vertices[6] = texture.x + texture.format.width;
        vertices[7] = texture.y + texture.format.height;

    //bottom left

        vertices[8] = texture.x;
        vertices[9] = texture.y + texture.format.height;

    //top left

        vertices[10] = texture.x;
        vertices[11] = texture.y;

       
    //----------------- uvs
        
    //top right

        this->UVs[0] = texture.format.u2;
        this->UVs[1] = texture.format.v2;

    //bottom right

        this->UVs[2] = texture.format.u2;
        this->UVs[3] = texture.format.v1;

    //top left

        this->UVs[4] = texture.format.u1;
        this->UVs[5] = texture.format.v2;

    //bottom right

        this->UVs[6] = texture.format.u2;
        this->UVs[7] = texture.format.v1;

    //bottom left

        this->UVs[8] = texture.format.u1;
        this->UVs[9] = texture.format.v1;

    //top left

        this->UVs[10] = texture.format.u1;
        this->UVs[11] = texture.format.v2;

    this->Bind();

    this->Bind_Buffer(vertices, this->VBO, this->VAO, 0, 2, GL_SHORT, GL_FALSE, GL_DYNAMIC_DRAW, 2 * sizeof(short)); 
    this->Bind_Buffer(this->UVs, this->UVBO, this->VAO, 1, 2, GL_FLOAT, GL_TRUE, GL_STATIC_DRAW, 2 * sizeof(GLfloat));

    #ifdef __EMSCRIPTEN__
        Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 6, 0);
    #else
        Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 6, drawStyle); 
    #endif
}

