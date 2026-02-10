#include "../../build/sdk/include/app.h"
#include "../../build/sdk/include/camera.h"
#include "../../build/sdk/include/window.h"
#include "../core/src/debug.h"
#include "./renderer.h"
#include "../../build/sdk/include/geometry.h"
#include "../vendors/glm/gtc/matrix_transform.hpp" 
using namespace System;

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


//---------------------------------


void Renderer::BindFrameBuffer() {
    glBindBuffer(GL_FRAMEBUFFER, s_FBO);
}


//---------------------------------



void Renderer::UnbindFrameBuffer() {
    glBindBuffer(GL_FRAMEBUFFER, 0);
}

//---------------------------------

GLuint texture_id;

void Renderer::CreateFrameBuffer()
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

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG("Error Framebuffer: Incomplete Buffer.");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

}

//---------------------------------


void Renderer::RescaleFrameBuffer(float width, float height)
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


//---------------------------------- batch rendering

#include <iostream>
int frames = 0;bool t = true;

static inline Graphics::Vertex* QuadBuffer = nullptr; //pointer to the beginning of buffer data on cpu
static inline Graphics::Vertex* QuadBufferPtr = nullptr; //trackeing pointer as to where we are when copying data into buffer

Graphics::Vertex* Renderer::GetQuadBufferPtr() {
return QuadBufferPtr;
}
int prev=0;
void Renderer::Init() 
{QuadBuffer = new Graphics::Vertex[MAX_QUADS * 4];
    //quad vertices

    Graphics::Vertex vertices[18] = {
        //x      y      u     v     texID
        -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
        0.5f, 0.5f,  1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f,  0.0f, 1.0f, 1.0f,
    }; 

    //indices 2 triangles = 1 quad { 0, 1, 2, 2, 3, 0 } 

    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < MAX_QUADS /* * 6 */; ++i) {
        indices.push_back(i * 4);
        indices.push_back(i * 4 + 1);
        indices.push_back(i * 4 + 2);
        indices.push_back(i * 4 + 2);
        indices.push_back(i * 4 + 3);
        indices.push_back(i * 4);
    }

    glGenVertexArrays(1, &VAO); 

    glGenBuffers(1, &VBO);   
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);  

    glBindBuffer(GL_ARRAY_BUFFER, VBO);    
    glBufferData(GL_ARRAY_BUFFER, MAX_QUADS * sizeof(Graphics::Vertex), nullptr, GL_STREAM_DRAW/* GL_DYNAMIC_DRAW */);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STREAM_DRAW/* GL_DYNAMIC_DRAW */);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Graphics::Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Graphics::Vertex), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Graphics::Vertex), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glDisable(GL_CULL_FACE); 
    glDisable(GL_DEPTH_TEST);
    glBindBuffer(GL_ARRAY_BUFFER, 0);       
    glBindVertexArray(0); 
}


//------------------------------------ update


void Renderer::Update(void* camera) 
{ if (t){t=false;prev= System::Game::GetScene()->GetContext().time->GetSeconds();}
    //clear background

    const auto bg = ((Camera*)camera)->GetBackgroundColor();

    glfwSwapInterval(s_vsync); //Enable vsync

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    glClear(GL_COLOR_BUFFER_BIT);   

    glClearColor( 
        bg->x * bg->w,
        bg->y * bg->w,
        bg->z * bg->w,
        bg->w
    );

    glViewport(0, 0, Window::s_width, Window::s_height);
      //bind textures


 const float now = System::Game::GetScene()->GetContext().time->GetSeconds();
    
    if (frames < 60)
        frames++;

    if (now - prev >= 1.0f) {
        std::cout << "FPS: " << std::to_string(frames) << "\n";
        frames = 0;
        prev = now;
    }

}

int m_textureSlotIndex = 0;
std::array<uint32_t, 32> m_textureSlots;
float Renderer::GetTextureIndex(uint32_t textureID) {
//check if in batch already

    for (uint32_t i = 0; i < m_textureSlotIndex; i++) {
        if (m_textureSlots[i] == textureID)
            return (float)i;
    }

    if (m_textureSlotIndex >= MAX_TEXTURES)
        Flush();

    //add tex
    m_textureSlots[m_textureSlotIndex] = textureID;
    return (float)m_textureSlotIndex++;
}


void Renderer::Flush()
{
    std::vector<std::pair<const std::string, Graphics::Texture2D>> textures(System::Application::resources->textures.begin(), System::Application::resources->textures.end());
    auto shader = Graphics::Shader::Get("sprite");

    glUseProgram(shader.ID);
    
    for (int index = 0; index < m_textureSlotIndex/* MAX_TEXTURES */; index++) {
        glActiveTexture(GL_TEXTURE0 + index);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D, textures[index].second.ID);
        //textureSlotIndex++;
        std::string uniform = "images[" + std::to_string(index) + "]";
        int loc = glGetUniformLocation(shader.ID, uniform.c_str());
        if (loc != -1)
            glUniform1i(loc, index);
    }

        //int samplers[MAX_TEXTURES] = { 0, 1 };
        //glUniform1iv(glGetUniformLocation(shader.ID, "images"), 2, samplers);
    //flush batch

    int elementCount = 6 * MAX_QUADS;


    //bind vertex array and update vertices buffer

    glBindVertexArray(VAO); 
    glBindBuffer(GL_ARRAY_BUFFER, VBO);    
    glBufferData(GL_ARRAY_BUFFER, MAX_QUADS * sizeof(Graphics::Vertex), nullptr, GL_STREAM_DRAW);
    //glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Graphics::Vertex), vertices.data()); 

    GLsizeiptr size = (uint8_t*)QuadBufferPtr - (uint8_t*)QuadBuffer;

    glBufferSubData(GL_ARRAY_BUFFER, 0, size, QuadBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    //draw elements
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    indexCount = 0; 
    m_textureSlotIndex = 0;
    QuadBufferPtr = QuadBuffer;



    //     if (indexCount >= elementCount || textureSlotIndex > MAX_TEXTURES) // 31 because first slot we have reserved for 1x1 white texture
//     {

//         //bind vertex array and update vertices buffer

//         auto vertices = System::Game::GetScene()->batchSprites_verts;

//         glBindVertexArray(VAO); 
//         glBindBuffer(GL_ARRAY_BUFFER, VBO);    
//         glBufferData(GL_ARRAY_BUFFER, MAX_QUADS * sizeof(Graphics::Vertex), nullptr, GL_STREAM_DRAW);
//         glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Graphics::Vertex), vertices.data()); 
//         glBindBuffer(GL_ARRAY_BUFFER, 0); 

//         //draw elements

//         glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0); //tell ui about it
//         indexCount = 0; 
//         textureSlotIndex = 0;
//         vertices.clear();
// 	}
}

//---------------------------------


void Renderer::ShutDown() { delete[] QuadBuffer;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
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

//     const Math::Vector4& pm = camera->GetProjectionMatrix(System::Window::s_scaleWidth, System::Window::s_scaleHeight);
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






