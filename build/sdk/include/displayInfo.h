#if DEVELOPMENT == 1

#pragma once

#include "./entity.h"

class DisplayInfo {

    public: 

        //constructor, called on start

        inline DisplayInfo()
        {
            this->fps_text = std::make_shared<Text>("FPS", 10, 0);
            this->fps_text->SetScale(2.0f, 1.0f);

            this->cam_text = std::make_shared<Text>("CAMERA", 10, 50);
            this->cam_text->SetScale(2.0f, 1.0f);
        }

        //update every frame

        inline void Update(Process::Context& context) {

            //FPS counter

            float fps = 60.0f - context.time->m_delta;
            this->fps_text->SetText("FPS: " + std::to_string(fps));
            this->fps_text->Render();

            //camera stats
            
            const std::string camInfo = "CAMERA: x: " + std::to_string(-context.camera->position.x) + " y: " + std::to_string(-context.camera->position.y);
            this->cam_text->SetText(camInfo);
            this->cam_text->Render();
        }

    private:
        
        std::shared_ptr<Text> fps_text;
        std::shared_ptr<Text> cam_text;

};

#endif