#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

   class Menu_ScrollText : public Behavior {

       public:

           //constructor, called on start

            Menu_ScrollText(std::shared_ptr<Entity> entity):
               Behavior(entity, typeid(Menu_ScrollText).name()),
                    m_text(std::static_pointer_cast<Text>(entity)),
                    m_exclamations("")
            {
                
                Time::setInterval(500, [this] { 
                
                    if (this->m_exclamations.length() < 3) 
                        this->m_exclamations += "!"; 
                }); 

           } 

           //update every frame 

           void Update() override {
                this->m_text->SetText("SWANKY VELVET" + this->m_exclamations);
           }

        private:

            std::shared_ptr<Text> m_text;
            std::string m_exclamations;

   };
};