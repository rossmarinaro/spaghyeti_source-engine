#include "./node.h"
#include "../gui/gui.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../../../../build/include/app.h"

using namespace editor;


AudioNode::AudioNode(): 
    Node("Audio"),
        audioTexture(Graphics::Texture2D::GetTexture("audio src"))
{}


//--------------------------


AudioNode::AudioNode(const std::string& id): 
    Node(id, "Audio"),
        audio_source_name(""),
        audioTexture(Graphics::Texture2D::GetTexture("audio src")),
        loop(false),
        volume(1)
{
    System::Resources::Manager::RegisterAssets();
    Editor::Log("Audio node " + this->m_name + " created.");   
}


//---------------------------
 

AudioNode::~AudioNode() {

    if (!this->virtual_node)  
        Editor::Log("audio node " + this->m_name + " deleted.");
}


//---------------------------


void AudioNode::Reset(const char* component_type)
{}


//---------------------------


void AudioNode::Render(std::shared_ptr<Node> node)
{

    ImGui::Separator(); 

    {

        ImGui::PushID(("(Audio) " + this->m_name).c_str());

        if (ImGui::TreeNode(("(Audio) " + this->m_name).c_str()))
        {
        
            static char buf1[32] = ""; ImGui::InputText("name", buf1, 32, ImGuiInputTextFlags_CallbackCompletion, ChangeName, &this->m_ID);

            if (ImGui::BeginMenu("Delete"))
            {
                if (ImGui::MenuItem("Are You Sure?")) 
                    DeleteNode(node);

                ImGui::EndMenu();
            }

            ImGui::Checkbox("Edit", &this->show_options);

            if (this->show_options)
            {

                if (ImGui::ImageButton("audio source", (void*)(intptr_t)this->audioTexture.ID, ImVec2(25, 25), ImVec2(0, 1), ImVec2(1, 0))) {
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