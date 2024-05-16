#pragma once

#include "./actor.h"

namespace entity_behaviors {
    
    class Crook : public Actor {

        public:

            Crook(std::shared_ptr<Entity> entity);
            
            void Update() override;

        private:

            float m_startPos;

            bool m_canMoveLeft,
                 m_canMoveRight,
                 m_reverse;

    };
}