#include "./gui.h"
#include "../assets/assets.h"
#include "../editor.h"

static int _thumbnail_begin = 0, 
           _thumbnail_end = 20;

//-------------- apply currently opened folder


void SetFolder(bool isOpen, const std::string& type = "") {

    editor::AssetManager::Get()->folderSelected = isOpen;

    if (type.length())
        editor::AssetManager::Get()->currentFolder = type;
}


//--------------- display thumbnail


void editor::GUI::displayThumbnail(const std::vector<std::pair<std::string, unsigned int>>& vec) 
{
    if (vec.size()) 
    {
        if (_thumbnail_begin < vec.size()) 
        {
            if (ImGui::Button("next")) 
            {
                if (_thumbnail_end < vec.size() - 5) {
                    _thumbnail_begin += 5; 
                    _thumbnail_end += 5;
                }
                else if (_thumbnail_end < vec.size()) {
                    _thumbnail_begin++;
                    _thumbnail_end++;
                }
            }
        }

        if (_thumbnail_begin > 0) 
        {
            ImGui::SameLine();
            
            if (ImGui::Button("back")) 
            {
                if (_thumbnail_begin > 5) {
                    _thumbnail_begin -= 5; 
                    _thumbnail_end -= 5;
                }
                else {
                    _thumbnail_begin--;
                    _thumbnail_end--;
                }
            }
        }

        for (int i = _thumbnail_begin; i < _thumbnail_end; i++) 
        {
            if (i >= vec.size())
                continue;

            if (vec.at(i).second == NULL)
                continue;

            const std::string folder = AssetManager::GetFolder(vec.at(i).first);

            if (!folder.length())
                continue;

            if (folder == AssetManager::Get()->currentFolder)
            {
                ImGui::PushID(i);  

                if (ImGui::ImageButton("asset icon", (void*)(intptr_t) vec.at(i).second, ImVec2(70, 70))) { 
                    AssetManager::Get()->selectedAsset = vec.at(i).first;
                    Editor::Log("Current asset selected: " + AssetManager::Get()->selectedAsset);
                }

                //asset tool tip

                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip(vec.at(i).first.c_str());

                ImGui::PopID();

                //determine if image btn is on new line

                if (i < 9 || (i >= 10 && (i + 1) % 10 != 0)) 
                    ImGui::SameLine(); 
            }
        }
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

    if (editor::AssetManager::Get()->folderSelected) 
    {
        std::vector<std::pair<std::string, GLuint>>* vec;

        if (AssetManager::Get()->currentFolder == "/images/")
            vec = &AssetManager::Get()->images;

        if (AssetManager::Get()->currentFolder == "/audio/") 
            vec = &AssetManager::Get()->audio;

        if (AssetManager::Get()->currentFolder == "/data/")
            vec = &AssetManager::Get()->data;

        if (AssetManager::Get()->currentFolder == "/fonts/")
            vec = &AssetManager::Get()->text;
    }
    
    ImGui::Separator();  

    if (AssetManager::Get()->folderSelected)
    {
        if (AssetManager::Get()->currentFolder == "/images/")
            displayThumbnail(AssetManager::Get()->images);

        if (AssetManager::Get()->currentFolder == "/audio/") 
            displayThumbnail(AssetManager::Get()->audio);

        if (AssetManager::Get()->currentFolder == "/data/")
            displayThumbnail(AssetManager::Get()->data);

        if (AssetManager::Get()->currentFolder == "/fonts/")
            displayThumbnail(AssetManager::Get()->text);
    }

    else 
    {
        if (ImGui::ImageButton("image", (void*)(intptr_t) Graphics::Texture2D::Get("folder src").ID, ImVec2(70, 70)/* , ImVec2(0, 1), ImVec2(1, 0) */))
            SetFolder(true, "/images/");
 
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("images");

        ImGui::SameLine(); 

        if (ImGui::ImageButton("audio", (void*)(intptr_t) Graphics::Texture2D::Get("folder src").ID, ImVec2(70, 70)/* , ImVec2(0, 1), ImVec2(1, 0) */))
            SetFolder(true, "/audio/");

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("audio");

        ImGui::SameLine(); 

        if (ImGui::ImageButton("data", (void*)(intptr_t) Graphics::Texture2D::Get("folder src").ID, ImVec2(70, 70)/* , ImVec2(0, 1), ImVec2(1, 0) */))
            SetFolder(true, "/data/");

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("data");

        ImGui::SameLine(); 

        if (ImGui::ImageButton("fonts", (void*)(intptr_t) Graphics::Texture2D::Get("folder src").ID, ImVec2(70, 70)/* , ImVec2(0, 1), ImVec2(1, 0) */))
            SetFolder(true, "/fonts/");

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("fonts");
    }

}