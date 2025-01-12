#if DEVELOPMENT == 1

#include "../../../build/sdk/include/window.h"
#include "../../../build/sdk/include/context.h"
#include "../../../build/sdk/include/displayInfo.h"

DisplayInfo::DisplayInfo()
{
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
    auto context = ((Process::Context*)_context);

    //FPS counter

    float fps = 60.0f - context->time->m_delta;
    fps_text->SetText("FPS: " + std::to_string(fps));
    fps_text->Render(System::Window::s_scaleWidth, System::Window::s_scaleHeight);

    //camera stats
    
    const std::string camInfo = "CAMERA: x: " + std::to_string(-context->camera->GetPosition().x) + " y: " + std::to_string(-context->camera->GetPosition().y);
    cam_text->SetText(camInfo);
    cam_text->Render(System::Window::s_scaleWidth, System::Window::s_scaleHeight);

    //entities
    
    const std::string calls = "ENTITIES: " + std::to_string(Entity::s_count);
    entity_text->SetText(calls);
    entity_text->Render(System::Window::s_scaleWidth, System::Window::s_scaleHeight);
}

#endif