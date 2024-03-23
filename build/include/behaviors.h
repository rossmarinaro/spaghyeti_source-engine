#pragma once

#include "./entity.h"
#include "./context.h"

namespace entity_behaviors {

    class Behavior {

        public:

            std::string key;
            
            virtual void Update(Process::Context context) {}

            Behavior(std::shared_ptr<Entity> entity, const std::string& key) {

                this->key = key; 
                
                if (strcmp(entity->type, "sprite") == 0)
                    this->sprite = std::dynamic_pointer_cast<Sprite>(entity);

                if (strcmp(entity->type, "text") == 0)
                    this->text = std::dynamic_pointer_cast<Text>(entity);

                if (strcmp(entity->type, "geometry") == 0)
                    this->geometry = std::dynamic_pointer_cast<Geometry>(entity);
            }

            virtual ~Behavior() = default;
         
        protected:

            std::shared_ptr<Sprite> sprite;
            std::shared_ptr<Text> text;
            std::shared_ptr<Geometry> geometry;

    };

}

