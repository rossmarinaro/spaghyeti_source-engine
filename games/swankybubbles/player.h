#pragma once

#include "../../src/core/game/entities/entity.h"

class SwankyVelvet : public Player {

    public:

        int speed = 1;
        bool canHit = true, 
             canFire = false;

        void Move(const char* direction);
        void Update();

        SwankyVelvet(const char* key, const glm::vec2 &position):
            Player(key, position) {};
        
        ~SwankyVelvet() = default;
};