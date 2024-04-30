#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

   class Menu_Player : public Behavior {

       public:

           //constructor, called on start

           Menu_Player(std::shared_ptr<Entity> entity):
               Behavior(entity, "Menu_Player"),
                    m_hasStarted(false),
                    m_rev(false),
                    m_player(std::static_pointer_cast<Sprite>(entity)),
                    m_r(1.0f),
                    m_g(0.0f),
                    m_b(1.0f),
                    m_a(1.0f)
           {

                Time::setInterval(1500, [this] { this->m_rev = !this->m_rev; });
           }

           //update every frame

           void Update(Process::Context& context, void* scene) override
           {
      
                //cycle background color

                if (!this->m_rev) {
                    this->m_r -= 0.01f;
                    this->m_b += 0.01f;
                }

                else {
                    this->m_r += 0.01f;
                    this->m_b -= 0.01f;
                }

                context.camera->SetBackgroundColor({ this->m_r, this->m_g, this->m_b, this->m_a });  
                context.camera->Fade(0.01f, "out");

                //set player animation

                this->m_player->SetAnimation("idle", true, 4);

                if (context.inputs->SPACE && !this->m_hasStarted) {
                    this->m_hasStarted = true;
                    System::Game::StartScene("CAVE");
                }
           }

        private:

            float m_r, m_g, m_b, m_a;
            bool m_hasStarted, m_rev;
            std::shared_ptr<Sprite> m_player;
   };
};