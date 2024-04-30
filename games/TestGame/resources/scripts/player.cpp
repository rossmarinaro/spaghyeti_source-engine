#include "./player.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/game.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/audio.h"

using namespace entity_behaviors;

PlayerController::PlayerController(std::shared_ptr<Entity> entity):
    Behavior(entity, "PlayerController"),
        m_alive(true),
        m_active(true),
        m_follow(true),  
        m_flipX(false), 
        m_canJump(true),
        m_canAttack(true),
        m_canDamage(true),
        m_attacking(false),
        m_shootFireball(false),
        m_health(4),
        m_heart1(System::Game::CreateUI("heart.png", 1056.821, 30)),
        m_heart2(System::Game::CreateUI("heart.png", 1120.538, 30)),
        m_heart3(System::Game::CreateUI("heart.png", 1184.253, 30))
{ 
    this->player = std::static_pointer_cast<Sprite>(entity); 
    this->hb = Physics::CreateDynamicBody("box", 0, 0, 10, 10, true, 1);   
}

//-------------------------------------
 
PlayerController::~PlayerController() {
    Physics::DestroyBody(this->hb);
}

//-------------------------------------

void PlayerController::Update(Process::Context& context, void* scene) 
{

    if (this->m_follow)
        this->player->StartFollow(context.camera, 500);

    //jump

    if (context.inputs->UP && this->m_canJump && this->player->bodies[0].first->GetLinearVelocity().y == 0) {               
        this->m_canJump = false;
        this->Jump(context.inputs);
    }

    //move   

    else if (this->player->IsContacting()) {      
        this->m_canJump = true;
        this->Move(context.inputs); 
    }

    //attack

    if (context.inputs->SPACE && this->m_canAttack) {
        this->m_canAttack = false;
        this->m_attacking = true;
    }

    if (this->m_attacking) 
        this->Attack(context.physics);

    else {
        this->hb->SetTransform(b2Vec2(this->player->position.x + 20, this->player->position.y + 20), 0);
        this->hb->SetEnabled(false);  
    }

    if (!this->m_active.load()) {
        this->m_active = true;
        System::Game::StartScene("GAMEOVER");
        return;
    }

    if (!this->m_alive) {
        this->player->SetVelocity(0.0f, 0.0f);
        context.camera->Fade(0.1f, "in");

    }

        
}

//-------------------------------------

void PlayerController::Move(Inputs* inputs)
{

    if (this->m_attacking || !this->m_alive)
        return;

    if (inputs->LEFT) {       
        this->player->SetVelocityX(-300); 
        this->player->Animate("walk-left", false, 5); 
        this->m_flipX = true;
    }

    else if (inputs->RIGHT) {
        this->player->SetVelocityX(300); 
        this->player->Animate("walk-right", false, 5); 
        this->m_flipX = false;
    }

    else {
        this->player->SetVelocityX(0); 
        this->player->Animate(this->m_flipX ? "idle-left" : "idle-right", true);
    }
}

//-------------------------------------

void PlayerController::Jump(Inputs* inputs)
{

    if (inputs->LEFT || inputs->RIGHT)
        this->player->SetImpulse(this->m_flipX ? -900 : 900, -2600);

    else
        this->player->SetImpulseY(-2500);  

    //this->player->Animate(this->m_flipX ? "jump-left" : "jump-right");
    this->player->SetFrame(this->m_flipX ? 14 : 12);
}

//-------------------------------------

void PlayerController::Attack(Physics* physics)
{

    this->hb->SetEnabled(true);
    
    if (this->m_flipX) {
        this->player->Animate("attack-left", true, 4);
        this->hb->SetTransform(b2Vec2(this->player->position.x - 10, this->player->position.y + 45), 0);
    }

    else {
        this->player->Animate("attack-right", true, 4);
        this->hb->SetTransform(b2Vec2(this->player->position.x + 90, this->player->position.y + 45), 0);
    }

    if (this->player->IsAnimComplete()) 
    {
        this->m_canAttack = true; 
        this->m_attacking = false;
    } 

    if (this->m_shootFireball)
    {
        //todo: implement fireball
        //Time::delayedCall(1000, [&]() { });
    }           
}

//-------------------------------------

void PlayerController::DoDamage(int amount)
{

    if (!this->m_canDamage) 
        return;

    this->m_canDamage = false;
    this->m_health -= amount;

    if (this->m_health < 4 && this->m_health > 2)
        this->m_heart1->SetTint({ 0.0f, 0.0f, 0.0f });

    else if (this->m_health < 3 && this->m_health > 1)
        this->m_heart2->SetTint({ 0.0f, 0.0f, 0.0f });

    else 
        this->m_heart3->SetTint({ 0.0f, 0.0f, 0.0f });

    this->player->SetAlpha(0.75f);
    this->player->SetTint({ 1.0f, 0.0f, 0.0f });

    if (this->m_health <= 0 && this->m_alive) 
    {
        this->m_alive = false;   
        System::Audio::play("error.flac", false, 1.000000);
        this->player->SetTint({ 0.0f, 0.0f, 0.0f });
        Time::delayedCall(500, [this] { this->m_active = false; });
    }

    else if (this->m_health > 0)
        Time::delayedCall(500, [this] { 
            this->player->ClearTint(); 
            this->player->SetAlpha(1.0f);
            this->m_canDamage = true;
        });
    
}   



