#pragma once

#include "../../build/include/behaviors.h"

class Waiter : public Behavior {

    public:

        Waiter(std::shared_ptr<Entity> entity): 
            Behavior(entity) 
        {
            
        };
        
        ~Waiter() = default;

        void Update(Inputs* inputs, Camera* camera) override;

    private:

        bool canMove, canJump;

		glm::vec2 linearVelocity = glm::vec2(0.0f);
};