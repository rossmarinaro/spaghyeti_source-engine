#include "../../src/core/src/app/app.h"
#include "./player.h"

void Waiter::Update()
{ 

    if (System::Application::inputs->m_down && this->canJump)
    {
        System::Application::game->time->delayedCall(500, [&]() { this->canJump = false; });
        this->Animate("run", true, 7);

        this->linearVelocity.y = -50000; 
    }

    else if (!this->canJump && this->m_body.self->GetPosition().y <= 710)
        this->linearVelocity.y = 10000;

    if (this->m_body.self->GetPosition().y >= 740) 
        this->canJump = true; 
 
    if (System::Application::inputs->m_right)   
    {
        this->linearVelocity.x = 10000.0f;

        this->SetFlipX(false);
        this->Animate("run", true, 7);
 
        if (this->m_body.self->GetLinearVelocity().x >= 30000)
            this->m_body.self->SetLinearVelocity(b2Vec2(30000, this->linearVelocity.y));
    }
    
    else if (System::Application::inputs->m_left)
    {

        this->linearVelocity.x = -10000.0f;
        
        this->SetFlipX(true);
        this->Animate("run", true, 7);

        if (this->m_body.self->GetLinearVelocity().x <= -30000)
            this->m_body.self->SetLinearVelocity(b2Vec2(-30000, this->linearVelocity.y));
    }

    else
    {

        this->linearVelocity.x = 0; 
        
        if (this->canJump && this->m_body.self->GetLinearVelocity().y != 0) 
           this->SetFrame(0);
        
        else
            this->SetFrame(1);

        this->m_body.self->SetLinearVelocity(b2Vec2(0, this->linearVelocity.y));
        
    }

    this->m_body.self->ApplyLinearImpulse(b2Vec2(this->linearVelocity.x, this->linearVelocity.y), this->m_body.self->GetWorldCenter(), true);

}





