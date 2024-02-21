#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../build/include/app.h"
#include "../../build/include/window.h"

#ifdef __EMSCRIPTEN__
    #include <emscripten/eventloop.h>
#endif

using namespace System;



//---------- Initialize Window


void Window::Init()
{

    if(!glfwInit())
    {

        #ifndef __EMSCRIPTEN__
            std::cout << "GLFW Error: " << glfwGetError(NULL) << "\n";
        #endif

        exit(EXIT_FAILURE);

        return;
    }

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);

	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);

    // Decide GL+GLSL versions
    #if defined(IMGUI_IMPL_OPENGL_ES2)
        // GL ES 2.0 + GLSL 100
        m_glsl_version = "#version 100";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    #elif defined(__APPLE__)
        // GL 3.2 + GLSL 150
        m_glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
    #else
        // GL 3.0 + GLSL 130
        m_glsl_version = "#version 330"; //"#version 330";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    #endif

    glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    //glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);

    GLFWmonitor* isFullScreen = NULL;

    #if STANDALONE == 1
        //isFullScreen = glfwGetPrimaryMonitor();
    #endif

    //set window dimensions for desktop builds

    m_width = Application::isMobile ? 800.0f : 1200.0f;
    m_height = Application::isMobile ? 400.0f : 600.0f;

    //create window

    s_instance = glfwCreateWindow(
            m_width, 
            m_height, (Application::game->name + " POWERED BY ::SpaghYeti Source Engine:: PASTABOSS ENTERPRISE 2024 🍝👌").c_str(), 
            isFullScreen, 
            NULL
        );

    //glfwSetWindowMonitor()

    // #ifndef __EMSCRIPTEN__

    //     GLFWimage image; 
        
    //     image.width = 70;
    //     image.height = 70;
    //     image.pixels = reinterpret_cast<unsigned char*>(const_cast<char*>(System::Resources::Manager::GetRawData("logo")));
    
    //     glfwSetWindowIcon(s_instance, 1, &image);

    // #endif 

    if (!s_instance) {
        glfwTerminate();
        return;
    }
    
    glfwShowWindow(s_instance);
    glfwMakeContextCurrent(s_instance);

    std::cout << "window initialized.\n";

    #ifdef __EMSCRIPTEN__

    #else
        if (!gladLoadGL() || !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout <<  "Failed to initialize GLAD\n";
            return;
        }

    #endif

    std::cout <<  "GL Version: " << glGetString(GL_VERSION) << "\n";


}



