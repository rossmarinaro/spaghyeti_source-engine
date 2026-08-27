#include "./gui.h"
#include "../assets/assets.h"
#include "../editor.h"

static int _thumbnail_begin = 0;

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
        ImGui::BeginGroup();

        const ImVec2 thumbnailSize = ImVec2(70, 70);

        int columnCount = 30,
            stride = 5,
            currentBegin = _thumbnail_begin,
            maxValidBegin = vec.size() - columnCount;

        bool disableBack = (currentBegin <= 0),
             disableNext = (currentBegin >= maxValidBegin);
        
        if (columnCount < 1) 
            columnCount = 1;
            
        if (columnCount > vec.size()) 
            columnCount = vec.size();

        if (maxValidBegin < 0) 
            maxValidBegin = 0;

        if (disableBack) 
            ImGui::BeginDisabled();

        if (ImGui::Button("back")) 
            currentBegin -= stride;
     
        if (disableBack) 
            ImGui::EndDisabled();

        ImGui::SameLine();

        if (disableNext) 
            ImGui::BeginDisabled();

        if (ImGui::Button("next"))
            currentBegin += stride;

        if (disableNext) 
            ImGui::EndDisabled();

        ImGui::EndGroup();

        if (currentBegin > maxValidBegin) 
            currentBegin = maxValidBegin;

        if (currentBegin < 0) currentBegin = 0;
            _thumbnail_begin = currentBegin;

        int itemsToRender = std::min(columnCount, (static_cast<int>(vec.size()) - _thumbnail_begin));

        //render thumbail buttons
        
        ImGui::Columns(5, "image thumbnails", false);

        for (int step = 0; step < itemsToRender; step++) 
        {
            int i = _thumbnail_begin + step;

            if (i < 0 || i >= static_cast<int>(vec.size())) 
                break;

            ImGui::PushID(i);  

            const std::string folder = AssetManager::GetFolder(vec.at(i).first);
            const auto tex = Graphics::Texture2D::Get(vec.at(i).first);

            if (!folder.length())
                continue;

            if (tex.Width > 2400 || tex.Height > 2400) 
               Editor::Log("Warning: image dimensions exceed 2400px. This may impact performance.");

            if (folder == AssetManager::Get()->currentFolder)
            {
                int textureID = vec == AssetManager::Get()->images ? tex.ID : vec.at(i).second;
                
                if (tex.ID != 0 && tex.ID != -1) 
                    if (ImGui::ImageButton("##thumbnail_slot_1", (void*)(intptr_t)textureID, thumbnailSize)) { 
                        AssetManager::Get()->selectedAsset = vec.at(i).first;
                        Editor::Log("Current asset selected: " + AssetManager::Get()->selectedAsset);
                    }

                //asset tool tip

                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip(vec.at(i).first.c_str());

                ImGui::NextColumn();
            }

            ImGui::PopID();
        }
    
        ImGui::Columns(1);
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