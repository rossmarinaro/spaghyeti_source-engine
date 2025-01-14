#pragma once

#include "./renderer.h"


//camera class
class Camera {

    public:

        float currentBoundsWidthBegin,
              currentBoundsWidthEnd,
              currentBoundsHeightBegin,
              currentBoundsHeightEnd;

        inline bool const IsFollow() { 
            return m_canFollow; 
        }

        inline float const GetZoom() { 
            return m_zoom; 
        }

        inline float* GetZoomPtr() { 
            return &m_zoom; 
        }

        inline float const GetRotation() { 
            return m_rotation; 
        }

        inline const glm::vec2& GetPosition() { 
            return m_position; 
        }

        inline const glm::vec4& GetBackgroundColor() { 
            return m_backgroundColor; 
        }

        inline void SetPosition(const glm::vec2& position) { m_position = position; }
        inline void SetBackgroundColor(const glm::vec4& color) { m_backgroundColor = color; }
        inline void SetZoom(float zoom) { m_zoom = zoom; }
        inline void SetRotation(float rotation) { m_rotation = rotation; }
        inline void Pan(float rateX, float rateY) { SetPosition(glm::vec2(m_position.x + rateX, m_position.y + rateY)); }
        inline void StopFollow() { m_canFollow = false; }
        
        void StartFollow(glm::vec2* position, float offsetX = 0.0f, float offsetY = 0.0f);
        void Reset();
        void Update(); 
        void SetVignette(float alpha);
        void Fade(float rate, const char* direction);
        void SetBounds(float widthBegin, float widthEnd, float heightBegin, float heightEnd); 
        bool InBounds(); 

        glm::highp_mat4 GetProjectionMatrix(float width, float height),
                        GetViewMatrix(float x, float y);

        Camera();
        ~Camera() = default;

    private:
    
        float m_zoom, m_rotation;

        bool m_canFollow;

        glm::vec2 m_position;
        glm::vec4 m_backgroundColor;

        std::pair<glm::vec2*, std::pair<float, float>> m_target;

};
