#include "./elf.h"
#include "../player.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/game.h"

using namespace entity_behaviors;

Elf::Elf(std::shared_ptr<Entity> entity):
    Behavior(entity, "Elf"),
        m_rev(false),
        m_canDamage(true),
        m_canDestroy(false),
        m_canHit(false)
{
    this->health = 3; 
    this->hb = Physics::CreateDynamicBody("box", 0, 0, 30, 50, true, 1);
    this->sprite = std::static_pointer_cast<Sprite>(entity);
    this->sprite->SetAnimation("walk", false, 4);
 
    Time::setInterval(2000, [this] { this->m_rev = !this->m_rev; }); 
}

//-----------------------------------

void Elf::Update(Process::Context& context, void* scene) 
{

    this->sprite->SetVelocityX(this->m_rev ? -1 : 1); 
    this->sprite->SetFlipX(this->m_rev);

    if (this->hb)
        this->hb->SetTransform(b2Vec2(this->sprite->position.x + 180, this->sprite->position.y + 130), 0);

    auto s = static_cast<System::Scene*>(scene);
    auto playerBehavior = Behavior::GetBehavior<PlayerController>("PlayerController", s->behaviors);

    if (this->hb && b2TestOverlap(this->hb->GetFixtureList()->GetAABB(0), playerBehavior->player->bodies[0].first->GetFixtureList()->GetAABB(0)))
       playerBehavior->DoDamage(1);
    
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