#include "../../../build/sdk/include/app.h"

Camera::Camera()
{
    rotation = 0.0f;
    zoom = 1.0f;
    targetX = 0.0f;
    targetY = 0.0f;
    currentBoundsWidthBegin = 0.0f;
    currentBoundsWidthEnd = 0.0f;
    currentBoundsHeightBegin = 0.0f;
    currentBoundsHeightEnd = 0.0f;
    position = glm::vec2(0.0f, 0.0f);
    backgroundColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

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


bool Camera::InBounds() { 

    return targetX > currentBoundsWidthBegin &&
           targetX < currentBoundsWidthEnd && 
           targetY > currentBoundsHeightBegin &&
           targetY < currentBoundsHeightEnd;
}


//------------------------------


glm::highp_mat4 Camera::GetProjectionMatrix(float width, float height)
{
    return (glm::highp_mat4)glm::ortho(
        1.0f / zoom, width / zoom,  
        height / zoom, 0.0f / zoom, 
        -1.0f, 1.0f 
    );
}





