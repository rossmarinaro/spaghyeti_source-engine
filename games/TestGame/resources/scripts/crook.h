#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/behaviors.h"

namespace entity_behaviors {
    
    class Crook : public Behavior {

        public:

            int health;

            //constructor, called on start

            Crook(std::shared_ptr<Entity> entity):
                Behavior(entity, "Crook")
            {
                this->health = 3; 
            }

            //update every frame

            void Update(Process::Context context, const std::vector<std::shared_ptr<Behavior>>& behaviors) override 
            { 

                //Game::GetBehavior("PlayerController");
            }

    };
}