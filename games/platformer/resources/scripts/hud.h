#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

    class HUD : public Behavior {

        public: 

            HUD (std::shared_ptr<Entity> entity);

            void Update() override;

        private:

            std::atomic_bool m_isOpen, m_canToggle;           
    };
};