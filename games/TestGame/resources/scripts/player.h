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

                if (inputs->m_up && this->canJump) {
                    this->canJump = false;
                    this->Jump(inputs);   
                }

                else if (this->sprite->IsContacting()) {
                    this->canJump = true;
                    this->Move(inputs);
                }

                else if (inputs->m_SPACE) 
                    this->Attack();

            }

            //-------------------------------------

            void Move(Inputs* inputs)
            {
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

                if (inputs->m_left) {       
                    this->sprite->SetImpulse(-20, -400); 
                    this->sprite->Animate("jump-left"); 
                    this->flipX = true;
                }

                else if (inputs->m_right) {
                    this->sprite->SetImpulse(20, -400); 
                    this->sprite->Animate("jump-right"); 
                    this->flipX = false;
                }

                else {
                    this->sprite->SetVelocityY(-3800);//SetImpulseY(-1800);
                    this->sprite->Animate(this->flipX ? "jump-left" : "jump-right");
                }
            }

            //---------------------------------------

            void Attack()
            {

            
            }

    };
}
