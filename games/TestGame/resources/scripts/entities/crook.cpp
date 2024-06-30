#include "./crook.h"
#include "../player.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/game.h"


entity_behaviors::Crook::Crook(std::shared_ptr<Entity> entity):
    Actor(entity, typeid(Crook).name())
{
    this->health = 5; 
    this->m_damage = 1;
    this->sprite = std::static_pointer_cast<Sprite>(entity);
    this->sprite->SetAnimation("idle", false, 4);
    this->hb = Physics::CreateDynamicBody("box", 0, 0, 50, 60, true, 1);
}


void entity_behaviors::Crook::Update() 
{ 

    Actor::Update(110.0f, 80.0f);

    auto playerBehavior = System::Game::GetBehavior<PlayerController>();

    if (glm::length(fabs(this->sprite->position.x - playerBehavior->positionX)) <= 150.0f) 
    {
        this->sprite->SetAnimation("attack", true, 4);
        this->sprite->SetVelocityX(playerBehavior->positionX > this->sprite->position.x ? 2.5f : -2.5f);
    }

    else if (glm::length(fabs(this->sprite->position.x - playerBehavior->positionX)) <= 250.0f)
    {

        this->sprite->SetAnimation("walk", true, 7);

        this->sprite->SetFlipX(playerBehavior->positionX > this->sprite->position.x);
 
        this->sprite->SetVelocityX(playerBehavior->positionX > this->sprite->position.x ? 1 : -1); 
    }

    else {
        this->sprite->SetAnimation("idle", false, 3);
        this->sprite->SetVelocityX(0);
    }

        
    
}
