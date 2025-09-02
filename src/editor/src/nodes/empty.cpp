#include "./node.h"
#include "../gui/gui.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../../../../build/sdk/include/app.h"

using namespace editor;

EmptyNode::EmptyNode(bool init): 
    Node(init, EMPTY)
{ 
    rectWidth = 0.0f;
    rectHeight = 0.0f;
    radius = 0.0f;
    line_weight = 1.0f;
    depth = 1;
    show_debug = false;
    debug_fill = false;
    currentShape = "";

    if (m_init)
        Editor::Log("Empty node " + name + " created."); 
}


//---------------------------


EmptyNode::~EmptyNode() {

    if (m_debugGraphic)
        System::Game::DestroyEntity(m_debugGraphic);

    if (m_init)
        Editor::Log("Empty node " + name + " deleted.");
}


//---------------------------


void EmptyNode::Reset(int component_type) {

    bool passAll = component_type == Component::NONE;

    if (component_type == Component::SHADER || passAll)
        if (m_debugGraphic.get())
                m_debugGraphic->SetShader("graphics");

    if (component_type == Component::SCRIPT || passAll)
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
        
            Node::Update(node, arr);

            if (ImGui::BeginMenu("Add Component"))
            {
                if (ImGui::MenuItem("Scripts")) 
                    AddComponent(Component::SCRIPT); 

                if (ImGui::MenuItem("Shader")) 
                    AddComponent(Component::SHADER);
            
                ImGui::EndMenu();
            }

            //component options

            //------------------------------ script


            if (HasComponent(Component::SCRIPT) && ImGui::BeginMenu("Script")) {

                GUI::Get()->RenderScriptOptions(ID);
                ImGui::EndMenu();
            }

            //------------------------------ shader


            if (HasComponent(Component::SHADER) && ImGui::BeginMenu("Shader")) {

                GUI::Get()->RenderShaderOptions(ID);
                ImGui::EndMenu();
            }

            if (show_options)
            {
                if (currentShape != "") 
                {
                    ImGui::SliderInt("depth", &depth, 0, 1000);
                    ImGui::SliderFloat("line weight", &line_weight, 0.1f, 3.0f); 
                    ImGui::ColorEdit3("tint", (float*)&m_debugGraphic->tint); 
                    ImGui::SliderFloat("alpha", &m_debugGraphic->alpha, 0.0f, 1.0f);
                    ImGui::SliderFloat("position x", &positionX, -System::Window::s_width, System::Window::s_width);  
                    ImGui::SliderFloat("position y", &positionY, -System::Window::s_width, System::Window::s_width); 
                }

                if (m_debugGraphic)
                {
                    
                    ImGui::Checkbox("debug", &show_debug);
                    ImGui::Checkbox("fill", &debug_fill);

                    if (currentShape == "rectangle") {
                        ImGui::SliderFloat("width", &rectWidth, 10.0f, 1000.0f); 
                        ImGui::SliderFloat("height", &rectHeight, 10.0f, 1000.0f); 

                        m_debugGraphic->SetSize(rectWidth, rectHeight);
                    }

                    if (currentShape == "ellipse") {
                        ImGui::SliderFloat("radius", &radius, 10.0f, 1000.0f);  
                        m_debugGraphic->SetSize(radius);
                    }
                }

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
    if (m_debugGraphic && show_options)
    {
        m_debugGraphic->renderable = show_debug;

        m_debugGraphic->SetPosition(positionX, positionY);
        m_debugGraphic->SetDrawStyle(debug_fill ? 1 : 0); 
        m_debugGraphic->SetThickness(line_weight);
    }
}