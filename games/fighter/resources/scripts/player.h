#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

   class PLAYER : public Behavior {

       public:

            std::shared_ptr<Sprite> player;

           //constructor, called on start

           PLAYER(std::shared_ptr<Entity> entity):
            Behavior(entity, typeid(PLAYER).name())
            {
                this->player = std::static_pointer_cast<Sprite>(entity); 

            }

           //update every frame

            void Update() override {

                auto context = System::Game::GetScene()->GetContext();

                if (this->player->name == "player2")
                    this->player->Animate("idle", true, 15); 

                else 
                {
                    this->player->StartFollow(context.camera, 500);

                    if (context.inputs->SPACE /* && this->m_canAttack */) 
                    {
                        this->player->Animate("high punch", false, 5); 
                    }

                    else if (context.inputs->LEFT) {       

                        this->player->Animate("walk back", false, 5); 
                        this->player->position.x -= 5;
                    }

                    else if (context.inputs->RIGHT) {
 
                        this->player->Animate("walk forward", false, 5); 
                        this->player->position.x += 5;
                    }

                    else {

                        this->player->Animate("idle", true, 12);
              
                    }
                }


                for (auto& spr : System::Game::GetScene()->entities) 
                {
                    if (spr->name == "bg2")
                        std::static_pointer_cast<Sprite>(spr)->SetScrollFactor({ -0.9f, 1.0f });

                }
           }

   };
};