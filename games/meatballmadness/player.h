#pragma once

#include "../../build/include/behaviors.h"

//player script

class Waiter : public Behavior {

    public: 
    
        bool canMove;

        Waiter(std::shared_ptr<Entity> entity): 
            Behavior(entity, "Waiter") 
        {
            this->canMove = false;
            this->canJump = false;
            this->speed_x = 0.0f;
        };
        
        ~Waiter() = default;

        void Update(Inputs* inputs, Camera* camera) override
        {

            if (!this->canMove)
                return;

            if (inputs->m_up && this->canJump && this->sprite->bodies[0].first->GetLinearVelocity().y == 0) 
            {
                this->sprite->SetImpulseY(-10000);
                this->sprite->SetFrame(0);
                this->canJump = false;
            }
            
            else if (this->sprite->IsContacting())
            {    
            
                this->canJump = true;
        
                if (inputs->m_right) 
                {
                    this->sprite->SetFlipX(false);
                    this->sprite->Animate("run", true, 7);
                    this->sprite->SetVelocityX(3200);
                } 
                
                else if (inputs->m_left)
                {
                    this->sprite->SetFlipX(true);
                    this->sprite->Animate("run", true, 7);
                    this->sprite->SetVelocityX(-3200);
                }

                else {
                    this->sprite->SetFrame(1);
                    this->sprite->SetVelocityX(0);
                }

            }

        }

    private:

        bool canJump;

};