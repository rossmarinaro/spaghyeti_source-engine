#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

   class Menu_Player : public Behavior {

       public:

           //constructor, called on start

           inline Menu_Player(std::shared_ptr<Entity> entity);

           //update every frame

           inline void Update(Process::Context& context, void* scene) override;

        private:

            float m_r, m_g, m_b, m_a;
            bool m_init, m_rev;
            std::shared_ptr<Sprite> m_player;
   };
};