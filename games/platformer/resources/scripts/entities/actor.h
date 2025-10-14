#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {
    
    class Actor : public Behavior {

        public:

            int health;

            Actor(std::shared_ptr<Entity> entity):
                Behavior(entity->ID, typeid(Actor).name(), "Actor") {
                    m_Init();
                }

            Actor(std::shared_ptr<Entity> entity, const std::string& key, const std::string& name):
                Behavior(entity->ID, key, name) {
                    m_Init();
                }

            virtual ~Actor() = default;
            void Update() override {};

            inline void SetActive(bool m_active = true) { 
                this->m_active = m_active; 
            }

            inline bool GetActive() { 
                return m_active; 
            }

        protected:

            bool
                m_canAttack, 
                m_canDamage,
                m_flipX,
                m_attacking,
                m_isHit,
                m_canAttackAlt,
                m_willDestroy = false,
                m_startDestroy = false,
                m_canDestroy = false;

            int m_state;
            std::atomic_bool m_active;

            inline void m_Init() {
                m_canAttack = true;        
                m_canDamage = true;
                m_flipX = false;
                m_isHit = false;
                m_canAttackAlt = true;
                m_attacking = false;
                m_active = false;
            }

            virtual bool DoDamage(int amount, const std::string& key = "") {};

    };
}