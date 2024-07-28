#include "./gui.h"
#include "../editor.h"


void editor::GUI::RenderCamera()
{

    ImGui::Text("viewport");
    ImGui::Separator();

    ImGui::InputInt("width", &Editor::worldWidth);
    ImGui::InputInt("height", &Editor::worldHeight); 
    ImGui::SliderFloat("x", (float*)&Editor::game->camera->position.x, 0.0f, -Editor::worldWidth);
    ImGui::SliderFloat("y", (float*)&Editor::game->camera->position.y, 0.0f, -Editor::worldHeight);
    ImGui::SliderFloat("zoom", (float*)&Editor::game->camera->zoom, -10.0f, 10.0f);
    ImGui::SliderFloat("vignette", &Editor::vignetteVisibility, 0.0f, 1.0f);
    ImGui::ColorEdit4("color", (float*)&Editor::game->camera->backgroundColor); 

    ImGui::Text("grid");
    ImGui::Separator();

    ImGui::SliderFloat("alpha", (float*)&s_grid->alpha, 0.0f, 1.0f);
    ImGui::SliderFloat("pitch", (float*)&s_grid_quantity, 0.0f, 200.0f);

    //vignette visibility

    Editor::game->currentScene->vignette->SetAlpha(Editor::vignetteVisibility);

}

 