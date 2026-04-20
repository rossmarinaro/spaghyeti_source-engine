#pragma once

#include "./actor.h"

namespace entity_behaviors {
    
    class PlayerController : public Actor {

        public:

            enum State { NONE, IDLE, JUMP, WALK, ATTACK };

            int magic, cigar;
            bool magicEnabled, cigarEnabled;

            std::shared_ptr<Physics::Body> attackBox;

            PlayerController(std::shared_ptr<Entity> entity);
            ~PlayerController();

            void Update() override; 
            bool DoDamage(int amount) override;
            void SetState(const State& state, int option = 0);

            inline void SetInvincible(bool invincible = true) { m_invincible = invincible; }

        private:

            static inline int s_time;

            State m_state;

            int m_timesJumped; 

            bool m_isJump,
                 m_canJump,
                 m_canIdle,
                 m_jumpGate,
                 m_invincible,
                 m_shootFireball;

            std::shared_ptr<Sprite> m_shadow;
 
    };
}

