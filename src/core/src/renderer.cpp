#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/camera.h"


void System::Renderer::Update(Camera* camera)
{

    glfwSwapInterval(s_vsync); // Enable vsync

    glEnable(GL_TEXTURE_2D); 
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 
    glClear(GL_COLOR_BUFFER_BIT);
    
    Shader::Update(camera);

}


//---------------------------------

GLuint texture_id;

void System::Renderer::CreateFrameBuffer()
{

    for (auto &tex : System::Application::resources->textures)
    {
        //tex.second.ID
    }

    glGenFramebuffers(1, &s_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, s_FBO);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 400, 400, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, s_RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 800);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, s_RBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Error::FRAMEBUFFER:: Incomplete Buffer.\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

}


//---------------------------------


void System::Renderer::RescaleFrameBuffer(float width, float height)
{
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, s_RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, s_RBO);
}



//--------------------------------------



// static std::array<uint32_t, BatchRenderer::MaxTextures> TextureSlots;


// void BatchRenderer::Init()
// {

//     m_Data.QuadBuffer = new Vertex[MaxVertexCount];

//     glGenVertexArrays(1, &m_Data.QuadVA); 
//     glBindVertexArray(m_Data.QuadVA);
    
//     glGenBuffers(1, &m_Data.QuadVB);
//     glBind_Buffer(GL_ARRAY_BUFFER, m_Data.QuadVB);
//     glBufferData(GL_ARRAY_BUFFER, MaxVertexCount * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Position));
//     glEnableVertexAttribArray(0);

//     glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Color));
//     glEnableVertexAttribArray(1);

//     glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexCoords));
//     glEnableVertexAttribArray(2);

//     glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexIndex));
//     glEnableVertexAttribArray(3);

//     uint32_t indices[MaxIndexCount];
//     uint32_t offset = 0;

//     for (int i = 0; i < MaxIndexCount; i += 6)
//     {
//         indices[i + 0] = 0 + offset;
//         indices[i + 1] = 1 + offset;
//         indices[i + 2] = 2 + offset;
//         indices[i + 3] = 2 + offset;
//         indices[i + 4] = 3 + offset;
//         indices[i + 5] = 0 + offset;

//         offset += 4;
//     }

//     glGenBuffers(1, &m_Data.QuadIB);//glCreateBuffers(1, &m_Data.QuadIB);
//     glBind_Buffer(GL_ELEMENT_ARRAY_BUFFER, m_Data.QuadIB);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

//     glGenTextures(1, &m_Data.WhiteTexture);//glCreateTextures(GL_TEXTURE_2D, 1, &m_Data.WhiteTexture);
//     glBindTexture(GL_TEXTURE_2D, m_Data.WhiteTexture);


//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//     uint32_t color = 0xffffffff;
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);
    
//     TextureSlots[0] = m_Data.WhiteTexture;

//     for (size_t i = 1; i < MaxTextures; i++)
//         TextureSlots[i] = 0; 
// }

// void BatchRenderer::Render(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color, float textureIndex)
// {

//     m_Data.QuadBufferPtr->Position = { position.x, position.y, 0.0f };
//     m_Data.QuadBufferPtr->Color = color;
//     m_Data.QuadBufferPtr->TexCoords = { 0.0f, 0.0f };
//     m_Data.QuadBufferPtr->TexIndex = textureIndex;
//     m_Data.QuadBufferPtr++;

//     m_Data.QuadBufferPtr->Position = { position.x + size.x, position.y, 0.0f };
//     m_Data.QuadBufferPtr->Color = color;
//     m_Data.QuadBufferPtr->TexCoords = { 1.0f, 0.0f };
//     m_Data.QuadBufferPtr->TexIndex = textureIndex;
//     m_Data.QuadBufferPtr++;

//     m_Data.QuadBufferPtr->Position = { position.x + size.x, position.y + size.y, 0.0f };
//     m_Data.QuadBufferPtr->Color = color;
//     m_Data.QuadBufferPtr->TexCoords = { 1.0f, 1.0f };
//     m_Data.QuadBufferPtr->TexIndex = textureIndex;
//     m_Data.QuadBufferPtr++;

//     m_Data.QuadBufferPtr->Position = { position.x, position.y + size.y, 0.0f };
//     m_Data.QuadBufferPtr->Color = color;
//     m_Data.QuadBufferPtr->TexCoords = { 0.0f, 1.0f };
//     m_Data.QuadBufferPtr->TexIndex = textureIndex;
//     m_Data.QuadBufferPtr++;

//     m_Data.IndexCount += 6;
    
//     RenderStats.QuadCount++;


// }

// void BatchRenderer::ShutDown()
// {

//     glDeleteVertexArrays(1, &m_Data.QuadVA); 
//     glDeleteBuffers(1, &m_Data.QuadVB);
//     glDeleteBuffers(1, &m_Data.QuadIB);

//     glDeleteTextures(1, &m_Data.WhiteTexture);

//     delete[] m_Data.QuadBuffer;
// }

// void BatchRenderer::Begin()
// {
//     m_Data.QuadBufferPtr = m_Data.QuadBuffer;
// }

// void BatchRenderer::End()
// {

//     GLsizeiptr size = (uint8_t*)m_Data.QuadBufferPtr - (uint8_t*)m_Data.QuadBuffer;
//     glBind_Buffer(GL_ARRAY_BUFFER, m_Data.QuadVB);
//     glBufferSubData(GL_ARRAY_BUFFER, 0, size, m_Data.QuadBuffer);

//     //flush

//     for (uint32_t i = 0; i < m_Data.TextureSlotIndex; i++)
//     {
//         //glBindTextureUnit(0, m_Data.TextureSlots[0]);
//         glActiveTexture(GL_TEXTURE0 + i);
//         glBindTexture(i, TextureSlots[i]);
//     }

//     glBindVertexArray(m_Data.QuadVA);
//     glDrawElements(GL_TRIANGLES, m_Data.IndexCount, GL_UNSIGNED_INT, nullptr);
//     /* m_Data. */RenderStats.DrawCount++;
//     m_Data.IndexCount = 0;
    
// }


// void BatchRenderer::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color)
// {
//     if (m_Data.IndexCount >= MaxIndexCount)
//     {
//         End();
//         Begin();
//     }

//     float textureIndex = 0.0f;

//     BatchRenderer::Render(position, size, color, textureIndex);

// }

// void BatchRenderer::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, uint32_t texID)
// {
//     if (m_Data.IndexCount >= MaxIndexCount)
//     {
//         End();
//         Begin();
//     }

//     constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

//     float textureIndex  = 0.0f;

//     for (uint32_t i = 1; i < m_Data.TextureSlotIndex; i++)
//     {
//         if (TextureSlots[i] == texID)
//         {
//             textureIndex = (float)i;
//             break;
//         }
//     }

//     if (textureIndex == 0.0f)
//     {
//         textureIndex = (float)m_Data.TextureSlotIndex;
//         TextureSlots[m_Data.TextureSlotIndex] = texID;
//         m_Data.TextureSlotIndex++;
//     }

//     BatchRenderer::Render(position, size, color, textureIndex);
// }

// void BatchRenderer::ResetStats()
// {
//     memset(&RenderStats, 0, sizeof(Stats));
// }

// const Stats& BatchRenderer::GetStats()
// {
//     return RenderStats;
// }




