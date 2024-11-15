#include "../../../build/sdk/include/app.h"

Camera::Camera():
    m_zoom(1.0f),
    m_rotation(0.0f),
    m_position(glm::vec2(0.0f, 0.0f)),
    m_backgroundColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)),
    m_target({ nullptr, { 0.0f, 0.0f } }),
    m_canFollow(true)
{
    currentBoundsWidthBegin = 0.0f;
    currentBoundsWidthEnd = 0.0f;
    currentBoundsHeightBegin = 0.0f;
    currentBoundsHeightEnd = 0.0f;

    LOG("Camera: initialized.");
}

//-------------------------------


void Camera::SetVignette(float alpha) 
{ 

    if (!System::Game::GetScene()->vignette)
        return;

    System::Game::GetScene()->vignette->SetAlpha(alpha);
}



//-------------------------------


void Camera::Fade(float rate, const char* direction) 
{ 

    if (!System::Game::GetScene()->vignette)
        return;

    if (strcmp(direction, "in") == 0)
        System::Game::GetScene()->vignette->alpha += rate;

    if (strcmp(direction, "out") == 0)
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


bool Camera::InBounds() 
{ 

    if (!m_target.first)
        return false;

    return m_target.first->x > currentBoundsWidthBegin &&
           m_target.first->x < currentBoundsWidthEnd && 
           m_target.first->y > currentBoundsHeightBegin &&
           m_target.first->y < currentBoundsHeightEnd;
}


//------------------------------


glm::highp_mat4 Camera::GetProjectionMatrix(float width, float height)
{
    return (glm::highp_mat4)glm::ortho(
        1.0f / m_zoom, width / m_zoom,  
        height / m_zoom, 0.0f / m_zoom, 
        -1.0f, 1.0f 
    );
}


//-------------------------------


void Camera::Update() {

    if (m_canFollow && InBounds()) {
        m_position.x = (-m_target.first->x + m_target.second.first) / 2; 
        m_position.y = m_target.second.second != 0.0f ? (-m_target.first->y + m_target.second.second) / 2 : 0.0f;  
    }
}




