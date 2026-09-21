#include "../../build/sdk/include/app.h"
#include "../../build/sdk/include/camera.h"
#include "../core/src/debug.h"
#include "./window.h"
#include "./renderer.h"


using namespace System;

static inline Renderer* s_instance;


void EnableBlending() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
}

//--------------------------------- enable vertex attributes, allowing data to be baked into a vertex


void EnableAttributes() 
{
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)0); //position
    glEnableVertexAttribArray(0);
    //glVertexAttribDivisor(0, 0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)offsetof(Math::Graphics::Vertex, u)); //uv (tex coords)
    glEnableVertexAttribArray(1);
    //glVertexAttribDivisor(1, 0);

    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)offsetof(Math::Graphics::Vertex, minU)); //min uv (tex coords)
    glEnableVertexAttribArray(2);
    //glVertexAttribDivisor(2, 0);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)offsetof(Math::Graphics::Vertex, texID)); //texID
    glEnableVertexAttribArray(3);
    //glVertexAttribDivisor(3, 0);

    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)offsetof(Math::Graphics::Vertex, r)); //rgba
    glEnableVertexAttribArray(4);
    //glVertexAttribDivisor(4, 0);

    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)offsetof(Math::Graphics::Vertex, outlineR)); //outline rgba
    glEnableVertexAttribArray(5);
    //glVertexAttribDivisor(5, 0);

    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)offsetof(Math::Graphics::Vertex, outlineWidth)); //outline size
    glEnableVertexAttribArray(6); 
    //glVertexAttribDivisor(6, 0);

    glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)offsetof(Math::Graphics::Vertex, whiteout)); //outline size
    glEnableVertexAttribArray(7); 
    //glVertexAttribDivisor(7, 0);

    for (int i = 0; i < 4; i++) { //4 x 4 model view projection matrix 
        size_t offset = offsetof(Math::Graphics::Vertex, mvp) + sizeof(float) * 4 * i;
        glVertexAttribPointer(8 + i, 4, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), reinterpret_cast<const void*>(offset)); 
        glEnableVertexAttribArray(8 + i);
    }
}


//----------------------------------


Renderer::Renderer() 
{
   textureSlotIndex = 1;
   drawStyle = 1;

    for (size_t i = 0; i < BUFFERS; i++)  
        m_fences[i] = nullptr;

    for (size_t i = 1; i < MAX_TEXTURES; i++)
        textureSlots[i] = 0;
}


//----------------------------------


Renderer* Renderer::Get() { 
    return s_instance; 
}


//---------------------------------- batch rendering


void Renderer::Init() 
{
    s_instance = new Renderer;
    s_vsync = 0;
    s_currentBufferIndex = 0;

    LOG("Renderer: Initializing buffers.");

    //2 triangles = 1 quad
    //quad vertices format x4: x y scaleX scaleY rotation u v texID r g b a
    //indices format x6: 0 1 2 2 3 0  

    for (unsigned int i = 0; i < MAX_QUADS * 6; ++i) {
        s_instance->m_indices.push_back(i * 4);
        s_instance->m_indices.push_back(i * 4 + 1);
        s_instance->m_indices.push_back(i * 4 + 2);
        s_instance->m_indices.push_back(i * 4 + 2);
        s_instance->m_indices.push_back(i * 4 + 3);
        s_instance->m_indices.push_back(i * 4);
    }

    glGenVertexArrays(1, &s_instance->m_VAO); 
    glBindVertexArray(s_instance->m_VAO); 

    glGenBuffers(BUFFERS, s_instance->m_VBOs);   
    glGenBuffers(1, &s_instance->m_EBO);

    //bind vbos (ring buffering)

    for (unsigned int i = 0; i < BUFFERS; i++) {
        glBindBuffer(GL_ARRAY_BUFFER, s_instance->m_VBOs[i]);    
        glBufferData(GL_ARRAY_BUFFER, MAX_QUADS * 4 * sizeof(Math::Graphics::Vertex), nullptr, GL_STREAM_DRAW);
    }

    //bind element buffer

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_instance->m_EBO);    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, s_instance->m_indices.size() * sizeof(GLint), s_instance->m_indices.data(), GL_STREAM_DRAW);

    EnableBlending();
    EnableAttributes();

    //frame buffer

    #if STANDALONE == 1

        glGenFramebuffers(1, &s_instance->m_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, s_instance->m_FBO);
        glGenTextures(1, &s_instance->m_textureColorBuffer);
        glBindTexture(GL_TEXTURE_2D, s_instance->m_textureColorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_frameBufferWidth, m_frameBufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        #ifndef __EMSCRIPTEN__
            glGenerateMipmap(GL_TEXTURE_2D);
        #endif

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_instance->m_textureColorBuffer, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LOG("Renderer: Error Framebuffer: Incomplete Buffer.");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //render buffer

        // glBindRenderbuffer(GL_RENDERBUFFER, s_instance->m_RBO);
        // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 800);
        // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, s_instance->m_RBO);
        // glBindRenderbuffer(GL_RENDERBUFFER, 0); 
    #endif   

    glDisable(GL_CULL_FACE); 
    glBindBuffer(GL_ARRAY_BUFFER, 0);       
    glBindVertexArray(0); 
}

//------------------------------------

void Renderer::SetVsync(int rate) { 
    s_vsync = rate; 
    #ifndef __EMSCRIPTEN__
        glfwSwapInterval(s_vsync); //enable / disable vsync
    #endif
}


//------------------------------------ update


void Renderer::Update(void* camera) 
{ 
    const auto backgroundColor = static_cast<Camera*>(camera)->GetBackgroundColor();

    #if STANDALONE == 1
        if (s_instance) {
            glBindFramebuffer(GL_FRAMEBUFFER, s_instance->m_FBO);
            glViewport(0, 0, m_frameBufferWidth, m_frameBufferHeight); 
        }
    #else
        int screenWidth, screenHeight;
        glfwGetFramebufferSize(Window::GLFW_window_instance, &screenWidth, &screenHeight);
        glViewport(0, 0, screenWidth, screenHeight); 
    #endif
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   
    glClearColor( 
        backgroundColor->r * backgroundColor->a, //r
        backgroundColor->g * backgroundColor->a, //g
        backgroundColor->b * backgroundColor->a, //b
        backgroundColor->a //a
    ); 
}


//------------------------------------


Renderer::Renderable& Renderer::GetOrCreateRenderBucket(unsigned int shaderID)
{
    for (size_t i = 0; i < s_instance->activeLayers.size(); i++) 
        if (s_instance->activeLayers[i].shaderID == shaderID)
            return s_instance->activeLayers[i];

    Renderer::Renderable renderable;

    renderable.shaderDepth = 0;
    renderable.shaderID = shaderID;  
    renderable.vertices.reserve(4 * MAX_QUADS); 
    renderable.indices.reserve(6 * MAX_QUADS);

    s_instance->activeLayers.emplace_back(renderable);
    
    return s_instance->activeLayers.back();
}


//-----------------------------------------------


void Renderer::RenderBatch(Renderable& renderable) 
{
    //render quads in verts vector
    
    if (!renderable.vertices.empty()) 
    {
        GLuint activeVBO = s_instance->m_VBOs[s_currentBufferIndex];

        glBindBuffer(GL_ARRAY_BUFFER, activeVBO);    
        glBufferSubData(GL_ARRAY_BUFFER, 0, renderable.vertices.size() * sizeof(Math::Graphics::Vertex), renderable.vertices.data());

        //glBindBuffer(GL_ARRAY_BUFFER, activeEBO);    
        //glBufferSubData(GL_ARRAY_BUFFER, 0, s_instance->indices.size() * sizeof(Math::Graphics::Vertex), s_instance->indices.data());

        //update image samplers / bind textures to defined slot indices

        for (unsigned int i = 0; i < s_instance->textureSlotIndex; i++) { 
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, s_instance->textureSlots[i]);  
        } 

        EnableAttributes();

        glUseProgram(renderable.shaderID);
  
        //update uniforms

        const auto it = std::find_if(System::Application::resources->shaders.begin(), System::Application::resources->shaders.end(), [&renderable](const auto& s) { return s.second->ID == renderable.shaderID; });

        if (it != System::Application::resources->shaders.end()) {
            const auto activeShader = it->second;
            activeShader->Update();
        }

        //draw elements from vertices vector

        glDrawElements(s_instance->drawStyle == 0 ? GL_LINE_LOOP : GL_TRIANGLES, renderable.indices.size(), GL_UNSIGNED_INT, 0);  

        for (unsigned int i = 0; i < s_instance->textureSlotIndex; i++) { 
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        } 

        //disable attributes and unbind

        for (int i = 0; i < 16; i++)
            glDisableVertexAttribArray(i);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);

        //reset buffer containers and texture tracking pointer
  
        renderable.vertices.clear();
        renderable.indices.clear();
        s_instance->textureSlotIndex = 1;
    }
}


//----------------------------------------------- flush batch   


void Renderer::Flush(bool renderOpaque, int shaderID)
{
    if (!s_instance)
        return;

    glBindVertexArray(s_instance->m_VAO);

    if (System::Game::GetScene()->GetDepthSort()) 
    {
        if (renderOpaque) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }
        else {
            EnableBlending();
            glDepthMask(GL_FALSE);
        }
    }
    else {
        EnableBlending();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_SCISSOR_TEST);
    }

    //wait for gpu to finish rendering current buffer

    if (s_instance->m_fences[s_currentBufferIndex] != nullptr) 
    {
        GLenum result;

        #ifdef __EMSCRIPTEN__
            result = glClientWaitSync(s_instance->m_fences[s_currentBufferIndex], 0, 0);
        #else
            result = glClientWaitSync(s_instance->m_fences[s_currentBufferIndex], GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
        #endif

        if (result == GL_WAIT_FAILED) { 
            LOG("Renderer: skipping render. buffer wait failed.");
            return;
        }

        //remove fence

        glDeleteSync(s_instance->m_fences[s_currentBufferIndex]);
        s_instance->m_fences[s_currentBufferIndex] = nullptr;
    }

    //render batches, single or all shaders

    if (shaderID != -1) {
        const auto it = std::find_if(s_instance->activeLayers.begin(), s_instance->activeLayers.end(), [shaderID](const Renderable& r) { return r.shaderID == shaderID; });
        if (it != s_instance->activeLayers.end()) {
            Renderable& renderable = *it;
            RenderBatch(renderable);
        }
    }

    else 
    {
        //sort buckets first by depth, then by id

        std::sort(s_instance->activeLayers.begin(), s_instance->activeLayers.end(), [](const Renderable& a, const Renderable& b) { 
            if (a.shaderDepth != b.shaderDepth)
                return a.shaderDepth < b.shaderDepth;
            return a.shaderID < b.shaderID;
        }); 

        for (auto& layer : s_instance->activeLayers)
            RenderBatch(layer);
    }    

    glBindVertexArray(0);

    s_instance->m_fences[s_currentBufferIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0); 

    //cycle to next buffer in ring

    s_currentBufferIndex = (s_currentBufferIndex + 1) % BUFFERS; 
}


//---------------------------------


void Renderer::UpdateFrameBuffer(void* camera)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);

    glViewport(0, 0, Window::s_scaleWidth, Window::s_scaleHeight); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   

    if (s_instance) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, s_instance->m_FBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); 

        int screenWidth, screenHeight;
        glfwGetFramebufferSize(Window::GLFW_window_instance, &screenWidth, &screenHeight);

        glBlitFramebuffer(0, 0, m_frameBufferWidth, m_frameBufferHeight, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST); 
    }
}


//---------------------------------


void Renderer::ShutDown() 
{ 
    for (auto it = s_instance->activeLayers.begin(); it != s_instance->activeLayers.end(); ++it)
        if (it != s_instance->activeLayers.end()) {
            it = s_instance->activeLayers.erase(std::move(it));
            --it;
        }

    glDeleteVertexArrays(1, &s_instance->m_VAO); 
    glDeleteBuffers(BUFFERS, s_instance->m_VBOs);    
    glDeleteBuffers(1, &s_instance->m_EBO);

    delete s_instance;
    s_instance = nullptr;

    LOG("Renderer: shutting down...");
}


//--------------------------------- instance rendering 


// #include "../vendors/glm/gtc/type_ptr.hpp"
// void Renderer::InitInstances(/* const std::vector<std::shared_ptr<Sprite>>& sprites */) 
// {
//     auto shader = Graphics::Shader::Get("instance");
//     const auto sprites = System::Game::GetScene()->tileDefs;

//     glm::vec2 translations[10];

//     //float x = 0,xOff=0;//{ 0.0f, 0.0f };

//     for (int i = 0; i < sprites.size(); i++) {
//         const auto sprite = sprites[i];
//         translations[i++] = glm::vec2(sprite.x, sprite.y);
//     }

//     glUseProgram(shader.ID);
//     glUniform2fv(glGetUniformLocation(shader.ID, "offsets"), 10, glm::value_ptr(translations[0]));

// }








