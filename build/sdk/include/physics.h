#pragma once

#include "./entity.h"
#include "./collisionManager.h"

#include "./debug.h"


//game physics powered by Box2D
class Physics {

    public:

        float gravityX, gravityY;

        bool sleeping = true, 
             setWarmStart = false, 
             continuous = true, 
             subStep = false,
             clearForces = false;

        b2World world;
        
        CollisionManager collisions;
        
        #if DEVELOPMENT == 1
            DebugDraw* debug;
        #endif

        static inline const int32 velocityIterations = 3;
        static inline const int32 positionIterations = 2;

        static b2Body* CreateStaticBody(
            float x, 
            float y, 
            float width, 
            float height, 
            bool isSensor = false, 
            int pointer = 0
        );

        static b2Body* CreateDynamicBody(
            const std::string& type,
            float x,
            float y,
            float width,
            float height,
            bool isSensor = true,
            int pointer = 0, 
            float density = 0.0f, 
            float friction = 0.0f, 
            float restitution = 0.0f
        );

        static void DestroyBody(b2Body* b);

        void Update();
        void ClearBodies();

        Physics();
        ~Physics() = default;

        inline void SetGravity(float x, float y) {
            this->gravityX = x;
            this->gravityY = y;
        };

    private:

        uint32 m_flags;

        struct Body {
            b2BodyDef def;
            b2Body* self;
            b2Fixture* fixture;
            b2FixtureDef fixtureDef;
        };

        b2Vec2 gravity;
        std::set<b2Body*> bodiesToRemove;
        std::vector<b2Body*> active_bodies;

};

