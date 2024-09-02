#define STB_IMAGE_IMPLEMENTATION
#include "../../vendors/stb/stb_image.h" 

#include "../../../build/sdk/include/manager.h"
#include "../../../build/sdk/include/app.h"

using namespace Graphics;


Texture2D::Texture2D():
    m_channels(3),
    m_internal_format(GL_RGB), 
    m_image_format(GL_RGB),
    Width(0.0f),
    Height(0.0f),
    FrameWidth(0.0f),
    FrameHeight(0.0f),
    U1(0.0f),
    V1(0.0f),
    U2(1.0f),
    V2(1.0f),
    Wrap_S(GL_REPEAT),
    Wrap_T(GL_REPEAT), 
    Filter_Min(GL_NEAREST),   
    Filter_Max(GL_NEAREST),
    Repeat(1)
{ 
    glGenTextures(1, &ID); 

    glGenVertexArrays(1, &VAO); 
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &UVBO);
    glBindVertexArray(VAO); 
}


//-------------------------------


void Texture2D::Delete() {
    glDeleteTextures(1, &ID);   
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &UVBO); 
}



//-------------------------------


Texture2D& Texture2D::Get(const std::string& key) {
    return System::Application::resources->textures[key];
}


//-----------------------------------


void Texture2D::SetChannels(Texture2D& texture, unsigned int channels) 
{
    
    texture.m_channels = channels;

    if (texture.m_channels == 4) { 
        texture.m_internal_format = GL_RGBA;
        texture.m_image_format = GL_RGBA;
    }
}


//-----------------------------------


void Texture2D::SetFiltering() 
{
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Wrap_S);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Wrap_T);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter_Min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter_Max); 
}


//-----------------------------------


void Texture2D::Load(const std::string& key, bool flipY) 
{

    bool success = false;

    //creates texture objects

    Texture2D texture;

    const char* filepath = System::Resources::Manager::GetFilePath(key);

    //file asset found in cache

    if (strcmp(filepath, "not found") != 0) 
    {

        int width, height, nrChannels;    

        stbi_set_flip_vertically_on_load(flipY);

        unsigned char* data = stbi_load(filepath, &width, &height, &nrChannels, 0);

        SetChannels(texture, nrChannels);

        if (data)
        {
            texture.Generate(width, height, data); 

            stbi_image_free(data); 

            success = true;
        }

        else 
            LOG("Texture2D: image file loading failed.");    
      
    }

    //raw asset

    else
    {   

        auto rawData = System::Resources::Manager::GetRawData(key); 

        if (strcmp(rawData, "not found") != 0) 
        {        

            std::array<int, 3> dimensionsAndChannels = System::Resources::Manager::GetRawDimensionsAndChannels(key);
   
            SetChannels(texture, dimensionsAndChannels[2]);

            texture.Generate(dimensionsAndChannels[0], dimensionsAndChannels[1], rawData);

            success = true;
        }
    }

    if (!success) {

        LOG("Texture2D: Image of key: " + key + " not found.");    
   
        return;
    }
    
    else 
        LOG("Texture2D: " + key + " loaded.");
      
    //assign texture in manager

    System::Application::resources->textures[key] = texture;  

} 


//--------------------------------------


void Texture2D::UnLoad(const std::string& key) 
{

    auto it = System::Application::resources->textures.find(key);

    if (it != System::Application::resources->textures.end()) {

        (*it).second.Delete();

        System::Application::resources->textures.erase(it);

    }

    LOG("Texture2D: Deleted texture " + key);

}


//--------------------------------------


void Texture2D::Generate(unsigned int width, unsigned int height, const void* data)
{

    Width = width;
    Height = height; 

    FrameWidth = Width;
    FrameHeight = Height;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ID); 
    
    #ifndef __EMSCRIPTEN__
        glGenerateMipmap(GL_TEXTURE_2D);
    #endif

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   
 
    glTexImage2D(GL_TEXTURE_2D, 0, m_internal_format, Width, Height, 0, m_image_format, GL_UNSIGNED_BYTE, data);

    //set Texture wrap and filter modes

    SetFiltering();

    //unbind texture 

    glBindTexture(GL_TEXTURE_2D, 0);

    Bind();

}

//----------------------------------------


void Texture2D::Update(const glm::vec2& position, bool flipX, bool flipY, int drawStyle, float thickness) 
{   

    //format texture
 
    Format offset;

    //flip X

    if (flipX && !flipY) 
        offset = { FrameWidth, FrameHeight, U2, V2, U1, V1 }; 

    //flip Y

    else if (!flipX && flipY)
        offset = { FrameWidth, FrameHeight, U1, V1, U2, V2 }; 

    //flip X, Y

    else if (flipX && flipY)
        offset = { FrameWidth, FrameHeight, U2, V1, U1, V2 }; 

    //no flip

    else
        offset = { FrameWidth, FrameHeight, U1, V2, U2, V1 };

    
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

        m_UVs[0] = texture.format.u2;
        m_UVs[1] = texture.format.v2;

    //bottom right

        m_UVs[2] = texture.format.u2;
        m_UVs[3] = texture.format.v1;

    //top left

        m_UVs[4] = texture.format.u1;
        m_UVs[5] = texture.format.v2;

    //bottom right 

        m_UVs[6] = texture.format.u2;
        m_UVs[7] = texture.format.v1;

    //bottom left

        m_UVs[8] = texture.format.u1;
        m_UVs[9] = texture.format.v1;

    //top left

        m_UVs[10] = texture.format.u1;
        m_UVs[11] = texture.format.v2;

    Bind();

    glBindVertexArray(VAO); 
    glBindBuffer(GL_ARRAY_BUFFER, VBO);    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, 2 * sizeof(short), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAO); 
    glBindBuffer(GL_ARRAY_BUFFER, UVBO);    
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_UVs), m_UVs, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);    

    #ifndef __EMSCRIPTEN__
        glPolygonMode(GL_FRONT_AND_BACK, drawStyle);
    #endif
    
    glLineWidth(thickness);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
}

