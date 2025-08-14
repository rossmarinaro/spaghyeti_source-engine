#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

   class Gameplay : public Behavior {

        public:

           //constructor, called on start

           Gameplay(std::shared_ptr<Entity> entity);
           ~Gameplay() = default;

            void Update() override;

        private:

            int m_charIterator;

            float m_alpha;

            bool m_canFade,
                 m_startFade,
                 m_startSequence;

            std::shared_ptr<Sprite> m_dialog_box;
            std::shared_ptr<Text> m_dialog_text;

            std::string m_textContent, 
                        m_subject, 
                        m_text;

            void SetTextContent(const std::string& content, const std::string& subject = "", bool fade = true); 
            void ResetText();
   };
};