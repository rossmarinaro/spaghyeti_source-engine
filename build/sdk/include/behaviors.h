#pragma once

#include "./entity.h"
#include "./context.h"

namespace entity_behaviors {

    //base interactivity which all behaviors are derived
    class Behavior {

        public:

            std::string key; 
            
            inline Behavior(std::shared_ptr<Entity> entity, const std::string& key) 
            {
                this->key = key; 
                this->m_entity = entity;
                this->m_isActive = true;
            }

            virtual ~Behavior() {
                this->m_isActive = false;
            };

            virtual void Update(Process::Context& context, void* scene) {};

            template <typename T>
            static inline const std::shared_ptr<T> GetBehavior(const std::string& key, const std::vector<std::shared_ptr<Behavior>>& behaviors) {
                return std::dynamic_pointer_cast<T>(*std::find_if(behaviors.begin(), behaviors.end(), [&](std::shared_ptr<Behavior> behavior) { 
                    return behavior->key == key; 
                }));
            }

        protected:

            bool m_isActive; 

            std::shared_ptr<Entity> m_entity;

            template <typename T>
            inline std::shared_ptr<T> GetHandle(const char* type) {

                if (strcmp(this->m_entity->type, type) == 0)
                    return std::dynamic_pointer_cast<T>(this->m_entity);

                else 
                    return nullptr; 
            }            
      
    };

}

