#include "./node.h"
#include "../gui/gui.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../../../../build/sdk/include/app.h"

using namespace editor;


TextNode::TextNode(): 
    Node("Text")
{
    isUI = 2;
    size = 1;
    alpha = 1;
    UIFlag = true;
    tint = glm::vec3(1.0f);
    depth = 1;
    textBuf = "";
    textHandle = System::Game::CreateText(textBuf, positionX, positionY);
    System::Game::GetScene()->SetInteractive(textHandle);

    Editor::Log("Text node " + name + " created.");   
}


//---------------------------
 

TextNode::~TextNode() {

    if (textHandle != nullptr)
        System::Game::DestroyEntity(textHandle);

    if (!virtual_node)
        Editor::Log("Text node " + name + " deleted.");
}


//---------------------------


void TextNode::Reset(const char* component_type)
{
    bool passAll = strcmp(component_type, "") == 0;

    if (strcmp(component_type, "Shader") == 0 || passAll)
    {}

    if (strcmp(component_type, "Script") == 0 || passAll)
      behaviors.clear();
}


//---------------------------


void TextNode::Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr)
{

    ImGui::Separator(); 

    {

        ImGui::PushID(("(Text) " + name).c_str());

        std::string selText = (Editor::selectedEntity && textHandle) && Editor::selectedEntity->ID == textHandle->ID ? "<SELECTED> " : "";

        if (ImGui::TreeNode((selText + "(Text) " + name).c_str()))
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
                ImGui::InputText("content", &textBuf);
                ImGui::Checkbox("UI", &UIFlag);
                ImGui::ColorEdit3("tint", (float*)&tint); 
                ImGui::SliderInt("depth", &depth, 0, 1000);
                ImGui::SliderFloat("alpha", &alpha, 0.0f, 1.0f);
                ImGui::SliderFloat("position x", &positionX, -System::Window::s_width, System::Window::s_scaleWidth); 
                ImGui::SliderFloat("position y", &positionY, -System::Window::s_height, System::Window::s_scaleHeight); 
                ImGui::SliderFloat("rotation", &rotation, 0.0f, 360.0f); 
                ImGui::SliderFloat("scale x", &scaleX, -100.0f, 100.0f); 
                ImGui::SliderFloat("scale y", &scaleY, -100.0f, 100.0f);
            }

            ImGui::TreePop();
        }

        ImGui::PopID();

    }
}


//-----------------------------


void TextNode::Render()
{
    //text transform

    if (textHandle)
    {

        textHandle->SetText(textBuf);
        textHandle->SetScale(scaleX, scaleY);
        textHandle->SetPosition(positionX, positionY);
        textHandle->SetRotation(rotation);
        textHandle->SetTint(tint);
        textHandle->SetDepth(depth);

        if (System::Game::GetScene()->ListenForInteraction(textHandle) && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            Editor::selectedEntity = textHandle;
    }

    isUI = UIFlag ? 2 : 1;
}

