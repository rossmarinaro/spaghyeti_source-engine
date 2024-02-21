#include "./camera.h"
#include "./inputs.h"
#include "./entity.h"

class Behavior {

    public:

        Entity* entity;
        Sprite* sprite;
        Text* text;
        Geometry* geometry;

        void Update(Inputs* inputs, Camera* camera) {}

        Behavior(Entity* entity) {

            this->sprite = dynamic_cast<Sprite*>(entity);
            this->text = dynamic_cast<Text*>(entity);
            this->geometry = dynamic_cast<Geometry*>(entity);
        }

        ~Behavior() = default;

};