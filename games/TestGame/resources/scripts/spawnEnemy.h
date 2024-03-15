#pragma once

#include <mutex>

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/include/behaviors.h"

namespace entity_behaviors {

    class SpawnEnemy_Behavior : public Behavior {

        public:


            //constructor, called on start

            SpawnEnemy_Behavior(std::shared_ptr<Entity> entity):
                Behavior(entity, "SpawnEnemy")
            {
                //interval spawn timer

                System::Application::game->time->setInterval(3000, [&]() { 
                    float x = System::Utils::floatBetween(0.0f, 1000.0f);
                    float y = System::Utils::floatBetween(0.0f, 1000.0f);
                    auto spr = Game::CreateSprite("test3.png", x, y);
                }, m_lock);

            }

            //update every frame

            void Update(Inputs* inputs, Camera* camera) override { }

        private:

            static inline std::mutex m_lock;

    };
}