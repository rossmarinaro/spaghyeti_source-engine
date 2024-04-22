#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"

namespace entity_behaviors {
    
    class Elf : public Behavior {

        public:

            int health;

            std::shared_ptr<Sprite> sprite;
            b2Body* hb;

            inline Elf(std::shared_ptr<Entity> entity);
            inline void Update(Process::Context& context, void* scene) override;

        private:

            bool m_rev;

    };
}