#pragma once

#include <string>
#include "./math.h"

namespace Graphics {  

    struct Format { float width, height; float u1, v1, u2, v2; };
    struct Renderable { float x, y; Format format; int textureID; };

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
            static void InitBaseTexture();
            
            void Delete();
            void SetFiltering(bool filterMin = true, bool filterMax = true, bool wrapS = true, bool wrapT = true);
            void Update(
                unsigned int shaderID, 
                const Math::Vector2& position, 
                const Math::Vector2& scale,
                const Math::Vector4& rgba, 
                const Math::Vector3& outline,
                const Math::Matrix4& modelView,
                float outlineWidth,
                float rotation,
                int depth,
                bool flipX = false, 
                bool flipY = false
            );

        private: 

            unsigned int m_internal_format, // format of texture object RGB, RGBA
                         m_image_format; // format of loaded image
    }; 

}
