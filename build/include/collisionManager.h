#pragma once

#include "./entity.h"


class CollisionManager : public b2ContactListener {

    public:

        static bool CheckCollisions(const std::shared_ptr<Sprite> &spriteA, const std::shared_ptr<Sprite> &spriteB);
        static bool CheckCollisions(const std::shared_ptr<Sprite> &sprite, const std::shared_ptr<Graphics::Rectangle> &rect);

        static bool CheckCollisions(const std::shared_ptr<Sprite> &sprite, const std::unique_ptr<Graphics::Rectangle> &rect);
        static bool CheckCollisions(const Sprite &sprite, const std::unique_ptr<Graphics::Rectangle> &rect);

        static bool CheckCollisions(const Sprite &spriteA, const Sprite &spriteB);

        virtual void BeginContact(b2Contact* contact);
        virtual void EndContact(b2Contact* contact);


};