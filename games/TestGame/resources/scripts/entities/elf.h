#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"

namespace entity_behaviors {
    
    class Elf : public Behavior {

        public:

            int health;

            //constructor, called on start

            Elf(std::shared_ptr<Entity> entity):
                Behavior(entity, "Elf")
            {
                this->health = 3; 
            }

            //update every frame

            void Update(Process::Context& context, const std::vector<std::shared_ptr<Behavior>>& behaviors) override 
            { 

                //Game::GetBehavior("PlayerController");
            }

    };
}