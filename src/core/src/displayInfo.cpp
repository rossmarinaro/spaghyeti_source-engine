#if DEVELOPMENT == 1

#include "../../../build/sdk/include/context.h"
#include "./displayInfo.h"

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
    const auto context = ((Process::Context*)_context);

    //FPS counter

    const float fps = 60.0f - context->time->delta;
    fps_text->SetText("FPS: " + std::to_string(fps));
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