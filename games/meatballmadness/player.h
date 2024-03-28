#pragma once

#include "../../build/include/behaviors.h"

//player script

namespace entity_behaviors {

    class Waiter : public Behavior {

        public: 

            bool canMove;
        
            Waiter(std::shared_ptr<Entity> entity): 
                Behavior(entity, "Waiter") 
            {
                this->canMove = false;
                this->canJump = false;
                this->speed_x = 0.0f;

                this->player = std::static_pointer_cast<Sprite>(this->entity);
            };
            
            ~Waiter() = default;

            void Update(Process::Context context, const std::vector<std::shared_ptr<Behavior>>& behaviors) override
            {

                if (!this->canMove)
                    return;

                //jump

                if (context.inputs->m_up && this->canJump && this->player->bodies[0].first->GetLinearVelocity().y == 0) 
                {
                    this->canJump = false;
                    this->player->SetImpulseY(-150);
                }

                //walk
                
                else if (this->player->IsContacting())
                {    
                
                    this->canJump = true;
            
                    if (context.inputs->m_right) 
                    {
                        this->player->SetFlipX(false);
                        this->player->Animate("run", true, 5);
                        this->player->SetVelocityX(320);
                    } 
                    
                    else if (context.inputs->m_left)
                    {
                        this->player->SetFlipX(true);
                        this->player->Animate("run", true, 5);
                        this->player->SetVelocityX(-320);
                    }

                    else {
                        this->player->SetFrame(1);
                        this->player->SetVelocityX(0);
                    }

                }

                else //mid air
                    this->player->SetFrame(0);

            }

        private:

            float speed_x;
            bool canJump;

            std::shared_ptr<Sprite> player;

    };

}