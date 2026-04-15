#pragma once

#include <memory>

//game physics powered by Box2D
class Physics {

    public:

        struct Body 
        {
            enum Type { STATIC, KINEMATIC, DYNAMIC };
            enum Shape { BOX, CIRCLE };

            int type, shape;
            bool isSensor, isEnabled;
            uintptr_t pointer;
            float x, y, 
                 width = 0.0f, height = 0.0f, radius = 0.0f, 
                 density, friction, restitution;

            std::string id;

            Body(int physicsType, float x, float y, float width, float height, int pointer, bool isSensor, float density, float friction, float restitution);
            Body(int physicsType, float x, float y, float radius, int pointer, bool isSensor, float density, float friction, float restitution);
            Body() = default;

            const Math::Vector2 GetPosition();
            const Math::Vector2 GetLinearVelocity();
            
            void Init(int physicsType, float x, float y, int pointer, bool isSensor, float density, float friction, float restitution);
            void SetTransform(float x, float y, float angle = 0.0f);
            void SetLinearVelocity(float velocityX, float velocityY); 
            void ApplyLinearImpulse(float velocityX, float velocityY);
            void SetEnabled(bool enabled);
            void SetSensor(bool sensor);
            void SetFixedRotation(bool isFixedRotation);
            void SetGravityScale(float gs);
            void CreateFixture(float width, float height, bool isSensor, float density, float friction, float restitution);
            void CreateFixture(float radius, bool isSensor, float density, float friction, float restitution);
            void UpdateFixture(float width, float height, bool isSensor = false, float density = 0.0f, float friction = 0.0f, float restitution = 0.0f);
            void UpdateFixture(float radius, bool isSensor = false, float density = 0.0f, float friction = 0.0f, float restitution = 0.0f);
            void DestroyFixture();

            const int GetType();
            const bool CollidesWith(const std::shared_ptr<Body>& bodyB);
            const bool Exists();
            const bool IsEnabled(); 
            const bool IsSensor(); 
        };

        float gravityX = 0.0f, 
              gravityY = 500.0f;

        bool enableDebug,
             sleeping, 
             setWarmStart,  
             continuous, 
             subStep,
             clearForces;

        //factory for dynamic, static, and kinematic rectangle bodies
        static std::shared_ptr<Body> CreateBody(
            int type, 
            float x, 
            float y, 
            float width = 0.0f, 
            float height = 0.0f, 
            bool isSensor = false, 
            int pointer = -1, 
            float density = 0.0f, 
            float friction = 0.0f, 
            float restitution = 0.0f
        );

        //factory for dynamic, static, and kinematic circle bodies
        static std::shared_ptr<Body> CreateBody(
            int type, 
            float x, 
            float y, 
            float radius = 0.0f,
            bool isSensor = false, 
            int pointer = -1, 
            float density = 0.0f, 
            float friction = 0.0f, 
            float restitution = 0.0f
        );

        //does not destroy body immediately. body will be destroyed after next timestep
        static void DestroyBody(const std::shared_ptr<Body>& body);

        void Update();
        void ClearBodies();
        void SetGravity(float x, float y);
        
        Physics();
        ~Physics();

        void* GetWorld();

    private:

        static inline const signed int s_velocityIterations = 3,
                                       s_positionIterations = 2;

        void Cleanup();

};

 