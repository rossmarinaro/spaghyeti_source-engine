
#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/collisionManager.h"


//---------------- box2d overlaps


void CollisionManager::BeginContact(b2Contact* contact)
{

    b2BodyUserData bodyUserDataA = contact->GetFixtureA()->GetBody()->GetUserData(),
                   bodyUserDataB = contact->GetFixtureB()->GetBody()->GetUserData();

    b2Fixture* bodyFixtureA = contact->GetFixtureA()->GetBody()->GetFixtureList();
    b2Fixture* bodyFixtureB = contact->GetFixtureA()->GetBody()->GetFixtureList();

    for (const auto& entity : System::Game::GetScene()->entities)
    {
        if (strcmp(entity->type, "sprite") != 0) 
            continue;       

        auto sprite = std::static_pointer_cast<Sprite>(entity); 

        if (sprite->bodies.size())  
        {

            sprite->num_contacts++; 

            for (const auto& body : sprite->bodies)
            {
                b2BodyUserData data = body.first->GetFixtureList()->GetBody()->GetUserData();

                if (
                    body.first->IsEnabled() &&
                    (data.pointer == bodyUserDataA.pointer || data.pointer == bodyUserDataB.pointer) && 
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

        if (strcmp(entity->type, "sprite") != 0) 
            continue; 

        auto sprite = std::static_pointer_cast<Sprite>(entity);

        if (sprite->bodies.size()) 
        {        

            sprite->num_contacts--;

            if (sprite->num_contacts > 0) 
                break;

            for (const auto& body : sprite->bodies)
            {
                b2BodyUserData data = body.first->GetFixtureList()->GetBody()->GetUserData();

                if (
                    body.first->IsEnabled() &&
                    (data.pointer == bodyUserDataA.pointer || data.pointer == bodyUserDataB.pointer) && 
                    (!bodyFixtureA->IsSensor() || !bodyFixtureB->IsSensor()) 
                )
                    sprite->SetContact(false);
            }
        }        
    }
}


//-----------------------------------


bool CollisionManager::CheckCollisions(const std::shared_ptr<Sprite>& spriteA, const std::shared_ptr<Sprite>& spriteB)
{
    //x axis

    bool collisionX = spriteA->position.x + spriteA->texture.FrameWidth / 2 >= spriteB->position.x &&
                      spriteB->position.x + spriteB->texture.FrameWidth / 2 >= spriteA->position.x;

    //y axis

    bool collisionY = spriteA->position.y + spriteA->texture.FrameHeight / 2 >= spriteB->position.y &&
                      spriteB->position.y + spriteB->texture.FrameHeight / 2 >= spriteA->position.y;

    return collisionX && collisionY;

}

