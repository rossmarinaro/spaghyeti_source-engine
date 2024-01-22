#include "./gui.h"
#include "../editor.h"


void GUI::RenderCamera()
{
    
    ImGui::SliderFloat("x", (float*)&Editor::camera->m_position.x, -500.0f, 500.0f);
    ImGui::SliderFloat("y", (float*)&Editor::camera->m_position.y, -500.0f, 500.0f);
    ImGui::SliderFloat("zoom", (float*)&Editor::camera->m_zoom, -10.0f, 10.0f);
    ImGui::ColorEdit4("color", (float*)&Editor::camera->m_backgroundColor); 
}

