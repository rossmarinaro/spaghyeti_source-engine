#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"

namespace entity_behaviors {
    
    class Actor : public Behavior {

        public:

            int health;

            std::shared_ptr<Sprite> sprite;
            b2Body* hb; 

            inline Actor(std::shared_ptr<Entity> entity, const std::string& key): 
                Behavior(entity, key),
                    m_canDamage(true),
                    m_canHit(false),
                    m_canDestroy(false) {}

            void Update(float offsetX = 0.0f, float offsetY = 0.0f);

            ~Actor() = default;

        protected:

            int m_damage;

            bool m_canDamage, 
                 m_canHit,
                 m_canDestroy;

    };
}