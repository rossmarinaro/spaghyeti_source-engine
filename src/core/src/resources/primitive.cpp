#define STB_IMAGE_IMPLEMENTATION
#include "../../../../vendors/stb/stb_image.h" 

#include "./manager/manager.h"
#include "../app/app.h"
#include "../game/entities/entity.h"

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


void Primitive::Draw (int shape, int dimension, int slot, int vertices, int drawStyle) { 

    glBindBuffer(GL_ARRAY_BUFFER, 0);  
    glDrawArrays(shape, 0, vertices);

}


//------------------------------- generic shape container


void Geometry::Render()
{

    this->m_model = glm::mat4(1.0f); 

    this->m_shader.SetVec3f("tint", this->m_strokeColor, true);
    this->m_shader.SetMat4("model", this->m_model, true);  
    this->m_shader.SetFloat("alphaVal", this->m_alpha, true);


    //------------------------------------- 


    if (strcmp(this->m_type, "line") == 0)
    {

        float vertices[6] = { this->start.x, this->start.y, 0.0f, this->end.x, this->end.y, 0.0f };

        this->Bind_Buffer(vertices, this->shape->VBO, this->shape->VAO, 0, 2, GL_FLOAT, GL_FALSE, GL_STATIC_DRAW, 2 * sizeof(float));
        
        #ifdef __EMSCRIPTEN__
            Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 2, 0);
        #else
            Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 2, this->shape->drawStyle == 0 ? GL_FILL : GL_LINE); 
        #endif
    }


    //-------------------------------------


    if (strcmp(this->m_type, "quad") == 0)
    {

        if (strcmp(this->m_type, "cursor") == 0)
            this->m_model = glm::translate(this->m_model, glm::vec3(this->x, this->y, 0.0f));  

        this->m_model = glm::translate(this->m_model, glm::vec3(0.5f * this->width + this->m_position.x, 0.5f * this->height + this->m_position.y, 0.0f)); 

        if (this->m_rotation > 0)
            this->m_model = glm::rotate(this->m_model, glm::radians(this->m_rotation), glm::vec3(0.0f, 0.0f, 1.0f)); 

        this->m_model = glm::translate(this->m_model, glm::vec3(-0.5f * this->width - this->m_position.x, -0.5f * this->height - this->m_position.y, 0.0f));
      
        float vertices[12];
        
        //triangle A

        vertices[0] = this->m_position.x + this->width;  
        vertices[1] = this->m_position.y;
        vertices[2] = this->m_position.x + this->width;
        vertices[3] = this->m_position.y + this->height;
        vertices[4] = this->m_position.x;
        vertices[5] = this->m_position.y;

        //triangle B

        vertices[6] = this->m_position.x + this->width;
        vertices[7] = this->m_position.y + this->height;
        vertices[8] = this->m_position.x;
        vertices[9] = this->m_position.y + this->height;
        vertices[10] = this->m_position.x;
        vertices[11] = this->m_position.y;

        this->shape->Bind_Buffer(vertices, this->shape->VBO, this->shape->VAO , 0, 2, GL_SHORT, GL_FALSE, GL_DYNAMIC_DRAW, 2 * sizeof(short)); 

        #ifdef __EMSCRIPTEN__
            Primitive::Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 6, 0);
        #else
            Primitive::Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 6, this->shape->drawStyle == 0 ? GL_FILL : GL_LINE); 
        #endif
    }

}


//--------------------------------- Texture 2D


Texture2D& Texture2D::GetTexture(const std::string &key) {
    return System::Resources::Manager::textures[key];
}


//-----------------------------------


void Texture2D::SetChannels(Texture2D &texture, unsigned int channels) 
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


void Texture2D::Load(const std::string &key) 
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

        std::cout << "Texture2D: Image of key: " + key + " not found.\n";    
        
        return;
    }
    
    else
        std::cout << "Texture2D: " + key + " loaded.\n";


    //assign texture in manager

    System::Resources::Manager::textures[key] = texture;  


}


//--------------------------------------


void Texture2D::UnLoad(const std::string &key) 
{
    
    Texture2D* tex = &System::Resources::Manager::textures[key];

    glDeleteTextures(1, &tex->ID);   
    glBindTexture(GL_TEXTURE_2D, 0); 
    glDeleteVertexArrays(1, &tex->VAO); 
    glDeleteBuffers(1, &tex->VBO);
    glDeleteBuffers(1, &tex->UVBO);

    System::Resources::Manager::textures.erase(key);

    std::cout << "Texture2D: Deleted texture " + key + "\n";
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


void Texture2D::Update(const glm::vec2 &position, bool flipX, bool flipY, int drawStyle) 
{   
 
    Format offset;

    if (flipX) 
        offset = { this->FrameWidth, this->FrameHeight, this->U2, this->V1, this->U1, this->V2 }; 

    else
        offset = { this->FrameWidth, this->FrameHeight, this->U1, this->V1, this->U2, this->V2 }; 
    
    Renderable texture = { position.x, position.y, offset }; //posX, posY, UV

    //----------------- vertices 
    
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
        UVs[0] = texture.format.u2;
        UVs[1] = texture.format.v2;

    //bottom right
        UVs[2] = texture.format.u2;
        UVs[3] = texture.format.v1;

    //top left
        UVs[4] = texture.format.u1;
        UVs[5] = texture.format.v2;

    //bottom right
        UVs[6] = texture.format.u2;
        UVs[7] = texture.format.v1;

    //bottom left
        UVs[8] = texture.format.u1;
        UVs[9] = texture.format.v1;

    //top left
        UVs[10] = texture.format.u1;
        UVs[11] = texture.format.v2;

    this->Bind();

    this->Bind_Buffer(vertices, this->VBO, this->VAO, 0, 2, GL_SHORT, GL_FALSE, GL_DYNAMIC_DRAW, 2 * sizeof(short)); 
    this->Bind_Buffer(UVs, this->UVBO, this->VAO, 1, 2, GL_FLOAT, GL_TRUE, GL_STATIC_DRAW, 2 * sizeof(GLfloat));

    #ifdef __EMSCRIPTEN__
        Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 6, 0);
    #else
        Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 6, GL_FILL); 
    #endif
}
