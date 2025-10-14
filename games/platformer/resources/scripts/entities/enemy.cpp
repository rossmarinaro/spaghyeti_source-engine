#include "./enemy.h"
#include "./player.h"
#include "../gameplay.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/game.h"

entity_behaviors::Enemy::Enemy(std::shared_ptr<Entity> entity):
    Actor(entity, typeid(Enemy).name(), "Enemy"),
        m_key(entity->name)
{
    Initialize(entity);
}

entity_behaviors::Enemy::Enemy(std::shared_ptr<Entity> entity, const std::string& key, const std::string& name):
    Actor(entity, key, name),
        m_key(entity->name)
{
    Initialize(entity);
}

//-----------------------------------


entity_behaviors::Enemy::Enemy(std::shared_ptr<Entity> entity, const std::string& key):
    Actor(entity, key, "Enemy"),
        m_key(entity->name)
{
    Initialize(entity);
}


//-----------------------------------


void entity_behaviors::Enemy::Initialize(std::shared_ptr<Entity> entity)
{
    const auto sprite = System::Game::GetScene()->GetEntity<Sprite>(m_key);

    if (!sprite)
        return;

    sprite->SetData("alive", true);

    if (System::Utils::str_includes(entity->name, "hooligan")) {
        health = 4;
        m_damage = 1;
        hb = Physics::CreateBody(Physics::Body::Type::DYNAMIC, Physics::Body::Shape::BOX, 0, 0, 50, 60, true, 1);
    }

    if (System::Utils::str_includes(sprite->name, "pidgeon")) {
        health = 1;
        m_damage = 1;
        m_canMoveRight = true;
        sprite->SetAnimation("flap", false, 14);
        hb = Physics::CreateBody(Physics::Body::Type::DYNAMIC, Physics::Body::Shape::BOX, 0, 0, 60, 20, true, 1);
    }

    if (System::Utils::str_includes(entity->name, "pitbull")) {
        health = 2;
        m_damage = 2;
        m_canMoveLeft = true;
        m_reverse = true;
        hb = Physics::CreateBody(Physics::Body::Type::DYNAMIC, Physics::Body::Shape::BOX, 0, 0, 100, 70, true, 1);
    }

    m_startPos = sprite->position.x;
    m_active = true;
    sprite->SetData("beginDestroy", false);    

}


//-----------------------------------


void entity_behaviors::Enemy::Update()
{
    if (!active || !System::Game::GetBehavior<Gameplay>()->gameState)
        return;

    const auto sprite = System::Game::GetScene()->GetEntity<Sprite>(m_key);
    const auto playerBehavior = System::Game::GetBehavior<PlayerController>();
    const auto player = System::Game::GetScene()->GetEntity<Sprite>("player");

    if (!sprite || !player)
        return;

    //bounce off of enemy

    if (hb && (sprite->GetBody()->IsEnabled() && hb->IsEnabled()) && hb->CollidesWith(player->GetBody()))
        player->SetImpulse(System::Utils::randInt(10) > 5 ? -300.0f : 300.0f, -20.0f);

    //hooligan

    if (System::Utils::str_includes(sprite->name, "hooligan"))
    {
        if (hb)
           hb->SetTransform(sprite->position.x * sprite->scale.x + 110.0f, sprite->position.y * sprite->scale.y + 80.0f);

        if (!m_willDestroy)
        {
            sprite->SetFlipX(player->position.x > sprite->position.x && Math::distanceBetween(sprite->position.x - player->position.x) >= 10.0f);

            if (Math::distanceBetween(sprite->position.x - player->position.x) <= 80.0f) {
                sprite->SetAnimation("attack", true, 4);
                sprite->SetVelocityX(player->position.x > sprite->position.x ? 120.5f : -120.5f);
            }

            else if (Math::distanceBetween(fabs(sprite->position.x - player->position.x)) <= 150.0f) {
                sprite->SetAnimation("walk", true, 17);
                sprite->SetVelocityX(player->position.x > sprite->position.x ? 80 : -80);
            }

            else if (Math::distanceBetween(sprite->position.x - player->position.x) <= 200.0f && Math::distanceBetween(sprite->position.x - player->position.x) >= 30.0f) {
                sprite->SetAnimation("walk", true, 17);
                sprite->SetVelocityX(player->position.x > sprite->position.x ? 200 : -200);
            }

            else {
                sprite->SetAnimation("idle", false, 3);
                sprite->SetVelocityX(0);
            }
        }
        else if (sprite)
            sprite->SetAnimation("die", true, 5); 
    }

    //pitbull

    if (System::Utils::str_includes(sprite->name, "pitbull"))
    {

        if (hb)
           hb->SetTransform(sprite->position.x * sprite->scale.x + 150.0f, sprite->position.y * sprite->scale.y + 60.0f);

        if (!m_willDestroy)
        {
            //dog attacks

            if (
                (player->position.x > sprite->position.x && sprite->flipX || player->position.x < sprite->position.x && !sprite->flipX) &&
                Math::distanceBetween(fabs(sprite->position.x - player->position.x)) <= 500.0f &&
                Math::distanceBetween(fabs(sprite->position.y - player->position.y)) <= 45.0f
            )
            {
                m_canMoveLeft = false;
                m_canMoveRight = false;

                sprite->StopAnimation();
                sprite->SetFrame(16);
                sprite->SetVelocityX(player->position.x > sprite->position.x ? 250.0f : -250.0f);
                sprite->SetFlipX(player->position.x > sprite->position.x);
            }

            else
            {
                if (Math::distanceBetween(fabs(sprite->position.x - player->position.x)) <= 600.0f)
                {
                    sprite->SetAnimation("walk", false, 6);

                    if (sprite->position.x > m_startPos + 80 && !m_reverse) {
                        m_canMoveLeft = true;
                        m_canMoveRight = false;
                    }

                    if (sprite->position.x >= m_startPos + 60)
                        m_reverse = false;

                    if (sprite->position.x < m_startPos - 20 && m_reverse) {
                        m_canMoveLeft = false;
                        m_canMoveRight = true;
                    }

                    if (sprite->position.x <= m_startPos - 10)
                        m_reverse = true;

                    if (m_canMoveLeft) {
                        sprite->SetVelocityX(-50);
                        sprite->SetFlipX(false);
                    }

                    else if (m_canMoveRight) {
                        sprite->SetVelocityX(50);
                        sprite->SetFlipX(true);
                    }
                }

                else
                    sprite->SetVelocityX(0);
            }
        }

        else {
            const Math::Vector2& pos = sprite->GetBody()->GetPosition();
            sprite->GetBody()->SetTransform(pos.x, pos.y, 1.5);
            sprite->StopAnimation();
            sprite->SetFrame(17);
            sprite->SetVelocityX(0);
        }

    }

    //pidgeon

    if (System::Utils::str_includes(sprite->name, "pidgeon"))
    {

        if (hb)
            hb->SetTransform(sprite->position.x * sprite->scale.x + 50.0f, sprite->position.y * sprite->scale.y + 60.0f);

        if (!m_willDestroy)
        {
            if (Math::distanceBetween(fabs(sprite->position.x - player->position.x)) <= 600.0f)
            {
                if (sprite->position.x > m_startPos + 120) {
                    m_canMoveLeft = true;
                    m_canMoveRight = false;
                }

                if (sprite->position.x < m_startPos - 120)
                {
                    m_canMoveLeft = false;
                    m_canMoveRight = true;
                }

                if (sprite->position.x > m_startPos + 110)
                    m_reverse = true;

                if (sprite->position.x < m_startPos - 110)
                    m_reverse = false;

                if (m_canMoveLeft && m_reverse) {
                    sprite->SetVelocityX(-100);
                    sprite->SetFlipX(true);
                }

                if (m_canMoveRight) {
                    sprite->SetVelocityX(100);
                    sprite->SetFlipX(false);
                }
            }

            else
                sprite->SetVelocityX(0);

            if (Math::distanceBetween(fabs(sprite->position.x - player->position.x)) <= 80.0f && m_canAttack)
            {
                m_canAttack = false;

                const auto poo = System::Game::CreateSprite("pidgeon_poo.png", sprite->position.x + 100, sprite->position.y + 90);
                poo->SetName("pidgeon_poo");
                poo->SetData("dead", false);
                poo->AddBody(Physics::CreateBody(Physics::Body::Type::DYNAMIC, Physics::Body::Shape::BOX, sprite->position.x + 100, sprite->position.y + 90, 10, 10, false, 0, 0.5f), { 0, 0, 10, 10 });

                Time::DelayedCall(500, [this] { m_canAttack = true; });

                Time::DelayedCall(4000, [poo] {
                    if (poo)
                        poo->SetData("dead", true);
                });
                
            }
        }

        else {
            sprite->StopAnimation();
            sprite->SetFrame(9);
            sprite->SetVelocityX(0);
        }

        for (const auto& entity : System::Game::GetScene()->entities)
        {
            if (System::Utils::str_includes(entity->name, "pidgeon_poo"))
            {
                auto poo = std::static_pointer_cast<Sprite>(entity);

                if (poo->currentFrame == 2) 
                    poo->StopAnimation();

                else {
                    poo->SetDepth(10000);
                    poo->SetAnimation("splat");
                }

                if (poo && !poo->GetData<bool>("dead") && poo->GetBody()->CollidesWith(player->GetBody())) {
                    poo->SetData("dead", true);
                    playerBehavior->DoDamage(1);
                }
            }
                
        }

    }

    if (hb && m_willDestroy) {
        Physics::DestroyBody(hb);
        hb = nullptr;
    }

    //damage actor

    ListenForDamage(sprite->name);

    //damage player

    if (hb && hb->CollidesWith(player->GetBody()))
        playerBehavior->DoDamage(m_damage);

    //start destroy

    if (m_startDestroy && !sprite->GetData<bool>("beginDestroy"))
    {
        sprite->SetData("beginDestroy", true);
        sprite->SetVelocityX(0.0f);
        sprite->GetBody()->SetSensor(true);

        Time::DelayedCall(700, [this] {
            m_canDestroy = true;
            m_startDestroy = false;
        });
    }

    else if (health <= 0 && sprite->GetData<bool>("alive")) {
       sprite->SetData("alive", false);
       sprite->SetTint({ 1.0f, 0.0f, 0.0f });
       Time::DelayedCall(25, [sprite] { sprite->SetAlpha(sprite->alpha == 1.0f ? 0.2f : 1.0f); }, 15);
    }

    if (active && m_canDestroy) {
       active = false;
       m_canDestroy = false;
       System::Game::DestroyEntity(sprite);
    }

}


//-----------------------------------


bool entity_behaviors::Enemy::ListenForDamage(const std::string& key) 
{
    if (health > 0)
    {
        if (!m_canDamage)
            return false;

        //cane

        if ((hb && System::Game::GetBehavior<PlayerController>()->attackBox->IsEnabled()) && hb->CollidesWith(System::Game::GetBehavior<PlayerController>()->attackBox))
            return DoDamage(1, key);

        //projectiles
        
        else
            for (const auto& entity : System::Game::GetScene()->entities)
            {
                if (System::Utils::str_includes(entity->name, "star") || System::Utils::str_includes(entity->name, "smoke"))
                {
                    auto projectile = std::static_pointer_cast<Sprite>(entity);

                    if ((projectile && projectile->GetBodies().size()) && hb && hb->CollidesWith(projectile->GetBody())) 
                    {
                        if (System::Utils::str_includes(entity->name, "smoke") && projectile->GetBody()) {
                            
                            if (System::Utils::str_includes(entity->name, "smoke"))
                                std::static_pointer_cast<Sprite>(entity)->SetAnimation("destroy", true, 8);

                            Time::DelayedCall(350, [projectile] { projectile->SetData("dead", true); });

                        }

                        else 
                            projectile->SetData("dead", true);

                        return DoDamage(System::Utils::str_includes(entity->name, "smoke") ? 2 : 3, key);
                    }
                }   
            }
    }

    //flashes before destroying

    else if (!m_willDestroy) {
       m_willDestroy = true;
       Time::DelayedCall(350, [this] { m_startDestroy = true; });
       return true;
    } 

    return false;
}


//-----------------------------------


bool entity_behaviors::Enemy::DoDamage(int amount, const std::string& key)
{  
    auto sprite = System::Game::GetScene()->GetEntity<Sprite>(key.length() ? key : m_key);
    
    if (!sprite)
        return false;

    sprite->texture.Whiteout = 1; 

    health -= amount;
    m_canDamage = false;

    Time::DelayedCall(280, [sprite, this] { 
        
        if (sprite)
            sprite->texture.Whiteout = 0; 

        Time::DelayedCall(200, [sprite, this] { 
            
            if (!sprite) 
                return;

            if (health <= 0) 
               Time::DelayedCall(400, [sprite]() { 
                    if (sprite) 
                        sprite->SetData("dead", true); 
                });
            else
                m_canDamage = true;
            
        });

    });
    
 return true;
}


