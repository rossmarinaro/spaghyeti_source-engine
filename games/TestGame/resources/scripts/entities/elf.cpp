#include "./elf.h"
#include "../player.h"

using namespace entity_behaviors;

Elf::Elf(std::shared_ptr<Entity> entity):
    Behavior(entity, "Elf")
{
    this->health = 3; 
    this->rev = false;
    this->hb = Physics::CreateDynamicBody("box", 0, 0, 30, 50, true, 1);
    this->sprite = std::static_pointer_cast<Sprite>(this->entity);
    this->sprite->SetAnimation("walk", false, 4);

    Time::delayedCall(100,[&]() { 
        Time::setInterval(2000, [=]() { 

            if (!this->isActive) 
                return;

            this->rev = !this->rev; 
        }); 
    });
}

void Elf::Update(Process::Context& context, const std::vector<std::shared_ptr<Behavior>>& behaviors) 
{ 

    this->sprite->SetVelocityX(this->rev ? -1 : 1); 
    this->sprite->SetFlipX(this->rev);
    this->hb->SetTransform(b2Vec2(this->sprite->m_position.x + 180, this->sprite->m_position.y + 130), 0);

    auto playerBehavior = Behavior::GetBehavior<PlayerController>("PlayerController", behaviors);

    if (b2TestOverlap(this->hb->GetFixtureList()->GetAABB(0), playerBehavior->player->bodies[0].first->GetFixtureList()->GetAABB(0)))
        playerBehavior->DoDamage(1);
}