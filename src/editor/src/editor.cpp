#include <fstream>
#include <sstream>
#include <time.h>
#include <ctime>

#include "./editor.h"
#include "./assets/assets.h"
#include "./gui/gui.h"
#include "./nodes/node.h"
#include "../../../build/include/app.h"

using namespace /* SPAGHYETI_CORE */ System;
using namespace editor;

//-----------------------------


void Editor::Update()
{

    Renderer::Update(game->camera);

    if (GUI::grid)
        GUI::grid->Render();

    Time::Update(glfwGetTime());
    
    glfwPollEvents();

    GUI::Render();

    glViewport(0, 0, Window::m_width, Window::m_height);
	glfwSetFramebufferSizeCallback(Window::s_instance, Window::framebuffer_size_callback);
	glfwSwapBuffers(Window::s_instance);

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

    Application::name = "SPAGHYETITOR";

    Window::Init();

    Application::Init(game);

    GUI::Launch();

    //set top-left header and bottom toolbar icon

    GLFWimage image;

    image.width = 66;
    image.height = 65;
    image.pixels = reinterpret_cast<unsigned char*>(const_cast<char*>(Resources::Manager::GetRawData("editor logo")));

    glfwSetWindowIcon(Window::s_instance, 1, &image);

    //main update loop

    while (!glfwWindowShouldClose(Window::s_instance))
        Update();

    while (!Editor::events.exitFlag) 
        if (Editor::events.shouldSave) 
            Editor::events.saveFlag = true;
        else 
            Editor::events.exitFlag = true;

    Node::ClearAll();

    GUI::Close();

}


//------------------------------


Editor::~Editor()
{

    remove((projectPath + "\\src\\game.cpp").c_str());
    remove((projectPath + "\\spaghyeti_parse.json").c_str());

    Resources::Manager::UnLoadRawImage("editor logo");
    Resources::Manager::UnLoadRawImage("icon large");
    Resources::Manager::UnLoadRawImage("audio src");
    Resources::Manager::UnLoadRawImage("data src");

    Editor::Log("Editor closed.");

}


//------------------------------


void Editor::Log(const std::string& message)
{

    std::filesystem::current_path(Editor::rootPath);

    std::ofstream src("appLog.txt", std::ofstream::app | std::ofstream::out);

    std::time_t time_stamp = std::time(nullptr);

    src << message << " @ " << std::ctime(&time_stamp);

}


//--------------------------------


void Editor::Reset()
{

    remove((projectPath + "\\src\\game.cpp").c_str());

    Node::ClearAll();

    game->camera->Reset();

    worldWidth = 2000;
	worldHeight = 2000;

    globals_applied = false;
    gravity_continuous = true;
    gravity_sleeping = true;

    selectedAsset.clear();
    globals.clear();
    scenes.clear();

    AssetManager::images.clear();
    AssetManager::loadedAssets.clear();

    Log("Workspace reset.");
}






