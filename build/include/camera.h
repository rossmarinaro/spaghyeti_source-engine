#pragma once

#include "./renderer.h"


//camera class
class Camera {

    public:

        float 
            rotation, 
            m_zoom,
            targetX,
            targetY,
            currentBoundsWidthBegin,
            currentBoundsWidthEnd,
            currentBoundsHeightBegin,
            currentBoundsHeightEnd;

        glm::vec2 m_position;
        glm::vec4 m_backgroundColor;

        inline void Reset() {
            
            this->SetPosition(glm::vec2(0.0f, 0.0f));
            this->SetZoom(1.0f);
            this->SetBackgroundColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    
            this->rotation = 0.0f;
        }

        inline void SetPosition(const glm::vec2 &position) { this->m_position = position; }
        inline void SetBackgroundColor(const glm::vec4 &color) { this->m_backgroundColor = color; }
        inline const glm::vec4 &GetBackgroundColor() { return this->m_backgroundColor; }
        inline void SetZoom(float zoom) { this->m_zoom = zoom; }
        inline float const GetZoom() { return this->m_zoom; }
        
        bool InBounds();
        glm::highp_mat4 GetViewMatrix(Camera* camera);
        glm::highp_mat4 GetProjectionMatrix(float width, float height);

        Camera();
        ~Camera() = default;

    private:
    
        float m_screenLeft, m_screenTop;
};
