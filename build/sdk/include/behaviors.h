#pragma once

#include "./entity.h"
#include "./context.h"

namespace entity_behaviors {

    //base interactivity which all behaviors are derived
    class Behavior {

        public:

            int layer;

            std::string ID, key; 

            std::atomic_char active;
            
            inline Behavior(const std::string& ID, const std::string& key) {
                this->key = key; 
                this->ID = ID;
                layer = 0;
                active = true;
            }

            virtual ~Behavior() {}
            virtual void Update() {}           

    };

}

