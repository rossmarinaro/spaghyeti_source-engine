
#include "../../../../../../build/include/app.h"
#include "../../../../../../build/include/collisionManager.h"


//---------------- box2d overlaps


void CollisionManager::BeginContact(b2Contact* contact)
{

    b2BodyUserData bodyUserDataA = contact->GetFixtureA()->GetBody()->GetUserData(),
                   bodyUserDataB = contact->GetFixtureB()->GetBody()->GetUserData();

    b2Fixture* bodyFixtureA = contact->GetFixtureA()->GetBody()->GetFixtureList();
    b2Fixture* bodyFixtureB = contact->GetFixtureA()->GetBody()->GetFixtureList();

    for (const auto &entity : System::Application::game->entities)
    {
        if (strcmp(entity->type, "sprite") != 0) 
            return;

        auto sprite = std::static_pointer_cast<Sprite>(entity);

        if (sprite->bodies.size())  
        {
            sprite->num_contacts++;

            for (const auto &body : sprite->bodies)
            {
                b2BodyUserData data = body.first->GetFixtureList()->GetBody()->GetUserData();

                if ((data.pointer == bodyUserDataA.pointer || data.pointer == bodyUserDataB.pointer) && 
                    !bodyFixtureA->IsSensor() || !bodyFixtureB->IsSensor())
                    sprite->SetContact(true);  //sprite->SetTint(glm::vec3(1.0f, 0.0f, 0.0f));   }
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

    for (const auto &entity : System::Application::game->entities)
    {

        if (strcmp(entity->type, "sprite") != 0) 
            return;

        auto sprite = std::static_pointer_cast<Sprite>(entity);
        
        if (sprite->bodies.size()) 
        {
            sprite->num_contacts--;

            if (sprite->num_contacts > 0)
                break;

            for (const auto &body : sprite->bodies)
            {
                b2BodyUserData data = body.first->GetFixtureList()->GetBody()->GetUserData();

                if ((data.pointer == bodyUserDataA.pointer || data.pointer == bodyUserDataB.pointer) && 
                    !bodyFixtureA->IsSensor() || !bodyFixtureB->IsSensor())
                        sprite->SetContact(false); //sprite->ClearTint();
            }
        }        
    }
}


//----------------------------------- UI related collisions (standard / ui layer projection matrix * factor)


bool CollisionManager::CheckCollisions(const std::shared_ptr<Sprite> &sprite, const std::shared_ptr<Geometry> &rect, int factor)
{
    //x axis

    bool collisionX = sprite->m_position.x + sprite->m_texture.FrameWidth / 2 >= rect->m_position.x * factor &&
                      rect->m_position.x * factor + rect->width / 2 >= sprite->m_position.x;

    //y axis

    bool collisionY = sprite->m_position.y + sprite->m_texture.FrameHeight / 2 >= rect->m_position.y * factor &&
                      rect->m_position.y * factor + rect->height / 2 >= sprite->m_position.y;

    return collisionX && collisionY;

}

//-----------------------------------


bool CollisionManager::CheckCollisions(const std::shared_ptr<Sprite> &spriteA, const std::shared_ptr<Sprite> &spriteB)
{
    //x axis

    bool collisionX = spriteA->m_position.x + spriteA->m_texture.FrameWidth / 2 >= spriteB->m_position.x &&
                      spriteB->m_position.x + spriteB->m_texture.FrameWidth / 2 >= spriteA->m_position.x;

    //y axis

    bool collisionY = spriteA->m_position.y + spriteA->m_texture.FrameHeight / 2 >= spriteB->m_position.y &&
                      spriteB->m_position.y + spriteB->m_texture.FrameHeight / 2 >= spriteA->m_position.y;

    return collisionX && collisionY;

}

