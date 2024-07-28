#include "./actor.h"
#include "../player.h"
#include "../ui.h"
#include "../gameplay.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/game.h"

using namespace entity_behaviors;

void Actor::Update(float offsetX, float offsetY) 
{

    if (this->hb)
        this->hb->SetTransform(b2Vec2(this->sprite->position.x * this->sprite->scale.x + offsetX, this->sprite->position.y * this->sprite->scale.y + offsetY), 0);

    auto playerBehavior = System::Game::GetBehavior<PlayerController>();

    if (this->hb && b2TestOverlap(this->hb->GetFixtureList()->GetAABB(0), playerBehavior->player->bodies[0].first->GetFixtureList()->GetAABB(0)))
        playerBehavior->DoDamage(this->m_damage);
    
    if (
        (this->hb && playerBehavior->hb->IsEnabled()) &&
        b2TestOverlap(this->hb->GetFixtureList()->GetAABB(0), playerBehavior->hb->GetFixtureList()->GetAABB(0)) && 
        this->health > 0 && 
        this->m_canDamage
    ) 
    {
        this->m_canDamage = false;
        this->m_canHit = true;
        this->health--;
        this->sprite->SetAlpha(0.75f);
        this->sprite->SetTint({ 1.0f, 0.0f, 0.0f });
    }
    
    if (this->health <= 0)
    {
        this->health = 1;
        this->sprite->SetAlpha(1.0f);
        this->sprite->SetTint({ 0.0f, 0.0f, 0.0f });

        if (System::Game::GetBehavior<GAMEPLAY>())
            System::Game::GetBehavior<GAMEPLAY>()->score++;

        Time::delayedCall(400, [this] { this->m_canDestroy = true; });
    }

    else if (!this->m_canDamage && this->m_canHit)
    {
        this->m_canHit = false;

        Time::delayedCall(700, [this] { 
            this->sprite->ClearTint(); 
            this->sprite->SetAlpha(1.0f);
            this->m_canDamage = true;
        });
    }

    if (this->m_canDestroy && this->m_isActive) 
    {
        this->m_isActive = false;
        this->m_canHit = false;
        this->m_canDestroy = false;

        System::Game::DestroyEntity(this->sprite);
        Physics::DestroyBody(this->hb);

        this->hb = nullptr;
    }
}