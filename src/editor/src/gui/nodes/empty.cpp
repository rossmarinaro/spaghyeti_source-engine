#include "./node.h"
#include "../gui.h"
#include "../../editor.h"
#include "../../assets/assets.h"
#include "../../../../../build/include/app.h"


EmptyNode::EmptyNode(const std::string &id): 
    Node(id, "Empty"),
        rectWidth(0.0f),
        rectHeight(0.0f),
        radius(0.0f),
        show_debug(false),
        debug_fill(false),
        currentShape("")
{ Editor::Log("Empty node " + this->m_name + " created."); }


//---------------------------
 

EmptyNode::~EmptyNode() {

    if (this->m_debugGraphic)
        Game::DestroyEntity(this->m_debugGraphic);

    Editor::Log("Empty node " + this->m_name + " deleted.");
}


//---------------------------


void EmptyNode::Reset(const char* component_type)
{

    bool passAll = strcmp(component_type, "") == 0;

    if (strcmp(component_type, "Shader") == 0 || passAll)
        if (this->m_debugGraphic.get())
                this->m_debugGraphic->m_shader = Shader::GetShader("graphics");
}


//--------------------------


void EmptyNode::CreateShape(const std::string &shape)
{

    if (!this->m_debugGraphic) {

        if (shape == "rectangle")
            this->m_debugGraphic = Game::CreateGeom(20.0f, 20.0f, 10.0f, 10.0f); 

        else if (shape == "ellipse") {}

        else    
            return;
            
        this->currentShape = shape;   

    }
}


//---------------------------


void EmptyNode::Render(std::shared_ptr<Node> node)
{

    ImGui::Separator(); 

    {

        ImGui::PushID(("(Empty) " + this->m_name).c_str());
 
        if (ImGui::TreeNode(("(Empty) " + this->m_name).c_str()))
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

                if (ImGui::BeginMenu("Create Graphic")) {

                    if (ImGui::MenuItem("rectangle"))
                        this->CreateShape("rectangle");

                    ImGui::EndMenu();
                }
                    
                if (this->m_debugGraphic)
                {

                    ImGui::Checkbox("debug", &this->show_debug);

                    ImGui::SameLine();

                    if (this->currentShape != "") {
                        ImGui::Checkbox("fill", &this->debug_fill); 
                        ImGui::ColorEdit3("tint", (float*)&this->m_debugGraphic->m_tint); ImGui::SameLine();
                        ImGui::SliderFloat("alpha", &this->m_debugGraphic->m_alpha, 0.0f, 1.0f);
                        ImGui::SliderFloat("position x", &this->positionX, 10.0f, 1000.0f); 
                        ImGui::SliderFloat("position y", &this->positionY, 10.0f, 1000.0f);
                    }

                    if (this->currentShape == "rectangle") {
 
                        ImGui::SliderFloat("width", &this->rectWidth, 10.0f, 1000.0f); 
                        ImGui::SliderFloat("height", &this->rectHeight, 10.0f, 1000.0f); 

                        this->m_debugGraphic->SetSize(this->rectWidth, this->rectHeight);
                    }

                    if (this->currentShape == "ellipse") {

                        ImGui::SliderFloat("radius", &this->radius, 10.0f, 1000.0f);  

                        this->m_debugGraphic->SetSize(this->radius);
                    }

                    this->m_debugGraphic->m_renderable = this->show_debug ? true : false;

                    this->m_debugGraphic->SetPosition(this->positionX, this->positionY);

                    this->m_debugGraphic->SetDrawStyle(this->debug_fill ? 0 : 1); 
                
                }
            }

            ImGui::TreePop();
        }

    }

    ImGui::PopID();

}