#include "./node.h"
#include "../gui/gui.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../../../../build/sdk/include/app.h"

using namespace editor;


SpawnerNode::SpawnerNode(bool init):  
    Node(init, SPAWNER)
{  
    typeOf = Entity::SPRITE;
    m_spawnType = "sprite";
    textureKey = "";
    behaviorKey = "";
    animationKey = "";
    width = 0.0f;
    height = 0.0f;
    loop = false;
    alpha = 1.0f;
    tint = { 1.0f, 1.0f, 1.0f }; 

    m_rectHandle = nullptr;
    m_textHandle = nullptr;

    if (m_init) {
        CreateMarker();
        Editor::Log("Spawner node " + name + " created.");   
    }
}


         
//---------------------------


SpawnerNode::~SpawnerNode() {

    if (m_init)
        Editor::Log("Spawner node " + name + " deleted.");
}


//---------------------------


void SpawnerNode::Reset(int component_type)
{

    // bool passAll = component_type == Component::NONE;

    // if (component_type == Component::SHADER || passAll)
    //     if (m_rectHandle.get())
    //         m_rectHandle->SetShader("sprite");

    // if (component_type == Component::SCRIPT || passAll)
    //     behaviors.clear();

    // if (component_type == Component::ANIMATOR || passAll) 
    //     animations.clear();

    // if (component_type == Component::PHYSICS || passAll)
    // {
    //     for (const auto& body : bodies)
    //         Physics::DestroyBody(body.pb);
        
    //     body_pointer.clear();
    //     bodies.clear();
    //     is_sensor.clear();

    //     restitution = 0.0f;
    //     density = 0.0f;
    //     friction = 0.0f;
    // }
    
}

//---------------------------


void SpawnerNode::ApplyTexture(const std::string& asset) {
    m_currentTexture = Graphics::Texture2D::Get(asset).ID;  
    textureKey = asset;
}

//---------------------------


void SpawnerNode::CreateMarker() {
    m_rectHandle = System::Game::CreateGeom(0.0f, 0.0f, 80.0f, 80.0f);
    m_textHandle = System::Game::CreateText("spawner", 0.0f, 0.0f, "", 1);

    m_rectHandle->SetTint({ 1.0f, 0.0f, 0.0f });
    m_textHandle->SetTint({ 0.0f, 0.0f, 1.0f });

    System::Game::GetScene()->SetInteractive(m_rectHandle);
}


//---------------------------  


void SpawnerNode::Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr)
{

    ImGui::Separator(); 

    {
        assert(active);

        ImGui::PushID(("(Spawner) " + name).c_str());

        std::string selText = (Editor::selectedEntity && m_rectHandle) && Editor::selectedEntity->ID == m_rectHandle->ID ? "<SELECTED> " : "";

        if (ImGui::TreeNode((selText + "(Spawner) " + name).c_str()))
        {
            Node::Update(node, arr);

            if (ImGui::Button("Select") && m_rectHandle)
                Editor::selectedEntity = m_rectHandle;

            if (show_options)
            {
                static const char* items[] = { "sprite", "geometry" };

                if (ImGui::BeginCombo("type", m_spawnType.c_str()))
                {
                    for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                    {
                        bool is_sel = (m_spawnType == items[n]);

                        if (ImGui::Selectable(items[n], is_sel)) 
                            switch (n) {
                                case 0: typeOf = Entity::SPRITE; break;
                                case 1: typeOf = Entity::GEOMETRY; break;
                                default: break;
                            }

                        if (is_sel)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }

                if (ImGui::BeginMenu("select behavior script")) 
                {
                    if (behaviorKey.size())
                        ImGui::Text(behaviorKey.c_str());
                    else 
                        ImGui::Text("no behavior selected.");

                    ImGui::Separator();

                    if (ImGui::MenuItem("no behavior"))
                        behaviorKey = "";

                    //iterate over script dirs

                    std::string scriptDir = AssetManager::Get()->script_dir;

                    if (!std::filesystem::is_empty(Editor::projectPath + scriptDir))
                        for (const auto& script : std::filesystem::recursive_directory_iterator(Editor::projectPath + scriptDir)) 
                            if (!script.is_directory()) 
                            {
                                std::string filename = script.path().filename().string();

                                if (System::Utils::str_endsWith(filename, ".h")) {

                                    std::string file_name = System::Utils::ReplaceFrom(filename, ".", "");

                                    if (ImGui::MenuItem(file_name.c_str())) { 
                                        std::string className(file_name);
                                        className[0] = toupper(className[0]);
                                        behaviorKey = className;
                                    }
                                }
                        
                            }

                    ImGui::EndMenu();
                }

                //sprite

                if (typeOf == Entity::SPRITE)
                {
                    if (ImGui::ImageButton("texture button", (void*)(intptr_t)m_currentTexture, ImVec2(50, 50)) && System::Utils::GetFileType(AssetManager::Get()->selectedAsset) == System::Resources::Manager::IMAGE)
                        ApplyTexture(AssetManager::Get()->selectedAsset);
                
                    else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && AssetManager::Get()->selectedAsset.length() && System::Utils::GetFileType(AssetManager::Get()->selectedAsset) != System::Resources::Manager::IMAGE)
                        ImGui::SetTooltip("cannot set texture because selected asset is not of type image."); 

                    if (ImGui::BeginMenu("select preloaded spritesheet")) 
                    {
                        if (spriteSheetKey.first.size())
                            ImGui::Text(spriteSheetKey.first.c_str());
                        else 
                            ImGui::Text("no spritesheet selected.");

                        ImGui::Separator();

                        if (Editor::Get()->spritesheets.size())
                            for (const auto& spritesheet : Editor::Get()->spritesheets) {   
                                if (ImGui::MenuItem(spritesheet.first.c_str()))
                                    spriteSheetKey = spritesheet;
                            }
                        else
                            ImGui::Text("no spritesheets loaded.");

                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("select preloaded animations")) 
                    {
                        if (animationKey.size())
                            ImGui::Text(animationKey.c_str());
                        else 
                            ImGui::Text("no animation selected.");

                        ImGui::Separator();

                        if (Editor::Get()->animations.size())
                            for (const auto& animation : Editor::Get()->animations) {   
                                if (ImGui::MenuItem(animation.first.c_str()))
                                    animationKey = animation.first;
                            }
                        else
                            ImGui::Text("no spritesheets loaded.");

                        ImGui::EndMenu();
                    }
                }

                //geometry

                if (typeOf == Entity::GEOMETRY)
                {
                    ImGui::SliderFloat("width", &width, -System::Window::s_width, System::Window::s_width);  
                    ImGui::SliderFloat("height", &height, -System::Window::s_height, System::Window::s_height); 
                }

                ImGui::SliderFloat("position x", &positionX, -System::Window::s_width, System::Window::s_width);  
                ImGui::SliderFloat("position y", &positionY, -System::Window::s_height, System::Window::s_height); 
                ImGui::ColorEdit3("tint", (float*)&tint); 
                ImGui::SliderFloat("alpha", &alpha, 0.0f, 1.0f);
            }

            ImGui::TreePop();
        }

        ImGui::PopID();

    }

}


//------------------------------------


void SpawnerNode::Render()
{
    if (m_rectHandle && m_textHandle) {
        m_rectHandle->SetPosition(positionX, positionY);
        m_textHandle->SetPosition(positionX + 8.0f, positionY + 30.0f);
    }

    switch (typeOf) {
        case Entity::SPRITE: m_spawnType = "sprite"; break;
        case Entity::GEOMETRY: m_spawnType = "geometry"; break;
        default: break;
    }
}


