#pragma once

#include "./renderer.h"
#include <future>

namespace /* SPAGHYETI_CORE */ System {

    //main window
    class Window {

        public: 
      
            static inline float s_width, 
                                s_height, 
                                s_scaleWidth = 1500.0f, 
                                s_scaleHeight = 900.0f;

            static inline GLFWwindow* s_instance = nullptr; 

            static inline const char* s_glsl_version = "";

            static void Init();
            static const glm::vec2 GetNDCToPixel(float x, float y);
            static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    };

}
