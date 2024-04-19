#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"

namespace entity_behaviors {
    
    class PlayerController : public Behavior {

        public:

            int health;

            std::shared_ptr<Sprite> player;
            b2Body* hb;

            inline PlayerController(std::shared_ptr<Entity> entity);
            inline ~PlayerController();

            inline void Update(Process::Context& context, const std::vector<std::shared_ptr<Behavior>>& behaviors) override; 
            inline void DoDamage(int amount);

        private:

            bool 
                canJump, 
                canAttack, 
                canDamage,
                follow, 
                flipX,
                attacking,
                shootFireball;

            std::shared_ptr<Sprite> heart1;
            std::shared_ptr<Sprite> heart2;
            std::shared_ptr<Sprite> heart3;

            inline void Move(Inputs* inputs);
            inline void Jump(Inputs* inputs);
            inline void Attack(Physics* physics);

    };
}

