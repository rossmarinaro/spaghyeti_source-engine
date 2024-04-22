#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

    class TEXT_Behavior : public Behavior {

        public: 

            int score;

           //constructor, called on start

            TEXT_Behavior (std::shared_ptr<Entity> entity):
                Behavior(entity, "TEXT")
            {
                this->m_text = std::static_pointer_cast<Text>(entity);
                this->score = 0;
            }

            //update every frame

            void Update(Process::Context& context, void* scene) override {

                float fps = 60.0f - context.time->m_delta;
                this->m_text->SetText("score: " + std::to_string(this->score));
            }

        private:
            
            std::shared_ptr<Text> m_text;

    };
};