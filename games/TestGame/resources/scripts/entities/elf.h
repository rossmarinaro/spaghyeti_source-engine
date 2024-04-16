#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"

namespace entity_behaviors {
    
    class Elf : public Behavior {

        public:

            int health;

            std::shared_ptr<Sprite> sprite;
            b2Body* hb;

            //constructor, called on start

            inline Elf(std::shared_ptr<Entity> entity);

            //update every frame

            inline void Update(Process::Context& context, const std::vector<std::shared_ptr<Behavior>>& behaviors) override;

        private:

            bool rev;

    };
}