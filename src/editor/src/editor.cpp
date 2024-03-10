#include <fstream>
#include <sstream>
#include <time.h>
#include <ctime> 

#include "./editor.h"
#include "./assets/assets.h"
#include "../src/gui/gui.h"
#include "../src/gui/nodes/node.h"
#include "../../../build/include/app.h"

using namespace /* SPAGHYETI_CORE */ System;



//-----------------------------


void Editor::Update() 
{

    Application::Update(); 

    Shader::Update(camera);
    
    if (GUI::grid) 
        GUI::grid->Render();
        
    glfwPollEvents(); 

    Time::Update(glfwGetTime());
    
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

    Camera cam;
    camera = &cam;

    Window::Init(); 
 
    Application::Init(); 

    GUI::Launch(); 

    //set top-left header and bottom toolbar icon 
    
    GLFWimage image; 
    
    image.width = 66;
    image.height = 65;
    image.pixels = reinterpret_cast<unsigned char*>(const_cast<char*>(System::Resources::Manager::GetRawData("editor logo")));
 
    glfwSetWindowIcon(Window::s_instance, 1, &image);

    //main update loop
    
    while (!glfwWindowShouldClose(Window::s_instance))
       Update();

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


void Editor::Log(const std::string &message)
{

    std::filesystem::current_path(Editor::rootPath);   

    std::ofstream src;

    std::time_t time_stamp = std::time(nullptr);

    src.open("appLog.txt", std::ofstream::app | std::ofstream::out);

    src << message << " @ " << std::ctime(&time_stamp);

    src.close(); 
}


//--------------------------------


void Editor::Reset()
{

    remove((projectPath + "\\src\\game.cpp").c_str());

    Node::ClearAll();

    worldWidth = 0; 
	worldHeight = 0;
    selectedAsset.clear();  
    camera->Reset();
    globals_applied = false;
    gravity_continuous = true;
    gravity_sleeping = true;
    globals.clear();
 
    AssetManager::images.clear();
    AssetManager::loadedAssets.clear();

    Log("Workspace reset.");
}











