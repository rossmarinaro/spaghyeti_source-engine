#include "./gui.h"
#include "../editor.h"


void GUI::RenderCamera()
{

    ImGui::Text("viewport");

    ImGui::InputInt("width", &Game::worldWidth);
    ImGui::InputInt("height", &Game::worldHeight);
    ImGui::SliderFloat("x", (float*)&Editor::camera->m_position.x, 0.0f, -Game::worldWidth);
    ImGui::SliderFloat("y", (float*)&Editor::camera->m_position.y, 0.0f, -Game::worldHeight);
    ImGui::SliderFloat("zoom", (float*)&Editor::camera->m_zoom, -10.0f, 10.0f);
    ImGui::ColorEdit4("color", (float*)&Editor::camera->m_backgroundColor); 

    ImGui::Text("grid");

    ImGui::SliderFloat("alpha", (float*)&grid->m_alpha, 0.0f, 1.0f);

}

 