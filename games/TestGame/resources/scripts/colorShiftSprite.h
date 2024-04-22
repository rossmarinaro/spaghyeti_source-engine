#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

   class COLORSHIFTSPRITE : public Behavior {

       public:

           //constructor, called on start

           COLORSHIFTSPRITE(std::shared_ptr<Entity> entity):
               Behavior(entity, "COLORSHIFTSPRITE")
           {
                this->m_r = 1.0f; 
                this->m_g = 1.0f;
                this->m_b = 1.0f;
                this->m_rev = false;
                this->m_sprite = std::static_pointer_cast<Sprite>(entity);
                
                Time::delayedCall(100,[&]() { 
                    Time::setInterval(3000, [=]() { 

                        if (!this->m_isActive) 
                            return;

                        this->m_rev = !this->m_rev; 
                    }); 
                });
           }

           //update every frame

           void Update(Process::Context& context, void* scene) override {
                
                if (!this->m_rev) {
                    this->m_r -= 0.01f;
                    this->m_b += 0.01f;
                }
                
                else {
                    this->m_r += 0.01f;
                    this->m_b -= 0.01f; 
                }

                this->m_sprite->SetTint({ m_r, m_g, m_b });
           }

        private:

            float m_r, m_g, m_b;
            bool m_rev;
            std::shared_ptr<Sprite> m_sprite;

   };
};