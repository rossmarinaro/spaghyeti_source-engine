#pragma once

#include "./math.h" 

//camera class
class Camera {

    public:

        float currentBoundsWidthBegin,
              currentBoundsWidthEnd,
              currentBoundsHeightBegin,
              currentBoundsHeightEnd;

        inline const bool IsFollow() { return m_canFollow; }

        inline float* GetZoom() { return &m_zoom; }
        inline float* GetRotation() { return &m_rotation; }
        inline Math::Vector2* GetPosition() { return &m_position; }
        inline Math::Vector4* GetBackgroundColor() { return &m_backgroundColor; }
        
        inline void SetPosition(const Math::Vector2& position) { m_position = position; }
        inline void SetBackgroundColor(const Math::Vector4& color) { m_backgroundColor = color; }
        inline void SetZoom(float zoom) { m_zoom = zoom; }
        inline void SetRotation(float rotation) { m_rotation = rotation; }
        inline void Pan(float rateX, float rateY) { SetPosition({ m_position.x + rateX, m_position.y + rateY } ); }
        inline void StopFollow() { m_canFollow = false; }
        
        void Reset();
        void Update(); 
        void StartFollow(Math::Vector2* position, float offsetX = 0.0f, float offsetY = 0.0f);
        void SetVignette(float alpha);
        void Fade(float rate, const std::string& direction);
        void SetBounds(float widthBegin, float widthEnd, float heightBegin, float heightEnd); 
        const bool InBounds(); 

        const Math::Vector4 GetProjectionMatrix(float width, float height);
        const Math::Matrix4 GetViewMatrix(float x, float y);

        Camera();
        ~Camera() = default;

    private:
    
        float m_zoom, m_rotation;

        bool m_canFollow;

        Math::Vector2 m_position;
        Math::Vector4 m_backgroundColor;

        std::pair<Math::Vector2*, std::pair<float, float>> m_target;

};
