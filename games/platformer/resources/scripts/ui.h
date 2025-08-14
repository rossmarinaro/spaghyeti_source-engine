#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

    class UI : public Behavior {

        public: 

            int score;

            UI (std::shared_ptr<Entity> entity);

            void Update() override;

        private:

            std::atomic_bool m_isOpen, m_canToggle;           
    };
};