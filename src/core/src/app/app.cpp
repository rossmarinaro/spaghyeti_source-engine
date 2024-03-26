#include "../../../../build/include/app.h"


void System::Application::Init(Game* layer)
{

    #if STANDALONE == 0
        game = layer;
    #endif
    
    Shader::InitBaseShaders();

    //run game layer
    
    Game::Boot();

    //init input callbacks 

    glfwSetKeyCallback(Window::s_instance, Inputs::key_callback); 
    glfwSetCursorPosCallback(Window::s_instance, Inputs::cursor_callback); 
    
    #ifndef __EMSCRIPTEN__
        glfwSetMouseButtonCallback(Window::s_instance, Inputs::input_callback);
    #else 
        //emscripten_set_canvas_element_size("#canvas", m_width, m_height); 
    #endif

}


//----------------------------- 


void System::Application::Update(Camera* camera) 
{

    Renderer::Update(camera);

    #if STANDALONE == 1 

        Time::Update(glfwGetTime()); 
  
        glfwPollEvents();  
        
        if (game != nullptr)
            game->inputs->ProcessInput(Window::s_instance);

        glViewport(0, 0, Window::m_width, Window::m_height);
        glfwSetFramebufferSizeCallback(Window::s_instance, Window::framebuffer_size_callback); 
        glfwSwapBuffers(Window::s_instance);

    #endif

}



//-----------------------------


System::Application::Application(Game* layer)
{ 

    std::cout << "PASTABOSS ENTERPRISE:: SpagYETI Engine: application started. 👌\n";  

    //set global time object 

    resources = new Resources::Manager;

    #if STANDALONE == 1

        if (!layer) {
            std::cout << "Error: No target layer present.\n";  
            return;    
        }

        game = layer;

        Window::Init();    

        //run main app process

        Init(game);

        #ifdef __EMSCRIPTEN__
            emscripten_set_main_loop(Update, 0, 1);
        #else

            while (!glfwWindowShouldClose(Window::s_instance))
                Update(game->camera);
    
        #endif
 
    #endif
}



//-----------------------------


System::Application::~Application()
{
    
    #if STANDALONE == 1
        Game::Exit();
    #endif

    Resources::Manager::Clear();

    #ifndef __EMSCRIPTEN__
        if (Window::s_instance != nullptr)
            delete Window::s_instance;   
    #endif

    delete resources;

    std::cout << "Application terminated. 👌\n";

};
