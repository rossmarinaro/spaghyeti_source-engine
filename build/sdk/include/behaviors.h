#pragma once

#include "./entity.h"
#include "./context.h"

namespace entity_behaviors {

    //base interactivity which all behaviors are derived
    class Behavior {

        public:

            std::string key; 
            
            inline Behavior(std::shared_ptr<Entity> entity, const std::string& key):
                m_entity(entity),
                m_isActive(true)
            {
                this->key = key; 
            }

            virtual ~Behavior() {
                this->m_isActive = false;
                this->m_entity = nullptr;
            }

            virtual void Update() {}

            template <typename T>
            static inline const std::shared_ptr<T> GetBehavior(const std::string& key, const std::vector<std::shared_ptr<Behavior>>& behaviors) {
                return std::dynamic_pointer_cast<T>(*std::find_if(behaviors.begin(), behaviors.end(), [&](std::shared_ptr<Behavior> behavior) { 
                    return behavior->key == key; 
                }));
            }
         
        protected:

            std::atomic_char m_isActive;

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

