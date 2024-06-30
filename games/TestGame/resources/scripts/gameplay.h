#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

   class GAMEPLAY : public Behavior {

       public:

           //constructor, called on start

           GAMEPLAY(std::shared_ptr<Entity> entity):
            Behavior(entity, typeid(GAMEPLAY).name())
            {

            }

           //update every frame

            void Update() override {

                for (auto& tile : System::Game::GetScene()->entities) 
                {
                    if (strcmp(tile->type, "tile") == 0 && tile->name == "town_mountains.csv")
                        std::static_pointer_cast<Sprite>(tile)->SetScrollFactor({ -0.3f, 1.0f });

                    if (strcmp(tile->type, "tile") == 0 && tile->name == "town_background0.csv")
                        std::static_pointer_cast<Sprite>(tile)->SetScrollFactor({ -0.25f, 1.0f });

                    if (strcmp(tile->type, "tile") == 0 && tile->name == "town_background1.csv")
                        std::static_pointer_cast<Sprite>(tile)->SetScrollFactor({ -0.2f, 1.0f });

                    if (strcmp(tile->type, "tile") == 0 && tile->name == "town_background2.csv")
                        std::static_pointer_cast<Sprite>(tile)->SetScrollFactor({ -0.15f, 1.0f });

                    if (strcmp(tile->type, "tile") == 0 && tile->name == "town_background3.csv")
                        std::static_pointer_cast<Sprite>(tile)->SetScrollFactor({ -0.1f, 1.0f });

                    if (strcmp(tile->type, "tile") == 0 && tile->name == "town_background4.csv")
                        std::static_pointer_cast<Sprite>(tile)->SetScrollFactor({ -0.05f, 1.0f });
                }
           }

   };
};