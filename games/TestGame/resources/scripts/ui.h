#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

    class UI : public Behavior {

        public: 

            int score;

           //constructor, called on start

            inline UI (std::shared_ptr<Entity> entity);

            //update every frame

            inline void Update() override;

        private:

            std::atomic_bool m_isOpen, m_canToggle;
            
            std::shared_ptr<Sprite> m_heart1;
            std::shared_ptr<Sprite> m_heart2;
            std::shared_ptr<Sprite> m_heart3;
            std::shared_ptr<Sprite> m_menu;

            std::shared_ptr<Text> m_score;
            std::shared_ptr<Text> m_quitText;

    };
};