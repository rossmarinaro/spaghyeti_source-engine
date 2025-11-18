#include "./player.h"
#include "./enemy.h"
#include "../Gameplay.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/game.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/audio.h"

using namespace entity_behaviors;

PlayerController::PlayerController(std::shared_ptr<Entity> entity):
    Actor(entity, typeid(PlayerController).name(), "Player"), 
        m_canIdle(true),
        m_canJump(true),
        m_jumpGate(true),
        m_isJump(false),
        m_shootFireball(false),
        m_invincible(false),
        m_timesJumped(0),
        m_state(NONE),
        m_shadow(System::Game::CreateSprite("player.png", 0.0f, 0.0f, 55))
{ 

    health = 4; 
    magic = 10;
    cigar = 10;

    magicEnabled = false;
    cigarEnabled = false;
  
    attackBox = Physics::CreateBody(Physics::Body::Type::DYNAMIC, Physics::Body::Shape::BOX, 0, 0, 30, 40, true, 1);   
    attackBox->SetEnabled(false);

    const auto player = std::static_pointer_cast<Sprite>(entity);
    System::Game::GetScene()->GetContext().camera->StartFollow(&player->position, 500);

}


//-------------------------------------


PlayerController::~PlayerController() {
    Physics::DestroyBody(attackBox);
}


//-------------------------------------


void PlayerController::Update() 
{

    if (!active || !System::Game::GetBehavior<Gameplay>()->gameState)
        return;

    s_time += 0.1f;

    auto context = System::Game::GetScene()->GetContext();
    auto player = System::Game::GetScene()->GetEntity<Sprite>("player");
    
    if (!player)
        return;
        
    const Math::Vector2& vel = player->GetBody()->GetLinearVelocity();

    //player shadow

    if (m_shadow) 
    {
        m_shadow->SetPosition(player->position.x, 650.0f);
        m_shadow->SetAlpha(vel.y != 0 && player->position.y >= 420.0f ? 1.0f : 0.0f);
        m_shadow->SetDepth(player->depth);
        
        if (vel.y > 0)
            Time::DelayedCall(100, [this]() { m_shadow->SetFrame(55); });
        else 
            m_shadow->SetFrame(54);
    }

    //input actions

 
    if (health > 0)
    {
        //jump

        if (context.inputs->UP && m_canJump && m_jumpGate) 
        {
            m_jumpGate = false;  
            m_canIdle = false;          
            m_timesJumped++;

            SetState(JUMP); 

            Time::DelayedCall(180, [this]() { 
                m_jumpGate = true; 
                m_canIdle = true;
            });
        }   

        //move   

        else if (!m_isJump && m_canIdle) {
            if (context.inputs->LEFT) 
                SetState(WALK); 

            else if (context.inputs->RIGHT) 
                SetState(WALK, 1);

            else if (!m_attacking && player->IsContacting())
                SetState(IDLE);
        }

        if (player->IsContacting() && vel.y == 0) {
            m_timesJumped = 0; 
            m_isJump = false;            
            Time::DelayedCall(200, [this]() { m_canJump = true; });
        }

        //inputs

        if ((context.inputs->SPACE || context.inputs->RIGHT_CLICK) && m_canAttack) 
        {
            m_canAttack = false;
            m_attacking = true;

            SetState(ATTACK); 
        }

        if ((context.inputs->D || context.inputs->LEFT_CLICK) && magicEnabled && m_canAttackAlt)
        {
            m_canAttackAlt = false;
            m_attacking = true;

            SetState(ATTACK, 1); 

            Time::DelayedCall(500, [this] { m_canAttackAlt = true; });   
        }

        if ((context.inputs->W) && cigarEnabled && m_canAttackAlt)
        {
            m_canAttackAlt = false;
            m_attacking = true;

            SetState(ATTACK, 2); 

            Time::DelayedCall(500, [this] { m_canAttackAlt = true; });   
        }
    }
    
    else {
        player->SetVelocityX(0.0f); 
        player->StopAnimation();
        player->SetFrame(53);
    }

    //in air 

    if (!player->IsContacting() && vel.y != 0) {
        if (context.inputs->LEFT)
            player->SetVelocityX(-350.0f);  
        if (context.inputs->RIGHT)
            player->SetVelocityX(350.0f); 
    }

    //max vel

    if (vel.x >= 1400)
        player->SetVelocityX(300);

    if (vel.y >= 1400)
        player->SetVelocityY(300);

    //min vel

    if (vel.x <= -1400)
        player->SetVelocityX(-300);

    if (vel.y <= -1400)
        player->SetVelocityY(-300);
    

    if (m_attacking && player->IsAnimComplete()) 
    {
        m_canAttack = true; 
        
        Time::DelayedCall(100, [this] { 
            if (m_canAttack)
                m_attacking = false; 
        });
    } 

    else if (m_state != ATTACK) { 
        attackBox->SetTransform(player->position.x + 20, player->position.y + 20);
        attackBox->SetEnabled(false);  
    }

    if (!m_invincible)
       player->SetShader("sprite");
        
    player->shader.SetFloat("time", context.time->GetMilliseconds() / 4);
    
    const float posX = player->position.x;

    for (const auto& entity : System::Game::GetScene()->entities) 
    {
        if (System::Utils::str_includes(entity->name, "star") || System::Utils::str_includes(entity->name, "smoke"))
        {
            auto projectile = std::static_pointer_cast<Sprite>(entity);

            const bool dir = projectile->GetData<bool>("direction");

            projectile->SetDepth(10000);
            projectile->shader.SetFloat("time", s_time);

            if (System::Utils::str_includes(entity->name, "star") && projectile->rotation < 360)
                player->flipX ? projectile->rotation -= 30 : projectile->rotation += 30;

            if (projectile->GetBodies().size())
                projectile->GetBody()->SetTransform(dir ? projectile->position.x -= 15 : projectile->position.x += 15, projectile->position.y);

            if (System::Utils::str_includes(entity->name, "star") && (projectile->position.x >= posX + 200) || (projectile->position.x <= posX - 200)) {
                projectile->SetAnimation("glow", true, 20);
                projectile->SetShader("sprite");
            }

            if (System::Utils::str_includes(entity->name, "smoke") && (projectile->position.x >= posX + 200) || (projectile->position.x <= posX - 200)) {
                projectile->SetAnimation("destroy", true, 12);
                projectile->alpha -= 0.1f;
            }

            if ((dir && projectile->position.x >= player->position.x + 500) || (!dir && projectile->position.x >= player->position.x + 500)) 
                projectile->SetData("dead", true);
        }
    }
        
}



//-------------------------------------


bool PlayerController::DoDamage(int amount)
{
    if (!m_canDamage || m_invincible) 
        return false;

    m_canDamage = false;
    health -= amount;

    const auto player = System::Game::GetScene()->GetEntity<Sprite>(ID, true);

    if (!player)
        return false;

    player->SetTint({ 1.0f, 0.0f, 0.0f });

    Time::DelayedCall(25, [player] { player->SetAlpha(player->alpha == 1.0f ? 0.2f : 1.0f); }, 10);

    if (health <= 0) 
        player->SetAlpha(1.0f);

    Time::DelayedCall(500, [this, player] 
    { 
        if (!player)
            return false;

        player->ClearTint(); 
        player->SetAlpha(1.0f);
        
        if (health > 0)
            m_canDamage = true;

        else
            System::Game::GetBehavior<Gameplay>()->StopScene();

        return true;
    });
}   


//-------------------------------------


void PlayerController::SetState(const State& state, int option) 
{
    const auto player = System::Game::GetScene()->GetEntity<Sprite>(ID, true);
    const auto inputs = System::Game::GetScene()->GetContext().inputs;

    m_state = state; 

    //idle

    if (state == IDLE) {
        m_isJump = false;
        player->SetVelocityX(0); 
        player->SetAnimation(m_flipX ? "idle-left" : "idle-right", true);
    }

    //jump

    else if (state == JUMP) 
    {
        if (m_attacking)
            return;

        m_isJump = true;

        player->StopAnimation();
        
        if (inputs->LEFT || inputs->RIGHT) 
            player->SetImpulse(m_flipX ? -300 : 300, -1400);
        else 
        {
            player->SetFrame(m_flipX ? 25 : 23);          
            m_canJump = false;
            
            player->SetImpulseY(-1500); 
            player->SetFrame(m_flipX ? 26 : 24);

            return;
        }

        if (m_timesJumped >= 2) {
            m_canJump = false;
            player->SetFrame(m_flipX ? 26 : 24); 
        }
        else if (m_canJump)
            Time::DelayedCall(150, [this, player] { player->SetFrame(m_flipX ? 25 : 23); });
    }

    //walk

    else if (state == WALK) 
    {
        if (m_attacking || m_isJump)
            return;

        if (option == 1) {       
            player->SetVelocityX(300); 
            player->SetAnimation("walk-right", true, 10); 
            m_flipX = false;
        }

        else {
            player->SetVelocityX(-300); 
            player->SetAnimation("walk-left", true, 10); 
            m_flipX = true;
        }
    }

    //attack

    else if (state == ATTACK)
    {
        player->SetVelocityX(0);

        const auto projectileAttack = [player, this](const std::string& type) -> void {

            const std::string texture = type == "ring" ? "star" : "smoke";

            if ((type == "ring" && magicEnabled) || (type == "cigar" && cigarEnabled))
            {
                if (type == "ring")
                    player->SetAnimation(m_flipX ? "ring-left" : "ring-right", true, 10);

                if ((type == "ring" && magic > 0) || (type == "cigar" && cigar > 0))
                {
                    type == "ring" ? magic-- : cigar--;

                    if (type == "cigar")
                        player->SetAnimation(m_flipX ? "cigar-left" : "cigar-right", true, 7);

                    const auto projectile = System::Game::CreateSprite(texture + ".png", m_flipX ? player->position.x - 40 : player->position.x + 50, player->position.y + 30);

                    projectile->SetName(texture);
                    projectile->SetFlipX(m_flipX);
                    projectile->SetAnimation("fire", false, type == "ring" ? 4 : 12);
                    projectile->SetData("direction", m_flipX);

                    if (type == "ring")
                        projectile->SetShader("trippy2");

                    projectile->AddBody(Physics::CreateBody(Physics::Body::Type::DYNAMIC, Physics::Body::Shape::BOX, 0, 0, 10, 10, true, 1), { 0, 0, 10, 10 });
                }

                if (type == "cigar" && cigar <= 0) 
                    cigarEnabled = false; 

            } 
        };

        switch (option)
        {
            default:
            case 0: 

                attackBox->SetEnabled(true);

                if (m_flipX) {
                    player->SetAnimation(rand() % 5 > 3 ? "attack-left1" : "attack-left2", true, 10);
                    attackBox->SetTransform(player->position.x - 10, player->position.y + 45);
                }

                else {
                    player->SetAnimation(rand() % 5 > 3 ? "attack-right1" : "attack-right2", true, 10);
                    attackBox->SetTransform(player->position.x + 110, player->position.y + 45);
                }

            break;

            case 1: projectileAttack("ring"); break;
            case 2: projectileAttack("cigar"); break;
        }
    }

}

