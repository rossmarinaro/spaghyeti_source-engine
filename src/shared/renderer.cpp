#include "../../build/sdk/include/app.h"
#include "../../build/sdk/include/camera.h"
#include "../../build/sdk/include/window.h"
#include "../core/src/debug.h"
#include "./renderer.h"
#include "../vendors/glm/glm.hpp"
#include "../vendors/glm/gtc/type_ptr.hpp"
using namespace System;

//--------------------------------- 


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

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)offsetof(Math::Graphics::Vertex, scaleX)); //scale
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)offsetof(Math::Graphics::Vertex, rotation)); //rotation
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)offsetof(Math::Graphics::Vertex, u)); //uv
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)offsetof(Math::Graphics::Vertex, texID)); //texID
    glEnableVertexAttribArray(4);

    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)offsetof(Math::Graphics::Vertex, r)); //rgba
    glEnableVertexAttribArray(5);

    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)offsetof(Math::Graphics::Vertex, outlineR)); //rgba
    glEnableVertexAttribArray(6);

    glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)offsetof(Math::Graphics::Vertex, outlineWidth)); //rgba
    glEnableVertexAttribArray(7);

    for (int i = 0; i < 4; i++) { //4 x 4 model view matrix 
        glVertexAttribPointer(8 + i, 4, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)offsetof(Math::Graphics::Vertex, modelView) + (sizeof(float) * 4 * i)); 
        glEnableVertexAttribArray(8 + i);
    }

}


//----------------------------------


Renderer::Renderer() 
{
   textureSlotIndex = 1;
   indexCount = 0;
   drawStyle = 1;
   activeShaderID = 0;

    #ifndef __EMSCRIPTEN__
        drawStyle = GL_FILL;
    #endif

    for (size_t i = 0; i < BUFFERS; i++)  
        m_fences[i] = nullptr;

    for (size_t i = 1; i < MAX_TEXTURES; i++)
        textureSlots[i] = 0;
}



//---------------------------------- batch rendering


void Renderer::Init(Renderer* instance) 
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
        glBufferData(GL_ARRAY_BUFFER, MAX_QUADS * sizeof(Math::Graphics::Vertex), nullptr, GL_STREAM_DRAW);
    }

    //bind element buffer

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_instance->m_EBO);    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, s_instance->m_indices.size() * sizeof(GLint), s_instance->m_indices.data(), GL_STREAM_DRAW);

    EnableBlending();
    EnableAttributes();
    //CreateFrameBuffer();

    glDisable(GL_CULL_FACE); 
    glBindBuffer(GL_ARRAY_BUFFER, 0);       
    glBindVertexArray(0); 
}


//------------------------------------ update


void Renderer::Update(void* camera) 
{ 
    const auto backgroundColor = static_cast<Camera*>(camera)->GetBackgroundColor();

    glfwSwapInterval(s_vsync); //enable / disable vsync
    
   // if (s_instance)
       //glBindFramebuffer(GL_FRAMEBUFFER, s_instance->m_FBO);

    //glViewport(0, 0, m_frameBufferWidth, m_frameBufferHeight); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   
    glClearColor( 
        backgroundColor->r * backgroundColor->a, //r
        backgroundColor->g * backgroundColor->a, //g
        backgroundColor->b * backgroundColor->a, //b
        backgroundColor->a //a
    ); 
    
    
    
    int screenWidth, screenHeight;
        glfwGetFramebufferSize(GLFW_window_instance, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight); 
}


//----------------------------------------------- flush batch   


void Renderer::Flush(bool renderOpaque)
{glDisable(GL_DEPTH_TEST);
    //render quads in verts vector
    
    if (!s_instance->vertices.empty()) 
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

        //wait for gpu to finish rendering current buffer

        if (s_instance->m_fences[s_currentBufferIndex] != nullptr) 
        {
            GLenum result = glClientWaitSync(s_instance->m_fences[s_currentBufferIndex], GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000);

            if (result == GL_WAIT_FAILED) { 
                LOG("Renderer: skipping render. buffer wait failed.");
                return;
            }

            //remove fence

            glDeleteSync(s_instance->m_fences[s_currentBufferIndex]);
            s_instance->m_fences[s_currentBufferIndex] = nullptr;
        }

        //bind textures to defined slot indices

        glBindTexture(GL_TEXTURE_2D, 0);
        
        for (unsigned int i = 0; i < s_instance->textureSlotIndex; i++) { 
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, s_instance->textureSlots[i]);  
        }   

        //bind vertex array and current vbo

        glBindVertexArray(s_instance->m_VAO); 

        GLuint activeVBO = s_instance->m_VBOs[s_currentBufferIndex];

        glBindBuffer(GL_ARRAY_BUFFER, activeVBO);           
        glBufferSubData(GL_ARRAY_BUFFER, 0, s_instance->vertices.size() * sizeof(Math::Graphics::Vertex), s_instance->vertices.data());

        EnableAttributes();
        glUseProgram(s_instance->activeShaderID);
//  auto shader = Graphics::Shader::Get("sprite");
//     int samplers[System::Renderer::MAX_TEXTURES];

//     for (int i = 0; i < System::Renderer::MAX_TEXTURES; i++) 
//         samplers[i] = i;

//     shader.SetIntV("images", System::Renderer::MAX_TEXTURES, samplers);

//     const auto camera = System::Application::game->camera;

//     const Math::Vector4& pm = camera->GetProjectionMatrix(System::Window::s_scaleWidth, System::Window::s_scaleHeight);
//     const glm::mat4 ortho = (glm::highp_mat4)glm::ortho(pm.r, pm.g, pm.b, pm.a, -1.0f, 1.0f);

//     const Math::Matrix4 proj = { 
//         { ortho[0][0], ortho[0][1], ortho[0][2], ortho[0][3] }, 
//         { ortho[1][0], ortho[1][1], ortho[1][2], ortho[1][3] },   
//         { ortho[2][0], ortho[2][1], ortho[2][2], ortho[2][3] },  
//         { ortho[3][0], ortho[3][1], ortho[3][2], ortho[3][3] }
//     };

//     shader.SetMat4("proj", proj);  
        //draw elements from vertices vector

        glDrawElements(s_instance->drawStyle == GL_LINE ? GL_LINES : GL_TRIANGLES, s_instance->indexCount, GL_UNSIGNED_INT, 0); //tell ui about it
        glBindVertexArray(0);

        //disable attributes and unbind

        for (int i = 0; i < 16; i++)
            glDisableVertexAttribArray(i);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);

        //start new gpu async await call fence 

        s_instance->m_fences[s_currentBufferIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0); 

        //cycle to next buffer in ring

        s_currentBufferIndex = (s_currentBufferIndex + 1) % BUFFERS;
    }

    //reset batch

    s_instance->vertices.clear();
    s_instance->textureSlotIndex = 1;
    s_instance->indexCount = 0;
}


//---------------------------------


void Renderer::CreateFrameBuffer()
{
    if (s_instance)
    {
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

        glBindRenderbuffer(GL_RENDERBUFFER, s_instance->m_RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 800);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, s_instance->m_RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, 0); 

    }
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
        glfwGetFramebufferSize(GLFW_window_instance, &screenWidth, &screenHeight);

        glBlitFramebuffer(0, 0, m_frameBufferWidth, m_frameBufferHeight, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST); 
    }
}

void Renderer::RescaleFrameBuffer(float width, float height)
{
    if (!s_instance)
        return;

    glBindTexture(GL_TEXTURE_2D, s_instance->m_textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_instance->m_textureColorBuffer, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, s_instance->m_RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, s_instance->m_RBO);
}

//---------------------------------


void Renderer::ShutDown() 
{ 
    glDeleteVertexArrays(1, &s_instance->m_VAO);
    glDeleteBuffers(BUFFERS, s_instance->m_VBOs);
    glDeleteBuffers(1, &s_instance->m_EBO);
    
    s_instance->vertices.clear();

    delete s_instance;
    s_instance = nullptr;

    LOG("Renderer: shutting down...");
}


//----------------------------------------


void Renderer::cursor_callback(GLFWwindow* window, double xPos, double yPos)
{
    //set cursor object to movement, translate ndc coords to clip space

    auto position = Window::GetNDCToPixel((float)xPos, (float)yPos);

    Application::game->inputs->mouseX = position.x;
    Application::game->inputs->mouseY = position.y;
}


//----------------------------------------


void Renderer::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
        Application::game->inputs->SetKeyInputs(true, key, window);
        Application::game->inputs->numInputs++;
    }

    if (action == GLFW_RELEASE) {
        Application::game->inputs->SetKeyInputs(false, key, window);
        Application::game->inputs->numInputs--;
    }
}


//----------------------------------------


void Renderer::input_callback(GLFWwindow* window, int input, int action, int mods)
{
    if (input == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        Application::game->inputs->RIGHT_CLICK = true;

    else
        Application::game->inputs->RIGHT_CLICK = false;

    if (input == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        Application::game->inputs->LEFT_CLICK = true;

    else
        Application::game->inputs->LEFT_CLICK = false;

    if (action == GLFW_PRESS)
        Application::game->inputs->numInputs++;

    if (action == GLFW_RELEASE)
        Application::game->inputs->numInputs--;

}


//----------------------------------------


void Renderer::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Window::s_width = width;
    Window::s_height = height;
    glViewport(0, 0,400, 400); LOG(11);
    auto shader = Graphics::Shader::Get("sprite");
// LOG(111);
//     const auto camera = System::Game::GetScene()->GetContext().camera;LOG(12);
// if (!camera)return;LOG(13);
//     const Math::Vector4& pm = camera->GetProjectionMatrix(System::Window::s_scaleWidth, System::Window::s_scaleHeight);
//     const glm::mat4 ortho = (glm::highp_mat4)glm::ortho(pm.r, pm.g, pm.b, pm.a, -1.0f, 1.0f);

//     const Math::Matrix4 proj = { 
//         { ortho[0][0], ortho[0][1], ortho[0][2], ortho[0][3] }, 
//         { ortho[1][0], ortho[1][1], ortho[1][2], ortho[1][3] },   
//         { ortho[2][0], ortho[2][1], ortho[2][2], ortho[2][3] },  
//         { ortho[3][0], ortho[3][1], ortho[3][2], ortho[3][3] }
//     };

//     shader.SetMat4("proj", proj);  
}

//-----------------------------------


void Renderer::window_size_callback(GLFWwindow* window, int width, int height) {
    Window::s_width = width;
    Window::s_height = height;
    glViewport(0, 0, Window::s_width, Window::s_height);
}



//--------------------------------- instance rendering 


//#include "../vendors/glm/gtc/type_ptr.hpp"
// void Renderer::InitInstances(/* const std::vector<std::shared_ptr<Sprite>>& sprites */) 
// {
//     shader = Graphics::Shader::Get("instance");
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



// void Renderer::RenderInstances() 
// {
//     auto texture = Graphics::Texture2D::Get("sv_icon.png");
//     auto shader = Graphics::Shader::Get("sprite");
//     Math::Vector2 scrollFactor = { 1.0f, 1.0f }, scale = { 1.0f, 1.0f },position = { 0.0f, 0.0f };

//     const auto camera = System::Application::game->camera;

//     const Math::Vector4& pm = camera->GetProjectionMatrix(800,600);
//     const Math::Matrix4& vm = camera->GetViewMatrix((camera->GetPosition()->x * scrollFactor.x * scale.x), (camera->GetPosition()->y * scrollFactor.y * scale.y));
    
//     const glm::mat4 view = /* !IsSprite() ? glm::mat4(1.0f) : */ glm::mat4({ vm.a.x, vm.a.y, vm.a.z, vm.a.w }, { vm.b.x, vm.b.y, vm.b.z, vm.b.w }, { vm.c.x, vm.c.y, vm.c.z, vm.c.w }, { vm.d.x, vm.d.y, vm.d.z, vm.d.w }), 
//                     proj = (glm::highp_mat4)glm::ortho(pm.x, pm.y, pm.z, pm.w, -1.0f, 1.0f);

//     glm::mat4 model = glm::mat4(1.0f); 

//     model = glm::translate(model, { 0.5f * texture.FrameWidth + position.x * scale.x, 0.5f * texture.FrameHeight + position.y * scale.y, 0.0f }); 
//     model = glm::rotate(model, glm::radians(0.0f/* rotation */), { 0.0f, 0.0f, 1.0f }); 
//     model = glm::translate(model, { -0.5f * texture.FrameWidth - position.x * scale.x, -0.5f * texture.FrameHeight - position.y * scale.y, 0.0f });

//     const glm::mat4 _mvp = proj * view * model;

//     const Math::Matrix4 mvp = { 
//         { _mvp[0][0], _mvp[0][1], _mvp[0][2], _mvp[0][3] }, 
//         { _mvp[1][0], _mvp[1][1], _mvp[1][2], _mvp[1][3] },   
//         { _mvp[2][0], _mvp[2][1], _mvp[2][2], _mvp[2][3] },  
//         { _mvp[3][0], _mvp[3][1], _mvp[3][2], _mvp[3][3] }
//     };

//     shader.SetInt("image", 0);shader.SetMat4("mvp", mvp); 
//     texture.Update(position, false, false, 1); 
 
//     glUseProgram(shader.ID);
//     glVertexAttribDivisor( texture.ID, 1); // Update this attribute per instance
//     glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 10); // 100 instances
//     glBindVertexArray(0); 
// }






