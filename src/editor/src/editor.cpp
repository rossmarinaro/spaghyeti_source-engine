#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <ctime>

#include "./editor.h"
#include "./assets/assets.h"
#include "./gui/gui.h"
#include "./nodes/node.h"
#include "../../../build/sdk/include/app.h"

using namespace /* SPAGHYETI_CORE */ System;
using namespace editor;


void Editor::Update()
{

    Renderer::Update(s_self->game->camera); 

    if (GUI::Get()->grid)
        GUI::Get()->grid->Render(System::Window::s_scaleWidth, System::Window::s_scaleHeight);

    Time::Update(glfwGetTime());

    //save hotkey
       
    if (s_self->projectOpen)
    {
        if (s_self->events->canSave && 
        (
            ImGui::IsKeyPressed(ImGuiKey_LeftCtrl) || ImGui::IsKeyPressed(ImGuiKey_RightCtrl)) && 
            ImGui::IsKeyPressed(ImGuiKey_S)) {

            s_self->events->canSave = false; 
            s_self->events->SaveScene(s_self->events->saveFlag);
        }

        else 
            s_self->events->canSave = true;
    }

    glfwPollEvents();

    GUI::Get()->Render(); 

    //track mouse position by translating screen space to world space 

    double xPos, yPos;
    glfwGetCursorPos(Window::s_instance, &xPos, &yPos);

    glm::mat4 localCoords = glm::inverse(s_self->game->camera->GetProjectionMatrix(Window::s_scaleWidth, Window::s_scaleHeight) * s_self->game->camera->GetViewMatrix(s_self->game->camera->GetPosition().x, s_self->game->camera->GetPosition().y));
    glm::vec4 ndc = glm::vec4(Window::GetPixelToNDC(xPos, yPos), 1.0f, 1.0f),
              worldCoords(ndc.x, ndc.y, 0.0f, 1.0f),
              resultPosition = localCoords * worldCoords;

    s_self->game->inputs->mouseX = resultPosition.x - s_self->game->camera->GetPosition().x - 50;        
    s_self->game->inputs->mouseY = resultPosition.y - s_self->game->camera->GetPosition().y;

    //current selected entity
       
    if (s_self->s_selector) 
    {
        if (selectedEntity)
        {
           s_self->s_selector->SetPosition(selectedEntity->position.x, selectedEntity->position.y); 
            s_self->s_selector->SetAlpha(1.0f);
        
            if (strcmp(selectedEntity->type, "sprite") == 0) {
                auto sprite = std::static_pointer_cast<Sprite>(selectedEntity);
                s_self->s_selector->SetSize(sprite->texture.FrameWidth, sprite->texture.FrameHeight);
            } 

            if (strcmp(selectedEntity->type, "text") == 0) {
                auto text = std::static_pointer_cast<Text>(selectedEntity);
                s_self->s_selector->SetSize(text->GetTextDimensions().x, text->GetTextDimensions().y + text->GetTextDimensions().x / 2); 
            }
        }

        else
            s_self->s_selector->SetAlpha(0.0f);
       
    } 

    glViewport(0, 0, Window::s_width, Window::s_height);
    
	glfwSetFramebufferSizeCallback(Window::s_instance, Window::framebuffer_size_callback);
	glfwSwapBuffers(Window::s_instance);

    //save and close editor

    if (glfwWindowShouldClose(Window::s_instance))
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
    s_self->preserveSrc = false;
    s_self->isMultiThreaded = false;
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

    EventListener el;
    s_self->events = &el;

    Game g;
    s_self->game = &g; 

    //init base sandox scene

    s_self->game->LoadScene<System::Scene>();

    Application::name = "SPAGHYEDITOR";

    Window::Init();
    Application::Init(s_self->game); 

    AssetManager am;
    GUI gui; 

    //set top-left header and bottom toolbar icon

    GLFWimage image;

    image.width = 66;
    image.height = 65;
    image.pixels = reinterpret_cast<unsigned char*>(const_cast<char*>(Resources::Manager::GetRawData("editor logo")));

    glfwSetWindowIcon(Window::s_instance, 1, &image);

    //create entity selector graphic

    s_self->s_selector = System::Game::CreateGeom(0.0f, 0.0f, 0.0f, 0.0f, 2);
    s_self->s_selector->SetTint(glm::vec3(0.0f, 1.0f, 0.0f));  
    s_self->s_selector->SetDrawStyle(GL_LINE);
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

    Resources::Manager::UnLoadRawImage("editor logo");
    Resources::Manager::UnLoadRawImage("icon large");
    Resources::Manager::UnLoadRawImage("audio src");
    Resources::Manager::UnLoadRawImage("data src");
    Resources::Manager::UnLoadRawImage("folder src");

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
