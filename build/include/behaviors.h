#pragma once

#include "./camera.h"
#include "./inputs.h"
#include "./entity.h"

class Behavior {

    public:

        std::string key;

        std::shared_ptr<Entity> entity;
        std::shared_ptr<Sprite> sprite;
        std::shared_ptr<Text> text;
        std::shared_ptr<Geometry> geometry;

        virtual void Update(Inputs* inputs, Camera* camera) {}

        Behavior(std::shared_ptr<Entity> entity, const std::string &key) {

            this->key = key; 

            this->sprite = std::dynamic_pointer_cast<Sprite>(entity);
            this->text = std::dynamic_pointer_cast<Text>(entity);
            this->geometry = std::dynamic_pointer_cast<Geometry>(entity);
        }

        virtual ~Behavior() = default;

};

