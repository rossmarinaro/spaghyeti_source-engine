#include "./spawnItem.h"
#include "./gameplay.h"
#include "./entities/player.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/game.h"

using namespace entity_behaviors;

SPAWNITEM::SPAWNITEM(std::shared_ptr<Entity> entity):
    Behavior(entity->ID, typeid(SPAWNITEM).name(), "SPAWNITEM"),
        m_name(entity->name),
        m_canSpawn(true),
        m_canDestroyBox(true),
        m_startDestroy(true),
        m_canTriggerDestroy(false),
        m_boxDestroyed(false),
        m_canDestroyItem(false),
        m_canTimeout(false),
        m_canFadeItem(false),
        m_rev(false)
{

    const auto sprite = System::Game::GetScene()->GetEntity<Sprite>(ID, true);

    if (System::Utils::str_includes(sprite->name, "coin")) {
        sprite->SetAnimation("spin", false, 8);
        sprite->SetData("type", "coin");
    }

    if (System::Utils::str_includes(sprite->name, "box")) 
    {
        const auto item = System::Game::CreateSprite("items.png", 0, 0);

        item->SetName("item_box" + ID);
        item->SetAlpha(0);
        item->AddBody(Physics::CreateBody(Physics::Body::Type::DYNAMIC, Physics::Body::Shape::BOX, sprite->position.x + 40, sprite->position.y + 40, 20, 20, true, 1), { 20, 20, 0, 0 });
    }

    else
        sprite->AddBody(Physics::CreateBody(Physics::Body::Type::DYNAMIC, Physics::Body::Shape::BOX, sprite->position.x, sprite->position.y, 20, 20, true, 1), { 20, 20, 0, 0 });
}


//-----------------------------------


void SPAWNITEM::Update()
{ 
    auto gamePlay = System::Game::GetBehavior<Gameplay>();
    auto playerBehavior = System::Game::GetBehavior<PlayerController>();
    auto sprite = System::Game::GetScene()->GetEntity<Sprite>(ID, true);
    
    const auto player = System::Game::GetScene()->GetEntity<Sprite>("player");
    const auto item = System::Game::GetScene()->GetEntity<Sprite>(("item_box" + ID));
    
    if (!player)
        return;

    //if item not in box / player collects item

    if ((sprite && player) && !System::Utils::str_includes(sprite->name, "box") && sprite->GetBodies().size() && sprite->GetBody()->CollidesWith(player->GetBody()))
    {
        int value = 0;

        if ((std::string)sprite->GetData<const char*>("type") == "coin")
            value = 1;

        if ((std::string)sprite->GetData<const char*>("type") == "goldbar")
            value = 5;

        if ((std::string)sprite->GetData<const char*>("type") == "jackpot")
            value = 10;

        gamePlay->score += value;

        System::Game::DestroyEntity(sprite);
        sprite = nullptr;
        return;
    }


    //item was in box / player collects item

    if (sprite && !sprite->GetBody()->IsEnabled() && item && item->GetBodies().size() && item->GetBody()->CollidesWith(player->GetBody()))
    {

        if ((std::string)item->GetData<const char*>("type") == "pasta")
            playerBehavior->health++;

        if ((std::string)item->GetData<const char*>("type") == "caviar")
            playerBehavior->health = 4;

        if ((std::string)item->GetData<const char*>("type") == "magic1")
            playerBehavior->magic += 5;

        if ((std::string)item->GetData<const char*>("type") == "magic2")
            playerBehavior->magic = 10;

        if ((std::string)item->GetData<const char*>("type") == "ring") {
            playerBehavior->magic = 10;
            if (!playerBehavior->magicEnabled)
                playerBehavior->magicEnabled = true;
        }

        if ((std::string)item->GetData<const char*>("type") == "cigar") {
            playerBehavior->cigar = 10;
            if (!playerBehavior->cigarEnabled)
                playerBehavior->cigarEnabled = true;
        }

        if ((std::string)item->GetData<const char*>("type") == "goldbar")
            gamePlay->score += 5;

        if ((std::string)item->GetData<const char*>("type") == "jackpot")
            gamePlay->score += 10;

        if ((std::string)item->GetData<const char*>("type") == "martini") {
            System::Game::GetScene()->GetEntity<Sprite>("player")->SetShader("trippy2");
            playerBehavior->SetInvincible();
            Time::DelayedCall(5000, [playerBehavior] { playerBehavior->SetInvincible(false); });
        }

        CleanUp();
    }


    //sprite is box


    //break box

    if (m_canDestroyBox && sprite && System::Utils::str_includes(sprite->name, "box") && (sprite->GetBodies().size() && playerBehavior->attackBox->IsEnabled()) && sprite->GetBody()->CollidesWith(playerBehavior->attackBox))
    {
        m_canDestroyBox = false;
        sprite->SetAnimation("break", false, 8);

        Time::DelayedCall(500, [this] { m_boxDestroyed = true; });
    }

    if (m_boxDestroyed && m_canSpawn && sprite && sprite->GetBodies().size())
    {
        m_canSpawn = false;

        switch(rand() % 8)
        {
            case 0: default:
                item->SetFrame(1);
                item->SetData("type", "goldbar");
            break;
            case 1:
                item->SetFrame(2);
                item->SetData("type", "pasta");
            break;
            case 2:
                item->SetFrame(3);
                item->SetData("type", "caviar");
            break;
            case 3:
                item->SetFrame(4);
                item->SetData("type", "jackpot");
            break;
            case 4:

                if (playerBehavior->magicEnabled) {
                    item->SetFrame(5);
                    item->SetData("type", "magic1");
                }

                else if (!playerBehavior->cigarEnabled) {
                    item->SetFrame(0);
                    item->SetData("type", "cigar");
                }

                else if (!playerBehavior->magicEnabled) {
                    item->SetAnimation("glow", true, 5);
                    item->SetData("type", "ring");
                }

            break;
            case 5:

                if (playerBehavior->magicEnabled) {
                    item->SetFrame(6);
                    item->SetData("type", "magic2");
                }

                else if (!playerBehavior->cigarEnabled) {
                    item->SetFrame(0);
                    item->SetData("type", "cigar");
                }

                else if (!playerBehavior->magicEnabled) {
                    item->SetAnimation("glow", true, 5);
                    item->SetData("type", "ring");
                }

            break;
            case 6:
     
                item->SetFrame(7);
                item->SetData("type", "martini");
   
            break;
            case 7:

                if (!playerBehavior->magicEnabled) {
                    item->SetAnimation("glow", true, 5);
                    item->SetData("type", "ring");
                }

            break;
            case 8:
                if (!playerBehavior->cigarEnabled) {
                    item->SetFrame(0);
                    item->SetData("type", "cigar");
                }
                else {
                    item->SetFrame(7);
                    item->SetData("type", "martini");
                }
            break;
      
        }

        sprite->SetAlpha(0);
        sprite->GetBody()->SetEnabled(false);

        Time::DelayedCall(5000, [this] { m_canTimeout = true; });

    }

    //item timeout

    if (item)
    {
        if (m_canTimeout) 
        {
            Time::DelayedCall(100, [this] { m_rev = !m_rev; }, 30);

            if (m_startDestroy)
            {
                m_startDestroy = false;
                m_canTimeout = false;
                m_canFadeItem = true;

                Time::DelayedCall(3000, [this] { m_canDestroyItem = true; });
            }
        }

        if (m_boxDestroyed) {
            if (!m_canFadeItem)
                item->SetAlpha(1);
            else
                item->SetAlpha(m_rev ? 0.45f : 0.85f);
        }

        if (m_canDestroyItem) 
            CleanUp();
    }

}


//----------------------------------------


void SPAWNITEM::CleanUp()
{
    auto sprite = System::Game::GetScene()->GetEntity<Sprite>(m_name);
    auto item = System::Game::GetScene()->GetEntity<Sprite>("item_box" + ID);

    if (item) {
        System::Game::DestroyEntity(item);
        item = nullptr;
    }

    if (sprite) {
        System::Game::DestroyEntity(sprite);
        sprite = nullptr;
    }
}

