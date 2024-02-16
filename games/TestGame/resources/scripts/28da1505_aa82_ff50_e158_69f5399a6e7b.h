#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/entity.h"

using app = System::Application;
class Sprite_28da1505_aa82_ff50_e158_69f5399a6e7b : public Sprite {

    public:

        //constructor, called on start

        Sprite_28da1505_aa82_ff50_e158_69f5399a6e7b (const std::string &key, float x, float y):
            Sprite(key, glm::vec2(x, y))
        {

        }

        //update every frame

        void Update(Inputs* inputs) override {
            if (inputs->m_left)
                this->SetVelocityX(-20);
            if (inputs->m_right)
                this->SetVelocityX(20);
        }

};