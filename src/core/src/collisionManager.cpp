
#include "../../../build/sdk/include/game.h"
#include "../../../build/sdk/include/entity.h"

#include "../../vendors/box2d/include/box2d/box2d.h"

#include "./collisionManager.h"


//---------------- box2d overlaps


void CollisionManager::BeginContact(b2Contact* contact)
{

    b2BodyUserData bodyUserDataA = contact->GetFixtureA()->GetBody()->GetUserData(),
                   bodyUserDataB = contact->GetFixtureB()->GetBody()->GetUserData();

    b2Fixture* bodyFixtureA = contact->GetFixtureA()->GetBody()->GetFixtureList();
    b2Fixture* bodyFixtureB = contact->GetFixtureA()->GetBody()->GetFixtureList();

    for (const auto& entity : System::Game::GetScene()->entities)
    {
        if (entity->type != Entity::SPRITE) 
            continue;       

        auto sprite = std::static_pointer_cast<Sprite>(entity); 

        if (sprite->bodies.size())  
        {

            sprite->num_contacts++; 

            for (const auto& body : sprite->bodies)
            {
                const int pointer = body.first->pointer;

                if (
                    body.first->IsEnabled() &&
                    (pointer == bodyUserDataA.pointer || pointer == bodyUserDataB.pointer) && 
                    (!bodyFixtureA->IsSensor() || !bodyFixtureB->IsSensor()) 
                )
                    sprite->SetContact(true);
            }
        }
    }

}


//---------------------------------


void CollisionManager::EndContact(b2Contact* contact)
{

    b2BodyUserData bodyUserDataA = contact->GetFixtureA()->GetBody()->GetUserData(),
                   bodyUserDataB = contact->GetFixtureB()->GetBody()->GetUserData();

    b2Fixture* bodyFixtureA = contact->GetFixtureA()->GetBody()->GetFixtureList();
    b2Fixture* bodyFixtureB = contact->GetFixtureA()->GetBody()->GetFixtureList();

    for (const auto& entity : System::Game::GetScene()->entities)
    {

        if (entity->type != Entity::SPRITE)  
            continue; 

        auto sprite = std::static_pointer_cast<Sprite>(entity);

        if (sprite->bodies.size()) 
        {        

            sprite->num_contacts--;

            if (sprite->num_contacts > 0) 
                break;

            for (const auto& body : sprite->bodies)
            {
                const int pointer = body.first->pointer;

                if (
                    body.first->IsEnabled() &&
                    (pointer == bodyUserDataA.pointer || pointer == bodyUserDataB.pointer) && 
                    (!bodyFixtureA->IsSensor() || !bodyFixtureB->IsSensor()) 
                )
                    sprite->SetContact(false);
            }
        }        
    }
}
