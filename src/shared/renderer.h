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
    

//-----------------------------

namespace /* SPAGHYETI_CORE */ System {

    class Renderer {

        public:

            static void CreateFrameBuffer();
            static void RescaleFrameBuffer(float width, float height);
            static void Update(void* camera);
            static void BindFrameBuffer();
            static void UnbindFrameBuffer();
            static inline void SetVSync(int rate) { s_vsync = rate; }
            static inline int GetVSync() { return s_vsync; }
            
            static void cursor_callback(GLFWwindow* window, double xPos, double yPos);
            static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void input_callback(GLFWwindow* window, int input, int action, int mods);
            static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
            static void window_size_callback(GLFWwindow* window, int width, int height);

            static inline GLFWwindow* GLFW_window_instance;

        private:

            static inline unsigned int s_FBO, s_RBO;
        
            static inline int s_vsync = 1;
    };
}

/* TODO: Batch Rendering :) */


//#include "../../../vendors/imgui/imgui_impl_opengl3.h"

// #ifndef RENDERER 
// #define RENDERER

// struct Vertex
// {
//     glm::vec3 Position;
//     glm::vec4 Color;
//     glm::vec2 TexCoords;
//     float TexIndex;
// };

// struct BatchData
// {
//     GLuint QuadVA = 0;
//     GLuint QuadVB = 0;
//     GLuint QuadIB = 0;

//     GLuint WhiteTexture = 0;
//     uint32_t WhiteTextureSlot = 0;
//     uint32_t IndexCount = 0;

//     Vertex* QuadBuffer = nullptr;
//     Vertex* QuadBufferPtr = nullptr;

//     //static std::array<uint32_t, MaxTextures> TextureSlots;
//     uint32_t TextureSlotIndex = 1;

// };


// struct Stats
// {
//     uint32_t DrawCount = 0;
//     uint32_t QuadCount = 0;
// };


// class BatchRenderer {


//     public: 
    
//         static inline BatchData m_Data;
//         static inline Stats RenderStats;
//         static const size_t MaxQuadCount = 1000;
//         static const size_t MaxVertexCount = MaxQuadCount * 4;
//         static const size_t MaxIndexCount = MaxQuadCount * 6;
//         static const size_t MaxTextures = 32;


//         static void Init();
//         static void ShutDown();

//         static void Begin();
//         static void End();

//         static void DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color);
//         static void DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const uint32_t texID);

//         static void Render(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color, float textureIndex);
    
//         static const Stats &GetStats();
//         static void ResetStats();
// };

// #endif