#define STB_IMAGE_IMPLEMENTATION
#include "../../vendors/stb/stb_image.h" 
#include "../../../build/sdk/include/app.h"
#include "../../shared/renderer.h"
#include "../../vendors/glm/glm.hpp"
#include "../../vendors/glm/gtc/type_ptr.hpp"

using namespace Graphics;


Texture2D::Texture2D():
    m_internal_format(GL_RGB32F), 
    m_image_format(GL_RGB),
    m_opaque(true)
{ 
    Width = 0.0f;
    Height = 0.0f;
    FrameWidth = 0.0f;
    FrameHeight = 0.0f;
    U1 = 0.0f;
    V1 = 0.0f;
    U2 = 1.0f;
    V2 = 1.0f;
    Wrap_S = GL_REPEAT;
    Wrap_T = GL_REPEAT;
    Filter_Min = GL_NEAREST;   
    Filter_Max = GL_NEAREST;
    Channels = 3;
    Repeat = 1;
    Whiteout = 0;

    glGenTextures(1, &ID); 
}


//-------------------------------


const bool Texture2D::IsOpaque() {
    return m_opaque;
}
 

//-------------------------------


void Texture2D::Delete() {
    glDeleteTextures(1, &ID);    
    glBindTexture(GL_TEXTURE_2D, 0);
}


//------------------------------- base texture at index 0


void Texture2D::InitBaseTexture() 
{   
    const std::string key = "base";

    Texture2D baseTexture;

    baseTexture.Width = 1;
    baseTexture.Height = 1; 
    baseTexture.FrameWidth = 1;
    baseTexture.FrameHeight = 1;
    baseTexture.Channels = 4;
    baseTexture.m_image_format = GL_RGBA8;
    baseTexture.key = key;

    glGenTextures(1, &baseTexture.ID);
	glBindTexture(GL_TEXTURE_2D, baseTexture.ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   

    glEnable(GL_TEXTURE_2D);
    
	uint32_t color = 0xffffffff; //white

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);

    #ifndef __EMSCRIPTEN__
        glGenerateMipmap(GL_TEXTURE_2D);
    #endif
    
    System::Application::resources->textures[key] = baseTexture; 
    System::Renderer::Get()->textureSlots[0] = baseTexture.ID; 

    glBindTexture(GL_TEXTURE_2D, 0);

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

    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Wrap_S);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Wrap_T);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter_Min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter_Max); 
    glBindTexture(GL_TEXTURE_2D, 0);
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
    unsigned char* image_data = nullptr;
    const auto filepath = System::Resources::Manager::GetFilePath(key);
 
    stbi_set_flip_vertically_on_load(false);

    //file asset found in cache

    if (filepath) {
        filetype = "filepath";
        image_data = stbi_load((*filepath).c_str(), &width, &height, &nrChannels, 4);
    }

    //byte encoded array buffer

    else 
    {   
        filetype = "binary";
  
        const auto data = System::Resources::Manager::GetResource(key);

        if (data) 
        {
            unsigned char* image_buffer = (unsigned char*)data->array_buffer;

            image_data = stbi_load_from_memory(image_buffer, data->byte_length, &width, &height, &nrChannels, 0);

            //image is compressed pixel data

            if (image_data == nullptr) {
                filetype = "compressed pixel data";
                image_data = image_buffer;
            }
        }

        else {
            LOG("Texture2D: Image of key: \"" + key + "\" failed to load pixel data.");    
            return;
        }
    }

    if (image_data == nullptr) {
        LOG("Texture2D: Image of key: \"" + key + "\" failed to load. (" + filetype + ")");    
        return;
    }

    texture.key = key;
    texture.Width = width;
    texture.Height = height; 
    texture.FrameWidth = width;
    texture.FrameHeight = height;
    texture.Channels = nrChannels;

    if (texture.Channels == 4) { 
        #ifdef __EMSCRIPTEN__
            texture.m_internal_format = GL_RGBA;  
        #else
            texture.m_internal_format = GL_RGBA32F;
        #endif

        texture.m_image_format = GL_RGBA;

        //check texture opacity

        for (int i = 3; i < texture.Width * texture.Height * 4; i += 4) 
            if (image_data[i] < 255) {
                texture.m_opaque = false;
                break;
            } 
    }

    glBindTexture(GL_TEXTURE_2D, texture.ID);
    
    #ifndef __EMSCRIPTEN__
        glGenerateMipmap(GL_TEXTURE_2D);
    #endif

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   
    glTexImage2D(GL_TEXTURE_2D, 0, texture.m_internal_format, texture.Width, texture.Height, 0, texture.m_image_format, GL_UNSIGNED_BYTE, image_data);

    texture.SetFiltering();

    if (filetype != "compressed pixel data")
        stbi_image_free(image_data);

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

//---------------------------------------- updates textures position and texture coordinates, appends to array for rendering

void Texture2D::Update(
    unsigned int shaderID, 
    const Math::Vector2& position, 
    const Math::Vector2& scale,
    const Math::Vector4& rgba, 
    const Math::Vector3& outline, 
    const Math::Matrix4& modelView, 
    float outlineWidth,
    float rotation,
    int depth,
    bool flipX, 
    bool flipY
) 
{   
    auto renderer = System::Renderer::Get();
    const int elementCount = 6 * System::Renderer::MAX_QUADS;

    //flush if max index count exceeds element count, or textures reached max OR shader is different than renderer's active shader

    if (renderer->indexCount >= elementCount || 
        renderer->textureSlotIndex > System::Renderer::MAX_TEXTURES - 1 ||
        shaderID != renderer->activeShaderID
    ) 
        System::Renderer::Flush();

    //set active shader

    renderer->activeShaderID = shaderID;

    //format texture
 
    Format offset;

    if (flipX && !flipY) //flip x
        offset = { FrameWidth, FrameHeight, U2, V1, U1, V2 };

    else if (!flipX && flipY) //flip y
        offset = { FrameWidth, FrameHeight, U1, V2, U2, V1 }; 

    else if (flipX && flipY) //flip x, y
        offset = { FrameWidth, FrameHeight, U2, V2, U1, V1 }; 

    else //no flip
        offset = { FrameWidth, FrameHeight, U1, V1, U2, V2 };

    //get / set texture unit from texture ID

    float textureUnit = 0.0f;

	const auto texture_exists = [&]() -> bool {
        for (uint32_t i = 0; i < renderer->textureSlotIndex; i++)
            if (renderer->textureSlots[i] == ID) {
                textureUnit = (float)i;
                return true;
            }
        return false;
    };

    if (!texture_exists()) {
        textureUnit = (float)renderer->textureSlotIndex;
        renderer->textureSlots[renderer->textureSlotIndex] = ID;
        renderer->textureSlotIndex++;
    }

    //update texture vertices attributes with data to be mapped to vertex shader

    const Renderable renderable = { position.x, position.y, offset }; //x, y, uvs

    Math::Graphics::Vertex vertices[4];

    //position and uv
    
    vertices[0].x = renderable.x;
    vertices[0].y = renderable.y;
    vertices[0].u = renderable.format.u1;
    vertices[0].v = renderable.format.v1;

    vertices[1].x = renderable.x + renderable.format.width;
    vertices[1].y = renderable.y;
    vertices[1].u = renderable.format.u2;
    vertices[1].v = renderable.format.v1;

    vertices[2].x = renderable.x + renderable.format.width;
    vertices[2].y = renderable.y + renderable.format.height;
    vertices[2].u = renderable.format.u2;
    vertices[2].v = renderable.format.v2;

    vertices[3].x = renderable.x;
    vertices[3].y = renderable.y + renderable.format.height;
    vertices[3].u = renderable.format.u1;
    vertices[3].v = renderable.format.v2;

    //other attributes

    const glm::mat4 modelMat = glm::mat4({ modelView.a.r, modelView.a.g, modelView.a.b, modelView.a.a }, 
        { modelView.b.r, modelView.b.g, modelView.b.b, modelView.b.a }, 
        { modelView.c.r, modelView.c.g, modelView.c.b, modelView.c.a }, 
        { modelView.d.r, modelView.d.g, modelView.d.b, modelView.d.a }); 

    for (int i = 0; i < 4; i++) 
    {       
        vertices[i].texID = textureUnit;
        vertices[i].z = static_cast<float>(depth) / 1000.0f;
        vertices[i].rotation = rotation;
        vertices[i].scaleX = scale.x;
        vertices[i].scaleY = scale.y;
        vertices[i].r = rgba.r;
        vertices[i].g = rgba.g;
        vertices[i].b = rgba.b;
        vertices[i].a = rgba.a;
        vertices[i].outlineR = outline.x;
        vertices[i].outlineG = outline.y;
        vertices[i].outlineB = outline.z;
        vertices[i].outlineWidth = outlineWidth;
        
        //copy float array into a standard glm mat4

        std::memcpy(vertices[i].modelView, glm::value_ptr(modelMat), sizeof(vertices[i].modelView)); 
    }  

    //add the vertices to the renderer's vector and increase index count by 6
    
    std::copy(vertices, vertices + sizeof(vertices) / sizeof(vertices[0]), std::back_inserter(renderer->vertices));

    renderer->indexCount += 6;

}

