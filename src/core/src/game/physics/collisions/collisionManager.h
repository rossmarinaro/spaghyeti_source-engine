#pragma once

#include "../../entities/entity.h"

class CollisionManager : public b2ContactListener {

    public:

        static bool CheckCollisions(std::shared_ptr<Sprite> spriteA, std::shared_ptr<Sprite> spriteB);
        static bool CheckCollisions(std::shared_ptr<Sprite> sprite, std::shared_ptr<Graphics::Rectangle> rect);

        static bool CheckCollisions(std::shared_ptr<Sprite> sprite, std::unique_ptr<Graphics::Rectangle> &rect);
        static bool CheckCollisions(Sprite &sprite, std::unique_ptr<Graphics::Rectangle> &rect);

        static bool CheckCollisions(Sprite &spriteA, Sprite &spriteB);

        virtual void BeginContact(b2Contact* contact);
        virtual void EndContact(b2Contact* contact);


};