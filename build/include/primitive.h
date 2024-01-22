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
        
            GLuint VBO, UVBO;

            glm::mat4 m_model; 

            int m_numOfVerts;
 
            unsigned int ID, quadVAO;

            void GenBuffer ();
            
            void Bind_Buffer (
                auto &data, 
                GLuint &buffer, 
                GLuint location, 
                GLint vecCount, 
                GLenum type, 
                GLsizei normal, 
                GLenum draw, 
                unsigned long long stride
            );

            void Draw (int shape, int dimension, int slot, int vertices, int drawStyle);

            Primitive(int numOfVerts): 
                m_model(glm::mat4(1.0f)),
                m_numOfVerts(numOfVerts)
                    { this->GenBuffer(); }

            virtual ~Primitive() = default;
    };


}

