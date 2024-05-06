#pragma once

#include "./entity.h"


class CollisionManager : public b2ContactListener {

    public:

        static bool CheckCollisions(const std::shared_ptr<Sprite>& spriteA, const std::shared_ptr<Sprite>& spriteB);
        virtual void BeginContact(b2Contact* contact);
        virtual void EndContact(b2Contact* contact);

};