#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

   class COLORSHIFTSPRITE : public Behavior {

       public:

           //constructor, called on start

           COLORSHIFTSPRITE(std::shared_ptr<Entity> entity):
               Behavior(entity, "COLORSHIFTSPRITE")
           {
                this->r = 1.0f; 
                this->g = 1.0f;
                this->b = 1.0f;
                this->rev = false;
                this->sprite = std::static_pointer_cast<Sprite>(this->entity);
                
                Time::delayedCall(100,[&]() { 
                    Time::setInterval(3000, [=]() { 

                        if (!this->isActive) 
                            return;

                        this->rev = !this->rev; 
                    }); 
                });
           }

           //update every frame

           void Update(Process::Context& context, const std::vector<std::shared_ptr<Behavior>>& behaviors) override {
                
                if (!this->rev) {
                    this->r -= 0.01f;
                    this->b += 0.01f;
                }
                
                else {
                    this->r += 0.01f;
                    this->b -= 0.01f; 
                }

                this->sprite->SetTint({r, g, b});
           }

        private:

            float r, g, b;
            bool rev;
            std::shared_ptr<Sprite> sprite;

   };
};