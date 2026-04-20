#include "./gameplay.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/game.h"
#include "C:/project_data/projects/c++/spaghyeti_source_engine/build/sdk/include/window.h"

using namespace entity_behaviors;

Gameplay::Gameplay(std::shared_ptr<Entity> entity):
    Behavior(entity->ID, typeid(Gameplay).name(), "GAMEPLAY"),
        m_charIterator(0),
        m_alpha(1.0f),
        m_startFade(false),
        m_startSequence(false),
        m_canFade(true),
        m_textContent(""),
        m_text(""),
        m_dialog_box(System::Game::CreateUISprite("dialogue window.png", 70.0f, 5.0f)),
        m_dialog_text(System::Game::CreateText("", 600.0f, 50.0f, "slkscr.ttf")),
        m_gameOver_overlay(System::Game::CreateGeom(0, 0, System::Window::s_scaleWidth, System::Window::s_scaleHeight, 2, true)),
        m_gameOver_text(System::Game::CreateText("GAME OVER", 600.0f, 400.0f))
    {
        score = 0;
        gameState = true;

        m_gameOver_text->SetAlpha(0.0f);
        m_gameOver_text->SetScale(3.0f);
        m_gameOver_text->SetTint({ 1.0f, 1.0f, 0.0f });
        m_gameOver_text->SetStroke(true, { 1.0f, 0.0f, 0.0f });
        m_gameOver_overlay->SetAlpha(0.0f);

        m_dialog_box->SetAlpha(0.0f);
        m_dialog_box->SetScale(5.2f, 3.0f);
        
        m_dialog_text->SetScale(0.7f);
        m_dialog_text->SetStroke(true, { 0.0f, 0.0f, 0.0f });
        
        Time::DelayedCall(60, [this] 
        {   
            if (!m_startSequence)
                return;

            if (m_charIterator < m_textContent.length()) {
                m_text += m_textContent[m_charIterator];
                m_charIterator++;
            }

            else {
                if (m_canFade)
                    Time::DelayedCall(2000, [this] { 
                        m_startFade = true;
                        Time::DelayedCall(500, [this] { ResetText(); });
                    });

                else 
                    ResetText();
            }
        
        }, -1); 

        //hint message

        Time::DelayedCall(5000, [this] { SetTextContent("This is a basic example to show\na platformer style game.                                              ", "hint"); });
    }

    
//----------------------------


void Gameplay::Update() 
{
    const auto dead_ent_it = std::find_if(System::Game::GetScene()->entities.begin(), System::Game::GetScene()->entities.end(), [](std::shared_ptr<Entity> ent){ return ent->GetData<bool>("dead"); });
        
    if (dead_ent_it != System::Game::GetScene()->entities.end())
        System::Game::DestroyEntity(*dead_ent_it);
        
    const auto clouds = System::Game::GetScene()->GetEntity<Sprite>("clouds");
    
    if (clouds) {
        clouds->SetAlpha(0.2f);
        clouds->texture.SetFiltering(); 
        clouds->shader.SetFloat("time", System::Game::GetScene()->GetContext().time->GetSeconds() / 20.0f); 
    }

    if (m_startFade)
        m_alpha -= 0.1f;

    if (m_startSequence && !System::Utils::str_includes(m_subject, ": "))
        m_subject = m_textContent.length() ? m_subject + ": " : "";

    m_dialog_text->SetText(m_subject + m_text);
    m_dialog_text->SetAlpha(m_startFade ? m_alpha : 1.0f);

    const auto player = System::Game::GetScene()->GetEntity<Sprite>("player");

    if (player && player->position.y >= 900 || player->position.x >= 12700)
        StopScene();

}


//----------------------------

void Gameplay::StopScene() 
{
    gameState = false;
}


//----------------------------


void Gameplay::SetTextContent(const std::string& content, const std::string& subject, bool fade) 
{
    m_startSequence = true;
    m_startFade = false;
    m_canFade = fade;
    m_textContent = content;
    m_subject = subject;
    m_dialog_box->SetAlpha(1.0f);
} 


//----------------------------


void Gameplay::ResetText() {
    m_startSequence = false;
    m_charIterator = 0;
    m_textContent = "";
    m_text = "";
    m_subject = "";
    m_alpha = 1.0f;
    m_dialog_box->SetAlpha(0.0f);
}