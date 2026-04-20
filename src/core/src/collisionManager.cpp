
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
        if (entity->GetType() != Entity::SPRITE) 
            continue;       

        auto sprite = std::static_pointer_cast<Sprite>(entity); 

        if (sprite->GetBodies().size())  
        {
            for (const auto& body : sprite->GetBodies()) 
            {
                const auto pointer = body.first->pointer;

                if (
                    body.first->IsEnabled() && 
                    !bodyFixtureA->IsSensor() && !bodyFixtureB->IsSensor() && 
                    (pointer == bodyUserDataA.pointer || pointer == bodyUserDataB.pointer))
                {
                    sprite->num_contacts++; 
                    sprite->SetContact(true); 
                    /* test */ //sprite->SetTint({1.0f, 0.0f,0.0f}); 
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

    b2Fixture* bodyFixtureA = contact->GetFixtureA()->GetBody()->GetFixtureList();
    b2Fixture* bodyFixtureB = contact->GetFixtureA()->GetBody()->GetFixtureList();

    for (const auto& entity : System::Game::GetScene()->entities)
    {
        if (entity->GetType() != Entity::SPRITE)  
            continue; 

        auto sprite = std::static_pointer_cast<Sprite>(entity);

        if (sprite->GetBodies().size()) 
        {        
            for (const auto& body : sprite->GetBodies()) 
            {
                const auto pointer = body.first->pointer;

                if (body.first->IsEnabled() && 
                    !bodyFixtureA->IsSensor() && !bodyFixtureB->IsSensor() && 
                    (pointer == bodyUserDataA.pointer || pointer == bodyUserDataB.pointer))
                        sprite->num_contacts--;
            }

            if (sprite->num_contacts <= 0) {
                sprite->SetContact(false); 
                /* test */ //sprite->ClearTint(); 
            } 
        }        
    }
}
