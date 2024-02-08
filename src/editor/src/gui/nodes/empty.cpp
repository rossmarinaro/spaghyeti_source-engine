#include "./node.h"
#include "../gui.h"
#include "../../editor.h"
#include "../../assets/assets.h"
#include "../../../../../build/include/app.h"


EmptyNode::EmptyNode(const std::string &id): 
    Node(id, "Empty"),
        show_debug(false),
        debug_fill(false)
{

    //this->m_debugGraphic = std::make_shared<Graphics::Quad>(0.0f, 0.0f, 10.0f, 10.0f);    

    Editor::Log("Empty node " + this->m_name + " created.");   
}


//---------------------------
 

EmptyNode::~EmptyNode()
{

    //if (this->m_debugGraphic)
      //  Game::DestroyGraphic(this->m_debugGraphic);

    Editor::Log("Empty node " + this->m_name + " deleted.");
}


//---------------------------


void EmptyNode::Render()
{

    ImGui::Separator(); 

    {

        ImGui::PushID(("(Empty) " + this->m_name).c_str());
 
        if (ImGui::TreeNode(("(Empty) " + this->m_name).c_str()))
        {
        
            // static char buf1[32] = ""; ImGui::InputText("name", buf1, 32, ImGuiInputTextFlags_CallbackCompletion, ChangeName, &this->m_ID);

            // if (ImGui::BeginMenu("Add Component"))
            // {

            //     if (ImGui::MenuItem("Physics Body"))
            //         this->AddComponent("Physics Body");

            //     if (ImGui::MenuItem("Scripts")) 
            //         this->AddComponent("Script");

            //     if (ImGui::MenuItem("Shader"))
            //         this->AddComponent("Shader");
            
            //     ImGui::EndMenu();
            // }

            // if (ImGui::BeginMenu("Delete"))
            // {
            //     if (ImGui::MenuItem("Are You Sure?"))
            //         DeleteNode(this);

            //     ImGui::EndMenu();
            // }

            // ImGui::Checkbox("Edit", &this->show_options);

            // if (this->show_options)
            // {
                
            //     if (this->m_debugGraphic)
            //     {

            //         ImGui::Checkbox("debug graphics", &this->show_debug);

            //         ImGui::SameLine();

            //         ImGui::Checkbox("fill", &this->debug_fill);

            //         if (this->show_debug)
            //         {

            //             this->m_debugGraphic->m_debug = true;

            //             ImGui::SliderFloat("width", &this->m_debugGraphic->width, 10.0f, 1000.0f); 
            //             ImGui::SliderFloat("height", &this->m_debugGraphic->height, 10.0f, 1000.0f); 

            //             ImGui::SliderFloat("position x", &this->positionX, 10.0f, 1000.0f); 
            //             ImGui::SliderFloat("position y", &this->positionY, 10.0f, 1000.0f); 
                        
            //         }
            //         else
            //             this->m_debugGraphic->m_debug = false; 

            //         this->m_debugGraphic->SetPosition(this->positionX, this->positionY);

            //         if (this->m_debugGraphic->m_debug)
            //             this->m_debugGraphic->Render(this->debug_fill ? 0 : 1); 
                
            //     }
            // }

            ImGui::TreePop();
        }

    }

    ImGui::PopID();

}