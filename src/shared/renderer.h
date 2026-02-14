#pragma once

#include <map>
#include <vector>

#ifdef __EMSCRIPTEN__
    #ifndef ES
    #define ES
        #include <unistd.h>
        #include <emscripten.h>
        #include <emscripten/html5.h>

        #define GL_GLEXT_PROTOTYPES
        #define EGL_EGLEXT_PROTOTYPES
        #include <GLES3/gl3.h>
    #endif

#else
    #include "../vendors/glad/include/glad/glad.h"    
#endif 

#ifndef _GLFW_WIN32
    #define _GLFW_WIN32
    #include "../vendors/GLFW/glfw3.h"
    #include "../vendors/GLFW/glfw3native.h" 
#endif

#include "../../build/sdk/include/math.h"

//-----------------------------

namespace /* SPAGHYETI_CORE */ System {
    //triple buffer batch renderer
    class Renderer {

        public:

            static inline const size_t MAX_TEXTURES = 32,
                                       MAX_QUADS = 7000;

            uint32_t indexCount, 
                     textureSlotIndex;

            unsigned int drawStyle, activeShaderID; 

            std::array<uint32_t, MAX_TEXTURES> textureSlots;
            std::vector<Math::Graphics::Vertex> vertices;

            Renderer();
            ~Renderer() = default;

            static inline void SetVSync(int rate) { s_vsync = rate; }
            static inline int GetVSync() { return s_vsync; }
                       
            static void CreateFrameBuffer();
            static void RescaleFrameBuffer(float width, float height);
            static void Update(void* camera);
            static void BindFrameBuffer();
            static void UnbindFrameBuffer();
            static void cursor_callback(GLFWwindow* window, double xPos, double yPos);
            static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void input_callback(GLFWwindow* window, int input, int action, int mods);
            static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
            static void window_size_callback(GLFWwindow* window, int width, int height);
            
	        //static void InitInstances();
            //static void RenderInstances();
            
            static void Init();
            static void ShutDown();  
            static void Flush(); 

            static inline GLFWwindow* GLFW_window_instance;

        private:

            static inline const int BUFFERS = 3;
            static inline int s_vsync, s_currentBufferIndex;

            GLsync m_fences[BUFFERS];
            GLuint m_VBOs[BUFFERS]; //ring buffer
            GLuint m_VAO, m_EBO, m_FBO, m_RBO;
            std::vector<GLuint> m_indices;
    };
}
