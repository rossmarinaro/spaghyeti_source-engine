#include "./node.h"
#include "../gui/gui.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../../../../build/sdk/include/app.h"

using namespace editor;


AudioNode::AudioNode(): 
    Node("Audio"),
        m_audioTexture(Graphics::Texture2D::GetTexture("audio src"))
     
{
    audio_source_name = "";
    loop = false;
    volume = 1.0f;

    System::Resources::Manager::RegisterTextures();
    
    Editor::Log("Audio node " + name + " created.");   
}


//--------------------------


void AudioNode::Load() {
    audio_source_name = AssetManager::selectedAsset;
    AssetManager::Register(audio_source_name, true);
}



//---------------------------
 

AudioNode::~AudioNode() {

    if (!virtual_node)  
        Editor::Log("audio node " + name + " deleted.");
}


//---------------------------


void AudioNode::Reset(const char* component_type)
{}


//---------------------------


void AudioNode::Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr)
{

    ImGui::Separator(); 

    {

        ImGui::PushID(("(Audio) " + name).c_str());

        if (ImGui::TreeNode(("(Audio) " + name).c_str()))
        {
        
            static char buf1[32] = ""; ImGui::InputText("name", buf1, 32, ImGuiInputTextFlags_CallbackCompletion, ChangeName, &ID);

            if (ImGui::BeginMenu("Options")) { 
                ShowOptions(node, arr);
                ImGui::EndMenu();
            }

            ImGui::Checkbox("Edit", &show_options);

            if (show_options)
            {

                if (ImGui::ImageButton("audio source", (void*)(intptr_t)m_audioTexture.ID, ImVec2(25, 25), ImVec2(0, 1), ImVec2(1, 0))) {
                    if (System::Utils::GetFileType(AssetManager::selectedAsset) == "audio")
                        Load();
                }
                    
                else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && System::Utils::GetFileType(audio_source_name) != "audio") 
                    ImGui::SetTooltip("cannot set texture because selected asset is not of type audio.");

                ImGui::SameLine();

                ImGui::Text(("source: " + audio_source_name).c_str());

                ImGui::SliderFloat("volume", &volume, 0.0f, 1.0f); 

                System::Audio::setVolume(volume);

                ImGui::Checkbox("loop", &loop);

                if (ImGui::Button("play"))
                    System::Audio::play(audio_source_name.c_str(), loop); 

                ImGui::SameLine();
                
                if (ImGui::Button("stop"))
                    System::Audio::stop(); 


            }

            ImGui::TreePop(); 
        }
                
    }

    ImGui::PopID();

}