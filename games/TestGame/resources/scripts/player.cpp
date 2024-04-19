#include "./player.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/game.h"

using namespace entity_behaviors;
    

PlayerController::PlayerController(std::shared_ptr<Entity> entity):
    Behavior(entity, "PlayerController")
{
    this->health = 4;

    this->follow = true;  
    this->flipX = false; 
    this->canJump = true;
    this->canAttack = true;
    this->canDamage = true;
    this->attacking = false;
    this->shootFireball = false; 

    this->player = std::static_pointer_cast<Sprite>(this->entity);
    this->hb = Physics::CreateDynamicBody("box", 0, 0, 10, 10, true, 1);     

    this->heart1 = System::Game::CreateUI("heart.png", 1056.821, 30);
    this->heart2 = System::Game::CreateUI("heart.png", 1120.538, 30);
    this->heart3 = System::Game::CreateUI("heart.png", 1184.253, 30);

}

//-------------------------------------
 
PlayerController::~PlayerController() {
    Physics::DestroyBody(this->hb);
}

//-------------------------------------

void PlayerController::Update(Process::Context& context, const std::vector<std::shared_ptr<Behavior>>& behaviors) 
{ 

    if (this->follow)
        this->player->StartFollow(context.camera, 500);

    //jump

    if (context.inputs->m_up && this->canJump && this->player->bodies[0].first->GetLinearVelocity().y == 0) {               
        this->canJump = false;
        this->Jump(context.inputs);
    }

    //move   

    else if (this->player->IsContacting()) {      
        this->canJump = true;
        this->Move(context.inputs); 
    }

    //attack

    if (context.inputs->m_SPACE && this->canAttack) {
        this->canAttack = false;
        this->attacking = true;
    }

    if (this->attacking) 
        this->Attack(context.physics);

    else {
        this->hb->SetTransform(b2Vec2(this->player->m_position.x + 20, this->player->m_position.y + 20), 0);
        this->hb->SetEnabled(false);  
    }

}

//-------------------------------------

void PlayerController::Move(Inputs* inputs)
{

    if (this->attacking)
        return;

    if (inputs->m_left) {       
        this->player->SetVelocityX(-300); 
        this->player->Animate("walk-left", false, 5); 
        this->flipX = true;
    }

    else if (inputs->m_right) {
        this->player->SetVelocityX(300); 
        this->player->Animate("walk-right", false, 5); 
        this->flipX = false;
    }

    else {
        this->player->SetVelocityX(0); 
        this->player->Animate(this->flipX ? "idle-left" : "idle-right", true);
    }
}

//-------------------------------------

void PlayerController::Jump(Inputs* inputs)
{

    if (inputs->m_left || inputs->m_right)
        this->player->SetImpulse(this->flipX ? -900 : 900, -2600);

    else
        this->player->SetImpulseY(-2500);  

    //this->player->Animate(this->flipX ? "jump-left" : "jump-right");
    this->player->SetFrame(this->flipX ? 14 : 12);
}

//-------------------------------------

void PlayerController::Attack(Physics* physics)
{

    this->hb->SetEnabled(true);
    
    if (this->flipX) {
        this->player->Animate("attack-left", true, 4);
        this->hb->SetTransform(b2Vec2(this->player->m_position.x - 10, this->player->m_position.y + 45), 0);
    }

    else {
        this->player->Animate("attack-right", true, 4);
        this->hb->SetTransform(b2Vec2(this->player->m_position.x + 90, this->player->m_position.y + 45), 0);
    }

    if (this->player->IsAnimComplete()) 
    {
        this->canAttack = true; 
        this->attacking = false;
    } 

    if (this->shootFireball)
    {
        //todo: implement fireball
        //Time::delayedCall(1000, [&]() { });
    }           
}

//-------------------------------------

void PlayerController::DoDamage(int amount)
{

    if (!this->canDamage) 
        return;

    this->canDamage = false;
    this->health -= amount;

    if (this->health < 4 && this->health > 2)
        this->heart1->SetTint({ 0.0f, 0.0f, 0.0f });

    else if (this->health < 3 && this->health > 1)
        this->heart2->SetTint({ 0.0f, 0.0f, 0.0f });

    else 
        this->heart3->SetTint({ 0.0f, 0.0f, 0.0f });

    if (this->health <= 0) {
        this->health = 1;
        System::Game::StartScene("GAMEOVER"); 
    }

    this->player->SetAlpha(0.75f);
    this->player->SetTint({ 1.0f, 0.0f, 0.0f });

    Time::delayedCall(500, [&]() { 
        this->player->ClearTint(); 
        this->player->SetAlpha(1.0f);
        this->canDamage = true;
    });
    
}   



