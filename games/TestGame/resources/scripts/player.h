#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"

namespace entity_behaviors {
    
    class PlayerController : public Behavior {

        public:

            std::shared_ptr<Sprite> player;
            b2Body* hb;

            inline PlayerController(std::shared_ptr<Entity> entity);
            inline ~PlayerController();

            inline void Update(Process::Context& context, void* scene) override; 
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

            std::atomic_bool m_active;
            
            std::shared_ptr<Sprite> m_heart1;
            std::shared_ptr<Sprite> m_heart2;
            std::shared_ptr<Sprite> m_heart3;

            inline void Move(Inputs* inputs);
            inline void Jump(Inputs* inputs);
            inline void Attack(Physics* physics);

    };
}

