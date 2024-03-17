#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/behaviors.h"

namespace entity_behaviors {
    
    class PlayerController_Behavior : public Behavior {

        public:

            int health;
            bool canJump;
            bool follow, flipX;

            //constructor, called on start

            PlayerController_Behavior(std::shared_ptr<Entity> entity):
                Behavior(entity, "PlayerController")
            {
                this->health = 10;
                this->canJump = true;
                this->follow = true;  
                this->flipX = false;
            }

            //update every frame

            void Update(Inputs* inputs, Camera* camera) override 
            { 

                if (this->follow)
                    this->sprite->StartFollow(camera, 500);

                if (inputs->m_up && this->canJump) 
                    {                this->canJump = true;this->Jump(inputs);}   

                else if (this->sprite->IsContacting()) 
                    {                this->canJump = true;this->Move(inputs);}

                if (inputs->m_SPACE) 
                    this->Attack();

            }

            //-------------------------------------

            void Move(Inputs* inputs)
            {


                this->sprite->SetTint({1.0f, 0.0f, 0.0f});

                if (inputs->m_left) {       
                    this->sprite->SetVelocityX(-3800); 
                    this->sprite->Animate("walk-left", false, 5); 
                    this->flipX = true;
                }

                else if (inputs->m_right) {
                    this->sprite->SetVelocityX(3800); 
                    this->sprite->Animate("walk-right", false, 5); 
                    this->flipX = false;
                }

                else {
                    this->sprite->SetVelocityX(0); 
                    this->sprite->Animate(this->flipX ? "idle-left" : "idle-right", true);
                }
            }

            //---------------------------------------

            void Jump(Inputs* inputs)
            {
                //this->sprite->SetImpulseY(-400);

                this->sprite->ClearTint();

                if (inputs->m_left) {       
                    this->sprite->SetImpulse(-20, -400); 
                    this->sprite->SetFrame(14);
                }

                else if (inputs->m_right) {
                    this->sprite->SetImpulse(20, -400); 
                    this->sprite->SetFrame(12); 
                }

                else {
                    this->sprite->SetImpulseY(-7800);
                    //this->sprite->Animate(this->flipX ? "jump-left" : "jump-right");
                    this->sprite->SetFrame(this->flipX ? 14 : 12);
                }
            }

            //---------------------------------------

            void Attack()
            {

            
            }

    };
}
