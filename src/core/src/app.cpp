#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/audio.h"
#include "../../shared/window.h"
#include "../../shared/renderer.h"

void System::Application::Init(Game* layer)
{
    //set top-left header and bottom toolbar icon (not binary, this image is stored as pixel data)

    /* const auto image_data = Resources::Manager::GetResource("icon small");

    GLFWimage image;

    image.width = 66; 
    image.height = 65;
    image.pixels = image_data.array_buffer;

    glfwSetWindowIcon(Renderer::GLFW_window_instance, 1, &image); */

    #if STANDALONE == 0
        game = layer;
    #endif

    Audio::Init();
    Renderer::Init();

    Graphics::Texture2D::InitBaseTexture();
    Graphics::Shader::InitBaseShaders();
    
    game->Boot();

    //glfwMaximizeWindow(Renderer::GLFW_window_instance);

    //init input callbacks 

    glfwSetKeyCallback(Window::GLFW_window_instance, Window::key_callback); 
    glfwSetCursorPosCallback(Window::GLFW_window_instance, Window::cursor_callback); 
 
    #ifndef __EMSCRIPTEN__
        glfwSetMouseButtonCallback(Window::GLFW_window_instance, Window::input_callback);
    //#else 
        //emscripten_set_canvas_element_size("#canvas", m_width, m_height); 
    #endif

}


//----------------------------- 


void System::Application::Update(void* layer)
{
    if (layer == nullptr)
        return; 

    const auto game = static_cast<Game*>(layer); 

    Renderer::Update(game->camera);
 
    Time::Update(glfwGetTime());  

    glfwPollEvents(); 

    glfwSetFramebufferSizeCallback(Window::GLFW_window_instance, Window::framebuffer_size_callback);
    glfwSetWindowSizeCallback(Window::GLFW_window_instance, Window::window_size_callback); 
    glfwSwapBuffers(Window::GLFW_window_instance); 

}

 

//----------------------------- 



void System::Application::Start (
    int screenWidth, 
    int screenHeight, 
    int resolutionWidth,
    int resolutionHeight,
    Game* layer, 
    const std::string& key, 
    bool isMultithreaded, 
    bool isMobileSupported,
    bool isFullScreen,
    int vsync
)
{ 
    name = key;

    remove("log.txt");

    LOG("PASTABOSS ENTERPRISE:: SpagYETI Engine: application started. 👌");  

    resources = new Resources::Manager;
    events = new Events;

    events->isMultiThreaded = isMultithreaded;
    events->isMobile = isMobileSupported;

    Display::resolutionWidth = resolutionWidth;
    Display::resolutionHeight = resolutionHeight;

    #if STANDALONE == 1

        if (!layer) {
            LOG("Error: No target layer present.");  
            return;    
        }

        game = layer;

        Window::Init(screenWidth, screenHeight, isFullScreen);  

        Renderer::SetVsync(vsync);
 
        //run main app process

        Init(game);

        #ifdef __EMSCRIPTEN__
            emscripten_set_main_loop_arg(Update, game, 0, 1);
        #else
            while (!glfwWindowShouldClose(Window::GLFW_window_instance))
                Update(game);
        #endif

        ShutDown();

    #endif
}



//-----------------------------


void System::Application::ShutDown()
{

    #if STANDALONE == 1
        game->Exit();
    #endif

    Resources::Manager::Clear();

    #ifndef __EMSCRIPTEN__
        if (Window::GLFW_window_instance != nullptr)
            delete Window::GLFW_window_instance;   
    #endif

    delete resources;
    resources = nullptr;

    delete events;
    events = nullptr;

    Audio::ShutDown();
    Renderer::ShutDown();

    LOG("Application terminated successfully. 👌");
};