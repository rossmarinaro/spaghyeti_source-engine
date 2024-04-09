#pragma once

#include "./behaviors.h"


class DisplayInfo {

    public: 

        //constructor, called on start

        inline DisplayInfo ()
        {
            this->fps_text = std::static_pointer_cast<Text>(this->entity);
        }

        //update every frame

        void Update(Process::Context context) {

            float fps = 60.0f - context.time->m_delta;
            this->fps_text->SetText("FPS: " + std::to_string(fps));
        }

    private:
        
        std::shared_ptr<Text> fps_text;

};
