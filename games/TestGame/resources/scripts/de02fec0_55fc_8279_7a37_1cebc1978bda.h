#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/game.h"


class Sprite_de02fec0_55fc_8279_7a37_1cebc1978bda : public Sprite {

    public:

        //constructor, called on start

        Sprite_de02fec0_55fc_8279_7a37_1cebc1978bda (const std::string &key, float x, float y):
            Sprite(key, glm::vec2(x, y))
        {

        }

        //update every frame

        void Update(Inputs* inputs, Camera* camera) override 
        { 
            this->velocityY = 2000;

            if (inputs->m_left)
                this->velocityX = -1000; 

            else if (inputs->m_right)
                this->velocityX = 1000;

            else this->velocityX = 0;

            this->SetVelocity(this->velocityX, this->velocityY);

            camera->SetPosition(glm::vec2(camera->m_position.x - 1, camera->m_position.y/* this->m_position */)); 
         
        }

};