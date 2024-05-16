#include "./elf.h"
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
    this->m_damage = 1;
    this->sprite = std::static_pointer_cast<Sprite>(entity);
    this->sprite->SetAnimation("walk", false, 4);
    this->m_startPos = this->sprite->position.x;

    this->hb = Physics::CreateDynamicBody("box", 0, 0, 30, 50, true, 1);
}

//-----------------------------------

void Elf::Update() 
{
    Actor::Update();

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
    
}