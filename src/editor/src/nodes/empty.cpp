#include "./node.h"
#include "../gui/gui.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../../../../build/sdk/include/app.h"

using namespace editor;

EmptyNode::EmptyNode(): 
    Node("Empty")
{ 

    rectWidth = 0.0f;
    rectHeight = 0.0f;
    radius = 0.0f;
    line_weight = 1.0f;
    depth = 1;
    show_debug = false;
    debug_fill = false;
    currentShape = "";

    Editor::Log("Empty node " + name + " created."); 
}


//---------------------------


EmptyNode::~EmptyNode() {

    if (m_debugGraphic)
        System::Game::DestroyEntity(m_debugGraphic);

    if (!virtual_node)
        Editor::Log("Empty node " + name + " deleted.");
}


//---------------------------


void EmptyNode::Reset(const char* component_type)
{

    bool passAll = strcmp(component_type, "") == 0;

    if (strcmp(component_type, "Shader") == 0 || passAll)
        if (m_debugGraphic.get())
                m_debugGraphic->shader = Shader::Get("graphics");

    if (strcmp(component_type, "Script") == 0 || passAll)
        behaviors.clear();
}


//--------------------------


void EmptyNode::CreateShape(const std::string &shape)
{

    if (!m_debugGraphic) {

        if (shape == "rectangle")
            m_debugGraphic = System::Game::CreateGeom(20.0f, 20.0f, 10.0f, 10.0f); 

        else if (shape == "ellipse") {}

        else    
            return;
            
        currentShape = shape;   

    }
}


//---------------------------


void EmptyNode::Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr)
{

    ImGui::Separator(); 

    {

        ImGui::PushID(("(Empty) " + name).c_str());
 
        if (ImGui::TreeNode(("(Empty) " + name).c_str()))
        {
        
            static char buf1[32] = ""; ImGui::InputText("name", buf1, 32, ImGuiInputTextFlags_CallbackCompletion, ChangeName, &ID);

            //save prefab

            if (ImGui::Button("Save prefab")) 
                SavePrefab();
            
            if (ImGui::BeginMenu("Add Component"))
            {
                if (ImGui::MenuItem("Scripts")) 
                    AddComponent("Script"); 

                if (ImGui::MenuItem("Shader")) 
                    AddComponent("Shader");
            
                ImGui::EndMenu();
            }

            //component options

            //------------------------------ script


            if (HasComponent("Script") && ImGui::BeginMenu("Script")) {

                GUI::RenderScriptOptions(ID);
                
                ImGui::EndMenu();
            }

            //------------------------------ shader


            if (HasComponent("Shader") && ImGui::BeginMenu("Shader")) {

                GUI::RenderShaderOptions(ID);
                
                ImGui::EndMenu();
            }


            if (ImGui::BeginMenu("Options")) {
                ShowOptions(node, arr);
                ImGui::EndMenu();
            }

            ImGui::Checkbox("Edit", &show_options);

            if (show_options)
            {

                if (ImGui::BeginMenu("Create Graphic")) {

                    if (ImGui::MenuItem("rectangle"))
                        CreateShape("rectangle");

                    ImGui::EndMenu();
                }
                    
            }

            ImGui::TreePop();
        }

    }

    ImGui::PopID();

}


//------------------------------------


void EmptyNode::Render()
{
    if (m_debugGraphic)
    {

        ImGui::Checkbox("debug", &show_debug);
        ImGui::Checkbox("fill", &debug_fill);

        if (currentShape != "") 
        {
            ImGui::SliderInt("depth", &depth, 0, 1000);
            ImGui::SliderFloat("line weight", &line_weight, 0.1f, 3.0f); 
            ImGui::ColorEdit3("tint", (float*)&m_debugGraphic->tint); 
            ImGui::SliderFloat("alpha", &m_debugGraphic->alpha, 0.0f, 1.0f);
            ImGui::SliderFloat("position x", &positionX, 10.0f, 1000.0f);  
            ImGui::SliderFloat("position y", &positionY, 10.0f, 1000.0f);
        }

        if (currentShape == "rectangle") {

            ImGui::SliderFloat("width", &rectWidth, 10.0f, 1000.0f); 
            ImGui::SliderFloat("height", &rectHeight, 10.0f, 1000.0f); 

            m_debugGraphic->SetSize(rectWidth, rectHeight);
        }

        if (currentShape == "ellipse") {

            ImGui::SliderFloat("radius", &radius, 10.0f, 1000.0f);  

            m_debugGraphic->SetSize(radius);
        }

        m_debugGraphic->renderable = show_debug;

        m_debugGraphic->SetPosition(positionX, positionY);
        m_debugGraphic->SetDrawStyle(debug_fill ? GL_FILL : GL_LINE); 
        m_debugGraphic->SetThickness(line_weight);
    
    }
}