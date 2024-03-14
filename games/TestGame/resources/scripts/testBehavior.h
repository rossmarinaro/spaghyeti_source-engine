#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/behaviors.h"


class TestBehavior_Behavior : public Behavior {

    public:

        int health;
        bool canJump;
        bool follow;

        //constructor, called on start

        TestBehavior_Behavior(std::shared_ptr<Entity> entity):
            Behavior(entity, "TestBehavior_Behavior")
        {
            this->health = 10;
            this->canJump = true;
            this->follow = true;  
        }

        //update every frame

        void Update(Inputs* inputs, Camera* camera) override 
        { 

            if (this->follow)
                this->sprite->StartFollow(camera, 500);

            if (inputs->m_up && this->canJump) {
                this->canJump = false;
                //this->sprite->bodies[0].first->ApplyForceToCenter(b2Vec2(this->sprite->m_flipX ? 20 : -20, -400), true);
                this->sprite->SetImpulseY(-400);
            }

            else if (this->sprite->IsContacting())
            {
                this->canJump = true;

                if (inputs->m_left) {       
                    this->sprite->SetVelocityX(-3800);
                    this->sprite->SetFlipX(false); 
                }

                else if (inputs->m_right) {
                    this->sprite->SetVelocityX(3800); 
                    this->sprite->SetFlipX(true);
                }

                else 
                    this->sprite->SetVelocityX(0);
            }

        }

};