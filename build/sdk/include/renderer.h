#pragma once


#include <map>
#include <vector>

#ifdef __EMSCRIPTEN__

    #include <unistd.h>
    #include <emscripten.h>
    #include <emscripten/html5.h>

    #define GL_GLEXT_PROTOTYPES
    #define EGL_EGLEXT_PROTOTYPES
    #include <GLES3/gl3.h>

#else 

    #ifndef _GLFW_WIN32
        #define _GLFW_WIN32
        #include "./vendors/glad/include/glad/glad.h" 
    #endif

#endif

#include "./vendors/GLFW/glfw3.h" 
#include "./vendors/GLFW/glfw3native.h" 
#include "./vendors/glm/glm.hpp" 
#include "./vendors/glm/gtc/type_ptr.hpp"
#include "./vendors/glm/gtc/matrix_transform.hpp" 

#if USE_JSON == 1 
	#include "./vendors/nlohmann/json.hpp"
	using json = nlohmann::json;
#endif

#include "./vendors/glm/gtc/matrix_transform.hpp"  

#include "./vendors/box2d/include/box2d/box2d.h"

#ifndef GLT_IMPLEMENTATION

    #if defined(_MSC_VER) && (_MSC_VER >= 1310)
    #	pragma warning(disable: 4996) // Disable the fopen, strcpy, sprintf being unsafe warning
    #endif
#define GLT_MANUAL_VIEWPORT
    #define GLT_IMPLEMENTATION
    #include "./vendors/glText/gltext.h"
#endif

#include "./camera.h"

//-----------------------------

namespace /* SPAGHYETI_CORE */ System {

    class Renderer {

        public:

            static void CreateFrameBuffer();
            static void RescaleFrameBuffer(float width, float height);
            static void Update(Camera* camera);
            
            static inline void BindFrameBuffer() { glBindBuffer(GL_FRAMEBUFFER, s_FBO); }
            static inline void UnbindFrameBuffer() { glBindBuffer(GL_FRAMEBUFFER, 0); }

            static inline void SetVSync(int rate) { s_vsync = rate; }
            static inline int GetVSync() { return s_vsync; }
            

        private:

            static inline GLuint s_FBO, s_RBO;
        
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