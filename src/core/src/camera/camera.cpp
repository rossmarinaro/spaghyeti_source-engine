#include "../app/app.h"
#include "./camera.h"


Camera::Camera():
    rotation(0.0f),
    m_zoom(1.0f),
    m_screenLeft(1.0f),
    m_screenTop(0.0f),
    m_position(glm::vec2(0.0f, 0.0f)),
    m_backgroundColor(glm::vec4(0.25f, 0.25f, 0.25f, 1.0f))
{
    std::cout << "Camera: initialized.\n";
}

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

