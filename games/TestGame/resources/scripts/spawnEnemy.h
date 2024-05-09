#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"

namespace entity_behaviors {

    class SpawnEnemy : public Behavior {

        public:


            //constructor, called on start

            SpawnEnemy(std::shared_ptr<Entity> entity):
                Behavior(entity, typeid(SpawnEnemy).name())
            {
                //interval spawn timer

                Time::setInterval(3000, []() { 
                    float x = System::Utils::floatBetween(0.0f, 1000.0f);
                    float y = System::Utils::floatBetween(0.0f, 1000.0f);
                    auto spr = System::Game::CreateSprite("test3.png", x, y);
                });
            }

            //update every frame

            void Update() override { }

    };
}