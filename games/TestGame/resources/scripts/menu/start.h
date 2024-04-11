#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

   class Menu_Start : public Behavior {

       public:

           //constructor, called on start

           Menu_Start(std::shared_ptr<Entity> entity):
               Behavior(entity, "Menu_Start")
           {
                this->startText = std::static_pointer_cast<Text>(this->entity);

                Time::delayedCall(3000, [&]() {
                    Time::setInterval(500, [&]() { 
                        this->startText->SetAlpha(this->startText->m_alpha == 1 ? 0.0f : 1.0f);
                    }, this->m_lock);
                });
           }

        private:

            std::shared_ptr<Text> startText;
            static inline std::mutex m_lock;
   };
};