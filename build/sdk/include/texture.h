#pragma once

#include <string>
#include "./math.h"
 /// Holds all state information relevant to a character as loaded using FreeType

namespace Graphics {  

    struct Format { float width, height; float u1, v1, u2, v2; };
    struct Renderable { float x, y; Format format; };

//--------------------------------------

    //Texture2D stores and configures textures. It is a quad with additional utilities for image data
    class Texture2D {

        public:

            std::string key;

            float FrameWidth, FrameHeight, U1, V1, U2, V2, Width, Height; // width and height of loaded image in pixels 
            
            unsigned int ID,
                         Channels,
                         Repeat,
                         Whiteout,
                         Wrap_S, // wrapping mode on S axis
                         Wrap_T, // wrapping mode on T axis
                         Filter_Min, // filtering mode if texture pixels < screen pixels
                         Filter_Max; // filtering mode if texture pixels > screen pixels

            Texture2D();
            ~Texture2D() = default;

            static const Texture2D& Get(const std::string& key);
            static void Load(const std::string& key);
            static void UnLoad(const std::string& key);
            
            const void Bind();
            void SetFiltering(bool filterMin = true, bool filterMax = true, bool wrapS = true, bool wrapT = true);
            void Generate(unsigned int width, unsigned int height, unsigned int channels, const void* data);  
            void Update(const Math::Vector2& position, bool flipX, bool flipY, int drawStyle, float thickness = 1.0f);
            void Delete();

        private: 

            unsigned int m_VBO, m_UVBO;

            float m_UVs[12];

            unsigned int m_VAO,
                         m_internal_format, // format of texture object RGB, RGBA
                         m_image_format; // format of loaded image
    }; 

}
