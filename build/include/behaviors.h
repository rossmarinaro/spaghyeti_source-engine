#pragma once

#include "./entity.h"
#include "./context.h"


namespace entity_behaviors {

    //base interactivity which all behaviors are derived
    class Behavior {

        public:

            std::string key;
            
            Behavior(std::shared_ptr<Entity> entity, const std::string& key) {

                this->key = key; 
                this->entity = entity;
            }

            virtual ~Behavior() = default;
            virtual void Update(Process::Context& context, const std::vector<std::shared_ptr<Behavior>>& behaviors) = 0;

            template <typename T>
            static inline std::shared_ptr<T> GetBehavior(const std::string& key, const std::vector<std::shared_ptr<Behavior>>& behaviors) {
                return std::dynamic_pointer_cast<T>(*std::find_if(behaviors.begin(), behaviors.end(), [&](std::shared_ptr<Behavior> behavior) { 
                    return behavior->key == key; 
                }));
            }

        protected:

            std::shared_ptr<Entity> entity;

            template <typename T>
            inline std::shared_ptr<T> GetHandle(const char* type) {

                if (strcmp(this->entity->type, type) == 0)
                    return std::dynamic_pointer_cast<T>(entity);

                else 
                    return nullptr; 
            }            
      
        private:

            const char* type;


    };

}

