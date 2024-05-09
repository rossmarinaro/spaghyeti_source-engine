#include "./elf.h"
#include "../player.h"
#include "../ui.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/game.h"

using namespace entity_behaviors;

Elf::Elf(std::shared_ptr<Entity> entity):
    Actor(entity, typeid(Elf).name()),
        m_canMoveLeft(false),
        m_canMoveRight(true),
        m_reverse(false),
        m_startPos(0)
{

    this->health = 3; 
    this->sprite = std::static_pointer_cast<Sprite>(entity);
    this->sprite->SetAnimation("walk", false, 4);
    this->m_startPos = this->sprite->position.x;

    this->hb = Physics::CreateDynamicBody("box", 0, 0, 30, 50, true, 1);
}

//-----------------------------------

void Elf::Update() 
{

    if (this->sprite->position.x > this->m_startPos - 80) {
        this->m_canMoveLeft = true;
        this->m_canMoveRight = false;
    }

    if (this->sprite->position.x == this->m_startPos - 80)
        this->m_reverse = false;

    if (this->sprite->position.x < this->m_startPos + 60 && !this->m_reverse) {
        this->m_canMoveLeft = false;
        this->m_canMoveRight = true;
    }

    if (this->sprite->position.x == this->m_startPos + 60)
        this->m_reverse = true;

    if (this->m_canMoveLeft) {
        this->sprite->SetVelocityX(-2); 
        this->sprite->SetFlipX(true);
    }
    
    if (this->m_canMoveRight) {
        this->sprite->SetVelocityX(2); 
        this->sprite->SetFlipX(false);
    }
    
    if (this->hb)
        this->hb->SetTransform(b2Vec2(this->sprite->position.x * this->sprite->scale.x + 50, this->sprite->position.y * this->sprite->scale.y + 60), 0);

    auto playerBehavior = Behavior::GetBehavior<PlayerController>(System::Game::GetScene()->behaviors);

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
        Behavior::GetBehavior<UI>(System::Game::GetScene()->behaviors)->score++;

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