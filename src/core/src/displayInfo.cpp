#if DEVELOPMENT == 1

#include "../../../build/sdk/include/context.h"
#include "./displayInfo.h"

DisplayInfo::DisplayInfo(void* _context):
    m_prev(0.0),
    m_frames(0)
{
    m_prev = ((Process::Context*)_context)->time->GetSeconds();

    fps_text = std::make_unique<Text>("FPS", 10, 0);
    fps_text->SetScale(2.0f, 1.0f);

    cam_text = std::make_unique<Text>("CAMERA", 10, 50);
    cam_text->SetScale(2.0f, 1.0f);

    entity_text = std::make_unique<Text>("ENTITIES", 10, 100);
    entity_text->SetScale(2.0f, 1.0f);
}

//update every frame

void DisplayInfo::Update(void* _context) 
{
    const auto context = ((Process::Context*)_context);

    if (!context->active)
        return;

    //FPS counter

    const float now = context->time->GetSeconds();
    
    if (m_frames < 60)
        m_frames++;

    if (now - m_prev >= 1.0f) {
        fps_text->SetText("FPS: " + std::to_string(m_frames));
        m_frames = 0;
        m_prev = now;
    }

    fps_text->Render();

    //camera stats
    
    const std::string camInfo = "CAMERA: x: " + std::to_string(-context->camera->GetPosition().x) + " y: " + std::to_string(-context->camera->GetPosition().y);
    cam_text->SetText(camInfo);
    cam_text->Render();

    //entities
    
    const std::string calls = "ENTITIES: " + std::to_string(Entity::s_count);
    entity_text->SetText(calls);
    entity_text->Render();
}

#endif