#pragma once

#include <string>
#include "./math.h"

namespace Graphics {  

    //Texture2D stores and configures textures. It is a quad with additional utilities for image data
    class Texture2D {

        public:

            std::string key;

            bool Whiteout;

            float FrameWidth, 
                  FrameHeight, 
                  U1, 
                  V1, 
                  U2, 
                  V2, 
                  MinU, 
                  MinV, 
                  MaxU, 
                  MaxV,  
                  Width, 
                  Height; 
            
            unsigned int ID,
                         Channels,
                         Repeat,
                         Wrap_S, 
                         Wrap_T, 
                         Filter_Min, //texture pixels < screen pixels
                         Filter_Max; //texture pixels > screen pixels

            Texture2D();
            ~Texture2D() = default;

            static const Texture2D& Get(const std::string& key);
            static void Load(const std::string& key);
            static void UnLoad(const std::string& key);
            static void InitBaseTexture();
            
            const bool IsOpaque();
            void Delete();
            void SetFiltering(bool filterMin = true, bool filterMax = true, bool wrapS = true, bool wrapT = true);
            void Update(
                Graphics::Shader& shader,
                const Math::Vector2& position, 
                const Math::Vector4& rgba, 
                const Math::Vector3& outline,
                const Math::Matrix4& mvp,
                float outlineWidth,
                float whiteout,
                int depth,
                bool flipX = false, 
                bool flipY = false
            );

        private: 

            unsigned int m_internal_format, //RGB, RGBA
                         m_image_format;

            bool m_opaque;
    }; 

}
