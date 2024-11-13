#pragma once

#include <set>

#include "./entity.h"
#include "./collisionManager.h"

#include "./debug.h"


//game physics powered by Box2D
class Physics {

    public:

        float gravityX, gravityY;

        bool enableDebug,
             sleeping = true, 
             setWarmStart = false, 
             continuous = true, 
             subStep = false,
             clearForces = false;
        
        CollisionManager collisions;
        
        #if DEVELOPMENT == 1
            DebugDraw* debug;
        #endif

        static b2Body* CreateStaticBody(float x, float y, float width, float height, bool isSensor = false, int pointer = 0);

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
            gravityX = x;
            gravityY = y;
        }

        inline b2World& GetWorld() { return this->m_world; }

    private:

        uint32 m_flags;
        b2World m_world;

        struct Body 
        {
            b2BodyDef def;
            b2Body* self;
            b2Fixture* fixture;
            b2FixtureDef fixtureDef;
        };

        b2Vec2 m_gravity;
        std::set<b2Body*> m_bodiesToRemove;
        std::vector<b2Body*> m_active_bodies;

        static inline const int32 s_velocityIterations = 3;
        static inline const int32 s_positionIterations = 2;

};

