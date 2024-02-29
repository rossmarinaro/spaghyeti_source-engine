#include "./node.h"
#include "../gui.h"
#include "../../editor.h"
#include "../../assets/assets.h"
#include "../../../../../build/include/app.h"


TextNode::TextNode(const std::string &id): 
    Node(id, "Text"),
        size(1),
        alpha(1),
        tint(glm::vec3(1.0f)),
        textBuf("")
{

    this->textHandle = Game::CreateText(this->textBuf, this->positionX, this->positionY);

    Editor::Log("Text node " + this->m_name + " created.");   
}


//---------------------------
 

TextNode::~TextNode() {

    if (this->textHandle != nullptr)
        Game::DestroyEntity(this->textHandle);

    Editor::Log("Text node " + this->m_name + " deleted.");
}



//---------------------------


void TextNode::Render()
{

    ImGui::Separator(); 

    {

        ImGui::PushID(("(Text) " + this->m_name).c_str());

        if (ImGui::TreeNode(("(Text) " + this->m_name).c_str()))
        {
        
            static char buf1[32] = ""; ImGui::InputText("name", buf1, 32, ImGuiInputTextFlags_CallbackCompletion, ChangeName, &this->m_ID);

            if (ImGui::BeginMenu("Add Component"))
            {
                if (ImGui::MenuItem("Scripts")) 
                    this->AddComponent("Script"); 

                if (ImGui::MenuItem("Shader")) 
                    this->AddComponent("Shader");
            
                ImGui::EndMenu();
            }

            //component options

            for (const auto &component : this->components)
            {


            //------------------------------ script


                if (strcmp(component->m_type, "Script") == 0 && ImGui::BeginMenu("Script")) {

                    GUI::RenderScriptOptions(this->m_ID);
                    
                    ImGui::EndMenu();
                }

                //------------------------------ shader


                if (strcmp(component->m_type, "Shader") == 0 && ImGui::BeginMenu("Shader")) {

                    GUI::RenderShaderOptions(this->m_ID);
                    
                    ImGui::EndMenu();
                }
            }

            if (ImGui::BeginMenu("Delete"))
            {
                if (ImGui::MenuItem("Are You Sure?")) 
                    DeleteNode(this);

                ImGui::EndMenu();
            }

            ImGui::Checkbox("Edit", &this->show_options);

            if (this->show_options)
            {
                ImGui::InputText("content", &this->textBuf);

                ImGui::ColorEdit3("tint", (float*)&this->tint); 
                ImGui::SliderFloat("alpha", &this->alpha, 0.0f, 1.0f);
                ImGui::SliderFloat("position x", &this->positionX, 0.0f, System::Window::m_width); 
                ImGui::SliderFloat("position y", &this->positionY, 0.0f, System::Window::m_height); 
                ImGui::SliderFloat("rotation", &this->rotation, 0.0f, 360.0f); 
                ImGui::SliderFloat("scale x", &this->scaleX, -2.0f, 100.0f); 
                ImGui::SliderFloat("scale y", &this->scaleY, -2.0f, 100.0f);
            }

            ImGui::TreePop();
        }

        //text transform

        if (this->textHandle)
        {

            this->textHandle->SetText(this->textBuf);
            this->textHandle->SetScale(this->scaleX, this->scaleY);
            this->textHandle->SetPosition(this->positionX, this->positionY);
            this->textHandle->SetRotation(this->rotation);
            this->textHandle->SetTint(this->tint);
        
        }

        ImGui::PopID();

    }
}


