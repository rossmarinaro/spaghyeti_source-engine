#include "../../../../build/sdk/include/game.h"
#include "./gui.h"
#include "../editor.h"

void editor::GUI::RenderLogs()
{
    
    ImGui::Text("Performance Average: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text(("active entities: " + std::to_string(System::Game::GetScene()->entities.size())).c_str());

    int rendered = 0;

    for (const auto& entity : System::Game::GetScene()->entities)
        if (entity->renderable)
            rendered++;

    ImGui::Text(("rendered entities: " + std::to_string(rendered)).c_str());

    ImGui::Separator();

    ImGui::Text("-----------::Logs::--------------------------------------------------------------------------------------------------------------------------------------------------------------------------"); 
    
    ImGui::Separator();   

    std::ifstream fs("appLog.txt");

    if (fs.is_open()) {
        std::string str((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());    
        ImGui::Text(str.c_str());
    }

}       


   



   



















