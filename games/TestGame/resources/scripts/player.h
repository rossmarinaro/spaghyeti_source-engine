#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"

namespace entity_behaviors {
    
    class PlayerController : public Behavior {

        public:

            std::shared_ptr<Sprite> player;
            b2Body* hb;

            inline PlayerController(std::shared_ptr<Entity> entity);
            inline ~PlayerController();

            inline void Update() override; 
            inline void DoDamage(int amount);

        private:

            int m_health; 

            bool
                m_alive,
                m_canJump, 
                m_canAttack, 
                m_canDamage,
                m_follow, 
                m_flipX,
                m_attacking,
                m_shootFireball;

            std::string m_state;

            std::atomic_bool m_active;

            inline void Move(Inputs* inputs);
            inline void Jump(Inputs* inputs);
            inline void Attack(Physics* physics);

    };
}

