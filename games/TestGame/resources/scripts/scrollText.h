#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/behaviors.h"


namespace entity_behaviors {

   class SCROLLTEXT : public Behavior {

       public:

           //constructor, called on start

           SCROLLTEXT(std::shared_ptr<Entity> entity):
               Behavior(entity, "SCROLLTEXT")
           {
                this->text = std::static_pointer_cast<Text>(this->entity);
                this->exclamations = "";

                Time::setInterval(500, [&]() { 
                    if (this->exclamations.length() < 3) 
                        this->exclamations += "!"; 
                    else 
                        Time::exitFlag = true;
                }, this->m_lock);
           }

           //update every frame

           void Update(Process::Context context, const std::vector<std::shared_ptr<Behavior>>& behaviors) override {
                this->text->SetText("SWANKY VELVET" + this->exclamations);
           }

        private:

            std::shared_ptr<Text> text;
            std::string exclamations;
            static inline std::mutex m_lock;

   };
};