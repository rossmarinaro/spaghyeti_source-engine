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
    m_textureUnit = System::Application::resources->textures.size() ? System::Application::resources->textures.size() /* - 1 */ : 0;

    glGenTextures(1, &ID); 
}
 

//-------------------------------


void Texture2D::Delete() {
    glDeleteTextures(1, &ID);    
    glBindTexture(GL_TEXTURE_2D, 0);
}


//-------------------------------


const void Texture2D::Bind() { 
    glActiveTexture(GL_TEXTURE0 + m_textureUnit);
    glBindTexture(GL_TEXTURE_2D, ID);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
}; 


//-------------------------------


const Texture2D& Texture2D::Get(const std::string& key) 
{
    if (System::Application::resources->textures.find(key) == System::Application::resources->textures.end()) {
        LOG("Texture2D: texture of key: " + key + " not loaded, defaulting to base texture.");
    }

    return System::Application::resources->textures[
        System::Application::resources->textures.find(key) != System::Application::resources->textures.end() ? key : "base"];
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
    if (System::Application::resources->textures.size() > 32) {
        LOG("Texture2D: cannot load texture. Max textures reached (32).")
        return;
    }

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

    texture.Width = width;
    texture.Height = height; 

    texture.FrameWidth = width;
    texture.FrameHeight = height;

    texture.Channels = nrChannels;

    if (texture.Channels == 4) 
    { 
        #ifdef __EMSCRIPTEN__
            texture.m_internal_format = GL_RGBA;  
        #else
            texture.m_internal_format = GL_RGBA32F;
        #endif

        texture.m_image_format = GL_RGBA;
    }

    texture.Bind();
    
    #ifndef __EMSCRIPTEN__
        glGenerateMipmap(GL_TEXTURE_2D);
    #endif

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   
    glTexImage2D(GL_TEXTURE_2D, 0, texture.m_internal_format, texture.Width, texture.Height, 0, texture.m_image_format, GL_UNSIGNED_BYTE, image);

    //set Texture wrap and filter modes

    texture.SetFiltering();

    //unbind texture 

    glBindTexture(GL_TEXTURE_2D, 0);

    if (filetype != "compressed pixel data")
        stbi_image_free(image);

    texture.key = key;

    System::Application::resources->textures[key] = texture; 

    LOG("Texture2D: \"" + key + "\" loaded. (" + filetype + ")");

} 


//--------------------------------------


void Texture2D::UnLoad(const std::string& key) 
{
    const auto it = System::Application::resources->textures.find(key.c_str());

    if (it != System::Application::resources->textures.end()) {

        (*it).second.Delete();

        System::Application::resources->textures.erase(it);
    }

    LOG("Texture2D: \"" + key + "\" deleted.");

}

//----------------------------------------

void Texture2D::Update(const Math::Vector2& position, bool flipX, bool flipY, int drawStyle, float thickness) 
{   
    //format texture
 
    Format offset;

    //flip X

    if (flipX && !flipY) 
        offset = { FrameWidth, FrameHeight, U2, V1, U1, V2 }; // offset = { FrameWidth, FrameHeight, U2, V2, U1, V1 }; 

    //flip Y

    else if (!flipX && flipY)
        offset = { FrameWidth, FrameHeight, U1, V1, U2, V2 }; 

    //flip X, Y

    else if (flipX && flipY)
        offset = { FrameWidth, FrameHeight, U2, V1, U1, V2 }; 

    //no flip

    else
        offset = { FrameWidth, FrameHeight, U1, V1, U2, V2 };//offset = { FrameWidth, FrameHeight, U1, V2, U2, V1 };

    
    const Renderable renderable = { position.x, position.y, offset }; //posX, posY, UV


    //----------------- vertices 
 
    // Vertex vertices[4] = {
    //     { renderable.x, renderable.y, renderable.format.u1, renderable.format.v1, m_textureUnit },
    //     { renderable.x + renderable.format.width, renderable.y, renderable.format.u2, renderable.format.v1, m_textureUnit },
    //     { renderable.x + renderable.format.width, renderable.y + renderable.format.height, renderable.format.u2, renderable.format.v2, m_textureUnit },
    //     { renderable.x, renderable.y + renderable.format.height, renderable.format.u1, renderable.format.v2, m_textureUnit }
    // }; 
            int elementCount = 6 * System::Renderer::MAX_QUADS;
if (System::Renderer::indexCount >= elementCount/*  || textureSlotIndex > MAX_TEXTURES */) {

    //glDrawArrays(drawStyle == GL_LINE ?  GL_LINES : GL_TRIANGLES, 0, 6);
System::Renderer::Flush();LOG(0);
}

    auto qp = System::Renderer::GetQuadBufferPtr(); 
    const float texID = System::Renderer::GetTextureIndex(ID);
    if (qp)
    {
        qp->x = renderable.x;
    qp->y = renderable.y;
    qp->u = renderable.format.u1;
    qp->v = renderable.format.v1;
    qp->texID = texID/* m_textureUnit */;
    qp++;

    qp->x = renderable.x + renderable.format.width;
    qp->y = renderable.y;
    qp->u = renderable.format.u2;
    qp->v = renderable.format.v1;
    qp->texID = texID/* m_textureUnit */;
    qp++;

    qp->x = renderable.x + renderable.format.width;
    qp->y = renderable.y + renderable.format.height;
    qp->u = renderable.format.u2;
    qp->v = renderable.format.v2;
    qp->texID = texID/* m_textureUnit */;
    qp++;

	qp->x = renderable.x;
    qp->y = renderable.y + renderable.format.height;
    qp->u = renderable.format.u1;
    qp->v = renderable.format.v2;
    qp->texID = texID/* m_textureUnit */;
    qp++;
    }

    //short vertices[12];
    //float uvs[12];

    // //top right

    //     vertices[0] = renderable.x + renderable.format.width; 
    //     vertices[1] = renderable.y;

    // //bottom right

    //     vertices[2] = renderable.x + renderable.format.width;
    //     vertices[3] = renderable.y + renderable.format.height;

    // //top left

    //     vertices[4] = renderable.x;
    //     vertices[5] = renderable.y;

    // //bottom right

    //     vertices[6] = renderable.x + renderable.format.width;
    //     vertices[7] = renderable.y + renderable.format.height;

    // //bottom left

    //     vertices[8] = renderable.x;
    //     vertices[9] = renderable.y + renderable.format.height;

    // //top left

    //     vertices[10] = renderable.x;
    //     vertices[11] = renderable.y;

       
    // //----------------- uvs
        
    // //top right

    //     uvs[0] = renderable.format.u2;
    //     uvs[1] = renderable.format.v2;

    // //bottom right

    //     uvs[2] = renderable.format.u2;
    //     uvs[3] = renderable.format.v1;

    // //top left

    //     uvs[4] = renderable.format.u1;
    //     uvs[5] = renderable.format.v2;

    // //bottom right 

    //     uvs[6] = renderable.format.u2;
    //     uvs[7] = renderable.format.v1;

    // //bottom left

    //     uvs[8] = renderable.format.u1;
    //     uvs[9] = renderable.format.v1;

    // //top left

    //     uvs[10] = renderable.format.u1;
    //     uvs[11] = renderable.format.v2; 

    //Bind();

    // glBindVertexArray(VAO); 
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);    
    // glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(short), vertices.data()); 

    // glBindVertexArray(VAO); 
    // glBindBuffer(GL_ARRAY_BUFFER, UVBO);    
    // glBufferSubData(GL_ARRAY_BUFFER, 0, uvs.size() * sizeof(GLfloat), uvs.data());  

    #ifndef __EMSCRIPTEN__

        if (drawStyle == GL_LINE) {
            glDisable(GL_LINE_SMOOTH);
            glLineWidth(thickness);
        } 
 



   //std::copy(vertices, vertices + sizeof(vertices) / sizeof(vertices[0]), std::back_inserter(System::Game::GetScene()->batchSprites_verts));
    System::Renderer::indexCount += 6;
 //static_cast<Vertex>(System::Renderer::GetQuadBufferPtr() += 4;
System::Renderer::QuadCount++;
    #else
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
    #endif

}

