#include "./ui.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/game.h"


using namespace entity_behaviors;

UI::UI(std::shared_ptr<Entity> entity):
    Behavior(entity, "UI"),
        m_isOpen(false),
        m_canToggle(true),
        m_score(std::static_pointer_cast<Text>(entity)),
        m_quitText(System::Game::CreateText("QUIT", 649, 423)),
        m_heart1(System::Game::CreateUI("heart.png", 1056.821, 30)),
        m_heart2(System::Game::CreateUI("heart.png", 1120.538, 30)),
        m_heart3(System::Game::CreateUI("heart.png", 1184.253, 30)),
        m_menu(System::Game::CreateUI("menu.png", 34, 22))
{ 

    this->score = 0;

    this->m_heart1->name = "heart1";
    this->m_heart2->name = "heart2";
    this->m_heart3->name = "heart3";

    this->m_menu->SetScale(12.0f, 10.0f);
    this->m_menu->SetAlpha(0.0f);

    this->m_quitText->SetDepth(1000);
    this->m_quitText->SetScale(2.7f, 1.0f);

    System::Game::GetScene()->virtual_buttons = { this->m_heart1, m };
}


//-------------------------------------

void UI::Update() {
    
   this->m_score->SetText("score: " + std::to_string(this->score));

    if (System::Game::GetScene()->GetContext().inputs->SHIFT && this->m_canToggle) 
    {
        
        this->m_canToggle = false;

        Time::delayedCall(500, [this] { 
            this->m_isOpen = !this->m_isOpen; 
            this->m_canToggle = true;
        });
    }

    if (this->m_isOpen) {
        this->m_menu->SetAlpha(1.0f);
        this->m_quitText->SetAlpha(1.0f);
    }
    else {
        this->m_menu->SetAlpha(0.0f);
        this->m_quitText->SetAlpha(0.0f);
    }
}

