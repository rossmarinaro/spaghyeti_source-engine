#include "../../../build/sdk/include/game.h"
#include "../../../build/sdk/include/entity.h"

#include "../../vendors/box2d/include/box2d/box2d.h"

#include "./collisionManager.h"


std::pair<b2Fixture*, b2Fixture*> GetSortedKey(b2Fixture* a, b2Fixture* b) {
    return (a < b) ? std::make_pair(a, b) : std::make_pair(b, a);
}
        
//----------------------------------


void CollisionManager::SaveNormal(b2Contact* contact) 
{
    b2Fixture* fA = contact->GetFixtureA();
    b2Fixture* fB = contact->GetFixtureB();

    b2WorldManifold worldManifold;
    contact->GetWorldManifold(&worldManifold);
    b2Vec2 normal = worldManifold.normal;

    const auto key = GetSortedKey(fA, fB);
    
    if (fA != key.first)
        normal = -normal;

    m_activeNormals[key] = normal;
}


//----------------------------------


void CollisionManager::ApplyCollision(const std::shared_ptr<Sprite>& sprite, b2Vec2 normal, bool isCollide) 
{
    if (abs(normal.x) > abs(normal.y)) {
        if (normal.x > 0)
            sprite->SetContact(isCollide, Sprite::ContactDirection::RIGHT); 
        else if (normal.x < 0)
            sprite->SetContact(isCollide, Sprite::ContactDirection::LEFT); 
    }
            
    else if (abs(normal.y) > abs(normal.x)) {
        if (normal.y > 0)
            sprite->SetContact(isCollide, Sprite::ContactDirection::BOTTOM); 
        else if (normal.y < 0)
            sprite->SetContact(isCollide, Sprite::ContactDirection::TOP); 
    }
}


//---------------------------------- box2d overlaps


void CollisionManager::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {
    if (contact->GetManifold()->pointCount > 0)
        SaveNormal(contact);
}

//---------------------------------


void CollisionManager::BeginContact(b2Contact* contact)
{
    b2BodyUserData bodyUserDataA = contact->GetFixtureA()->GetBody()->GetUserData(),
                   bodyUserDataB = contact->GetFixtureB()->GetBody()->GetUserData();

    b2Fixture* bodyFixtureA = contact->GetFixtureA();
    b2Fixture* bodyFixtureB = contact->GetFixtureA();

    b2WorldManifold worldManifold;
    contact->GetWorldManifold(&worldManifold);

    b2Vec2 normal = worldManifold.normal;

    for (const auto& entity : System::Game::GetScene()->entities)
    {
        if (entity->GetType() != Entity::SPRITE) 
            continue;       

        const auto sprite = std::static_pointer_cast<Sprite>(entity); 

        if (sprite->GetBodies().size())  
        {
            for (const auto& body : sprite->GetBodies()) 
            {
                const auto pointer = body.first->pointer;

                if (
                    body.first->IsEnabled() && 
                    !bodyFixtureA->IsSensor() && !bodyFixtureB->IsSensor() && 
                    (pointer == bodyUserDataA.pointer || pointer == bodyUserDataB.pointer)) {
                    ApplyCollision(sprite, normal, true);
                    SaveNormal(contact);
                }       
            }
        }
    }
}


//---------------------------------


void CollisionManager::EndContact(b2Contact* contact)
{
    b2BodyUserData bodyUserDataA = contact->GetFixtureA()->GetBody()->GetUserData(),
                   bodyUserDataB = contact->GetFixtureB()->GetBody()->GetUserData();

    b2Fixture* bodyFixtureA = contact->GetFixtureA();
    b2Fixture* bodyFixtureB = contact->GetFixtureB();

    for (const auto& entity : System::Game::GetScene()->entities)
    {
        if (entity->GetType() != Entity::SPRITE)  
            continue; 

        const auto sprite = std::static_pointer_cast<Sprite>(entity);

        if (sprite->GetBodies().size()) 
        {        
            for (const auto& body : sprite->GetBodies()) 
            {
                const auto pointer = body.first->pointer;

                if (body.first->IsEnabled() && 
                    !bodyFixtureA->IsSensor() && !bodyFixtureB->IsSensor() && 
                    (pointer == bodyUserDataA.pointer || pointer == bodyUserDataB.pointer)) 
                {
                    const auto key = GetSortedKey(bodyFixtureA, bodyFixtureB);
                    auto it = m_activeNormals.find(key);

                    if (it == m_activeNormals.end()) 
                        continue; 

                    b2Vec2 lastNormal = it->second;

                    ApplyCollision(sprite, lastNormal, false);
                    
                    it = m_activeNormals.erase(it);
                }
            }
        }
    }
}
