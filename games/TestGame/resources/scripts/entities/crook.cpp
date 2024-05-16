#include "./crook.h"
#include "../player.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/game.h"

using namespace entity_behaviors;

Crook::Crook(std::shared_ptr<Entity> entity):
    Actor(entity, typeid(Crook).name())
{
    this->health = 5; 
    this->sprite = std::static_pointer_cast<Sprite>(entity);
    this->sprite->SetAnimation("idle", false, 4);
    this->m_startPos = this->sprite->position.x;

    this->hb = Physics::CreateDynamicBody("box", 0, 0, 30, 50, true, 1);
}


void Crook::Update() 
{ 
    Actor::Update();

    auto playerBehavior = System::Game::GetBehavior<PlayerController>();

    if (playerBehavior == nullptr)
        return;

    if (playerBehavior->player.get() && std::sqrt(std::pow((this->sprite->position.x, playerBehavior->player->position.x), 2) + std::pow((this->sprite->position.y, playerBehavior->player->position.y), 2)) < 1)
    {

        this->sprite->SetAnimation("attack");

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
            // this->sprite->SetFlipX(true);
        }
        
        if (this->m_canMoveRight) {
            this->sprite->SetVelocityX(2); 
            //this->sprite->SetFlipX(false);
        } 
    }

    else 
        this->sprite->SetAnimation("idle");
    
}
