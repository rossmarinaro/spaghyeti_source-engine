#include "../../../../build/include/app.h"

Camera::Camera():
    rotation(0.0f),
    m_zoom(1.0f),
    m_screenLeft(1.0f),
    m_screenTop(0.0f),
    targetX(0.0f),
    targetY(0.0f),
    currentBoundsWidthBegin(500.0f),
    currentBoundsWidthEnd(1500.0f),
    currentBoundsHeightBegin(0.0f),
    currentBoundsHeightEnd(600.0f),
    m_position(glm::vec2(0.0f, 0.0f)),
    m_backgroundColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f))
{
    std::cout << "Camera: initialized.\n";
}

//------------------------------


glm::highp_mat4 Camera::GetProjectionMatrix(float width, float height)
{
    
    System::Window::currentWidth = width;  
    System::Window::currentHeight = height;

    return (glm::highp_mat4)glm::ortho(
        m_screenLeft / m_zoom, 
        width / m_zoom,  
        height / m_zoom, 
        m_screenTop / m_zoom, 
        -1.0f, 
        1.0f 
    );
}

//------------------------------


glm::highp_mat4 Camera::GetViewMatrix(Camera* camera) {

    return glm::translate(glm::mat4(1.0f), glm::vec3(camera->m_position, 0.0f));
}


//-------------------------------


bool Camera::InBounds() { 

    return this->targetX > this->currentBoundsWidthBegin &&
           this->targetX < this->currentBoundsWidthEnd && 
           this->targetY > this->currentBoundsHeightBegin &&
           this->targetY < this->currentBoundsHeightEnd;
 }
