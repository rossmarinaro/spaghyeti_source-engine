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
    category = 0;
    m_spawnType = "sprite";
    m_bodyType = "kinematic";
    m_category = "red";
    textureKey = "";
    behaviorKey = "";
    animationKey = ""; 
    width = 0.0f;
    height = 0.0f;
    spawnWidth = System::Window::s_scaleWidth;
    spawnHeight = System::Window::s_scaleHeight;
    loop = false;
    alpha = 1.0f;
    tint = { 1.0f, 1.0f, 1.0f }; 
    body = { Physics::Body::KINEMATIC, Physics::Body::Shape::BOX, false, false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

    rectHandle = nullptr;
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
    if (rectHandle)
        System::Game::DestroyEntity(rectHandle);

    if (m_textHandle)
        System::Game::DestroyEntity(m_textHandle);
}

//---------------------------


void SpawnerNode::ApplyTexture(const std::string& asset) {
    m_currentTexture = Graphics::Texture2D::Get(asset).ID;  
    textureKey = asset;
}

//---------------------------


void SpawnerNode::CreateMarker() {
    rectHandle = System::Game::CreateGeom(0.0f, 0.0f, 80.0f, 80.0f);
    m_textHandle = System::Game::CreateText("spawner", 0.0f, 0.0f, "", 1);

    rectHandle->SetTint({ 1.0f, 0.0f, 0.0f });
    m_textHandle->SetTint({ 0.0f, 0.0f, 1.0f });

    System::Game::GetScene()->SetInteractive(rectHandle); 
}


//---------------------------  


void SpawnerNode::Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr)
{

    ImGui::Separator(); 

    {
        assert(active);

        ImGui::PushID(("(Spawner) " + name).c_str());

        std::string selText = (Editor::selectedEntity && rectHandle) && Editor::selectedEntity->ID == rectHandle->ID ? "<SELECTED> " : "";

        if (ImGui::TreeNode((selText + "(Spawner) " + name).c_str()))
        {
            Node::Update(node, arr);

            if (ImGui::Button("Select") && rectHandle)
                Editor::selectedEntity = rectHandle;

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

                static const char* colors[] = { "red", "orange", "yellow", "green", "blue", "indigo", "violet", "black", "white" };

                if (ImGui::BeginCombo("category", m_category.c_str()))
                {
                    for (int n = 0; n < IM_ARRAYSIZE(colors); n++)
                    {
                        bool is_sel = (m_category == colors[n]);

                        if (ImGui::Selectable(colors[n], is_sel)) 
                            category = n;

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

                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && m_currentTexture)
                        ImGui::SetTooltip(textureKey.c_str());

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

                    if (ImGui::BeginMenu("define physics body")) 
                    {
                        const std::string exists = body.exist ? "true" : "false";

                        if (ImGui::Button(("exists: " + exists).c_str()))
                            body.exist = !body.exist;

                        ImGui::SliderFloat("width", &body.w, 0.0f, System::Window::s_width); 
                        ImGui::SliderFloat("height", &body.h, 0.0f, System::Window::s_height);
                        ImGui::SliderFloat("offset x", &body.xOff, -System::Window::s_width, System::Window::s_width);  
                        ImGui::SliderFloat("offset y", &body.yOff, -System::Window::s_height, System::Window::s_height);
                        ImGui::Checkbox("is sensor", &body.is_sensor); 
                        
                        static const char* items[] = { "kinematic", "static", "dynamic" };

                        if (ImGui::BeginCombo("type", m_bodyType.c_str()))
                        {
                            for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                            {
                                bool is_sel = (m_bodyType == items[n]);

                                if (ImGui::Selectable(items[n], is_sel)) 
                                    switch (n) {
                                        case 0: body.type = Physics::Body::Type::KINEMATIC; break;
                                        case 1: body.type = Physics::Body::Type::STATIC; break;
                                        case 2: default: body.type = Physics::Body::Type::DYNAMIC; break;
                                    }

                                if (is_sel)
                                    ImGui::SetItemDefaultFocus();
                            }

                            ImGui::EndCombo();
                        }

                        if (body.type == Physics::Body::Type::DYNAMIC) {
                            ImGui::SliderFloat("density", &body.density, 0.0f, 1000.0f);
                            ImGui::SliderFloat("friction", &body.friction, 0.0f, 1.0f);
                            ImGui::SliderFloat("restitution", &body.restitution, 0.0f, 1.0f);
                        }

                        ImGui::EndMenu();
                    }
                }

                //geometry

                if (typeOf == Entity::GEOMETRY)
                {
                    ImGui::SliderFloat("width", &width, -System::Window::s_width, System::Window::s_width);  
                    ImGui::SliderFloat("height", &height, -System::Window::s_height, System::Window::s_height); 
                }

                ImGui::SliderFloat("position x", &positionX, 0.0f, System::Window::s_width);  
                ImGui::SliderFloat("position y", &positionY, 0.0f, System::Window::s_height); 
                ImGui::SliderFloat("spawn width", &spawnWidth, 0.0f, System::Window::s_width);  
                ImGui::SliderFloat("spawn height", &spawnHeight, 0.0f, System::Window::s_height); 
                ImGui::ColorEdit3("tint", (float*)&tint); 
                ImGui::SliderFloat("alpha", &alpha, 0.0f, 1.0f);
                ImGui::Checkbox("loop", &loop);
            }

            ImGui::TreePop();
        }

        ImGui::PopID();

    }

}


//------------------------------------


void SpawnerNode::Render(float _positionX, float _positionY, float _rotation, float _scaleX, float _scaleY)
{
    if (rectHandle && m_textHandle) 
    {
        rectHandle->SetPosition(positionX + _positionX, positionY + _positionY);
        m_textHandle->SetPosition((positionX + _positionX) + 8.0f, (positionY + _positionY) + 30.0f);

        //color coded types

        switch (category) {
            case 0: default:
                m_category = "red";
                rectHandle->SetTint({ 1.0f, 0.0f, 0.0f }); 
                m_textHandle->SetTint({ 0.0f, 0.0f, 1.0f });
            break;
            case 1: 
                m_category = "orange";
                rectHandle->SetTint({ 1.0f, 0.5f, 0.0f }); 
                m_textHandle->SetTint({ 0.0f, 0.0f, 1.0f });
            break;
            case 2: 
                m_category = "yellow";
                rectHandle->SetTint({ 1.0f, 1.0f, 0.0f }); 
                m_textHandle->SetTint({ 0.0f, 1.0f, 0.0f });
            break; 
            case 3: 
                m_category = "green";
                rectHandle->SetTint({ 0.0f, 1.0f, 0.0f }); 
                m_textHandle->SetTint({ 1.0f, 0.0f, 0.0f });
            break; 
            case 4: 
                m_category = "blue";
                rectHandle->SetTint({ 0.0f, 0.0f, 1.0f }); 
                m_textHandle->SetTint({ 1.0f, 0.0f, 0.0f });
            break; 
            case 5: 
                m_category = "indigo";
                rectHandle->SetTint({ 0.5f, 0.0f, 0.5f });
                m_textHandle->SetTint({ 1.0f, 1.0f, 1.0f });
            break; 
            case 6: 
                m_category = "violet";
                rectHandle->SetTint({ 0.25f, 0.0f, 0.4f }); 
                m_textHandle->SetTint({ 1.0f, 1.0f, 1.0f });
            break; 
            case 7: 
                m_category = "black";
                rectHandle->SetTint({ 0.0f, 0.0f, 0.0f }); 
                m_textHandle->SetTint({ 1.0f, 1.0f, 1.0f });
            break; 
            case 8: 
                m_category = "white";
                rectHandle->SetTint({ 1.0f, 1.0f, 1.0f }); 
                m_textHandle->SetTint({ 0.0f, 0.0f, 1.0f });
            break; 
        }
    }

    switch (typeOf) {
        case Entity::SPRITE: m_spawnType = "sprite"; break;
        case Entity::GEOMETRY: m_spawnType = "geometry"; break;
        default: break;
    }

    switch (body.type) {
        case Physics::Body::Type::DYNAMIC: m_bodyType = "dynamic"; break;
        case Physics::Body::Type::KINEMATIC: m_bodyType = "kinematic"; break;
        case Physics::Body::Type::STATIC: 
        default: m_bodyType = "static"; break;
    }

}


