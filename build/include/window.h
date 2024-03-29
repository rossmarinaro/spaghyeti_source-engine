#pragma once

#include "./renderer.h"
#include <future>

namespace /* SPAGHYETI_CORE */ System {

    //main window
    class Window {

        public: 
      
            static inline float m_width, 
                                m_height, 
                                currentWidth, 
                                currentHeight,
                                m_scaleWidth = 1500.0f, 
                                m_scaleHeight = 900.0f;

            static inline GLFWwindow* s_instance = nullptr; 

            static inline const char* m_glsl_version = "";

            static void Init();

            static inline void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
                m_width = width;
                m_height = height;    
            }

    };

}
