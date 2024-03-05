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

        uint32 m_flags;

        b2Vec2 gravity;
        b2World world;
        std::set<b2Body*> bodiesToRemove;
        
        struct Body {

            float m_width, m_height;

            b2BodyDef def;
            b2Body* self;
            b2Fixture* fixture;
            b2FixtureDef fixtureDef;
            b2PolygonShape box;
        };

        static inline std::vector<std::shared_ptr<Body>> fixtureData;

        CollisionManager collisions;
        
        #if DEVELOPMENT == 1
            DebugDraw* debug;
        #endif

        static inline const int32 velocityIterations = 6;
        static inline const int32 positionIterations = 2;

        inline void SetGravity(float x, float y) {
            this->gravityX = x;
            this->gravityY = y;
        };

        b2Body* CreateStaticBody(float x, float y, float width, float height);

        b2Body* CreateDynamicBody(
            float x,
            float y,
            float width,
            float height,
            bool isSensor = true,
            int type = 0, 
            float density = 0.0f, 
            float friction = 0.0f, 
            float restitution = 0.0f
        );

        void DestroyBody(b2Body* b);
        void Update();

        Physics();
        ~Physics() = default;

};

