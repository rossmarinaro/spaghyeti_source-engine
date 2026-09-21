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
    #ifndef GLAD
    #define GLAD
        #include "../vendors/glad/include/glad/glad.h"    
    #endif 
#endif 

#if STANDALONE == 0
    #include <string>
#endif

#include "../../build/sdk/include/math.h"

//-----------------------------


namespace /* SPAGHYETI_CORE */ System {
    //triple buffer batch renderer
    class Renderer {

        public:

            //render bucket to submit in batches 
            struct Renderable { 
                unsigned int shaderID, shaderDepth; 
                std::vector<Math::Graphics::Vertex> vertices;
                std::vector<uint32_t> indices;
            };  
                
            static inline const size_t MAX_TEXTURES = 32,
                                       MAX_QUADS = 1000;

            uint32_t textureSlotIndex, drawStyle;
            std::array<uint32_t, MAX_TEXTURES> textureSlots;
            std::vector<Renderable> activeLayers;

            Renderer();
            ~Renderer() = default;
 
            static inline int GetVsync() { return s_vsync; }
            static void Init();
            static void Update(void* camera);
            static void ShutDown();  
            static void SetVsync(int rate);
            static void Flush(bool renderOpaque = true, int shaderID = -1); 
            static void UpdateFrameBuffer(void* camera);
            static Renderable& GetOrCreateRenderBucket(unsigned int shaderID);
            static Renderer* Get();

        private:

            static inline const int BUFFERS = 3, 
                                    m_frameBufferWidth = 1480,
                                    m_frameBufferHeight = 860;

            static inline int s_vsync, s_currentBufferIndex;

            GLsync m_fences[BUFFERS]; 
            GLuint m_VBOs[BUFFERS]; //ring buffer
            GLuint m_textureColorBuffer, m_VAO, m_EBO, m_FBO /* , m_RBO */; 
            std::vector<GLuint> m_indices;

            static void RenderBatch(Renderable& renderable);
    };
}
