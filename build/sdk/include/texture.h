#pragma once

#include <string>
#include "./renderer.h"

namespace Graphics {

    struct Format { float width, height; float u1, v1, u2, v2; };
    struct Renderable { float x, y; Format format; };

//--------------------------------------

    //Texture2D stores and configures textures. It is a quad with additional utilities for image data
    class Texture2D {

        public:

            float FrameWidth, FrameHeight, U1, V1, U2, V2, Width, Height; // width and height of loaded image in pixels 
            
            unsigned int ID,
                         Channels,
                         Repeat,
                         Wrap_S, // wrapping mode on S axis
                         Wrap_T, // wrapping mode on T axis
                         Filter_Min, // filtering mode if texture pixels < screen pixels
                         Filter_Max; // filtering mode if texture pixels > screen pixels

            Texture2D();
            ~Texture2D() = default;

            static Texture2D& Get(const std::string& key);
            static void Load(const std::string& key);
            static void UnLoad(const std::string& key);
            static void SetChannels(Texture2D& texture, unsigned int channels);
            
            void SetFiltering();
            void Generate(unsigned int width, unsigned int height, unsigned int channels, const void* data);  // generates texture from image data
            void Update(const glm::vec2& position, bool flipX, bool flipY, int drawStyle, float thickness = 1.0f);
            void Bind() const { glBindTexture(GL_TEXTURE_2D, ID); }; // binds the texture as the current active GL_TEXTURE_2D texture object
            void Delete();

        private: 

            GLuint m_VBO, m_UVBO;

            float m_UVs[12];

            unsigned int m_VAO,
                         m_internal_format, // format of texture object RGB, RGBA
                         m_image_format; // format of loaded image
    }; 

}
