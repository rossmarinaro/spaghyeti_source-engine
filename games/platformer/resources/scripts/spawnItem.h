#pragma once

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

   class SPAWNITEM : public Behavior {

       public:

           //constructor, called on start

           SPAWNITEM(std::shared_ptr<Entity> entity);

           //update every frame

            void Update() override;
            

        private:

            bool m_canSpawn,
                 m_canTriggerDestroy,
                 m_canDestroyBox, 
                 m_canDestroyItem, 
                 m_canFadeItem, 
                 m_canTimeout,
                 m_startDestroy,
                 m_boxDestroyed,
                 m_rev;

            std::string& m_name;

            void CleanUp();
   };
};