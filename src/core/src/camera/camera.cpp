#include "../../../../build/sdk/include/app.h"

Camera::Camera():
    rotation(0.0f),
    zoom(1.0f),
    m_screenLeft(1.0f),
    m_screenTop(0.0f),
    targetX(0.0f),
    targetY(0.0f),
    currentBoundsWidthBegin(0.0f),
    currentBoundsWidthEnd(0.0f),
    currentBoundsHeightBegin(0.0f),
    currentBoundsHeightEnd(0.0f),
    position(glm::vec2(0.0f, 0.0f)),
    backgroundColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f))
{
    #if DEVELOPMENT == 1
        std::cout << "Camera: initialized.\n";
    #endif
}

//--------------------------------


void Camera::SetBounds(float widthBegin, float widthEnd, float heightBegin, float heightEnd) 
{
 
    this->currentBoundsWidthBegin = widthBegin;
    this->currentBoundsWidthEnd = widthEnd;
    this->currentBoundsHeightBegin = heightBegin;
    this->currentBoundsHeightEnd = heightEnd;
}

 
//-------------------------------


bool Camera::InBounds() { 

    return this->targetX > this->currentBoundsWidthBegin &&
           this->targetX < this->currentBoundsWidthEnd && 
           this->targetY > this->currentBoundsHeightBegin &&
           this->targetY < this->currentBoundsHeightEnd;
}


//------------------------------


glm::highp_mat4 Camera::GetProjectionMatrix(float width, float height)
{
    
    System::Window::s_currentWidth = width;  
    System::Window::s_currentHeight = height;

    return (glm::highp_mat4)glm::ortho(
        this->m_screenLeft / this->zoom, width / this->zoom,  
        height / this->zoom, this->m_screenTop / this->zoom, 
        -1.0f, 1.0f 
    );
}





