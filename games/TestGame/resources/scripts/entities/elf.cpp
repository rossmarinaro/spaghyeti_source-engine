#include "./elf.h"
#include "../player.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/scene.h"

using namespace entity_behaviors;

Elf::Elf(std::shared_ptr<Entity> entity):
    Behavior(entity, "Elf")
{
    this->health = 3; 
    this->m_rev = false;
    this->hb = Physics::CreateDynamicBody("box", 0, 0, 30, 50, true, 1);
    this->sprite = std::static_pointer_cast<Sprite>(entity);
    this->sprite->SetAnimation("walk", false, 4);

    Time::delayedCall(100,[&]() { 
        Time::setInterval(2000, [=]() { 

            if (!this->m_isActive) 
                return;

            this->m_rev = !this->m_rev; 
        }); 
    });
}

void Elf::Update(Process::Context& context, void* scene) 
{ 

    this->sprite->SetVelocityX(this->m_rev ? -1 : 1); 
    this->sprite->SetFlipX(this->m_rev);
    this->hb->SetTransform(b2Vec2(this->sprite->position.x + 180, this->sprite->position.y + 130), 0);

    auto s = static_cast<System::Scene*>(scene);
    auto playerBehavior = Behavior::GetBehavior<PlayerController>("PlayerController", s->behaviors);

    if (b2TestOverlap(this->hb->GetFixtureList()->GetAABB(0), playerBehavior->player->bodies[0].first->GetFixtureList()->GetAABB(0)))
        playerBehavior->DoDamage(1);
}