#if DEVELOPMENT == 1

#pragma once

#include "./entity.h"

class DisplayInfo {

    public: 

        //constructor, called on start

        inline DisplayInfo()
        {
            fps_text = std::make_shared<Text>("FPS", 10, 0);
            fps_text->SetScale(2.0f, 1.0f);

            cam_text = std::make_shared<Text>("CAMERA", 10, 50);
            cam_text->SetScale(2.0f, 1.0f);
        }

        //update every frame

        inline void Update(Process::Context& context) {

            //FPS counter

            float fps = 60.0f - context.time->m_delta;
            fps_text->SetText("FPS: " + std::to_string(fps));
            fps_text->Render(System::Window::s_scaleWidth, System::Window::s_scaleHeight);

            //camera stats
            
            const std::string camInfo = "CAMERA: x: " + std::to_string(-context.camera->position.x) + " y: " + std::to_string(-context.camera->position.y);
            cam_text->SetText(camInfo);
            cam_text->Render(System::Window::s_scaleWidth, System::Window::s_scaleHeight);
        }

    private:
        
        std::shared_ptr<Text> fps_text;
        std::shared_ptr<Text> cam_text;

};

#endif