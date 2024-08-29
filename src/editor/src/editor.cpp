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

//-----------------------------


void Editor::Update()
{

    Renderer::Update(game->camera);

    if (GUI::s_grid)
        GUI::s_grid->Render(System::Window::s_scaleWidth, System::Window::s_scaleHeight);

    Time::Update(glfwGetTime());

    //save hotkey
       
    if (projectOpen)
    {
        if (events.canSave && 
        (
            ImGui::IsKeyPressed(ImGuiKey_LeftCtrl) || ImGui::IsKeyPressed(ImGuiKey_RightCtrl)) && 
            ImGui::IsKeyPressed(ImGuiKey_S)) 
        {

            events.canSave = false; 
            events.SaveScene(events.saveFlag);
        }

        else 
            events.canSave = true;
    }

    glfwPollEvents();

    GUI::Render(); 

    //track mouse position

    auto position = Window::GetNDCToPixel(ImGui::GetMousePos().x, ImGui::GetMousePos().y);

    game->inputs->mouseX = ImGui::GetMousePos().x - position.x;
    game->inputs->mouseY = ImGui::GetMousePos().y - Editor::game->camera->position.y;

    //current selected entity

    if (s_selector) {
        s_selector->SetPosition(selectedEntityTransform.second[0], selectedEntityTransform.second[1]); 
        s_selector->SetSize(selectedEntityTransform.second[2], selectedEntityTransform.second[3]); 
    } 

    glViewport(0, 0, Window::s_width, Window::s_height);
    
	glfwSetFramebufferSizeCallback(Window::s_instance, Window::framebuffer_size_callback);
	glfwSwapBuffers(Window::s_instance);

    //save and close editor

    if (glfwWindowShouldClose(Window::s_instance))
        if (events.canSave) {
            if (projectOpen)
                events.saveFlag = true;
            else 
                events.exitFlag = true;
        }

}


//-----------------------------


Editor::Editor()
{
    //AllocConsole();

    remove("appLog.txt");

    rootPath = std::filesystem::current_path().string();

    Log("Editor Root: " + rootPath);

    Game g;
    game = &g; 

    //init base sandox scene

    game->LoadScene<System::Scene>();

    Application::name = "SPAGHYEDITOR";

    Window::Init();

    Application::Init(game);

    GUI::Launch();    

    //set top-left header and bottom toolbar icon

    GLFWimage image;

    image.width = 66;
    image.height = 65;
    image.pixels = reinterpret_cast<unsigned char*>(const_cast<char*>(Resources::Manager::GetRawData("editor logo")));

    glfwSetWindowIcon(Window::s_instance, 1, &image);

    //create entity selector graphic

    s_selector = System::Game::CreateGeom(100.0f, 100.0f, 100.0f, 100.0f, 2);
    s_selector->SetTint(glm::vec3(0.0f, 1.0f, 0.0f));  
    s_selector->SetDrawStyle(GL_LINE);
    
    //main update loop

    while (!Editor::events.exitFlag) 
        Update();
        
    Node::ClearAll();

    GUI::Close();
}


//------------------------------


Editor::~Editor()
{

    remove((projectPath + "\\game.cpp").c_str());
    remove((projectPath + "\\spaghyeti_parse.json").c_str());

    Resources::Manager::UnLoadRawImage("editor logo");
    Resources::Manager::UnLoadRawImage("icon large");
    Resources::Manager::UnLoadRawImage("audio src");
    Resources::Manager::UnLoadRawImage("data src");
    Resources::Manager::UnLoadRawImage("folder src");

    Editor::Log("Editor closed.");

}


//------------------------------


void Editor::Log(const std::string& message)
{

    std::filesystem::current_path(Editor::rootPath);

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

    game->camera->Reset();

    worldWidth = 2000;
	worldHeight = 2000;

    globals_applied = false;
    gravity_continuous = true;
    gravity_sleeping = true;

    globals.clear();
    spritesheets.clear();
    scenes.clear();

    AssetManager::Reset();

    Log("Workspace reset.");
}
