#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/behaviors.h"


namespace entity_behaviors {

    class TEXT_Behavior : public Behavior {

        public: 

           //constructor, called on start

            TEXT_Behavior (/* Scene* scene,  */std::shared_ptr<Entity> entity):
                Behavior(entity, "TEXT")
            {
                this->text = std::static_pointer_cast<Text>(this->entity);
            }

            //update every frame

            void Update(Process::Context context, const std::vector<std::shared_ptr<Behavior>>& behaviors) override {

                float fps = 60.0f - context.time->m_delta;
                this->text->SetText("FPS: " + std::to_string(fps));
            }

        private:
            
            std::shared_ptr<Text> text;

    };
};