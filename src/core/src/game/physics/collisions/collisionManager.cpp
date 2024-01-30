#include "../../../app/app.h"
#include "./collisionManager.h"


//---------------- box2d overlaps

void CollisionManager::BeginContact(b2Contact* contact)
{

    b2BodyUserData bodyUserDataA = contact->GetFixtureA()->GetBody()->GetUserData();
    b2BodyUserData bodyUserDataB = contact->GetFixtureB()->GetBody()->GetUserData();

    std::set<std::shared_ptr<Entity>>::iterator it = System::Application::game->entities.begin();
    std::set<std::shared_ptr<Entity>>::iterator end = System::Application::game->entities.end();

    for (; it != end; ++it)
    {

        auto sprite = std::dynamic_pointer_cast<Sprite>(*it);
        
        if (sprite->m_body.self) 
        {

            b2Fixture* fixture = sprite->m_body.self->GetFixtureList();

            b2BodyUserData data = fixture->GetBody()->GetUserData();

            if (
                data.pointer == bodyUserDataA.pointer ||
                data.pointer == bodyUserDataB.pointer
            )
                sprite->BeginContact();
        }
    }

}

void CollisionManager::EndContact(b2Contact* contact)
{

    b2BodyUserData bodyUserDataA = contact->GetFixtureA()->GetBody()->GetUserData();
    b2BodyUserData bodyUserDataB = contact->GetFixtureB()->GetBody()->GetUserData();

    std::set<std::shared_ptr<Entity>>::iterator it = System::Application::game->entities.begin();
    std::set<std::shared_ptr<Entity>>::iterator end = System::Application::game->entities.end();

    for (; it != end; ++it)
    {

        auto sprite = std::dynamic_pointer_cast<Sprite>(*it);
        
        if (sprite->m_body.self) 
        {

            b2Fixture* fixture = sprite->m_body.self->GetFixtureList();

            b2BodyUserData data = fixture->GetBody()->GetUserData();

            if (
                data.pointer == bodyUserDataA.pointer ||
                data.pointer == bodyUserDataB.pointer
            )
                sprite->EndContact();
        }
    }
}



//--------------------------------- graphics overlaps (not box2d related)


bool CollisionManager::CheckCollisions(const std::shared_ptr<Sprite> &sprite, const std::shared_ptr<Graphics::Rectangle> &rect)
{
    //x axis

    bool collisionX = sprite->m_position.x + sprite->m_texture.FrameWidth / 2 >= rect->x &&
                      rect->x + rect->width / 2 <= sprite->m_position.x;

    //y axis

    bool collisionY = sprite->m_position.y + sprite->m_texture.FrameHeight / 2 >= rect->y &&
                      rect->y + rect->height / 2 >= sprite->m_position.y;

    return collisionX && collisionY;

}

//----------------------------------- UI related collisions (projection matrix * 2)


bool CollisionManager::CheckCollisions(const std::shared_ptr<Sprite> &sprite, const std::unique_ptr<Graphics::Rectangle> &rect)
{
    //x axis

    bool collisionX = sprite->m_position.x + sprite->m_texture.FrameWidth / 2 >= rect->x * 2 &&
                      rect->x * 2 + rect->width / 2 >= sprite->m_position.x;

    //y axis

    bool collisionY = sprite->m_position.y + sprite->m_texture.FrameHeight / 2 >= rect->y * 2 &&
                      rect->y * 2 + rect->height / 2 >= sprite->m_position.y;

    return collisionX && collisionY;

}


//-----------------------------------


bool CollisionManager::CheckCollisions(const Sprite &sprite, const std::unique_ptr<Graphics::Rectangle> &rect)
{
    //x axis

    bool collisionX = sprite.m_position.x + sprite.m_texture.FrameWidth / 2 >= rect->x &&
                      rect->x + rect->width / 2 >= sprite.m_position.x;

    //y axis

    bool collisionY = sprite.m_position.y + sprite.m_texture.FrameHeight / 2 >= rect->y &&
                      rect->y + rect->height / 2 >= sprite.m_position.y;

    return collisionX && collisionY;

}



//-----------------------------------


bool CollisionManager::CheckCollisions(const Sprite &spriteA, const Sprite &spriteB)
{
    //x axis

    bool collisionX = spriteA.m_position.x + spriteA.m_texture.FrameWidth / 2 >= spriteB.m_position.x &&
                      spriteB.m_position.x + spriteB.m_texture.FrameWidth / 2 >= spriteA.m_position.x;

    //y axis

    bool collisionY = spriteA.m_position.y + spriteA.m_texture.FrameHeight / 2 >= spriteB.m_position.y &&
                      spriteB.m_position.y + spriteB.m_texture.FrameHeight / 2 >= spriteA.m_position.y;

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

//-----------------------------------
