#include "./gui.h"
#include "../assets/assets.h"
#include "../editor.h"

//-------------- apply currently opened folder


void SetFolder(bool isOpen, const std::string& type = "") {

    editor::AssetManager::Get()->folderSelected = isOpen;

    if (type.length())
        editor::AssetManager::Get()->currentFolder = type;
}


//--------------- display thumbnail


void displayThumbnail(const std::vector<std::pair<std::string, GLuint>>& vec) 
{

    for (int i = 0; i < vec.size(); i++)  
    {
        if (vec[i].second != NULL)
        {
            std::string folder = System::Utils::GetFileType(vec[i].first);

            if (folder == "image")
                folder = "images";

            if (folder == editor::AssetManager::Get()->currentFolder)
            {
                ImGui::PushID(i);

                if (ImGui::ImageButton("asset icon", (void*)(intptr_t) vec[i].second, ImVec2(70, 70))) { 
                    editor::AssetManager::Get()->selectedAsset = vec[i].first;
                    editor::Editor::Log("Current asset selected: " + editor::AssetManager::Get()->selectedAsset);
                }
 
                //asset tool tip

                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip(vec[i].first.c_str());

                if (i != 0 && i % 10 == 0) {
                    ImGui::PopID();
                    continue;
                }
                
                else
                    ImGui::SameLine(); 

                ImGui::PopID();
            }
        }

        else break;
    }
}


//--------------- render folder contents


void editor::GUI::RenderAssets()
{

    if (ImGui::MenuItem("Open", "Ctrl+A"))
        Editor::Get()->events->OpenFile();

    if (AssetManager::Get()->folderSelected)
        if (ImGui::MenuItem("Go Back"))
            SetFolder(false);
    
    ImGui::Separator(); 

    if (AssetManager::Get()->folderSelected)
    {
        if (AssetManager::Get()->currentFolder == "images")
            displayThumbnail(AssetManager::Get()->images);

        if (AssetManager::Get()->currentFolder == "audio") 
            displayThumbnail(AssetManager::Get()->audio);

        if (AssetManager::Get()->currentFolder == "data")
            displayThumbnail(AssetManager::Get()->data);
    }

    else 
    {
        if (ImGui::ImageButton("image", (void*)(intptr_t) Graphics::Texture2D::Get("folder src").ID/* , ImVec2(70, 70), ImVec2(0, 1), ImVec2(1, 0) */))
            SetFolder(true, "images");
 
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("images");

        ImGui::SameLine(); 

        if (ImGui::ImageButton("audio", (void*)(intptr_t) Graphics::Texture2D::Get("folder src").ID/* , ImVec2(70, 70), ImVec2(0, 1), ImVec2(1, 0) */))
            SetFolder(true, "audio");

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("audio");

        ImGui::SameLine(); 

        if (ImGui::ImageButton("data", (void*)(intptr_t) Graphics::Texture2D::Get("folder src").ID/* , ImVec2(70, 70), ImVec2(0, 1), ImVec2(1, 0) */))
            SetFolder(true, "data");

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("data");
    }

}