#pragma once

#include <string>

#include "./renderer.h"
#include "./shader.h"

namespace Graphics {

    
    struct Format { float width, height; float u1, v1, u2, v2; };
    struct Renderable { float x, y; Format format; };


    //Base OpenGL Primitive for rendering textures and basic shapes
    class Primitive {

        public:

            unsigned int ID, VAO;
            GLuint VBO, UVBO;

            void GenBuffer ();
            
            void Bind_Buffer (
                const auto &data, 
                const GLuint &buffer, 
                const GLuint &VAO,
                GLuint location, 
                GLint vecCount, 
                GLenum type, 
                GLsizei normal, 
                GLenum draw, 
                unsigned long long stride
            );

            static void Draw (int shape, int dimension, int slot, int vertices, int drawStyle);

            Primitive() { this->GenBuffer(); };

            virtual ~Primitive() = default;
   
    };


//--------------------------------------

    // Texture2D is able to store and configure a texture in OpenGL.
    // It is essentially a quad with additional utilities for image data
    class Texture2D : public Primitive {

        public:

            float FrameWidth, FrameHeight, U1, V1, U2, V2, Width, Height; // width and height of loaded image in pixels 
            
            unsigned int Repeat,
                         Wrap_S, // wrapping mode on S axis
                         Wrap_T, // wrapping mode on T axis
                         Filter_Min, // filtering mode if texture pixels < screen pixels
                         Filter_Max; // filtering mode if texture pixels > screen pixels

            Texture2D() :  
                Primitive(), 
                    Width(0.0f),
                    Height(0.0f),
                    FrameWidth(0.0f),
                    FrameHeight(0.0f),
                    U1(0.0f),
                    V1(0.0f),
                    U2(1.0f),
                    V2(1.0f),
                    Internal_Format(GL_RGB), 
                    Image_Format(GL_RGB),
                    Wrap_S(GL_REPEAT),
                    Wrap_T(GL_REPEAT), 
                    Filter_Min(GL_NEAREST),   
                    Filter_Max(GL_NEAREST),
                    Channels(3),
                    Repeat(1)
            { glGenTextures(1, &this->ID);  };
            
            ~Texture2D() = default;

            static Texture2D& GetTexture(const std::string &key);
            static void Load(const std::string &key);
            static void UnLoad(const std::string &key);
            static void SetChannels(Texture2D &texture, unsigned int channels);
            
            void SetFiltering();
            void Generate(unsigned int width, unsigned int height, auto &data);  // generates texture from image data
            void Update(const glm::vec2 &position, bool flipX, bool flipY);
            void Bind() const { glBindTexture(GL_TEXTURE_2D, this->ID); }; // binds the texture as the current active GL_TEXTURE_2D texture object

        private: 

            unsigned int Channels;
            unsigned int Internal_Format; // format of texture object RGB, RGBA
            unsigned int Image_Format; // format of loaded image
            float UVs[12];
    }; 

}
