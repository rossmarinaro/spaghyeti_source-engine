#pragma once

#include "../../build/sdk/include/behaviors.h"

//player script

namespace entity_behaviors {

    class Waiter : public Behavior {

        public: 

            bool canMove;
        
            Waiter(std::shared_ptr<Entity> entity): 
                Behavior(entity, "Waiter") 
            {
                this->canMove = false;
                this->m_canJump = false;
                this->m_speed_x = 0.0f;

                this->m_player = std::static_pointer_cast<Sprite>(entity);
            };
            
            ~Waiter() = default;

            void Update(Process::Context context, void* scene) override
            {

                if (!this->canMove)
                    return;

                //jump

                if (context.inputs->UP && this->canJump && this->player->bodies[0].first->GetLinearVelocity().y == 0) 
                {
                    this->m_canJump = false;
                    this->m_player->SetImpulseY(-150);
                }

                //walk
                
                else if (this->m_player->IsContacting())
                {    
                
                    this->m_canJump = true;
            
                    if (context.inputs->RIGHT) 
                    {
                        this->m_player->SetFlipX(false);
                        this->m_player->Animate("run", true, 5);
                        this->m_player->SetVelocityX(320);
                    } 
                    
                    else if (context.inputs->LEFT)
                    {
                        this->m_player->SetFlipX(true);
                        this->m_player->Animate("run", true, 5);
                        this->m_player->SetVelocityX(-320);
                    }

                    else {
                        this->m_player->SetFrame(1);
                        this->m_player->SetVelocityX(0);
                    }

                }

                else //mid air
                    this->m_player->SetFrame(0);

            }

        private:

            float m_speed_x;
            bool m_canJump;

            std::shared_ptr<Sprite> m_player;

    };

}