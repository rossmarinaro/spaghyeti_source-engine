#pragma once

#include "./renderer.h"


//camera class
class Camera {

    public:

        float 
            rotation, 
            zoom,
            targetX,
            targetY,
            currentBoundsWidthBegin,
            currentBoundsWidthEnd,
            currentBoundsHeightBegin,
            currentBoundsHeightEnd;

        glm::vec2 position;
        glm::vec4 backgroundColor;

        inline void Reset() {
            
            this->SetPosition({ 0.0f, 0.0f });
            this->SetZoom(1.0f);
            this->SetBackgroundColor({ 0.5f, 0.5f, 0.5f, 1.0f });
            this->SetBounds(0.0f, 0.0f, 0.0f, 0.0f);
            
            this->targetX = 0.0f; 
            this->targetY = 0.0f;
            this->rotation = 0.0f;
        }

        inline const glm::vec4& GetBackgroundColor() { return this->backgroundColor; }
        inline void SetPosition(const glm::vec2& position) { this->position = position; }
        inline void SetBackgroundColor(const glm::vec4& color) { this->backgroundColor = color; }
        inline void SetZoom(float zoom) { this->zoom = zoom; }
        inline void Pan(float rateX, float rateY) { this->SetPosition(glm::vec2(this->position.x + rateX, this->position.y + rateY)); }
        inline float const GetZoom() { return this->zoom; }
        
        bool InBounds(); 
        void SetVignette(float alpha);
        void Fade(float rate, const char* direction);
        void SetBounds(float widthBegin, float widthEnd, float heightBegin, float heightEnd); 
        glm::highp_mat4 GetProjectionMatrix(float width, float height);

        Camera();
        ~Camera() = default;

};
