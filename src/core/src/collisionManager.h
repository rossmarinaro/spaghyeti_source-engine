#pragma once

#include "../../vendors/box2d/include/box2d/box2d.h" 

class CollisionManager : public b2ContactListener {

    public:
        virtual void BeginContact(b2Contact* contact) override;
        virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
        virtual void EndContact(b2Contact* contact) override;
    
    private:
        std::map<std::pair<b2Fixture*, b2Fixture*>, b2Vec2> m_activeNormals;

        void SaveNormal(b2Contact* contact);
        void ApplyCollision(const std::shared_ptr<Sprite>& sprite, b2Vec2 normal, bool isCollide); 
}; 