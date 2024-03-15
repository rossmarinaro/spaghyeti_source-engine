#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/behaviors.h"

namespace entity_behaviors {
    
    class Crook_Behavior : public Behavior {

        public:

            int health;

            //constructor, called on start

            Crook_Behavior(std::shared_ptr<Entity> entity):
                Behavior(entity, "Crook")
            {
                this->health = 3; 
            }

            //update every frame

            void Update(Inputs* inputs, Camera* camera) override 
            { 


            }

    };
}