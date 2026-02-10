#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../build/sdk/include/app.h"
#include "../../build/sdk/include/window.h"
#include "../core/src/debug.h"
#include "./renderer.h"

using namespace System;



const Math::Vector2 Window::GetPixelToNDC(float x, float y) 
{
    const float ndcX = ((2.0f * x) / s_width - 1.0f),
                ndcY = (1.0f - (2.0f * y) / s_height);  

    return { ndcX, ndcY };
}


//-----------------------------------


const Math::Vector2 Window::GetNDCToPixel(float x, float y) 
{
    Math::Vector2 ndc = GetPixelToNDC(x, y);

    const float pixelX = (ndc.x + 1.0f) * (s_scaleWidth / 2), 
                pixelY = s_scaleHeight - (ndc.y + 1.0f) * (s_scaleHeight / 2); 

    return { pixelX, pixelY };
} 


//------------------------------------ Initialize Window
 

void Window::Init()
{

    if(!glfwInit()) 
    { 
        #ifndef __EMSCRIPTEN__ 
            int err = glfwGetError(NULL);
            LOG("GLFW: Error:: " + std::to_string(err));
        #endif
        
        exit(EXIT_FAILURE);
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
        s_glsl_version = "#version 430"; //"#version 330";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    #endif

    glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    //set window dimensions for desktop builds

    GLFWmonitor* isFullScreen = NULL;

    #ifdef __EMSCRIPTEN__

        s_width = 1080.0f;
        s_height = 720.0f;

    #else

        #if STANDALONE == 0
        
            s_width = 1200.0f;
            s_height = 680.0f;

        #else 

            s_width = 380.0f; 
            s_height = 300.0f; 

        #endif

        //isFullScreen = glfwGetPrimaryMonitor();
        //glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
        
    #endif

    //create window

    Renderer::GLFW_window_instance = glfwCreateWindow(
        s_width, 
        s_height,
        (Application::name + " POWERED BY ::SpaghYeti Source Engine:: PASTABOSS ENTERPRISE 2025 🍝👌").c_str(), 
        isFullScreen, 
        NULL
    );

    //glfwSetWindowMonitor(Renderer::GLFW_window_instance, monitor, 0, 0, s_width, s_height, 60);

    if (!Renderer::GLFW_window_instance) {
        LOG("GLFW: window could not be created.");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    glfwShowWindow(Renderer::GLFW_window_instance);
    glfwMakeContextCurrent(Renderer::GLFW_window_instance);

    LOG("Window: initialized.");

    #ifndef __EMSCRIPTEN__ 

        if (!gladLoadGL() || !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            LOG("GLFW: Failed to initialize GLAD");
            exit(EXIT_FAILURE);
        }

    #endif

    Renderer::Init();

    LOG("Window: GL Version - " << glGetString(GL_VERSION));

}



