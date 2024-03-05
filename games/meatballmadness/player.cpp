#include "../../build/include/app.h"
#include "./player.h"

void Waiter::Update(Inputs* inputs, Camera* camera)
{ 

    if (!this->sprite->bodies.size() || this->sprite->GetData<bool>("can move") == false)
        return;

    if (System::Application::inputs->m_down && this->canJump)
    {
        System::Application::game->time->delayedCall(500, [&]() { this->canJump = false; });
        this->sprite->Animate("run", true, 7);

        this->linearVelocity.y = -50000; 
    }

    else if (!this->canJump && this->sprite->bodies[0].first->GetPosition().y <= 710)
        this->linearVelocity.y = 10000;

    if (this->sprite->bodies[0].first->GetPosition().y >= 740) 
        this->canJump = true; 
 
    if (System::Application::inputs->m_right)   
    {
        this->linearVelocity.x = 10000.0f;

        this->sprite->SetFlipX(false);
        this->sprite->Animate("run", true, 7);
 
        if (this->sprite->bodies[0].first->GetLinearVelocity().x >= 30000)
            this->sprite->bodies[0].first->SetLinearVelocity(b2Vec2(30000, this->linearVelocity.y));
    } 
    
    else if (System::Application::inputs->m_left)
    {

        this->linearVelocity.x = -10000.0f;
        
        this->sprite->SetFlipX(true);
        this->sprite->Animate("run", true, 7);

        if (this->sprite->bodies[0].first->GetLinearVelocity().x <= -30000)
            this->sprite->bodies[0].first->SetLinearVelocity(b2Vec2(-30000, this->linearVelocity.y));
    }

    else
    {

        this->linearVelocity.x = 0; 
        
        if (this->canJump && this->sprite->bodies[0].first->GetLinearVelocity().y != 0) 
           this->sprite->SetFrame(0);
        
        else
            this->sprite->SetFrame(1);

        this->sprite->bodies[0].first->SetLinearVelocity(b2Vec2(0, this->linearVelocity.y));
        
    }

    this->sprite->bodies[0].first->ApplyLinearImpulse(b2Vec2(this->linearVelocity.x, this->linearVelocity.y), this->sprite->bodies[0].first->GetWorldCenter(), true);

}





