#pragma once

#include "./actor.h"

namespace entity_behaviors {
    
    class Elf : public Actor {

        public:

            inline Elf(std::shared_ptr<Entity> entity);
            inline void Update() override;

        private:

            float m_startPos;

            bool m_canMoveLeft,
                 m_canMoveRight,
                 m_reverse;

    };
}