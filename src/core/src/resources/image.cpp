#define STB_IMAGE_IMPLEMENTATION
#include "../../../../vendors/stb/stb_image.h" 

#include "./manager/manager.h"
#include "./primitive.h"

#include "../app/app.h"
#include "./texture.h"
#include "../game/entities/entity.h"

using namespace Graphics;


//-------------------------------- base primitive


void Primitive::GenBuffer ()
{ 

    glGenVertexArrays(1, &this->quadVAO); 
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->UVBO);
    glBindVertexArray(this->quadVAO);
}


//--------------------------------


void Primitive::Bind_Buffer (
    auto &data, 
    GLuint &buffer, 
    GLuint location, 
    GLint vecCount, 
    GLenum type, 
    GLsizei normal,  
    GLenum draw,  
    unsigned long long stride
) 
{

    glBindVertexArray(this->quadVAO); 
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


//---------------------------------------- base shape


Shape::Shape(float x, float y, int numOfVerts, const char* type): 
    Primitive(numOfVerts), 
        m_self(this), 
        m_alpha(1),
        x(x),
        y(y), 
        m_type(type),
        m_graphicsShader(System::Resources::Manager::shader->GetShader(strcmp(type, "cursor") == 0 ? "cursor" : "graphics")),
        m_strokeColor(glm::vec3(0.0f, 1.0f, 0.0f)),
        m_fillColor(glm::vec3(1.0f, 1.0f, 1.0f)) {} 


//---------------------------------------


void Shape::Destroy() {
    this->m_self = nullptr;
    delete this->m_self;
}


//--------------------------------- shapes

void Line::Render(int drawStyle)
{

    this->m_model = glm::mat4(1.0f); 
    //this->m_model = glm::translate(this->m_model, glm::vec3(this->x, this->y, 0.0f));  

    float vertices[6] = { this->start.x, this->start.y, 0.0f, this->end.x, this->end.y, 0.0f };

    this->m_graphicsShader.SetMat4("model", this->m_model, true);
    this->m_graphicsShader.SetVec3f("tint", this->m_strokeColor, true);
    this->m_graphicsShader.SetFloat("alphaVal", this->m_alpha, true);

    this->Bind_Buffer(vertices, this->VBO, 0, 2, GL_FLOAT, GL_FALSE, GL_STATIC_DRAW, 2 * sizeof(float));
    
    #ifdef __EMSCRIPTEN__
        this->Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 2, 0);
    #else
        this->Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 2, drawStyle == 0 ? GL_FILL : GL_LINE); 
    #endif
} 


//------------------------------------- triangle



void Triangle::Render(int drawStyle)
{

    //this->m_model = glm::mat4(1.0f); 
    //this->m_model = glm::translate(this->m_model, glm::vec3(this->x, this->y, 0.0f));  

    float vertices[18] = {

        0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
       -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
    };

    this->m_graphicsShader.SetMat4("model", this->m_model, true);
    this->m_graphicsShader.SetVec3f("tint", this->m_fillColor, true);
    this->m_graphicsShader.SetFloat("alphaVal", this->m_alpha, true);

    this->Bind_Buffer(vertices, this->VBO, 0, 3, GL_FLOAT, GL_FALSE, GL_STATIC_DRAW, 3 * sizeof(float));

    #ifdef __EMSCRIPTEN__
        this->Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 3, 0);
    #else
        this->Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 3, drawStyle == 0 ? GL_FILL : GL_LINE); 
    #endif
    
}

//basic rectangle 
void Rectangle::Render(int drawStyle) 
{

    this->m_model = glm::mat4(1.0f); 

    if (strcmp(this->m_type, "cursor") == 0)
        this->m_model = glm::translate(this->m_model, glm::vec3(this->x, this->y, 0.0f));  

    this->m_model = glm::translate(this->m_model, glm::vec3(0.5f * this->width + this->x, 0.5f * this->height + this->y, 0.0f)); 

    if (this->rotation > 0)
        this->m_model = glm::rotate(this->m_model, glm::radians(this->rotation), glm::vec3(0.0f, 0.0f, 1.0f)); 

    this->m_model = glm::translate(this->m_model, glm::vec3(-0.5f * this->width - this->x, -0.5f * this->height - this->y, 0.0f));

    this->m_graphicsShader.SetVec3f("tint", this->m_strokeColor, true);
    this->m_graphicsShader.SetMat4("model", this->m_model, true);  
    this->m_graphicsShader.SetFloat("alphaVal", this->m_alpha, true);       

    //triangle A

    this->vertices[0] = this->x + this->width;  
    this->vertices[1] = this->y;
    this->vertices[2] = this->x + this->width;
    this->vertices[3] = this->y + this->height;
    this->vertices[4] = this->x;
    this->vertices[5] = this->y;

    //triangle B

    this->vertices[6] = this->x + this->width;
    this->vertices[7] = this->y + this->height;
    this->vertices[8] = this->x;
    this->vertices[9] = this->y + this->height;
    this->vertices[10] = this->x;
    this->vertices[11] = this->y;

    this->Bind_Buffer(this->vertices, this->VBO, 0, 2, GL_SHORT, GL_FALSE, GL_DYNAMIC_DRAW, 2 * sizeof(short)); 

    #ifdef __EMSCRIPTEN__
        this->Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 6, 0);
    #else
        this->Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 6, drawStyle == 0 ? GL_FILL : GL_LINE); 
    #endif
}

//quad 
void Quad::Render() 
{

    this->m_model = glm::mat4(1.0f); 

    this->m_model = glm::translate(this->m_model, glm::vec3(0.5f * this->width + this->m_position.x, 0.5f * this->height + this->m_position.y, 0.0f)); 

    if (this->m_rotation > 0)
        this->m_model = glm::rotate(this->m_model, glm::radians(this->m_rotation), glm::vec3(0.0f, 0.0f, 1.0f)); 

    this->m_model = glm::translate(this->m_model, glm::vec3(-0.5f * this->width - this->m_position.x, -0.5f * this->height - this->m_position.y, 0.0f));

    this->m_shader.SetVec3f("tint", this->m_strokeColor, true);
    this->m_shader.SetMat4("model", this->m_model, true);  
    this->m_shader.SetFloat("alphaVal", this->m_alpha, true);       

    //triangle A

    this->vertices[0] = this->m_position.x + this->width;  
    this->vertices[1] = this->m_position.y;
    this->vertices[2] = this->m_position.x + this->width;
    this->vertices[3] = this->m_position.y + this->height;
    this->vertices[4] = this->m_position.x;
    this->vertices[5] = this->m_position.y;

    //triangle B

    this->vertices[6] = this->m_position.x + this->width;
    this->vertices[7] = this->m_position.y + this->height;
    this->vertices[8] = this->m_position.x;
    this->vertices[9] = this->m_position.y + this->height;
    this->vertices[10] = this->m_position.x;
    this->vertices[11] = this->m_position.y;

    this->Bind_Buffer(this->vertices, this->VBO, 0, 2, GL_SHORT, GL_FALSE, GL_DYNAMIC_DRAW, 2 * sizeof(short)); 

    #ifdef __EMSCRIPTEN__
        this->Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 6, 0);
    #else
        this->Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 6, GL_FILL); 
    #endif
}

//--------------------------------- Texture 2D


Texture2D& Texture2D::GetTexture(const std::string &key) {
    return System::Resources::Manager::textures[key];
}


//-----------------------------------


void Texture2D::SetChannels(Texture2D &texture, unsigned int channels) 
{
    
    texture.Channels = channels;

    if (texture.Channels == 4)
    { 
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
    glDeleteVertexArrays(1, &tex->quadVAO); 
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
    this->GenBuffer();
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

    this->Bind_Buffer(vertices, this->VBO, 0, 2, GL_SHORT, GL_FALSE, GL_DYNAMIC_DRAW, 2 * sizeof(short)); 
    this->Bind_Buffer(UVs, this->UVBO, 1, 2, GL_FLOAT, GL_TRUE, GL_STATIC_DRAW, 2 * sizeof(GLfloat));

    #ifdef __EMSCRIPTEN__
        this->Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 6, 0);
    #else
        this->Draw(GL_TRIANGLES, GL_TEXTURE_2D, GL_TEXTURE0, 6, GL_FILL); 
    #endif
}
