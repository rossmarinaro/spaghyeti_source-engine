#include "./gui.h"
#include "../editor.h"

void editor::GUI::RenderLogs()
{
    
    ImGui::Text("Performance Average: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::Text("-----------::Logs::--------------------------------------------------------------------------------------------------------------------------------------------------------------------------"); 
    ImGui::Separator();   

    std::ifstream fs("appLog.txt");

    if (fs.is_open())
    {
        std::string str((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());    
        
        ImGui::Text(str.c_str());
    }

}       


   



   



















