#include "./node.h"
#include "../gui/gui.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../../../../build/sdk/include/app.h"

using namespace editor;


TextNode::TextNode(bool init): 
    Node(init, TEXT)
{
    size = 1;
    alpha = 1;
    UIFlag = true;
    tint = { 1.0f, 1.0f, 1.0f };
    depth = 1;
    charOffsetX = 0.0f;
    charOffsetY = 0.0f;
    textBuf = "";
    currentFont = "";
    textHandle = System::Game::CreateText(textBuf, positionX, positionY);

    System::Game::GetScene()->SetInteractive(textHandle);

    if (m_init)
        Editor::Log("Text node " + name + " created.");   
}


//---------------------------
 

TextNode::~TextNode() {

    if (textHandle != nullptr)
        System::Game::DestroyEntity(textHandle);

    if (m_init)
        Editor::Log("Text node " + name + " deleted.");
}


//---------------------------


void TextNode::Reset(int component_type)
{
    bool passAll = component_type == Component::NONE;

    if (component_type == Component::SHADER || passAll)
    {}

    if (component_type == Component::SCRIPT || passAll)
      behaviors.clear();
}


//---------------------------


void TextNode::ChangeFont(const std::string& font) {
    
    AssetManager::Register(font);
    System::Game::DestroyEntity(textHandle);

    textHandle = System::Game::CreateText(textBuf, positionX, positionY, font);
    currentFont = font;
}


//---------------------------


void TextNode::Update(std::vector<std::shared_ptr<Node>>& arr)
{
    ImGui::Separator(); 

    {
        ImGui::PushID(("(Text) " + name).c_str());

        std::string selText = (Editor::selectedEntity && textHandle) && Editor::selectedEntity->ID == textHandle->ID ? "<SELECTED> " : "";

        if (ImGui::TreeNode((selText + "(Text) " + name).c_str()))
        {
            Node::Update(arr);
            
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
                const std::string currFont = currentFont.length() ? currentFont : "default";

                if (ImGui::BeginMenu("select font"))
                {
                    for (const auto& asset : AssetManager::Get()->loadedAssets)
                    {
                        std::string key = asset.first;
                        std::string path = asset.second;

                        key.erase(std::remove(key.begin(), key.end(), '\"'), key.end());
                        path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());

                        if (System::Utils::GetFileType(key) == System::Resources::Manager::TEXT) {
                            if (ImGui::MenuItem(key.c_str())) 
                                ChangeFont(key);
                        }

                        else if (!AssetManager::Get()->text.size()) {
                            if (ImGui::MenuItem("default")) 
                                ChangeFont();
                        }
                    }

                    ImGui::EndMenu();
                }

                ImGui::Text(("current font: " + currFont).c_str());
                ImGui::InputText("content", &textBuf);
                ImGui::Checkbox("UI", &UIFlag);
                ImGui::Checkbox("stroke", &isStroked);
                ImGui::Checkbox("shadow", &isShadow);
                ImGui::ColorEdit3("tint", (float*)&tint); 
                ImGui::ColorEdit3("stroke color", (float*)&strokeColor); 
                ImGui::ColorEdit3("shadow color", (float*)&shadowColor); 
                ImGui::SliderInt("depth", &depth, 0, 1000);
                ImGui::SliderFloat("alpha", &alpha, 0.0f, 1.0f);
                ImGui::SliderFloat("position x", &positionX, -System::Window::s_width, System::Window::s_scaleWidth); 
                ImGui::SliderFloat("position y", &positionY, -System::Window::s_height, System::Window::s_scaleHeight); 
                ImGui::SliderFloat("rotation", &rotation, 0.0f, 360.0f); 
                ImGui::SliderFloat("scale x", &scaleX, -100.0f, 100.0f); 
                ImGui::SliderFloat("scale y", &scaleY, -100.0f, 100.0f);
                ImGui::SliderFloat("shadow distance x", &shadowDistanceX, 0.0f, 100.0f);
                ImGui::SliderFloat("shadow distance y", &shadowDistanceY, 0.0f, 100.0f);

                if (currFont != "default") {
                    ImGui::SliderFloat("stroke width", &strokeWidth, 0.0f, 6.0f);
                    ImGui::SliderFloat("offset x", &charOffsetX, 0.0f, 100.0f);
                    ImGui::SliderFloat("offset y", &charOffsetY, 0.0f, 100.0f);
                }
  
            } 

            ImGui::TreePop();
        }

        ImGui::PopID();

    }
}


//-----------------------------


void TextNode::Render(float _positionX, float _positionY, float _rotation, float _scaleX, float _scaleY)
{
    //text transform

    if (textHandle)
    {
        textHandle->SetText(textBuf);
        textHandle->SetScale(scaleX * _scaleX, scaleY * _scaleY);
        textHandle->SetPosition(positionX + _positionX, positionY + _positionY);
        textHandle->SetRotation(rotation + _rotation);
        textHandle->SetTint(tint);
        textHandle->SetAlpha(alpha);
        textHandle->SetDepth(depth);
        textHandle->SetStroke(isStroked, strokeColor, strokeWidth);
        textHandle->SetShadow(isShadow, shadowColor, shadowDistanceX, shadowDistanceY);
        textHandle->SetSlant(charOffsetX, charOffsetY);

        if (System::Game::GetScene()->ListenForInteraction(textHandle) && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            Editor::selectedEntity = textHandle;
    }
}

