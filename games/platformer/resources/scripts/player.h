#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"

namespace entity_behaviors {
    
    class PlayerController : public Behavior {

        public:

            static inline int s_livesLeft = 3;
            float positionX, positionY;

            std::shared_ptr<Sprite> player;
            b2Body* hb;

            PlayerController(std::shared_ptr<Entity> entity);
            ~PlayerController();

            void Update() override; 
            void DoDamage(int amount);

        private:

            int m_health, m_timesJumped; 

            bool
                m_alive,
                m_canAttack, 
                m_canDamage,
                m_follow, 
                m_flipX,
                m_attacking,
                m_shootFireball;

            std::string m_state;

            std::atomic_bool m_active;

            std::vector<std::shared_ptr<Sprite>> stars;

            void Move(Inputs* inputs);
            void Jump(Inputs* inputs);
            void Attack(Physics* physics);

    };
}

