#pragma once

#include "../../build/include/behaviors.h"

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

            if (inputs->m_up && this->canJump) 
            {
                this->sprite->SetImpulseY(-10000);
                this->sprite->SetFrame(1);
                this->canJump = false;
            }
            
            else if (this->sprite->IsContacting())
            {    
            
                this->canJump = true;
        
                if (inputs->m_right) 
                {
                    this->speed_x = 1000;
                    this->sprite->SetFlipX(false);
                    this->sprite->Animate("run", true, 7);
                    this->sprite->SetVelocityX(3200);
                    //sprite->bodies[0].first->ApplyLinearImpulse(b2Vec2(this->speed_x, sprite->bodies[0].first->GetLinearVelocity().y), sprite->bodies[0].first->GetWorldCenter(), true);

                } 
                
                else if (inputs->m_left)
                {
                    this->speed_x = -1000;
                    this->sprite->SetFlipX(true);
                    this->sprite->Animate("run", true, 7);
                    this->sprite->SetVelocityX(-3200);
                    //sprite->bodies[0].first->ApplyLinearImpulse(b2Vec2(this->speed_x, sprite->bodies[0].first->GetLinearVelocity().y), sprite->bodies[0].first->GetWorldCenter(), true);

                }

                else {
                    this->sprite->SetFrame(1);
                    this->sprite->SetVelocityX(0);
                }
            }

        }

    private:

        bool canJump;
        float speed_x;

};