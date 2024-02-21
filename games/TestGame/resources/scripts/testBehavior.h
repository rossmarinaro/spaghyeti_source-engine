#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/behaviors.h"


class TestBehavior : public Behavior {

    public:

        int health;
        bool canJump;
        bool follow;

        //constructor, called on start

        TestBehavior(Entity* entity):
            Behavior(entity)
        {
            this->health = 10;
            this->canJump = true;
            this->m_shader = Shader::GetShader("player");
            this->follow = true; 
            System::Application::game->time->delayedCall(3000, [&]() { this->follow = false; });
        }

        //update every frame

        void Update(Inputs* inputs, Camera* camera) override 
        { 

            if (inputs->m_left) {       
                this->sprite->SetVelocityX(-1200);
                this->sprite->SetFlipX(false); 
            }

            else if (inputs->m_right) {
                this->sprite->SetVelocityX(1200);
                this->sprite->SetFlipX(true);
            }

            else 
                this->sprite->SetVelocityX(0);

            if (inputs->m_down && this->canJump) {
                this->sprite->SetImpulseY(-2000);
                this->canJump = false;
            }

            if (this->sprite->IsContacting())
                this->canJump = true; 

            //if (this->follow)
                this->sprite->StartFollow(camera, 500);

        }

};