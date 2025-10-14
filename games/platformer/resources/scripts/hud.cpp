#include "./hud.h"
#include "./Gameplay.h"
#include "./entities/player.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/game.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/window.h"

using namespace entity_behaviors;

HUD::HUD(std::shared_ptr<Entity> entity):
    Behavior(entity->ID, typeid(HUD).name(), "HUD"),
        m_isOpen(false),
        m_canToggle(true)
{ 

    layer = 1;

    const auto livesText = System::Game::CreateText("", 335, 35);
    const auto overlay = System::Game::CreateGeom(0, 0, System::Window::s_scaleWidth, System::Window::s_scaleHeight, 2, true);
    const auto quitText = System::Game::CreateText("QUIT", 704, 430);
    const auto returnText = System::Game::CreateText("RETURN", 673, 335);
    const auto leftBorder = System::Game::CreateGeom(1, 0, 10, System::Window::s_scaleHeight, 2, true);
    const auto rightBorder = System::Game::CreateGeom(System::Window::s_scaleWidth - 9, 0, 10, System::Window::s_scaleHeight, 2, true);
    const auto topBorder = System::Game::CreateGeom(0, 1, System::Window::s_scaleWidth, 10, 2, true);
    const auto bottomBorder = System::Game::CreateGeom(0, System::Window::s_scaleHeight - 10, System::Window::s_scaleWidth, 10, 2, true);
    const auto heart1 = System::Game::CreateUI("heart.png", 56.821, 30);
    const auto heart2 = System::Game::CreateUI("heart.png", 120.538, 30);
    const auto heart3 = System::Game::CreateUI("heart.png", 184.253, 30);

    livesText->SetName("UI_lives"); 
    overlay->SetName("UI_overlay");
    quitText->SetName("UI_quit_text");
    returnText->SetName("UI_return_text");
    leftBorder->SetName("UI_left_border");
    rightBorder->SetName("UI_right_border");
    topBorder->SetName("UI_top_border");
    bottomBorder->SetName("UI_bottom_border");
    heart1->SetName("heart1");
    heart2->SetName("heart2");
    heart3->SetName("heart3");

    livesText->SetScale(2.0f);
    livesText->SetTint({ 0.0f, 1.0f, 0.0f });

    livesText->SetStatic(true);
    quitText->SetStatic(true);
    returnText->SetStatic(true);

    System::Game::CreateUI("sv_icon.png", 270, 30);

    const auto cigarIcon = System::Game::CreateUI("cigar UI.png", 870, 30);
    cigarIcon->SetName("cigar icon");

    const auto magicBar_base = System::Game::CreateUI("ui_box.png", 1140, 40);
    const auto magicBar = System::Game::CreateGeom(1150, 50, 110, 20, 2, true);

    magicBar_base->SetName("UI_magic_bar_base");
    magicBar->SetName("UI_magic_bar");
    
    magicBar->SetTint({ 0.0f, 1.0f, 0.0f });

    //pause menu

    leftBorder->SetTint({ 0.49f, 0.0f, 0.85f });
    rightBorder->SetTint({ 0.49f, 0.0f, 0.85f });
    topBorder->SetTint({ 0.49f, 0.0f, 0.85f });
    bottomBorder->SetTint({ 0.49f, 0.0f, 0.85f });
 
    overlay->SetAlpha(0.75f);
    overlay->SetTint({ 0.0f, 0.0f, 0.0f });

    quitText->SetScale(2.7f, 1.0f);
    quitText->SetAlpha(0.0f);

    returnText->SetScale(2.7f, 1.0f);
    returnText->SetAlpha(0.0f);

    System::Game::GetScene()->SetInteractive(returnText);
    System::Game::GetScene()->SetInteractive(quitText);
} 


//-------------------------------------


void HUD::Update() 
{
    auto playerController = System::Game::GetBehavior<PlayerController>();

    const auto context = System::Game::GetScene()->GetContext();

    const auto livesText = System::Game::GetScene()->GetEntity<Text>("UI_lives");
    const auto quitText = System::Game::GetScene()->GetEntity<Text>("UI_quit_text");
    const auto returnText = System::Game::GetScene()->GetEntity<Text>("UI_return_text");

    const auto overlay = System::Game::GetScene()->GetEntity<Geometry>("UI_overlay");
    const auto leftBorder = System::Game::GetScene()->GetEntity<Geometry>("UI_left_border");
    const auto rightBorder = System::Game::GetScene()->GetEntity<Geometry>("UI_right_border");
    const auto topBorder = System::Game::GetScene()->GetEntity<Geometry>("UI_top_border");
    const auto bottomBorder = System::Game::GetScene()->GetEntity<Geometry>("UI_bottom_border");
    const auto magicBar = System::Game::GetScene()->GetEntity<Geometry>("UI_magic_bar");

    const auto heart1 = System::Game::GetScene()->GetEntity<Sprite>("heart1");
    const auto heart2 = System::Game::GetScene()->GetEntity<Sprite>("heart2");
    const auto heart3 = System::Game::GetScene()->GetEntity<Sprite>("heart3");
    const auto cigarIcon = System::Game::GetScene()->GetEntity<Sprite>("cigar icon");
    const auto magicBar_base = System::Game::GetScene()->GetEntity<Sprite>("UI_magic_bar_base");
    
    if (!playerController)
        return;

    //health

    switch (playerController->health) {
        case 4:        
            heart1->SetTint({ 1.0f, 1.0f, 1.0f });
            heart2->SetTint({ 1.0f, 1.0f, 1.0f });
            heart3->SetTint({ 1.0f, 1.0f, 1.0f });
        break;
        case 3:
            heart1->SetTint({ 1.0f, 1.0f, 1.0f });
            heart2->SetTint({ 1.0f, 1.0f, 1.0f });
            heart3->SetTint({ 0.0f, 0.0f, 0.0f });
        break;
        case 2:
            heart1->SetTint({ 1.0f, 1.0f, 1.0f });
            heart2->SetTint({ 0.0f, 0.0f, 0.0f });
            heart3->SetTint({ 0.0f, 0.0f, 0.0f });
        break;
        case 1:
            heart1->SetTint({ 0.0f, 0.0f, 0.0f });
            heart2->SetTint({ 0.0f, 0.0f, 0.0f });
            heart3->SetTint({ 0.0f, 0.0f, 0.0f });
        break;
    }

    //cigar

    cigarIcon->SetAlpha(playerController->cigarEnabled ? 1.0f : 0.0f);

    switch (playerController->cigar) 
    {
        case 10: cigarIcon->SetFrame(0); break;
        case 9: cigarIcon->SetFrame(1); break;
        case 8: cigarIcon->SetFrame(2); break;
        case 7: cigarIcon->SetFrame(3); break;
        case 6: cigarIcon->SetFrame(4); break;
        case 5: cigarIcon->SetFrame(5); break;
        case 4: cigarIcon->SetFrame(6); break;
        case 3: cigarIcon->SetFrame(7); break;
        case 2: cigarIcon->SetFrame(8); break;
        case 1: cigarIcon->SetFrame(9); break;
    }

    if (playerController->cigar <= 0)
       cigarIcon->SetAlpha(0);

    if (playerController->cigar >= 10)
        playerController->cigar = 10;

    //magic ring

    magicBar_base->SetAlpha(playerController->magicEnabled ? 1.0f : 0.0f);
    magicBar->SetAlpha(playerController->magicEnabled ? 1.0f : 0.0f);

    switch (playerController->magic) 
    {
        case 10:        
            magicBar->width = 110;
            playerController->magic = 10; 
        break;
        case 9: magicBar->width = 100; break;
        case 8: magicBar->width = 95; break;
        case 7: magicBar->width = 80; break;
        case 6: magicBar->width = 75; break;
        case 5: magicBar->width = 60; break;
        case 4: magicBar->width = 55; break;
        case 3: magicBar->width = 40; break;
        case 2: magicBar->width = 25; break;
        case 1: magicBar->width = 10; break;
    }

    if (playerController->magic <= 0)
        magicBar->width = 0;    

    if (playerController->magic >= 10)
        playerController->magic = 10;  

    livesText->SetText("x1");

    //HUD
    
    System::Game::GetScene()->GetEntity<Text>("score")->SetText("score: " + std::to_string(System::Game::GetBehavior<Gameplay>()->score));

    if(System::Game::GetScene()->ListenForInteraction(returnText)) {
        returnText->SetTint({ 1.0f, 0.0f, 0.0f });

        if (context.inputs->LEFT_CLICK)
            m_isOpen = false;
    }

    else 
        returnText->SetTint({ 1.0f, 1.0f, 1.0f });

    if (System::Game::GetScene()->ListenForInteraction(quitText)) 
        quitText->SetTint({ 1.0f, 0.0f, 0.0f });

    else 
        quitText->SetTint({ 1.0f, 1.0f, 1.0f });

    if ((context.inputs->TAB || context.inputs->ENTER) && m_canToggle) 
    {   
        m_canToggle = false;

        Time::DelayedCall(500, [this] { 
            m_isOpen = !m_isOpen; 
            m_canToggle = true;
        });
    }

    //toggle pause menu visibility

    if (m_isOpen) {
        topBorder->SetEnabled(true);
        bottomBorder->SetEnabled(true);
        leftBorder->SetEnabled(true);
        rightBorder->SetEnabled(true);
        overlay->SetEnabled(true);
        quitText->SetAlpha(1.0f);
        returnText->SetAlpha(1.0f);

        System::Game::GetScene()->SetPause(true);
    }

    else {
        topBorder->SetEnabled(false);
        bottomBorder->SetEnabled(false);
        leftBorder->SetEnabled(false);
        rightBorder->SetEnabled(false);
        overlay->SetEnabled(false);
        quitText->SetAlpha(0.0f);
        returnText->SetAlpha(0.0f);

        System::Game::GetScene()->SetPause(false);
    }
}

