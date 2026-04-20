#include "../../shared/renderer.h"
#include "../../vendors/glm/gtc/matrix_transform.hpp" 
#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/window.h"


//quad
Geometry::Geometry(float x, float y, float width, float height, bool isSpawn): 
    Entity(GEOMETRY, x, y, isSpawn),
        m_type(QUAD)
{ 
    this->width = width;
    this->height = height;

    SetDrawStyle(1);
    SetShader("sprite");

    tint = { 0.0f, 0.0f, 1.0f };
    texture = Graphics::Texture2D::Get("base");
    renderable = true;

    LOG("Geometry: quad created."); 

}


//-------------------------------------------


Geometry::~Geometry() { 
    if (m_type == QUAD) {
        LOG("Geometry: quad destroyed."); 
    }

    //...more shapes?
}


//------------------------------------- 


void Geometry::Render()
{
    if (m_type == QUAD) {
        texture.FrameWidth = width;
        texture.FrameHeight = height;
    }

    //render other shapes...

    glm::mat4 transform = glm::mat4(1.0f); 

    transform = glm::translate(transform, { 0.5f * width + position.x * scale.x, 0.5f * height + position.y * scale.y, 0.0f }); 
    transform = glm::rotate(transform, glm::radians(rotation), { 0.0f, 0.0f, 1.0f }); 
    transform = glm::translate(transform, { -0.5f * width - position.x * scale.x, -0.5f * height - position.y * scale.y, 0.0f });

    const auto camera = System::Application::game->camera;

    float scrollX = scrollFactor.x, 
          scrollY = scrollFactor.y;

    #if STANDALONE == 0
        scrollX = 1.0f; 
        scrollY = 1.0f;
    #endif

    const Math::Vector4& pm = System::Application::game->camera->GetProjectionMatrix(System::Window::s_scaleWidth, System::Window::s_scaleHeight);
    const Math::Matrix4& vm = camera->GetViewMatrix((camera->GetPosition()->x * scrollX * scale.x), (camera->GetPosition()->y * scrollY * scale.y));

    glm::highp_mat4 projMat = (glm::highp_mat4)glm::ortho(pm.r, pm.g, pm.b, pm.a, -1.0f, 1.0f); 

    if (m_isStatic) 
        projMat = (glm::highp_mat4)glm::ortho(0.0f, System::Window::s_scaleWidth, System::Window::s_scaleHeight, 0.0f, -1.0f, 1.0f); 
   
    glm::mat4 mvp = projMat * glm::mat4({ vm.a.r, vm.a.g, vm.a.b, vm.a.a }, 
                    { vm.b.r, vm.b.g, vm.b.b, vm.b.a }, 
                    { vm.c.r, vm.c.g, vm.c.b, vm.c.a }, 
                    { vm.d.r, vm.d.g, vm.d.b, vm.d.a }) * transform;

    if (m_isStatic)
        mvp = projMat * glm::mat4(1.0f);

    const Math::Matrix4 modelViewProj = { 
        { mvp[0][0], mvp[0][1], mvp[0][2], mvp[0][3] }, 
        { mvp[1][0], mvp[1][1], mvp[1][2], mvp[1][3] },   
        { mvp[2][0], mvp[2][1], mvp[2][2], mvp[2][3] },  
        { mvp[3][0], mvp[3][1], mvp[3][2], mvp[3][3] }
    };

    const Math::Vector4 color = { tint.x, tint.y, tint.z, alpha }; 

    texture.Update(
        shader, 
        position, 
        scale, 
        color, 
        outlineColor,
        modelViewProj, 
        outlineEnabled ? outlineWidth : 0.0f, 
        depth
    ); 

    const auto renderer = System::Renderer::Get();

    if (renderer)
        renderer->drawStyle = m_drawStyle;   
}


//-------------------------------------- 


void Geometry::SetSize(float width, float height) { 
    this->width = width; 
    this->height = height;
} 


//-------------------------------------- 


void Geometry::SetDrawStyle(int style) { 
    m_drawStyle = style;
} 

//----------------------------- quad


std::shared_ptr<Geometry> System::Game::CreateGeom(float x, float y, float width, float height, int layer, bool isStatic, bool isSpawn)
{
    const auto geom = std::make_shared<Geometry>(x, y, width, height, isSpawn);

    if (isStatic)
        geom->SetStatic(true);

    if (layer == 0)
        GetScene()->entities.emplace_back(geom);

    if (layer == 1 || layer == 2)
        GetScene()->UI.emplace_back(geom);

    geom->render_layer = layer;

    return geom;
}
