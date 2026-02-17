#include "../../../build/sdk/include/game.h"
#include "../../vendors/glm/glm.hpp"
#include "../../vendors/glm/gtc/matrix_transform.hpp"
#include "debug.h"

Camera::Camera():
    m_zoom(1.0f),
    m_rotation(0.0f),
    m_position({ 0.0f, 0.0f }),
    m_backgroundColor({ 0.5f, 0.5f, 0.5f, 1.0f }),
    m_target({ nullptr, { 0.0f, 0.0f } }),
    m_canFollow(false)
{
    currentBoundsWidthBegin = 0.0f;
    currentBoundsWidthEnd = 0.0f;
    currentBoundsHeightBegin = 0.0f;
    currentBoundsHeightEnd = 0.0f;

    LOG("Camera: initialized.");
}

//-------------------------------


void Camera::StartFollow(Math::Vector2* position, float offsetX, float offsetY) {
    m_target = { position, { offsetX, offsetY } };
    m_canFollow = true;
}

//-------------------------------


void Camera::Reset() 
{
    SetPosition({ 0.0f, 0.0f });
    SetZoom(1.0f);
    SetBackgroundColor({ 0.5f, 0.5f, 0.5f, 1.0f });
    SetBounds(0.0f, 0.0f, 0.0f, 0.0f);

    m_target = { nullptr, { 0.0f, 0.0f } };
    m_rotation = 0.0f;
}

//-------------------------------


void Camera::SetVignette(float alpha) { 

    if (!System::Game::GetScene()->vignette)
        return;

    System::Game::GetScene()->vignette->SetAlpha(alpha);
}



//-------------------------------


void Camera::Fade(float rate, const std::string& direction) 
{ 
    if (!System::Game::GetScene()->vignette)
        return;

    if (direction == "in")
        System::Game::GetScene()->vignette->alpha += rate;

    if (direction == "out")
        System::Game::GetScene()->vignette->alpha -= rate;
}

//--------------------------------


void Camera::SetBounds(float widthBegin, float widthEnd, float heightBegin, float heightEnd) 
{
    currentBoundsWidthBegin = widthBegin;
    currentBoundsWidthEnd = widthEnd;
    currentBoundsHeightBegin = heightBegin;
    currentBoundsHeightEnd = heightEnd;
}

 
//-------------------------------


const bool Camera::InBounds() 
{ 
    if (!m_target.first)
        return false;

    return m_target.first->x > currentBoundsWidthBegin &&
           m_target.first->x < currentBoundsWidthEnd && 
           m_target.first->y > currentBoundsHeightBegin &&
           m_target.first->y < currentBoundsHeightEnd;
}


//------------------------------


const Math::Vector4 Camera::GetProjectionMatrix(float width, float height)
{
    const float left = 1.0f / m_zoom, 
                right = width / m_zoom, 
                bottom = height / m_zoom,  
                top = 0.0f / m_zoom;
                    
    return { left, right, bottom, top };
}

//------------------------------


const Math::Matrix4 Camera::GetViewMatrix(float x, float y)
{    
    const glm::vec2 midOffset = { GetPosition()->x, GetPosition()->y };

    glm::mat4 view(1.0f);

    view = glm::translate(view, glm::vec3(midOffset, 0.0f)); 
    view = glm::rotate(view, glm::radians(m_rotation), { 0.0f, 0.0f, 1.0f }); 
    view = glm::translate(view, glm::vec3(-midOffset, 0.0f));

    view = glm::translate(view, glm::vec3(x, y, 0.0f));

    return { 
        { view[0][0], view[0][1], view[0][2], view[0][3] }, 
        { view[1][0], view[1][1], view[1][2], view[1][3] },   
        { view[2][0], view[2][1], view[2][2], view[2][3] },  
        { view[3][0], view[3][1], view[3][2], view[3][3] }
    };
}


//-------------------------------


void Camera::Update() {
    if (m_canFollow && InBounds()) {
        m_position.x = (-m_target.first->x + m_target.second.first); 
        m_position.y = (-m_target.first->y + m_target.second.second);  
    }
}




