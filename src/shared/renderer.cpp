#include "../../build/sdk/include/app.h"
#include "../../build/sdk/include/camera.h"
#include "../../build/sdk/include/window.h"
#include "../core/src/debug.h"
#include "./renderer.h"


using namespace System;

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

    for (int i = 0; i < 4; i++) { //model view matrix 
        glVertexAttribPointer(6 + i, 4, GL_FLOAT, GL_FALSE, sizeof(Math::Graphics::Vertex), (void*)offsetof(Math::Graphics::Vertex, modelView) + (sizeof(float) * 4 * i)); 
        glEnableVertexAttribArray(6 + i);
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

//---------------------------------


void Renderer::ShutDown() 
{ 
    const auto renderer = System::Application::renderer;

    glDeleteVertexArrays(1, &renderer->m_VAO);
    glDeleteBuffers(BUFFERS, renderer->m_VBOs);
    glDeleteBuffers(1, &renderer->m_EBO);
    
    renderer->vertices.clear();

    LOG("Renderer: shutting down...");
}


//---------------------------------


void Renderer::BindFrameBuffer() {
    glBindBuffer(GL_FRAMEBUFFER, System::Application::renderer->m_FBO);
}


//---------------------------------



void Renderer::UnbindFrameBuffer() {
    glBindBuffer(GL_FRAMEBUFFER, 0);
}

//---------------------------------


void Renderer::CreateFrameBuffer()
{
    const auto renderer = System::Application::renderer;

    glGenFramebuffers(1, &renderer->m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, renderer->m_FBO);
    glGenTextures(1, &renderer->m_textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, renderer->m_textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800,600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    #ifndef __EMSCRIPTEN__
        glGenerateMipmap(GL_TEXTURE_2D);
    #endif

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->m_textureColorBuffer, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG("Renderer: Error Framebuffer: Incomplete Buffer.");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glBindTexture(GL_TEXTURE_2D, 0);
    //glBindRenderbuffer(GL_RENDERBUFFER, 0);

}

//---------------------------------


void Renderer::UpdateFrameBuffer(void* camera)
{
    const auto renderer = System::Application::renderer;
    const auto bg = ((Camera*)camera)->GetBackgroundColor();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer->m_FBO);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); glViewport(0, 0, Window::s_width, Window::s_height);
    glDrawBuffer(GL_BACK);
    //int screenWidth, screenHeight;
    //glfwGetFramebufferSize(GLFW_window_instance, &screenWidth, &screenHeight);
    glDisable(GL_BLEND);
    glBlitFramebuffer(0, 0, 800,600, 0, 0, 800,600, GL_COLOR_BUFFER_BIT, GL_NEAREST); int err = glfwGetError(NULL);
           if(err==GL_INVALID_OPERATION){LOG("GLFW: Error:: " + std::to_string(err));}
//glEnable(GL_BLEND);
}

//---------------------------------


void Renderer::RescaleFrameBuffer(float width, float height)
{
    const auto renderer = System::Application::renderer;

    glBindTexture(GL_TEXTURE_2D, renderer->m_textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->m_textureColorBuffer, 0);
}


//---------------------------------- batch rendering


void Renderer::Init() 
{
    const auto renderer = System::Application::renderer;

    s_vsync = 1;
    s_currentBufferIndex = 0;

    LOG("Renderer: Initializing buffers.");

    //2 triangles = 1 quad
    //quad vertices format x4: x y scaleX scaleY rotation u v texID r g b a
    //indices format x6: 0 1 2 2 3 0  

    for (unsigned int i = 0; i < MAX_QUADS * 6; ++i) {
        renderer->m_indices.push_back(i * 4);
        renderer->m_indices.push_back(i * 4 + 1);
        renderer->m_indices.push_back(i * 4 + 2);
        renderer->m_indices.push_back(i * 4 + 2);
        renderer->m_indices.push_back(i * 4 + 3);
        renderer->m_indices.push_back(i * 4);
    }

    glGenVertexArrays(1, &renderer->m_VAO); 
    glBindVertexArray(renderer->m_VAO); 

    glGenBuffers(BUFFERS, renderer->m_VBOs);   
    glGenBuffers(1, &renderer->m_EBO);

    //bind vbos (ring buffering)

    for (unsigned int i = 0; i < BUFFERS; i++) {
        glBindBuffer(GL_ARRAY_BUFFER, renderer->m_VBOs[i]);    
        glBufferData(GL_ARRAY_BUFFER, MAX_QUADS * sizeof(Math::Graphics::Vertex), nullptr, /* GL_DYNAMIC_DRAW *//* GL_STREAM_DRAW */GL_STATIC_DRAW);
    }

    //bind element buffer

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->m_EBO);    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderer->m_indices.size() * sizeof(GLint), renderer->m_indices.data(),GL_STATIC_DRAW/* GL_DYNAMIC_DRAW */ /* GL_STREAM_DRAW */);

    EnableAttributes();
    CreateFrameBuffer();
    
    glDisable(GL_CULL_FACE); 
    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    //glDepthFunc(GL_LEQUAL);
    //glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glBindBuffer(GL_ARRAY_BUFFER, 0);       
    glBindVertexArray(0); 

}


//----------------------------------------------- flush batch


void Renderer::Flush()
{
    auto renderer = System::Application::renderer;

    if (!renderer->vertices.empty()) 
    {
        //wait for gpu to finish rendering current buffer

        if (renderer->m_fences[s_currentBufferIndex] != nullptr) 
        {
            GLenum result = glClientWaitSync(renderer->m_fences[s_currentBufferIndex], GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000);

            if (result == GL_WAIT_FAILED) { 
                LOG("Renderer: skipping render. buffer wait failed.");
                return;
            }

            //remove fence

            glDeleteSync(renderer->m_fences[s_currentBufferIndex]);
            renderer->m_fences[s_currentBufferIndex] = nullptr;
        }

        //bind textures to defined slot indices

        glBindTexture(GL_TEXTURE_2D, 0);
        
        for (unsigned int i = 0; i < renderer->textureSlotIndex; i++) { 
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, renderer->textureSlots[i]);  
        }   

        //bind vertex array and vbos / draw elements from vertices vector

        GLuint activeVBO = renderer->m_VBOs[s_currentBufferIndex];

        glBindBuffer(GL_ARRAY_BUFFER, activeVBO);         
        glBufferSubData(GL_ARRAY_BUFFER, 0, renderer->vertices.size() * sizeof(Math::Graphics::Vertex), renderer->vertices.data());

        EnableAttributes();

        glUseProgram(renderer->activeShaderID);
        glBindVertexArray(renderer->m_VAO); 
        glDrawElements(renderer->drawStyle == GL_LINE ? GL_LINES : GL_TRIANGLES, renderer->indexCount, GL_UNSIGNED_INT, 0); //tell ui about it

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);

        //start new gpu async await call fence 

        renderer->m_fences[s_currentBufferIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0); 

        //cycle to next buffer in ring

        s_currentBufferIndex = (s_currentBufferIndex + 1) % BUFFERS;
    }

    //reset batch

    renderer->vertices.clear();
    renderer->textureSlotIndex = 1;
    renderer->indexCount = 0;

}



//------------------------------------ update


void Renderer::Update(void* camera) 
{ 
    //clear background

    const auto bg = ((Camera*)camera)->GetBackgroundColor();

    const auto renderer = System::Application::renderer;

    glfwSwapInterval(s_vsync); //Enable vsync

    glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer->m_FBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   

    glClearColor( 
        bg->r * bg->a,
        bg->g * bg->a,
        bg->b * bg->a,
        bg->a
    );

    glViewport(0, 0, 800,600/* Window::s_width, Window::s_height */);
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
    glViewport(0, 0, Window::s_width, Window::s_height);
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






