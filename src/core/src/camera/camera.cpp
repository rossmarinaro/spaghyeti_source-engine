#include "../../../../build/include/app.h"

Camera::Camera():
    rotation(0.0f),
    m_zoom(1.0f),
    m_screenLeft(1.0f),
    m_screenTop(0.0f),
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


void Camera::SetPosition(const glm::vec2 &position) { 

    if (this->m_position.x < System::Application::game->GetWorldDimensions().x &&
        this->m_position.y < System::Application::game->GetWorldDimensions().y)
            this->m_position = position; 
}
