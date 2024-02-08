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


//----------------------------------


void RemoveTempFiles()
{
    remove((Editor::projectPath + "\\src\\game.cpp").c_str());
    remove((Editor::projectPath + AssetManager::command_dir).c_str());
    remove((Editor::projectPath + AssetManager::preload_dir).c_str());
}


//-----------------------------


void Editor::Update() 
{

    Application::Update(); 

    Shader::Update(camera);

    glfwPollEvents(); 
    
    if (Application::inputs != nullptr)
       Application::inputs->processInput(Window::s_instance);

    Time::Update(glfwGetTime());
    
    GUI::Render(); 

    glViewport(camera->m_position.x, camera->m_position.y, Window::m_width, Window::m_height);
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

    GUI::Close();

}


//------------------------------


Editor::~Editor()
{

    Node::ClearAll();

    RemoveTempFiles();

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

    RemoveTempFiles();

    Node::ClearAll();

    Editor::camera->Reset();
 
    AssetManager::images.clear();
    AssetManager::loadedAssets.clear();

    Log("Workspace reset.");
}











