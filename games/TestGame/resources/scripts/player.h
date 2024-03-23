#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/behaviors.h"

namespace entity_behaviors {
    
    class PlayerController : public Behavior {

        public:

            int health;

            //constructor, called on start

            PlayerController(std::shared_ptr<Entity> entity):
                Behavior(entity, "PlayerController")
            {
                this->health = 10;
                this->canJump = true;
                this->follow = true;  
                this->flipX = false; 
                this->canAttack = true;
                this->attacking = false;
                this->shootFireball = false;
            }

            //update every frame

            void Update(Process::Context context) override 
            { 

                if (this->follow)
                    this->sprite->StartFollow(context.camera, 500);

                //jump

                if (context.inputs->m_up && this->canJump) {               
                    this->canJump = false;
                    this->Jump(context.inputs);
                }

                //move   

                else if (this->sprite->IsContacting()) {         
                    this->canJump = true;
                    this->Move(context.inputs);
                }

                //attack

                if (context.inputs->m_SPACE && this->canAttack) {
                    this->canAttack = false;
                    this->attacking = true;
                }

                if (this->attacking) 
                    this->Attack(context.physics);
            }

        //-------------------------------------

        private:

            bool 
                canJump, 
                canAttack, 
                follow, 
                flipX,
                attacking,
                shootFireball;

        
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

                else if (this->canAttack) {
                    this->sprite->SetVelocityX(0); 
                    this->sprite->Animate(this->flipX ? "idle-left" : "idle-right", true);
                }
            }

            //---------------------------------------


            void Jump(Inputs* inputs)
            {

                if (inputs->m_left || inputs->m_right)
                    this->sprite->SetImpulse(this->flipX ? -9000 : 9000, -9000);

                else
                    this->sprite->SetImpulseY(-9000);  

                //this->sprite->Animate(this->flipX ? "jump-left" : "jump-right");
                this->sprite->SetFrame(this->flipX ? 14 : 12);
            }


            //---------------------------------------


            void Attack(Physics* physics)
            {

                auto hb = physics->CreateDynamicBody("box", 0, 0, 10, 10, true, 1); 

                if (this->flipX) {
                    this->sprite->Animate("attack-left", true, 4);
                    hb->SetTransform(b2Vec2(this->sprite->m_position.x - 10, this->sprite->m_position.y + 45), 0);
                }

                else {
                    this->sprite->Animate("attack-right", true, 4);
                    hb->SetTransform(b2Vec2(this->sprite->m_position.x + 90, this->sprite->m_position.y + 45), 0);
                }

                if (this->sprite->IsAnimComplete()) 
                {
                    this->canAttack = true; 
                    this->attacking = false;

                    physics->DestroyBody(hb); 
                } 

                if (this->shootFireball)
                {
                    //todo: implement fireball
                    Time::delayedCall(1000, [&]() { });
                }           
            }

    };
}

