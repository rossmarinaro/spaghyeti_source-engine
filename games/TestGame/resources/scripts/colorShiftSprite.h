#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

   class COLORSHIFTSPRITE : public Behavior {

       public:

           //constructor, called on start

           COLORSHIFTSPRITE(std::shared_ptr<Entity> entity):
               Behavior(entity, "COLORSHIFTSPRITE"),
                    m_r(1.0f),
                    m_g(1.0f),
                    m_b(1.0f),
                    m_rev(false),
                    m_sprite(std::static_pointer_cast<Sprite>(entity))
           {         
                Time::setInterval(3000, [this] { this->m_rev = !this->m_rev; }); 
           }

           //update every frame

           void Update() override {
                
                if (!this->m_rev) {
                    this->m_r -= 0.01f;
                    this->m_b += 0.01f;
                }
                
                else {
                    this->m_r += 0.01f;
                    this->m_b -= 0.01f; 
                }

                this->m_sprite->SetTint({ this->m_r, this->m_g, this->m_b });
           }

        private:

            float m_r, m_g, m_b;
            bool m_rev;
            std::shared_ptr<Sprite> m_sprite;

   };
};