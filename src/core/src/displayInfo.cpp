#if DEVELOPMENT == 1

#include "../../../build/sdk/include/context.h"
#include "./displayInfo.h"

DisplayInfo::DisplayInfo(void* _context):
    m_prev(0.0),
    m_frames(0)
{
    m_prev = ((Process::Context*)_context)->time->GetSeconds();

    m_fps_text = std::make_unique<Text>("FPS", 10, 0);
    m_fps_text->SetScale(2.0f, 1.0f);
    m_fps_text->SetStatic(true);

    m_cam_text = std::make_unique<Text>("CAMERA", 10, 50);
    m_cam_text->SetScale(2.0f, 1.0f);
    m_cam_text->SetStatic(true);

    m_entity_text = std::make_unique<Text>("ENTITIES", 10, 100);
    m_entity_text->SetScale(2.0f, 1.0f);
    m_entity_text->SetStatic(true);

    m_rendered_text = std::make_unique<Text>("RENDERED", 10, 150);
    m_rendered_text->SetScale(2.0f, 1.0f);
    m_rendered_text->SetStatic(true);
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
        m_fps_text->SetText("FPS: " + std::to_string(m_frames));
        m_frames = 0;
        m_prev = now;
    }

    m_fps_text->Render();

    //camera stats
    
    const std::string camInfo = "CAMERA: x: " + std::to_string(-context->camera->GetPosition()->x) + " y: " + std::to_string(context->camera->GetPosition()->y);
    m_cam_text->SetText(camInfo);
    m_cam_text->Render();

    //entities
    
    const std::string calls = "ENTITIES: " + std::to_string(Entity::s_count);
    m_entity_text->SetText(calls);
    m_entity_text->Render();

    //rendered entities
    
    const std::string rendered_ents = "RENDERED: " + std::to_string(Entity::s_rendered);
    m_rendered_text->SetText(rendered_ents);
    m_rendered_text->Render();
}

#endif