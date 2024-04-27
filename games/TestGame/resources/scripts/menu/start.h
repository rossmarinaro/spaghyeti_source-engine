#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

   class Menu_Start : public Behavior {

       public:

           //constructor, called on start

           Menu_Start(std::shared_ptr<Entity> entity):
               Behavior(entity, "MenuStart"),
                    m_hasStarted(false),
                    m_startText(std::static_pointer_cast<Text>(entity))
           {

               Time::delayedCall(3000, [this] { 
                    Time::setInterval(500, [this] { this->m_startText->SetAlpha(this->m_startText->alpha == 1 ? 0.0f : 1.0f); });
                });  
           }

            inline void Update(Process::Context& context, void* scene) override
            {
                
                if (!this->m_hasStarted && this->m_startText->content == "GAME OVER" && context.inputs->SPACE) 
                {
                    this->m_hasStarted = true;
                    Time::delayedCall(500, [&]() { System::Game::StartScene("MENU"); });
                }
            }

        private:
            bool m_hasStarted;
            std::shared_ptr<Text> m_startText;
   };
};