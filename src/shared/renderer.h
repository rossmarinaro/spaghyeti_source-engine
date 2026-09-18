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

#if STANDALONE == 0
    #include <string>
#endif

#include "../../build/sdk/include/math.h"

//-----------------------------

    #include <memory>
namespace /* SPAGHYETI_CORE */ System {
    //triple buffer batch renderer
    class Renderer {

        public:
    struct Renderable { 
                int depth;
                unsigned int shaderID; 
                std::vector<Math::Graphics::Vertex> vertices;
                std::vector<uint32_t> indices;//uint32_t indices;
            };      
            static inline const size_t MAX_TEXTURES = 32,
                                       MAX_QUADS = 1000;

            uint32_t indexCount, 
                     textureSlotIndex;

            unsigned int drawStyle, activeShaderID;
            std::array<uint32_t, MAX_TEXTURES> textureSlots;
            std::vector<Math::Graphics::Vertex> vertices;
std::vector<unsigned int> indices;
std::vector<std::shared_ptr<Renderable>> activeLayers;
            Renderer();
            ~Renderer() = default;
 
            static inline int GetVsync() { return s_vsync; }
            static inline Renderer* Get() { return s_instance; }
                       
            static void cursor_callback(GLFWwindow* window, double xPos, double yPos);
            static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void input_callback(GLFWwindow* window, int input, int action, int mods);
            static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
            static void window_size_callback(GLFWwindow* window, int width, int height);

            static void Init();
            static void Update(void* camera);
            static void ShutDown();  
            static void SetVsync(int rate);
            static void Flush(bool renderOpaque = true, std::shared_ptr<Renderable> renderable = nullptr); 
            static void UpdateFrameBuffer(void* camera);
            static const bool CheckBufferLimit();

            static inline GLFWwindow* GLFW_window_instance;
static void RenderBatch(std::shared_ptr<Renderable>& renderable);
static std::shared_ptr<Renderable> GetOrCreateRenderBucket(unsigned int shaderID, int depth);
        private:

            static inline const int BUFFERS = 3, 
                                    m_frameBufferWidth = 1480,
                                    m_frameBufferHeight = 860;

            static inline int s_vsync, s_currentBufferIndex;

            GLsync m_fences[BUFFERS]; 
            GLuint m_VBOs[BUFFERS]/* , m_VAOs[BUFFERS] */; //ring buffer
            GLuint m_textureColorBuffer, m_EBO,m_VAO, /* m_EBO, */ m_FBO /* , m_RBO */; 
            std::vector<GLuint> m_indices;

            static inline Renderer* s_instance;

     
    };
}
