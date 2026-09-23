#pragma once

#include "./renderer.h"

#ifndef _GLFW_WIN32
    #define _GLFW_WIN32
    #include "../vendors/GLFW/glfw3.h"
    #include "../vendors/GLFW/glfw3native.h" 
#endif

namespace /* SPAGHYETI_CORE */ System {

    //main window
    class Window {

        public: 

            static inline const char* s_glsl_version;

            static void Init(int screenWidth, int screenHeight, bool isFullScreen = false);
            static const Math::Vector2 GetNDCToPixel(float x, float y);
            static const Math::Vector2 GetPixelToNDC(float x, float y);

            static void cursor_callback(GLFWwindow* window, double xPos, double yPos);
            static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void input_callback(GLFWwindow* window, int input, int action, int mods);
            static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
            static void window_size_callback(GLFWwindow* window, int width, int height);

            static inline GLFWwindow* GLFW_window_instance;
    };

}
 