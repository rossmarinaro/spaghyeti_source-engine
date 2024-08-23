#include "../../../build/sdk/include/app.h"


void System::Application::Init(Game* layer)
{

    #if STANDALONE == 0
        game = layer;
    #endif

    Shader::InitBaseShaders();
 
    //run game layer
     
    game->Boot();

    //glfwMaximizeWindow(Window::s_instance);

    //init input callbacks 

    glfwSetKeyCallback(Window::s_instance, Inputs::key_callback); 
    glfwSetCursorPosCallback(Window::s_instance, Inputs::cursor_callback); 
 
    #ifndef __EMSCRIPTEN__
        glfwSetMouseButtonCallback(Window::s_instance, Inputs::input_callback);
    //#else 
        //emscripten_set_canvas_element_size("#canvas", m_width, m_height); 
    #endif

}


//----------------------------- 


void System::Application::Update(void* layer)
{

    if (game == nullptr)
        return; 

    Game* game = static_cast<Game*>(layer);

    Renderer::Update(game->camera);
 
    Time::Update(glfwGetTime());

    glViewport(0, 0, Window::s_width, Window::s_height);

    glfwSetFramebufferSizeCallback(Window::s_instance, Window::framebuffer_size_callback); 
    glfwSwapBuffers(Window::s_instance); 


}



//-----------------------------


System::Application::Application(Game* layer, const std::string& key)
{ 

    name = key;

    #if DEVELOPMENT == 1
        remove("log.txt");
        LOG("PASTABOSS ENTERPRISE:: SpagYETI Engine: application started. 👌");  
    #endif

    //set global time object  

    resources = new Resources::Manager;

    #if STANDALONE == 1

        if (!layer) {
            LOG("Error: No target layer present.");  
            return;    
        }

        game = layer;

        Window::Init();    
 
        //run main app process

        Init(game);

        #ifdef __EMSCRIPTEN__
            emscripten_set_main_loop_arg(Update, game, 0, 1);
        #else
            while (!glfwWindowShouldClose(Window::s_instance))
                Update(game);
        #endif

    #endif
}



//-----------------------------


System::Application::~Application()
{

    #if STANDALONE == 1
        game->Exit();
    #endif

    Resources::Manager::Clear();

    #ifndef __EMSCRIPTEN__
        if (Window::s_instance != nullptr)
            delete Window::s_instance;   
    #endif

    delete resources;
    resources = nullptr;

    #if DEVELOPMENT == 1
        LOG("Application terminated successfully. 👌");
    #endif
};