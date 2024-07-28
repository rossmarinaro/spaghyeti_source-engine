#include "../../../build/sdk/include/app.h"

Camera::Camera():
    m_screenLeft(1.0f),
    m_screenTop(0.0f)

{
    this->rotation = 0.0f;
    this->zoom = 1.0f;
    this->targetX = 0.0f;
    this->targetY = 0.0f;
    this->currentBoundsWidthBegin = 0.0f;
    this->currentBoundsWidthEnd = 0.0f;
    this->currentBoundsHeightBegin = 0.0f;
    this->currentBoundsHeightEnd = 0.0f;
    this->position = glm::vec2(0.0f, 0.0f);
    this->backgroundColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

    #if DEVELOPMENT == 1
        std::cout << "Camera: initialized.\n";
    #endif

}

//-------------------------------


void Camera::SetVignette(float alpha) 
{ 

    if (!System::Application::game->currentScene->vignette)
        return;

    System::Application::game->currentScene->vignette->SetAlpha(alpha);
}



//-------------------------------


void Camera::Fade(float rate, const char* direction) 
{ 

    if (!System::Application::game->currentScene->vignette)
        return;

    if (strcmp(direction, "in") == 0)
        System::Application::game->currentScene->vignette->alpha += rate;

    if (strcmp(direction, "out") == 0)
        System::Application::game->currentScene->vignette->alpha -= rate;
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
    return (glm::highp_mat4)glm::ortho(
        this->m_screenLeft / this->zoom, width / this->zoom,  
        height / this->zoom, this->m_screenTop / this->zoom, 
        -1.0f, 1.0f 
    );
}





