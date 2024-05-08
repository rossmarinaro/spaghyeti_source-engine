#pragma once


#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"


namespace entity_behaviors {

   class COLORSHIFTSPRITE : public Behavior {

       public:

           //constructor, called on start

           inline COLORSHIFTSPRITE(std::shared_ptr<Entity> entity);

           //update every frame

           inline void Update() override;

           inline void SetBroadcastTint(const char* entityType, const glm::vec3& tint);

        private:

            float m_r, m_g, m_b;
            bool m_rev;
            std::shared_ptr<Sprite> m_sprite;

   };
};