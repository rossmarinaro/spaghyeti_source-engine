#pragma once

#include "./entity.h"
#include "./context.h"

namespace entity_behaviors {

    //base interactivity which all behaviors are derived
    class Behavior {

        public:

            int layer;

            std::string ID, key; 

            std::atomic_char active;
            
            inline Behavior(std::shared_ptr<Entity> entity, const std::string& key):
                m_entity(entity)
            {
                this->key = key; 
                layer = 0;
                active = true;
                ID = m_entity->ID;
            }

            virtual ~Behavior() {}

            virtual void Update() {}

        protected:

            std::shared_ptr<Entity> m_entity;

            template <typename T>
            inline std::shared_ptr<T> GetHandle(const char* type)
            {

                if (strcmp(m_entity->type, type) == 0)
                    return std::dynamic_pointer_cast<T>(m_entity);

                else 
                    return nullptr; 
            }            
      
    };

}

