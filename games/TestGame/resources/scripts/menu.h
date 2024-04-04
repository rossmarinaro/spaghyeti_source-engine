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
           }

           //update every frame

           void Update(Process::Context context, const std::vector<std::shared_ptr<Behavior>>& behaviors) override {

                if (context.inputs->m_SPACE && !this->hasStarted) {
                    this->hasStarted = true;
                    Time::delayedCall(1000, [&]() { System::Game::StartScene("SCENE2"); });
                }
           }

        private:

            bool hasStarted;
   };
};