#if DEVELOPMENT == 1
#pragma once

#include "./entity.h"


class DebugGraphic {

    public:

        virtual void Create() = 0;
        virtual void Flush() = 0;

        static void Destroy(auto *shape);

        static void Vertex(auto *shape, const b2Vec2& v, const b2Color& c, float size);
        static void Vertex(auto *shape, const b2Vec2& v, const b2Color& c);

        DebugGraphic() = default;
        virtual ~DebugGraphic() = default;

    protected:

        enum { 
            points = 512,
            lines = 2 * 512,
            triangles = 3 * 512
        };

        float m_X, m_Y;

        int32 m_count, m_maxVertices;

        GLuint m_vaoId;

        GLint m_vertexAttribute;
        GLint m_colorAttribute;
        GLint m_sizeAttribute;

        Shader shader;

};

//-------------------------------------


class Points : public DebugGraphic
{

    public:

        b2Vec2 m_vertices[points];
        b2Color m_colors[points];

        float m_sizes[points];

        GLuint m_vboIds[3];

        void Create();
        void Flush();
};

//-------------------------------------


class Lines : public DebugGraphic
{

    public:

        b2Vec2 m_vertices[lines];
        b2Color m_colors[lines];

        GLuint m_vboIds[2];
    
        void Create();
        void Flush();

        Lines() = default;
        ~Lines() = default;

};


//-------------------------------------


class Triangles : public DebugGraphic
{

    public:

        b2Vec2 m_vertices[triangles];
        b2Color m_colors[triangles]; 

        GLuint m_vboIds[2];

        void Create();
        void Flush();

        Triangles() = default;
        ~Triangles() = default;
};


//-------------------------------------


// This class implements debug drawing callbacks that are invoked inside b2World::Step
class DebugDraw : public b2Draw
{
    public:
        
        static inline Points* m_points = nullptr;
        static inline Lines* m_lines = nullptr;
        static inline Triangles* m_triangles = nullptr;

        void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
        void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
        void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override;
        void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override;
        void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;
        void DrawTransform(const b2Transform& xf) override;
        void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override;
        void DrawAABB(b2AABB* aabb, const b2Color& color);

        void Flush();

        DebugDraw();
       ~DebugDraw();

};  

#endif