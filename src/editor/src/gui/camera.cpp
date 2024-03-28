#include "./gui.h"
#include "../editor.h"


void editor::GUI::RenderCamera()
{

    ImGui::Text("viewport");

    ImGui::InputInt("width", &Editor::worldWidth);
    ImGui::InputInt("height", &Editor::worldHeight); 
    ImGui::SliderFloat("x", (float*)&Editor::game->camera->m_position.x, 0.0f, -Editor::worldWidth);
    ImGui::SliderFloat("y", (float*)&Editor::game->camera->m_position.y, 0.0f, -Editor::worldHeight);
    ImGui::SliderFloat("zoom", (float*)&Editor::game->camera->m_zoom, -10.0f, 10.0f);
    ImGui::ColorEdit4("color", (float*)&Editor::game->camera->m_backgroundColor); 

    ImGui::Text("grid");

    ImGui::SliderFloat("alpha", (float*)&grid->m_alpha, 0.0f, 1.0f);
    ImGui::SliderFloat("pitch", (float*)&grid_quantity, 0.0f, 200.0f);

}

 