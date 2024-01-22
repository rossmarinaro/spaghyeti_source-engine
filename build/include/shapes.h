#pragma once

#include "./renderer.h"
#include "./primitive.h"
#include "./shader.h"
#include "./manager.h"

namespace Graphics {
    
    //Base Shape Rendering class extending primitive
    class Shape : public Primitive {

        public:

            Shader &m_graphicsShader;

            glm::vec3 m_strokeColor;
            glm::vec3 m_fillColor;

            const char* m_type;
            bool m_debug = false;

            float x, y, rotation;

            Shape(float x, float y, int numOfVerts, const char* type);
            virtual ~Shape() = default;

            virtual void Render(int drawStyle = 1) = 0;
            void Destroy();

            inline void SetStroke(const glm::vec3 &stroke) { m_strokeColor = stroke; }
            inline void SetFill(const glm::vec3 &fill) { m_fillColor = fill; } 
            inline void SetDebug(bool debug = true) { m_debug = debug; }

            inline void SetPosition(float x, float y) 
            { 
                this->x = x; 
                this->y = y;
            }

        private:
            
            Shape* m_self;

    };


    //--------------------------------------


    class Line : public Shape {

        public:

            short vertices[6];

            Line(float x, float y, const glm::vec2 &start, const glm::vec2 &end): 
                Shape(x, y, 6, "line")
                    { std::cout << "Primitive: line drawn.\n";  }

           ~Line() = default;

            void Render(int drawStyle = 1);

        private:

            glm::vec2 start, end;

    };


    //--------------------------------------


    class Triangle : public Shape { 

        public:

            short vertices[18];

            Triangle(float x, float y): 
                Shape(x, y, 18, "triangle")
                    { std::cout << "Primitive: triangle drawn.\n"; }

           ~Triangle()= default;

            void Render(int drawStyle = 1);

    };


    //--------------------------------------


    class Rectangle : public Shape {

        public:

            short vertices[12];
            
            float width, height;

            Rectangle(
                const char* type, 
                float x = 0.0f, 
                float y = 0.0f, 
                float width = 20.0f, 
                float height = 20.0f
            ): 
                Shape(x, y, 12, type),
                width(width),
                height(height)
                    { std::cout << "Primitive: rectangle drawn.\n"; }

            Rectangle(float x, float y, float width, float height): 
                Shape(x, y, 12, "rectangle"),
                width(width),
                height(height)
                    { std::cout << "Primitive: rectangle drawn.\n"; }


           ~Rectangle() = default;

           void Render(int drawStyle = 1);
           void Render(auto &sprite);

    };

}