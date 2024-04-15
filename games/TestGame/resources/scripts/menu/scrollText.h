#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

   class Menu_ScrollText : public Behavior {

       public:

           //constructor, called on start

            Menu_ScrollText(std::shared_ptr<Entity> entity):
               Behavior(entity, "ScrollText")
            {
                this->text = std::static_pointer_cast<Text>(this->entity);
                this->exclamations = "";

                Time::delayedCall(500,[&]() { 
                    Time::setInterval(500, [=]() { 
                    
                        if (!this->isActive) 
                            return;
                            
                        if (this->exclamations.length() < 3) 
                            this->exclamations += "!"; 
                    }); 
                });
           }

           //update every frame 

           void Update(Process::Context& context, const std::vector<std::shared_ptr<Behavior>>& behaviors) override {
                this->text->SetText("SWANKY VELVET" + this->exclamations);
           }

        private:

            std::shared_ptr<Text> text;
            std::string exclamations;

   };
};