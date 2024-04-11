#include "./gui.h"
#include "../assets/assets.h"
#include "../editor.h"

#include "../../../../build/sdk/include/vendors/IconsFontAwesome5.h"


void editor::GUI::RenderAssets()
{
 

    //float arr[] = { 0.6f, 0.1f, 1.0f };
    //const char* labels[] = { "a", "b", "c" };
    //ImGui::PlotHistogram("Histogram", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 1.0f, ImVec2(0, 60), 0);
    //ImGui::Image((void*)(intptr_t) ICON_FA_FILE , ImVec2(131, 183));

    if (ImGui::MenuItem("Open", "Ctrl+A"))
        Editor::events.OpenFile();
    
    ImGui::Separator();

    for (int i = 0; i < AssetManager::images.size(); i++)  
        if (AssetManager::images[i].second != NULL)
        {

            ImGui::PushID(i);

            if (i % 12)
                ImGui::SameLine(); 

            if (ImGui::ImageButton("asset icon", (void*)(intptr_t) AssetManager::images[i].second, ImVec2(70, 70), ImVec2(0, 1), ImVec2(1, 0))) {
                Editor::selectedAsset = AssetManager::images[i].first;
                Editor::Log("Current asset selected: " + Editor::selectedAsset);
            }

            //asset tool tip

            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip(AssetManager::images[i].first.c_str());

            ImGui::PopID();
        }

        else break;

}