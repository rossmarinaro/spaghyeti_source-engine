#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../build/sdk/include/app.h"
#include "../../build/sdk/include/window.h"

#ifdef __EMSCRIPTEN__
    #include <emscripten/eventloop.h>
#endif

using namespace System;



void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    s_width = width;
    s_height = height;    
}



//-----------------------------------



const glm::vec2 Window::GetNDCToPixel(float x, float y) 
{
    float ndcX = ((2.0f * x) / s_width - 1.0f),
          ndcY = (1.0f - (2.0f * y) / s_height),
          pixelX = (ndcX + 1.0f) * (s_scaleWidth / 2), 
          pixelY = s_scaleHeight - (ndcY + 1.0f) * (s_scaleHeight / 2); 

    return { pixelX, pixelY };
}



//------------------------------------ Initialize Window



void Window::Init()
{

    if(!glfwInit())
    {

        #ifndef __EMSCRIPTEN__
            std::cout << "GLFW: Error - " << glfwGetError(NULL) << "\n";
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
        s_glsl_version = "#version 100";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    #elif defined(__APPLE__)
        // GL 3.2 + GLSL 150
        s_glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
    #else
        // GL 3.0 + GLSL 130
        s_glsl_version = "#version 330"; //"#version 330";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    #endif

    glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);


//set window dimensions for desktop builds

    GLFWmonitor* isFullScreen = NULL;

    #if STANDALONE == 0
        s_width = 1200.0f;
        s_height = 600.0f;
    #else
        s_width = 900.0f;
        s_height = 500.0f;

        //isFullScreen = glfwGetPrimaryMonitor();
        //glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
        
    #endif

    //create window

    s_instance = glfwCreateWindow(
        s_width, 
        s_height,
        (Application::name + " POWERED BY ::SpaghYeti Source Engine:: PASTABOSS ENTERPRISE 2024 🍝👌").c_str(), 
        isFullScreen, 
        NULL
    );

    //glfwSetWindowMonitor();

    if (!s_instance) {
        glfwTerminate();
        return;
    }
    
    glfwShowWindow(s_instance);
    glfwMakeContextCurrent(s_instance);

    #if DEVELOPMENT == 1
        std::cout << "Window: initialized.\n";
    #endif

    #ifndef __EMSCRIPTEN__

        if (!gladLoadGL() || !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout <<  "GLFW: Failed to initialize GLAD\n";
            return;
        }

    #endif

    #if DEVELOPMENT == 1
        std::cout <<  "GLFW: GL Version - " << glGetString(GL_VERSION) << "\n";
    #endif

}



