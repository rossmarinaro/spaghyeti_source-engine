#define STB_IMAGE_IMPLEMENTATION
#include "../../vendors/stb/stb_image.h" 
#include "../../../build/sdk/include/app.h"
#include "../../shared/renderer.h"

using namespace Graphics;


Texture2D::Texture2D():
    m_internal_format(GL_RGB32F), 
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
    Channels(3),
    Repeat(1),
    Whiteout(0)
{ 
    glGenTextures(1, &ID); 

    glGenVertexArrays(1, &m_VAO); 
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_UVBO);
    glBindVertexArray(m_VAO); 
}
 

//-------------------------------


void Texture2D::Delete() {
    glDeleteTextures(1, &ID);    
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_UVBO); 
}


//-------------------------------


const void Texture2D::Bind() { 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ID);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
}; 


//-------------------------------


const Texture2D& Texture2D::Get(const std::string& key) {
    return System::Application::resources->textures[
        System::Application::resources->textures.find(key) != System::Application::resources->textures.end() ? 
            key : "base"];
}


//-----------------------------------


void Texture2D::SetFiltering(bool filterMin, bool filterMax, bool wrapS, bool wrapT) 
{
    Filter_Min = filterMin ? GL_NEAREST : GL_LINEAR;
    Filter_Max = filterMax ? GL_NEAREST : GL_LINEAR;
    Wrap_S = wrapS ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    Wrap_T = wrapT ? GL_REPEAT : GL_CLAMP_TO_EDGE;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Wrap_S);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Wrap_T);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter_Min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter_Max); 
}


//-----------------------------------


void Texture2D::Load(const std::string& key) 
{

    std::string filetype = "none";

    int width = 1, 
        height = 1, 
        nrChannels = 4;

    //creates texture objects

    Texture2D texture;
    unsigned char* image = nullptr;
    const auto filepath = System::Resources::Manager::GetFilePath(key);
 
    stbi_set_flip_vertically_on_load(false);

    //file asset found in cache

    if (filepath) {
        filetype = "filepath";
        image = stbi_load((*filepath).c_str(), &width, &height, &nrChannels, 0);
    }

    //byte encoded array buffer

    else 
    {   
        filetype = "binary";
  
        const auto data = System::Resources::Manager::GetResource(key);

        if (data) 
        {
            unsigned char* image_buffer = (unsigned char*)data->array_buffer;

            image = stbi_load_from_memory(image_buffer, data->byte_length, &width, &height, &nrChannels, 0);

            //image is compressed pixel data

            if (image == nullptr) {
                filetype = "compressed pixel data";
                image = image_buffer;
            }
        }

        else {
            LOG("Texture2D: Image of key: \"" + key + "\" failed to load pixel data.");    
            return;
        }
    }

    if (image == nullptr) {
        LOG("Texture2D: Image of key: \"" + key + "\" failed to load. (" + filetype + ")");    
        return;
    }

    texture.Generate(width, height, nrChannels, image); 

    if (filetype != "compressed pixel data")
        stbi_image_free(image);

    texture.key = key;

    System::Application::resources->textures[key] = texture; 

    LOG("Texture2D: \"" + key + "\" loaded. (" + filetype + ")");

} 


//--------------------------------------


void Texture2D::UnLoad(const std::string& key) {

    const auto it = System::Application::resources->textures.find(key.c_str());

    if (it != System::Application::resources->textures.end()) {

        (*it).second.Delete();

        System::Application::resources->textures.erase(it);
    }

    LOG("Texture2D: \"" + key + "\" deleted.");

}


//--------------------------------------


void Texture2D::Generate(unsigned int width, unsigned int height, unsigned int channels, const void* data)
{
    Width = width;
    Height = height; 

    FrameWidth = Width;
    FrameHeight = Height;

    Channels = channels;

    if (Channels == 4) 
    { 
        #ifdef __EMSCRIPTEN__
            m_internal_format = GL_RGBA;  
        #else
            m_internal_format = GL_RGBA32F;
        #endif

        m_image_format = GL_RGBA;
    }

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


void Texture2D::Update(const Math::Vector2& position, bool flipX, bool flipY, int drawStyle, float thickness) 
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

    
    const Renderable texture = { position.x, position.y, offset }; //posX, posY, UV


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

    glBindVertexArray(m_VAO); 
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, 2 * sizeof(short), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(m_VAO); 
    glBindBuffer(GL_ARRAY_BUFFER, m_UVBO);    
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_UVs), m_UVs, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);    

    #ifndef __EMSCRIPTEN__

        if (drawStyle == GL_LINE) {
            glDisable(GL_LINE_SMOOTH);
            glLineWidth(thickness);
        }

        glDrawArrays(drawStyle == GL_LINE ?  GL_LINES : GL_TRIANGLES, 0, 6);

    #else
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
    #endif

    glBindVertexArray(0); 
}

