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
 

TextNode::~TextNode() 
{
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
     
        if (GUI::Get()->collapseFolders)
            ImGui::SetNextItemOpen(false, ImGuiCond_Always);

        if (ImGui::TreeNodeEx((selText + "(Text) " + name).c_str()))
        {
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                Editor::Get()->events->selected_nodes = &nodes;

            Node::Update(arr);
            
            if (ImGui::BeginMenu("Add Component"))
            {
                if (ImGui::MenuItem("Scripts")) {
                    AddComponent(Component::SCRIPT); 
                    EventListener::UpdateSession();
                }

                if (ImGui::MenuItem("Shader")) {
                    AddComponent(Component::SHADER);
                    EventListener::UpdateSession();
                }
            
                ImGui::EndMenu();
            }

            //component options

            //------------------------------ script


            if (HasComponent(Component::SCRIPT) && ImGui::BeginMenu("Script")) {
                RenderScriptOptions(ID, arr);
                ImGui::EndMenu();
            }

            //------------------------------ shader


            if (HasComponent(Component::SHADER) && ImGui::BeginMenu("Shader")) {
                RenderShaderOptions(ID, arr);
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
                            if (ImGui::MenuItem(key.c_str())) {
                                ChangeFont(key);
                                EventListener::UpdateSession();
                            }
                        }

                        else if (!AssetManager::Get()->text.size()) {
                            if (ImGui::MenuItem("default")) {
                                ChangeFont();
                                EventListener::UpdateSession();
                            }
                        }
                    }

                    ImGui::EndMenu();
                }

                ImGui::Text(("current font: " + currFont).c_str());

                if (ImGui::InputText("content", &textBuf))
                    EventListener::UpdateSession();
                if (ImGui::Checkbox("UI", &UIFlag))
                    EventListener::UpdateSession();
                if (ImGui::Checkbox("stroke", &isStroked))
                    EventListener::UpdateSession();
                if (ImGui::Checkbox("shadow", &isShadow))
                    EventListener::UpdateSession();

                ImGui::ColorEdit3("tint", (float*)&tint);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();

                ImGui::ColorEdit3("stroke color", (float*)&strokeColor);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();

                ImGui::ColorEdit3("shadow color", (float*)&shadowColor);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();

                ImGui::SliderInt("depth", &depth, 0, 1000);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();

                ImGui::SliderFloat("alpha", &alpha, 0.0f, 1.0f);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();

                ImGui::SliderFloat("position x", &positionX, -Editor::Get()->worldWidth, Editor::Get()->worldWidth); 
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();

                ImGui::SliderFloat("position y", &positionY, -Editor::Get()->worldHeight, Editor::Get()->worldHeight); 
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();

                ImGui::SliderFloat("scale x", &scaleX, -100.0f, 100.0f); 
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();

                ImGui::SliderFloat("scale y", &scaleY, -100.0f, 100.0f);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();

                ImGui::SliderFloat("shadow distance x", &shadowDistanceX, 0.0f, 100.0f);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();

                ImGui::SliderFloat("shadow distance y", &shadowDistanceY, 0.0f, 100.0f);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();

                if (currFont != "default") 
                {
                    ImGui::SliderFloat("stroke width", &strokeWidth, 0.0f, 6.0f);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        EventListener::UpdateSession();

                    ImGui::SliderFloat("offset x", &charOffsetX, 0.0f, 100.0f);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        EventListener::UpdateSession();

                    ImGui::SliderFloat("offset y", &charOffsetY, 0.0f, 100.0f);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        EventListener::UpdateSession();
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
        textHandle->SetTint(tint);
        textHandle->SetAlpha(alpha);
        textHandle->SetDepth(depth);
        textHandle->SetStroke(isStroked, strokeColor, strokeWidth);
        textHandle->SetShadow(isShadow, shadowColor, shadowDistanceX, shadowDistanceY);
        textHandle->SetSlant(charOffsetX, charOffsetY);
        textHandle->SetStatic(UIFlag);

        if (System::Game::GetScene()->ListenForInteraction(textHandle)) 
            Editor::FocusEntity(textHandle);
    }
}

