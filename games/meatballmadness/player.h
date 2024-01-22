#pragma once

#include "../../src/core/src/game/entities/entity.h"

class Waiter : public Player {

    public:

        void Update();

        Waiter(const std::string &key, const glm::vec2 &position): 
            Player(key, position) {};
        
        ~Waiter() = default;

    private:

        bool canMove, canJump;

		glm::vec2 linearVelocity = glm::vec2(0.0f);
};