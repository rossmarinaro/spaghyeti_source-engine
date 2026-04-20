#pragma once

#include "../../vendors/box2d/include/box2d/box2d.h" 

class CollisionManager : public b2ContactListener {

    public:

        virtual void BeginContact(b2Contact* contact);
        virtual void EndContact(b2Contact* contact);
}; 