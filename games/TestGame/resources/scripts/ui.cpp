#include "./ui.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/game.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/window.h"

using namespace entity_behaviors;

UI::UI(std::shared_ptr<Entity> entity):
    Behavior(entity, typeid(UI).name()),
        m_isOpen(false),
        m_canToggle(true),
        m_score(std::static_pointer_cast<Text>(entity)),
        m_heart1(System::Game::CreateUI("heart.png", 856.821, 30)),
        m_heart2(System::Game::CreateUI("heart.png", 920.538, 30)),
        m_heart3(System::Game::CreateUI("heart.png", 984.253, 30)),
        m_menu(System::Game::CreateUI("menu.png", 135, 75)),
        m_quitText(System::Game::CreateText("QUIT", 704, 430)),
        m_returnText(System::Game::CreateText("RETURN", 673, 335)),
        m_magicBar(System::Game::CreateGeom(555, 25, 110, 20))
{ 

    this->layer = 1;
    this->score = 0;

    this->m_heart1->name = "heart1";
    this->m_heart2->name = "heart2";
    this->m_heart3->name = "heart3";

    System::Game::CreateUI("ui_box.png", 1100, 40);
    this->m_magicBar->shader = Shader::GetShader("UI"); 
    this->m_magicBar->SetDepth(90000000); 
    this->m_magicBar->SetTint({ 0.0f, 1.0f, 0.0f });

    //pause menu

    this->m_menu->SetScale(4.5f);
    this->m_menu->SetAlpha(0.0f);

    this->m_quitText->SetDepth(1000);
    this->m_quitText->SetScale(2.7f, 1.0f);
    this->m_quitText->SetAlpha(0.0f);

    this->m_returnText->SetDepth(1000);
    this->m_returnText->SetScale(2.7f, 1.0f);
    this->m_returnText->SetAlpha(0.0f);

    System::Game::GetScene()->SetInteractive(this->m_returnText);
    System::Game::GetScene()->SetInteractive(this->m_quitText);
} 

//-------------------------------------

void UI::Update() 
{
    if (this->m_magicBar->width > 0)
        this->m_magicBar->width -= 0.5f;

    //HUD
    
    this->m_score->SetText("score: " + std::to_string(this->score));

    if(System::Game::GetScene()->ListenForInteraction(this->m_returnText)) 
    {
        this->m_returnText->SetTint({ 1.0f, 0.0f, 0.0f });

        if (System::Game::GetScene()->GetContext().inputs->LEFT_CLICK)
            this->m_isOpen = false;
    }
    else 
        this->m_returnText->SetTint({ 1.0f, 1.0f, 1.0f });

    if (System::Game::GetScene()->ListenForInteraction(this->m_quitText)) {
        this->m_quitText->SetTint({ 1.0f, 0.0f, 0.0f });
    }
    else {
        this->m_quitText->SetTint({ 1.0f, 1.0f, 1.0f });
    }
    

    if (System::Game::GetScene()->GetContext().inputs->SHIFT && this->m_canToggle) 
    {
        
        this->m_canToggle = false;

        Time::delayedCall(500, [this] { 
            this->m_isOpen = !this->m_isOpen; 
            this->m_canToggle = true;
        });
    }

    //toggle menu visibility

    if (this->m_isOpen) {
        this->m_menu->SetAlpha(1.0f);
        this->m_quitText->SetAlpha(1.0f);
        this->m_returnText->SetAlpha(1.0f);
        System::Game::GetScene()->SetPause(true);
    }

    else {
        this->m_menu->SetAlpha(0.0f);
        this->m_quitText->SetAlpha(0.0f);
        this->m_returnText->SetAlpha(0.0f);
        System::Game::GetScene()->SetPause(false);
    }
}

