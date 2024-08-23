#if DEVELOPMENT == 1

#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/physics.h"
#include "../../../build/sdk/include/debug.h"

using namespace System;
using namespace Graphics;

#define BUFFER_OFFSET(x)  ((const void*) (x))


//---------------------- POINTS


void Points::Create()
{

	m_maxVertices = points;

	shader = Shader::Get("Points");

	m_vertexAttribute = 0;
    m_colorAttribute = 1; 
    m_sizeAttribute = 2;

    // Generate
    glGenVertexArrays(1, &m_vaoId);
    glGenBuffers(3, m_vboIds);

    glBindVertexArray(m_vaoId);
    glEnableVertexAttribArray(m_vertexAttribute);
    glEnableVertexAttribArray(m_colorAttribute);
    glEnableVertexAttribArray(m_sizeAttribute);

    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
    glVertexAttribPointer(m_vertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
    glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_colors), m_colors, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[2]);
    glVertexAttribPointer(m_sizeAttribute, 1, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0)); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_sizes), m_sizes, GL_DYNAMIC_DRAW);

    // Cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_count = 0;
} 


//------------------------------------



void Points::Flush()
{
    #ifndef __EMSCRIPTEN__

	if (m_count == 0)
        return;

	this->shader.SetMat4("projection", System::Application::game->camera->GetProjectionMatrix(Window::s_scaleWidth, Window::s_scaleHeight), true);

    glBindVertexArray(m_vaoId);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Vec2), m_vertices);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Color), m_colors);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[2]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(float), m_sizes);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArrays(GL_POINTS, 0, m_count);
    glDisable(GL_PROGRAM_POINT_SIZE);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    m_count = 0;

	#endif
}


//--------------------------- LINES


void Lines::Create()
{

	m_maxVertices = lines;
    shader = Shader::Get("Lines");

    m_vertexAttribute = 0;
    m_colorAttribute = 1;

    // Generate
    glGenVertexArrays(1, &m_vaoId);
    glGenBuffers(2, m_vboIds);

    glBindVertexArray(m_vaoId);
    glEnableVertexAttribArray(m_vertexAttribute);
    glEnableVertexAttribArray(m_colorAttribute);

    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
    glVertexAttribPointer(m_vertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
    glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_colors), m_colors, GL_DYNAMIC_DRAW);

    // Cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_count = 0;
}


//------------------------------------


void Lines::Flush()
{

	#ifndef __EMSCRIPTEN__

    if (m_count == 0)
        return; 

	this->shader.SetMat4("projection", System::Application::game->camera->GetProjectionMatrix(Window::s_scaleWidth, Window::s_scaleHeight), true);

    glBindVertexArray(m_vaoId);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Vec2), m_vertices);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Color), m_colors);

    glDrawArrays(GL_LINES, 0, m_count);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    m_count = 0;

	#endif
}


//-------------------------- TRIANGLES


void Triangles::Create()
{

	m_maxVertices = triangles;
   	shader = Shader::Get("Triangles");

    m_vertexAttribute = 0;
    m_colorAttribute = 1;

    // Generate
    glGenVertexArrays(1, &m_vaoId);
    glGenBuffers(2, m_vboIds);

    glBindVertexArray(m_vaoId);
    glEnableVertexAttribArray(m_vertexAttribute);
    glEnableVertexAttribArray(m_colorAttribute); 

    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
    glVertexAttribPointer(m_vertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
    glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_colors), m_colors, GL_DYNAMIC_DRAW);

    // Cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_count = 0;
} 


//------------------------------------


void Triangles::Flush()
{

	#ifndef __EMSCRIPTEN__

    if (m_count == 0)
        return;

	this->shader.SetMat4("projection", System::Application::game->camera->GetProjectionMatrix(Window::s_scaleWidth, Window::s_scaleHeight), true);

    glBindVertexArray(m_vaoId);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Vec2), m_vertices);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Color), m_colors);

    //glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES, 0, m_count);
    //glDisable(GL_BLEND);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    m_count = 0;

	#endif
}


//--------------------------------------



void DebugGraphic::Vertex(auto *shape, const b2Vec2& v, const b2Color& c, float size)
{ 

	shape->m_X = v.x;
	shape->m_Y = v.y;
 
    if (shape->m_count == shape->m_maxVertices)
        shape->Flush();

    shape->m_vertices[shape->m_count] = v;
    shape->m_colors[shape->m_count] = c;
    shape->m_sizes[shape->m_count] = size;

    ++shape->m_count;
}


//------------------------------------


void DebugGraphic::Vertex(auto *shape, const b2Vec2& v, const b2Color& c)
{

	shape->m_X = v.x;
	shape->m_Y = v.y;	

    if (shape->m_count == shape->m_maxVertices)
        shape->Flush();

    shape->m_vertices[shape->m_count] = v;
    shape->m_colors[shape->m_count] = c;
    
	++shape->m_count;
}


//------------------------------------


void DebugGraphic::Destroy(auto *shape)
{

    if (shape->m_vaoId)
    {
        
		glDeleteVertexArrays(1, &shape->m_vaoId);
        glDeleteBuffers(2, shape->m_vboIds);
        
		shape->m_vaoId = 0;
    }
}


//---------------------------------------


DebugDraw::DebugDraw():
	enable(false)
{

	m_points = new Points;
	m_points->Create();
	
	m_lines = new Lines;
	m_lines->Create();
	
	m_triangles = new Triangles;
	m_triangles->Create();

	LOG("Physics Debug: graphic debug primitives initialized.");
}


//------------------------------------


DebugDraw::~DebugDraw()
{

	DebugGraphic::Destroy(m_points);
	delete m_points;
	m_points = nullptr;

	DebugGraphic::Destroy(m_lines);
	delete m_lines;
	m_lines = nullptr;

	DebugGraphic::Destroy(m_triangles);
	delete m_triangles;
	m_triangles = nullptr;

	LOG("Physics Debug: graphic debug primitives destroyed.");

}


//------------------------------------


void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{

	b2Vec2 p1 = vertices[vertexCount - 1];

	for (int32 i = 0; i < vertexCount; ++i)
	{
		b2Vec2 p2 = vertices[i];
		
		DebugGraphic::Vertex(m_lines, p1, color);
		DebugGraphic::Vertex(m_lines, p2, color);
		
		p1 = p2;
	}
}


//------------------------------------


void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{

	b2Color fillColor(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);

	for (int32 i = 1; i < vertexCount - 1; ++i)
	{
		DebugGraphic::Vertex(m_triangles, vertices[0], fillColor);
		DebugGraphic::Vertex(m_triangles, vertices[i], fillColor);
		DebugGraphic::Vertex(m_triangles, vertices[i + 1], fillColor);
	}

	b2Vec2 p1 = vertices[vertexCount - 1];

	for (int32 i = 0; i < vertexCount; ++i)
	{
		b2Vec2 p2 = vertices[i];
		
		DebugGraphic::Vertex(m_lines, p1, color);
		DebugGraphic::Vertex(m_lines, p2, color);
		
		p1 = p2;
	}
}


//------------------------------------



void DebugDraw::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
{

	const float k_segments = 16.0f;
	const float k_increment = 2.0f * b2_pi / k_segments;

	float sinInc = sinf(k_increment);
	float cosInc = cosf(k_increment);

	b2Vec2 r1(1.0f, 0.0f);
	b2Vec2 v1 = center + radius * r1;

	for (int32 i = 0; i < k_segments; ++i)
	{
		// Perform rotation to avoid additional trigonometry.
		
		b2Vec2 r2;
		
		r2.x = cosInc * r1.x - sinInc * r1.y;
		r2.y = sinInc * r1.x + cosInc * r1.y;
		
		b2Vec2 v2 = center + radius * r2;
		
		DebugGraphic::Vertex(m_lines, v1, color);
		DebugGraphic::Vertex(m_lines, v2, color);
		
		r1 = r2;
		v1 = v2;
	}
}


//------------------------------------



void DebugDraw::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{

	const float k_segments = 16.0f;
	const float k_increment = 2.0f * b2_pi / k_segments;

	float sinInc = sinf(k_increment);
	float cosInc = cosf(k_increment);

	b2Vec2 v0 = center;
	b2Vec2 r1(cosInc, sinInc);
	b2Vec2 v1 = center + radius * r1;
	b2Color fillColor(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);

	for (int32 i = 0; i < k_segments; ++i)
	{
		// Perform rotation to avoid additional trigonometry.
		b2Vec2 r2;
		
		r2.x = cosInc * r1.x - sinInc * r1.y;
		r2.y = sinInc * r1.x + cosInc * r1.y;
		
		b2Vec2 v2 = center + radius * r2;
		
		DebugGraphic::Vertex(m_triangles, v0, fillColor);
		DebugGraphic::Vertex(m_triangles, v1, fillColor);
		DebugGraphic::Vertex(m_triangles, v2, fillColor);
		
		r1 = r2;
		v1 = v2;
	}

	r1.Set(1.0f, 0.0f);
	v1 = center + radius * r1;

	for (int32 i = 0; i < k_segments; ++i)
	{
		b2Vec2 r2;
		
		r2.x = cosInc * r1.x - sinInc * r1.y;
		r2.y = sinInc * r1.x + cosInc * r1.y;
		
		b2Vec2 v2 = center + radius * r2;
		
		DebugGraphic::Vertex(m_lines, v1, color);
		DebugGraphic::Vertex(m_lines, v2, color);
		
		r1 = r2;
		v1 = v2;
	}

	// Draw a line fixed in the circle to animate rotation.
	b2Vec2 p = center + radius * axis;

	DebugGraphic::Vertex(m_lines, center, color);
	DebugGraphic::Vertex(m_lines, p, color);
}


//------------------------------------


void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	DebugGraphic::Vertex(m_lines, p1, color);
	DebugGraphic::Vertex(m_lines, p2, color);
}


//------------------------------------


void DebugDraw::DrawTransform(const b2Transform& xf)
{
	const float k_axisScale = 0.4f;

	b2Color red(1.0f, 0.0f, 0.0f);
	b2Color green(0.0f, 1.0f, 0.0f);
	b2Vec2 p1 = xf.p, p2;

	DebugGraphic::Vertex(m_lines, p1, red);

	p2 = p1 + k_axisScale * xf.q.GetXAxis();

	DebugGraphic::Vertex(m_lines, p2, red);

	DebugGraphic::Vertex(m_lines, p1, green);

	p2 = p1 + k_axisScale * xf.q.GetYAxis();

	DebugGraphic::Vertex(m_lines, p2, green);
}


//------------------------------------


void DebugDraw::DrawPoint(const b2Vec2& p, float size, const b2Color& color)
{
	DebugGraphic::Vertex(m_points, p, color, size);
}


//------------------------------------


void DebugDraw::DrawAABB(b2AABB* aabb, const b2Color& c)
{

	b2Vec2 p1 = aabb->lowerBound;

	b2Vec2 p2 = b2Vec2(aabb->upperBound.x, aabb->lowerBound.y);

	b2Vec2 p3 = aabb->upperBound;

	b2Vec2 p4 = b2Vec2(aabb->lowerBound.x, aabb->upperBound.y);

	DebugGraphic::Vertex(m_lines, p1, c);
	DebugGraphic::Vertex(m_lines, p2, c);

	DebugGraphic::Vertex(m_lines, p2, c);
	DebugGraphic::Vertex(m_lines, p3, c);

	DebugGraphic::Vertex(m_lines, p3, c);
	DebugGraphic::Vertex(m_lines, p4, c);

	DebugGraphic::Vertex(m_lines, p4, c);
	DebugGraphic::Vertex(m_lines, p1, c);
}



//------------------------------------


void DebugDraw::Flush()
{
	m_triangles->Flush();
	m_lines->Flush();
	m_points->Flush();
}


#endif