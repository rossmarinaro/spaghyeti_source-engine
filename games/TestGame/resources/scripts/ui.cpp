#include "./ui.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/game.h"


using namespace entity_behaviors;

UI::UI(std::shared_ptr<Entity> entity):
    Behavior(entity, "UI"),
        m_text(std::static_pointer_cast<Text>(entity)),
        m_heart1(System::Game::CreateUI("heart.png", 1056.821, 30)),
        m_heart2(System::Game::CreateUI("heart.png", 1120.538, 30)),
        m_heart3(System::Game::CreateUI("heart.png", 1184.253, 30))
{ 

    this->score = 0;

    this->m_heart1->name = "heart1";
    this->m_heart2->name = "heart2";
    this->m_heart3->name = "heart3";
}


//-------------------------------------

void UI::Update() {
    
   this->m_text->SetText("score: " + std::to_string(this->score));
 
}

