#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/behaviors.h"


namespace entity_behaviors {

   class TEXT_Behavior : public Behavior {

        public:

           //constructor, called on start

            TEXT_Behavior (std::shared_ptr<Entity> entity):
                Behavior(entity, "TEXT")
            {
                this->lt = 0.0f;
            }

            //update every frame

            void Update(Process::Context context) override {

                this->text->SetText("FPS: " + std::to_string(1 / context.time->GetSeconds()));
                //this->text->SetText(this->GetBehavior<PlayerController>("PlayerController")->sprite->m_key);
            }

        private:

            float lt;

   };
};