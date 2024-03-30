#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/behaviors.h"

namespace entity_behaviors {
    
    class PlayerController : public Behavior {

        public:

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

                this->player = std::static_pointer_cast<Sprite>(this->entity);
                this->hb = Physics::CreateDynamicBody("box", 0, 0, 10, 10, true, 1); 
  
            }

            //update every frame

            void Update(Process::Context context, const std::vector<std::shared_ptr<Behavior>>& behaviors) override 
            { 

                if (this->follow)
                    this->player->StartFollow(context.camera, 500);

                //jump

                if (context.inputs->m_up && this->canJump) {               
                    this->canJump = false;
                    this->Jump(context.inputs);
                }

                //move   

                else if (this->player->IsContacting()) {      
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

                else {
                    this->hb->SetTransform(b2Vec2(this->player->m_position.x + 20, this->player->m_position.y + 20), 0);
                    this->hb->SetEnabled(false);  
                }

            }

            ~PlayerController() {
                Physics::DestroyBody(this->hb);
            }


        protected:

            int health;

            std::shared_ptr<Sprite> player;

        private:

            bool 
                canJump, 
                canAttack, 
                follow, 
                flipX,
                attacking,
                shootFireball;

            b2Body* hb;

            //--------------------------------------- move
        
            void Move(Inputs* inputs)
            {

                if (this->attacking)
                    return;

                if (inputs->m_left) {       
                    this->player->SetVelocityX(-300); 
                    this->player->Animate("walk-left", false, 5); 
                    this->flipX = true;
                }

                else if (inputs->m_right) {
                    this->player->SetVelocityX(300); 
                    this->player->Animate("walk-right", false, 5); 
                    this->flipX = false;
                }

                else {
                    this->player->SetVelocityX(0); 
                    this->player->Animate(this->flipX ? "idle-left" : "idle-right", true);
                }
            }

            //--------------------------------------- jump


            void Jump(Inputs* inputs)
            {

                if (inputs->m_left || inputs->m_right)
                    this->player->SetImpulse(this->flipX ? -900 : 900, -1600);

                else
                    this->player->SetImpulseY(-1500);  

                //this->player->Animate(this->flipX ? "jump-left" : "jump-right");
                this->player->SetFrame(this->flipX ? 14 : 12);
            }


            //--------------------------------------- attack


            void Attack(Physics* physics)
            {

                this->hb->SetEnabled(true);
              
                if (this->flipX) {
                    this->player->Animate("attack-left", true, 4);
                    this->hb->SetTransform(b2Vec2(this->player->m_position.x - 10, this->player->m_position.y + 45), 0);
                }

                else {
                    this->player->Animate("attack-right", true, 4);
                    this->hb->SetTransform(b2Vec2(this->player->m_position.x + 90, this->player->m_position.y + 45), 0);
                }

                if (this->player->IsAnimComplete()) 
                {
                    this->canAttack = true; 
                    this->attacking = false;
                } 

                if (this->shootFireball)
                {
                    //todo: implement fireball
                    Time::delayedCall(1000, [&]() { });
                }           
            }

    };
}

