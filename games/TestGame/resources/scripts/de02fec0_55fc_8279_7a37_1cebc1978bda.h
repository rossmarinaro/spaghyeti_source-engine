#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/entity.h"


class Sprite_de02fec0_55fc_8279_7a37_1cebc1978bda : public Sprite {

    public:

        int health;
        bool canJump;
        bool follow;

        //constructor, called on start

        Sprite_de02fec0_55fc_8279_7a37_1cebc1978bda (const std::string &key, float x, float y):
            Sprite(key, x, y)
        {
            this->health = 10;
            this->canJump = true;
            this->m_shader = Shader::GetShader("player");
            this->follow = true; 
            System::Application::game->time->delayedCall(3000, [&]() {this->follow = false;});
        }

        //update every frame

        void Update(Inputs* inputs, Camera* camera) override 
        { 

            if (inputs->m_left) {       
                this->SetVelocityX(-1200);
                this->SetFlipX(false); 
            }

            else if (inputs->m_right) {
                this->SetVelocityX(1200);
                this->SetFlipX(true);
            }

            else 
                this->SetVelocityX(0);

            if (inputs->m_down && this->canJump) {
                this->SetImpulseY(-2000);
                this->canJump = false;
            }

            if (this->IsContacting())
                {this->canJump = true; /* this->SetAlpha(0.5f); */ }
//else this->SetAlpha(1.0f);
            //if (this->follow)
                this->StartFollow(camera, 500);

        }

};