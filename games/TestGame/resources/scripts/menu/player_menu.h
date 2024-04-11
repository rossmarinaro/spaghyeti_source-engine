#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

   class Menu_Player : public Behavior {

       public:

           //constructor, called on start

           Menu_Player(std::shared_ptr<Entity> entity):
               Behavior(entity, "Menu_Player")
           {
                this->hasStarted = false;
                this->rev = false;
                this->player = std::static_pointer_cast<Sprite>(this->entity);
                this->r = 1.0f; 
                this->g = 0;
                this->b = 1.0f;
           }

           //update every frame

           void Update(Process::Context& context, const std::vector<std::shared_ptr<Behavior>>& behaviors) override 
           {

                //cycle background color

                if (this->r == 0)
                    this->rev = false;

                if (this->r > 0 && !this->rev) {
                    this->r -= 0.01f;
                    this->b += 0.01f;
                }
                
                else if (this->r < 1) {
                    this->rev = true;
                    this->r += 0.01f;
                    this->b -= 0.01f;
                }

                context.camera->SetBackgroundColor({ r, g, b, 1 });

                //set player animation

                this->player->SetAnimation("idle", true, 4);

                if (context.inputs->m_SPACE && !this->hasStarted) {
                    this->hasStarted = true;
                    Time::delayedCall(1000, [&]() { System::Game::StartScene("CAVE"); });
                }
           }

        private:

            float r, g, b;
            bool hasStarted, rev;
            std::shared_ptr<Sprite> player; 
   };
};