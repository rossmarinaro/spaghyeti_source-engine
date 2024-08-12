#pragma once

#include "./entity.h"
#include "./context.h"
#include "./utils.h"

namespace entity_behaviors {

    //base interactivity which all behaviors are derived
    class Behavior {

        public:

            int layer;

            std::string ID, key, name; 

            std::atomic_char active;
            
            inline Behavior(std::shared_ptr<Entity> entity, const std::string& key):
                m_entity(entity)
            {
                this->key = key; 
                this->layer = 0;
                this->active = true;
                this->ID = m_entity->ID;
            }

            virtual ~Behavior() 
            {
                this->active = false;
                this->m_entity = nullptr;

                #if DEVELOPMENT == 1
                    std::cout << "Behavior: " << this->name << " destroyed.\n";
                #endif
            }

            virtual void Update() {}
         
        protected:

            std::shared_ptr<Entity> m_entity;

            template <typename T>
            inline std::shared_ptr<T> GetHandle(const char* type)
            {

                if (strcmp(this->m_entity->type, type) == 0)
                    return std::dynamic_pointer_cast<T>(this->m_entity);

                else 
                    return nullptr; 
            }            
      
    };

}

