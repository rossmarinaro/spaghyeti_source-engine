#pragma once

#include "./colorShiftSprite.h"

#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/behaviors.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/game.h"

using namespace entity_behaviors;


COLORSHIFTSPRITE::COLORSHIFTSPRITE(std::shared_ptr<Entity> entity):
    Behavior(entity, typeid(COLORSHIFTSPRITE).name()),
        m_r(1.0f),
        m_g(1.0f),
        m_b(1.0f),
        m_rev(false),
        m_sprite(std::static_pointer_cast<Sprite>(entity))
{         
    Time::setInterval(3000, [this] { this->m_rev = !this->m_rev; }); 
}


void COLORSHIFTSPRITE::Update()  
{
    
    if (!this->m_rev) {
        this->m_r -= 0.01f;
        this->m_b += 0.01f;
    }
    
    else {
        this->m_r += 0.01f;
        this->m_b -= 0.01f; 
    }

    this->m_sprite->SetTint({ this->m_r, this->m_g, this->m_b });
}

//-----------------------------------------

void COLORSHIFTSPRITE::SetBroadcastTint(const char* entityType, const glm::vec3& tint) {

    for (auto& ent : System::Game::GetScene()->entities) 
        if (strcmp(ent->type, entityType) == 0)
            ent->SetTint(tint);
}


