#include "./node.h"
#include "../gui/gui.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../../../../build/include/app.h"

using namespace editor;


TextNode::TextNode(const std::string &id): 
    Node(id, "Text"),
        size(1),
        alpha(1),
        tint(glm::vec3(1.0f)),
        depth(1),
        textBuf("")
{

    this->textHandle = System::Game::CreateText(this->textBuf, this->positionX, this->positionY);

    Editor::Log("Text node " + this->m_name + " created.");   
}


//---------------------------
 

TextNode::~TextNode() {

    if (this->textHandle != nullptr)
        System::Game::DestroyEntity(this->textHandle);

    if (!this->virtual_node)
        Editor::Log("Text node " + this->m_name + " deleted.");
}


//---------------------------


void TextNode::Reset(const char* component_type)
{
    bool passAll = strcmp(component_type, "") == 0;

    if (strcmp(component_type, "Shader") == 0 || passAll)
    {}

    if (strcmp(component_type, "Script") == 0 || passAll)
    {}
}


//---------------------------


void TextNode::Render(std::shared_ptr<Node> node)
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

            //------------------------------ script


            if (this->HasComponent("Script") && ImGui::BeginMenu("Script")) {

                GUI::RenderScriptOptions(this->m_ID);
                
                ImGui::EndMenu();
            }

            //------------------------------ shader


            if (this->HasComponent("Shader") && ImGui::BeginMenu("Shader")) {

                GUI::RenderShaderOptions(this->m_ID);
                
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Delete"))
            {
                if (ImGui::MenuItem("Are You Sure?")) 
                    DeleteNode(node);

                ImGui::EndMenu();
            }
 
            ImGui::Checkbox("Edit", &this->show_options);

            if (this->show_options)
            {
                ImGui::InputText("content", &this->textBuf);

                ImGui::ColorEdit3("tint", (float*)&this->tint); 
                ImGui::SliderInt("depth", &this->depth, 0, 1000);
                ImGui::SliderFloat("alpha", &this->alpha, 0.0f, 1.0f);
                ImGui::SliderFloat("position x", &this->positionX, -System::Window::m_width, System::Window::m_scaleWidth); 
                ImGui::SliderFloat("position y", &this->positionY, -System::Window::m_height, System::Window::m_scaleHeight); 
                ImGui::SliderFloat("rotation", &this->rotation, 0.0f, 360.0f); 
                ImGui::SliderFloat("scale x", &this->scaleX, -100.0f, 100.0f); 
                ImGui::SliderFloat("scale y", &this->scaleY, -100.0f, 100.0f);
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
            this->textHandle->SetDepth(this->depth);
        }

        ImGui::PopID();

    }
}


