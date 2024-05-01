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
    this->audio_source_name = "";
    this->loop = false;
    this->volume = 1.0f;

    System::Resources::Manager::RegisterAssets();
    
    Editor::Log("Audio node " + this->name + " created.");   
}


//---------------------------
 

AudioNode::~AudioNode() {

    if (!this->virtual_node)  
        Editor::Log("audio node " + this->name + " deleted.");
}


//---------------------------


void AudioNode::Reset(const char* component_type)
{}


//---------------------------


void AudioNode::Render(std::shared_ptr<Node> node)
{

    ImGui::Separator(); 

    {

        ImGui::PushID(("(Audio) " + this->name).c_str());

        if (ImGui::TreeNode(("(Audio) " + this->name).c_str()))
        {
        
            static char buf1[32] = ""; ImGui::InputText("name", buf1, 32, ImGuiInputTextFlags_CallbackCompletion, ChangeName, &this->ID);

            if (ImGui::BeginMenu("Delete"))
            {
                if (ImGui::MenuItem("Are You Sure?")) 
                    DeleteNode(node);

                ImGui::EndMenu();
            }

            ImGui::Checkbox("Edit", &this->show_options);

            if (this->show_options)
            {

                if (ImGui::ImageButton("audio source", (void*)(intptr_t)this->m_audioTexture.ID, ImVec2(25, 25), ImVec2(0, 1), ImVec2(1, 0))) {
                    if (System::Utils::GetFileType(Editor::selectedAsset) == "audio")
                        this->audio_source_name = Editor::selectedAsset;
                }
                    
                else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && System::Utils::GetFileType(this->audio_source_name) != "audio") 
                    ImGui::SetTooltip("cannot set texture because selected asset is not of type audio.");

                ImGui::SameLine();

                ImGui::Text(("source: " + this->audio_source_name).c_str());

                ImGui::SliderFloat("volume", &this->volume, 0.0f, 1.0f); 

                System::Audio::setVolume(this->volume);

                ImGui::Checkbox("loop", &this->loop);

                if (ImGui::Button("play"))
                    System::Audio::play(this->audio_source_name.c_str(), this->loop); 

                ImGui::SameLine();
                
                if (ImGui::Button("stop"))
                    System::Audio::stop(); 


            }

            ImGui::TreePop(); 
        }
                
    }

    ImGui::PopID();

}