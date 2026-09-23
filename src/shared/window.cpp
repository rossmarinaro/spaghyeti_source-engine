#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
#endif

#include "../../build/sdk/include/app.h"
#include "./window.h"
#include "../core/src/debug.h"


using namespace System;


const Math::Vector2 Window::GetPixelToNDC(float x, float y) 
{
    const float ndcX = ((2.0f * x) / Display::screenWidth - 1.0f),
                ndcY = (1.0f - (2.0f * y) / Display::screenHeight);  

    return { ndcX, ndcY };
}


//-----------------------------------


const Math::Vector2 Window::GetNDCToPixel(float x, float y) 
{
    Math::Vector2 ndc = GetPixelToNDC(x, y);

    const float pixelX = (ndc.x + 1.0f) * (Display::resolutionWidth / 2), 
                pixelY = Display::resolutionHeight - (ndc.y + 1.0f) * (Display::resolutionHeight / 2); 

    return { pixelX, pixelY };
}  


//------------------------------------ Initialize Window
 

void Window::Init(int screenWidth, int screenHeight, bool isFullScreen)
{
    Display::isFullscreen = isFullScreen;   

    #ifdef _WIN32
        SetProcessDPIAware();
    #endif

    if(!glfwInit()) 
    { 
        #ifndef __EMSCRIPTEN__ 
            int err = glfwGetError(NULL);
            LOG("GLFW: Error:: " + std::to_string(err));
        #endif
        
        exit(EXIT_FAILURE);
    }

    GLFWmonitor* monitor = nullptr;

    //GL+GLSL versions
    #if defined(IMGUI_IMPL_OPENGL_ES2)
        //GL ES 2.0 + GLSL 100
        s_glsl_version = "#version 100";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    #elif defined(__APPLE__)
        //GL 3.2 + GLSL 150
        s_glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
    #else
        //GL 3.0 + GLSL 130
        s_glsl_version = "#version 430"; //"#version 330";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    #endif

    glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    //set window dimensions for builds

    #ifdef __EMSCRIPTEN__
        Display::screenWidth = 1080;
        Display::screenHeight = 720;
    #else
    
        //fullscreen
        
        if (isFullScreen) 
        {
            Display::screenWidth = Display::resolutionWidth;
            Display::screenHeight = Display::resolutionHeight; 

            monitor = glfwGetPrimaryMonitor();
            glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE); 
        }
        else {
            Display::screenWidth = screenWidth; 
            Display::screenHeight = screenHeight;
        }   

    #endif

    if (monitor) {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    }

    //glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);

    //create window

    GLFW_window_instance = glfwCreateWindow(
        Display::screenWidth, 
        Display::screenHeight,
        (Application::name + " POWERED BY ::SpaghYeti Source Engine:: PASTABOSS ENTERPRISE 2026 🍝👌").c_str(), 
        monitor, 
        NULL
    );

    //glfwSetWindowMonitor(GLFW_window_instance, monitor, 0, 0, Display::screenWidth, Display::screenHeight, 60);

    if (!GLFW_window_instance) {
        LOG("GLFW: window could not be created.");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    glfwShowWindow(GLFW_window_instance);
    glfwMakeContextCurrent(GLFW_window_instance);

    LOG("Window: initialized.");

    #ifndef __EMSCRIPTEN__ 
        if (!gladLoadGL() || !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            LOG("GLFW: Failed to initialize GLAD");
            exit(EXIT_FAILURE);
        }
    #endif

    LOG("Window: GL Version - " + (std::string)reinterpret_cast<const char*>(glGetString(GL_VERSION)));
}

//----------------------------------------


void Window::cursor_callback(GLFWwindow* window, double xPos, double yPos)
{
    //set cursor object to movement, translate ndc coords to clip space

    auto position = Window::GetNDCToPixel((float)xPos, (float)yPos);

    Application::game->inputs->mouseX = position.x;
    Application::game->inputs->mouseY = position.y;
}


//----------------------------------------


void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
        Application::game->inputs->SetKeyInputs(true, key, window);
        Application::game->inputs->numInputs++;
    }

    if (action == GLFW_RELEASE) {
        Application::game->inputs->SetKeyInputs(false, key, window);
        Application::game->inputs->numInputs--;
    }
}


//----------------------------------------


void Window::input_callback(GLFWwindow* window, int input, int action, int mods)
{
    if (input == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwIconifyWindow(window);
        
    if (input == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        Application::game->inputs->RIGHT_CLICK = true;

    else
        Application::game->inputs->RIGHT_CLICK = false;

    if (input == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        Application::game->inputs->LEFT_CLICK = true;

    else
        Application::game->inputs->LEFT_CLICK = false;

    if (action == GLFW_PRESS)
        Application::game->inputs->numInputs++;

    if (action == GLFW_RELEASE)
        Application::game->inputs->numInputs--;
}

//----------------------------------------


void Resize(GLFWwindow* window, int width, int height) 
{
    if (!window) 
        return;

    Display::screenWidth = width;
    Display::screenHeight = height;

    glViewport(0, 0, Display::screenWidth, Display::screenHeight);
}


//----------------------------------------


void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Resize(window, width, height);
}

//-----------------------------------


void Window::window_size_callback(GLFWwindow* window, int width, int height) {
    Resize(window, width, height);
}
