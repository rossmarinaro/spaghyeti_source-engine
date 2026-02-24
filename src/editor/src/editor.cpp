#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <ctime>
#include "../../vendors/glm/glm.hpp"
#include "../../vendors/glm/gtc/matrix_transform.hpp"
#include "../../../build/sdk/include/app.h"
#include "../../shared/renderer.h"

#include "./editor.h"
#include "./assets/assets.h"
#include "./gui/gui.h"
#include "./nodes/node.h"


using namespace /* SPAGHYETI_CORE */ System;
using namespace editor;


void Editor::Update()
{
    Renderer::Update(s_self->game->camera); 

    const auto gui = GUI::Get();

    if (gui->grid) 
    {
        auto shader = Graphics::Shader::Get("grid");

        shader.SetFloat("pitch", gui->grid_quantity);
        shader.SetFloat("alpha", gui->grid->alpha);

        gui->grid->Render(); 
        Renderer::Flush(true);
    }

    Time::Update(glfwGetTime());

    //save hotkey
       
    if (s_self->projectOpen)
    {
        if (s_self->events->canSave && 
            (ImGui::IsKeyPressed(ImGuiKey_LeftCtrl) || ImGui::IsKeyPressed(ImGuiKey_RightCtrl)) && 
            ImGui::IsKeyPressed(ImGuiKey_S)) {
                s_self->events->canSave = false; 
                s_self->events->SaveScene(s_self->events->saveFlag);
        }

        else 
            s_self->events->canSave = true;
    }
 
    glfwPollEvents();

    gui->Render(); 

    //track mouse position by translating screen space to world space 

    double xPos, yPos;
    glfwGetCursorPos(Renderer::GLFW_window_instance, &xPos, &yPos);
    
    const Math::Vector4 pm = s_self->game->camera->GetProjectionMatrix(Window::s_scaleWidth, Window::s_scaleHeight);
    const Math::Matrix4 vm = s_self->game->camera->GetViewMatrix(s_self->game->camera->GetPosition()->x, s_self->game->camera->GetPosition()->y);

    const glm::mat4 localCoords = glm::inverse(glm::ortho(pm.r, pm.g, pm.b, pm.a, -1.0f, 1.0f) * glm::highp_mat4({ vm.a.r, vm.a.g, vm.a.b, vm.a.a }, { vm.b.r, vm.b.g, vm.b.b, vm.b.a }, { vm.c.r, vm.c.g, vm.c.b, vm.c.a }, { vm.d.r, vm.d.g, vm.d.b, vm.d.a }));
    const glm::vec2 ndc = { Window::GetPixelToNDC(xPos, yPos).x, Window::GetPixelToNDC(xPos, yPos).y };
    const glm::vec4 worldCoords(ndc.x, ndc.y, 0.0f, 1.0f),
                    resultPosition = localCoords * worldCoords;

    s_self->game->inputs->mouseX = resultPosition.x - s_self->game->camera->GetPosition()->x - 50;        
    s_self->game->inputs->mouseY = resultPosition.y - s_self->game->camera->GetPosition()->y;

    //current selected entity
       
    if (s_self->s_selector && selectedEntity)
    {
        s_self->s_selector->SetPosition(selectedEntity->position.x, selectedEntity->position.y); 
        s_self->s_selector->SetAlpha(1.0f);
    
        if (selectedEntity->GetType() == Entity::SPRITE) {
            const auto sprite = std::static_pointer_cast<Sprite>(selectedEntity);
            s_self->s_selector->SetSize(sprite->texture.FrameWidth, sprite->texture.FrameHeight);
        }
        
        if (selectedEntity->GetType() == Entity::GEOMETRY) {
            const auto geom = std::static_pointer_cast<Geometry>(selectedEntity);
            s_self->s_selector->SetSize(geom->width, geom->height);
        }
    
        if (selectedEntity->GetType() == Entity::TEXT) {
            const auto text = std::static_pointer_cast<Text>(selectedEntity);
            s_self->s_selector->SetSize(text->GetTextDimensions().x, text->GetTextDimensions().y + text->GetTextDimensions().x / 2); 
        } 

        s_self->s_selector->Render();

        Renderer::Flush(true);
    } 
    
	glfwSetFramebufferSizeCallback(Renderer::GLFW_window_instance, Renderer::framebuffer_size_callback);
    glfwSetWindowSizeCallback(Renderer::GLFW_window_instance, Renderer::window_size_callback); 
	glfwSwapBuffers(Renderer::GLFW_window_instance);

    //save and close editor

    if (glfwWindowShouldClose(Renderer::GLFW_window_instance))
        if (s_self->events->canSave) {
            if (s_self->projectOpen)
                s_self->events->saveFlag = true;
            else 
                s_self->events->exitFlag = true;
        }

}



//-----------------------------


void Editor::Start() 
{
    Editor session;
    s_self = &session;

    s_self->minVersion = 0;
    s_self->midVersion = 0;
    s_self->maxVersion = 1;
    s_self->worldWidth = 2000;
    s_self->worldHeight = 2000;
    s_self->gravityX = 0.0f;
    s_self->gravityY = 500.0f;
    s_self->vignetteVisibility = 0.0f;
    s_self->projectOpen = false;
    s_self->shaders_applied = false;
    s_self->globals_applied = false;
    s_self->animations_applied = false;
    s_self->gravity_continuous = true;
    s_self->gravity_sleeping = true;
    s_self->use_pthreads = true;
    s_self->shared_memory = false;
    s_self->allow_memory_growth = false;
    s_self->allow_exception_catching = true;
    s_self->export_all = true;
    s_self->wasm = true;
    s_self->gl_assertions = true;
    s_self->use_webgl2 = false;
    s_self->full_es3 = false;
    s_self->embed_files = false;
    s_self->webgl_embed_files = true;
    s_self->preserveSrc = false;
    s_self->isMultiThreaded = false;
    s_self->vsync = false;
    s_self->cullTarget = { "", { 0.0f, 0.0f } };

    //AllocConsole();

    platform = "Windows";
    releaseType = "debug";
    buildType = "dynamic";
    projectPath = "";
    rootPath;

    remove("appLog.txt");

    rootPath = std::filesystem::current_path().string();

    Log("Editor Root: " + rootPath);

    Application::name = "SPAGHYEDITOR";

    Window::Init();

    EventListener el;
    s_self->events = &el;

    Game g;
    s_self->game = &g; 

    //init base sandox scene

    s_self->game->LoadScene<System::Scene>();

    Application::Init(s_self->game); 
    Game::SetCullPosition(g.camera->GetPosition());

    AssetManager am;
    GUI gui; 

    //set top-left header and bottom toolbar icon (not binary, this image is stored as pixel data)

    const auto image_data = Resources::Manager::GetResource("icon small");

    if (image_data) {
        unsigned char* image_buffer = (unsigned char*)image_data->array_buffer;
        GLFWimage image;

        image.width = 66; 
        image.height = 65;
        image.pixels = image_buffer;

        glfwSetWindowIcon(Renderer::GLFW_window_instance, 1, &image);
    }

    //create entity selector graphic

    s_self->s_selector = System::Game::CreateGeom(0.0f, 0.0f, 0.0f, 0.0f, 2);
    s_self->s_selector->SetTint({ 0.0f, 1.0f, 0.0f });  
    s_self->s_selector->SetDrawStyle(0);
    s_self->s_selector->SetThickness(2.0f);
    s_self->s_selector->SetAlpha(0.0f);

    //main update loop

    while (!s_self->events->exitFlag) 
        Update();
        
    Node::ClearAll();

}


//------------------------------


void Editor::ShutDown()
{

    if (!s_self->preserveSrc)
        remove((projectPath + "\\game.cpp").c_str());
    
    remove((projectPath + "\\spaghyeti_parse.json").c_str());

    Resources::Manager::UnLoadRaw(Resources::Manager::IMAGE, "editor logo");
    Resources::Manager::UnLoadRaw(Resources::Manager::IMAGE, "icon large");
    Resources::Manager::UnLoadRaw(Resources::Manager::IMAGE, "audio src");
    Resources::Manager::UnLoadRaw(Resources::Manager::IMAGE, "data src"); 
    Resources::Manager::UnLoadRaw(Resources::Manager::IMAGE, "folder src");

    Application::ShutDown();

    Editor::Log("Editor closed.");

}


//------------------------------


void Editor::Log(const std::string& message) {

    std::filesystem::current_path(rootPath);
    std::ofstream src("appLog.txt", std::ofstream::app | std::ofstream::out);
    std::time_t time_stamp = std::time(nullptr);

    src << message << " @ " << std::ctime(&time_stamp);
    std::cout << message << " @ " << std::ctime(&time_stamp) << std::endl;

}


//--------------------------------


void Editor::Reset()
{

    remove((projectPath + "\\game.cpp").c_str());

    Node::ClearAll();

    s_self->game->camera->Reset();

    s_self->worldWidth = 2000;
	s_self->worldHeight = 2000;

    s_self->globals_applied = false;
    s_self->gravity_continuous = true;
    s_self->gravity_sleeping = true;

    s_self->globals.clear();
    s_self->spritesheets.clear();
    s_self->scenes.clear();
    s_self->shaders.clear();

    AssetManager::Get()->Reset();

    Log("Workspace reset.");
}
