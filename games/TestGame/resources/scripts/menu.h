#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/behaviors.h"


namespace entity_behaviors {

   class MENU : public Behavior {

       public:

           //constructor, called on start

           MENU(std::shared_ptr<Entity> entity):
               Behavior(entity, "MENU")
           {
                this->hasStarted = false;
                this->player = std::static_pointer_cast<Sprite>(this->entity);
                this->r = 255 * 0.1f; 
                this->g = 0;
                this->b = 255 * 0.1f;
           }

           //update every frame

           void Update(Process::Context& context, const std::vector<std::shared_ptr<Behavior>>& behaviors) override {

                //cycle background color

                if (r > 0) {
                    r -= 0.01f;
                    b += 0.01f;
                }
                
                else if (b > 0) {
                    r += 0.01f;
                    b -= 0.01f;
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
            bool hasStarted;
            std::shared_ptr<Sprite> player;
   };
};