#include "./node.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../../../../build/sdk/include/app.h"
#include "../../../vendors/box2d/include/box2d/box2d.h"
#include "../../../vendors/UUID.hpp"

using namespace editor;


TilemapNode::TilemapNode(bool init): 
    Node(init, TILEMAP)
{ 
    map_width = 10;
    map_height = 10;
    tile_width = 64;
    tile_height = 64;
    map = {};
    
    if (m_init)
        Editor::Log("Tilemap node " + name + " created."); 
}


//---------------------------
 

TilemapNode::~TilemapNode() 
{
    if (m_init)
        Editor::Log("Tilemap node " + name + " deleted.");
}


//---------------------------


void TilemapNode::AddLayer() 
{
    System::Scene::TilemapLayer layer;

    layer.ID = UUID::generate_uuid();
    layer.dataKey = "";
    layer.textureKey = "";
    layer.path = "";
    layer.shader = "";
    layer.depth = map.layers.size() ? map.layers.back().depth + 1 : 0;
    layer.scrollFactorX = 1.0f;
    layer.scrollFactorY = 1.0f;
    layer.spriteWidth = 0.0f; 

    map.layers.emplace_back(layer);
}


//---------------------------


void TilemapNode::Reset(int component_type)
{
    bool passAll = component_type == Component::NONE;

    if (passAll) 
    {
        for (const auto& layer : map.layers) 
           System::Game::RemoveTileLayer(layer.ID);

        map.layers.clear();
    }

    if (component_type == Component::PHYSICS || passAll)
    {
        for (const auto& body : map.bodies)
            Physics::DestroyBody(body);

        map.bodies.clear();
    }
}


//---------------------------


void TilemapNode::InitMapFromJSON(const std::string& key, const std::string& path) 
{
    if (System::Utils::str_endsWith(path, ".json"))
    {
        System::Resources::Manager::LoadTilemapFromJSON(key, (Editor::projectPath + path));

        map = System::Game::CreateTilemapFromJSON(key);

        map_width = map.width;
        map_height = map.height;
        tile_width = map.tileWidth;
        tile_height = map.tileHeight;

        if (map.bodies.size()) 
            AddComponent(Component::PHYSICS, false);
    }
}


//---------------------------


void TilemapNode::CreateBody(float x, float y, float width, float height) {
    const auto pb = Physics::CreateBody(Physics::Body::Type::STATIC, x, y, width, height);
    map.bodies.emplace_back(pb);
}


//---------------------------


void TilemapNode::UpdateBody(int index) {
    if (map.bodies.size()) {
        const auto body = map.bodies[index];
        body->UpdateFixture(body->width / 2, body->height / 2);
        body->SetTransform(body->x + body->width / 2, body->y + body->height / 2);
    }
}


//---------------------------


void TilemapNode::Update(std::vector<std::shared_ptr<Node>>& arr)
{
    ImGui::Separator(); 

    {
        ImGui::PushID(("(Tilemap) " + name).c_str());

        if (GUI::Get()->collapseFolders)
            ImGui::SetNextItemOpen(false, ImGuiCond_Always);

        if (ImGui::TreeNode(("(Tilemap) " + name).c_str()))
        {
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                Editor::Get()->events->selected_nodes = &nodes;

            Node::Update(arr);

            if (ImGui::BeginMenu("Add Component"))
            {
                if (ImGui::MenuItem("Physics")) {
                    AddComponent(Component::PHYSICS);
                    EventListener::UpdateSession();
                }
            
                ImGui::EndMenu();
            }

            //components

            if (HasComponent(Component::PHYSICS) && ImGui::BeginMenu("Physics"))
            { 
                const auto physics_component = GetComponent(Component::PHYSICS, ID);

                if (physics_component)
                {
                    if (ImGui::BeginMenu("bodies"))
                    {
                        ImGui::Text("manual input: CTRL + click");

                        //container for removing bodies in any arbitrary order from main loop over bodies

                        std::vector<std::string> bodies_to_remove;

                        for (int i = 0; i < map.bodies.size(); i++)
                        {
                            ImGui::PushID(i);

                            ImGui::Text("body: %d", i);

                            auto body = map.bodies[i]; 

                            ImGui::SliderFloat("width", &body->width, 0.0f, Editor::Get()->game->camera->currentBoundsWidthEnd, NULL, ImGuiSliderFlags_AlwaysClamp); 
                            ImGui::SliderFloat("height", &body->height, 0.0f, Editor::Get()->game->camera->currentBoundsHeightEnd, NULL, ImGuiSliderFlags_AlwaysClamp); 

                            ImGui::SliderFloat("x", &body->x, 0.0f, Editor::Get()->game->camera->currentBoundsWidthEnd, NULL, ImGuiSliderFlags_AlwaysClamp);  
                            ImGui::SliderFloat("y", &body->y, 0.0f, Editor::Get()->game->camera->currentBoundsHeightEnd, NULL, ImGuiSliderFlags_AlwaysClamp); 
                            
                            UpdateBody(i);

                            if (ImGui::Button("remove") && map.bodies.size()) 
                            {
                                bodies_to_remove.emplace_back(body->id);
             
                                if (body)
                                    Physics::DestroyBody(body); 

                                EventListener::UpdateSession();
                            }

                            if (i < map.bodies.size() - 1)
                                ImGui::Separator();             
                            
                            ImGui::PopID();
                        }

                        //remove any bodies qeued for removal

                        for (const auto& id : bodies_to_remove)
                            map.bodies.erase(std::remove_if(map.bodies.begin(), map.bodies.end(), [&id](const std::shared_ptr<Physics::Body>& b) { return b->id == id; }), map.bodies.end());
                        
                            bodies_to_remove.clear();

                        ImGui::EndMenu();
                    }

                    //load collision data from file

                    if (ImGui::BeginMenu("load collision data")) 
                    {
                        for (const auto& asset : AssetManager::Get()->loadedAssets)
                        {
                            std::string key = asset.first,
                                        path = asset.second;

                            key.erase(std::remove(key.begin(), key.end(), '\"'), key.end());
                            path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());

                            if (System::Utils::str_endsWith(path, ".json")) 
                            {
                                if (ImGui::MenuItem(key.c_str())) 
                                {
                                    for (const auto& body : map.bodies)
                                        Physics::DestroyBody(body);

                                    map.bodies.clear();

                                    //parse json to extract body data

                                    std::ifstream JSON(Editor::projectPath + path);

                                    json data = json::parse(JSON);

                                    const auto createBodies = [&](float x, float y, float w, float h) {

                                        CreateBody(x, y, w, h);
                                        
                                        for (int i = 0; i < map.bodies.size(); i++)
                                            UpdateBody(i);
                                    };

                                    if (data.contains("objects"))
                                        for (const auto& body : data["objects"])
                                            createBodies(body["x"], body["y"], body["width"], body["height"]);

                                    else if (data.contains("layers"))
                                        for (const auto& layer : data["layers"])
                                            if (layer.contains("objects"))
                                                for (const auto& body : layer["objects"])
                                                    createBodies(body["x"], body["y"], body["width"], body["height"]);
             
                                    EventListener::UpdateSession();
                                }
                            }
                        }
                        
                        ImGui::EndMenu();
                    }
                    
                    if (ImGui::Button("add")) {
                        CreateBody();
                        EventListener::UpdateSession();
                    }

                    ImGui::SameLine();

                    if (ImGui::BeginMenu("remove physics?"))
                    {
                        if (ImGui::MenuItem("yes"))
                            RemoveComponent(physics_component); 

                        ImGui::EndMenu();
                    }                
                }

                ImGui::EndMenu();
            }

            if (show_options)
            {
                //map dimensions
                
                if (ImGui::InputInt("map width", &map_width))
                    EventListener::UpdateSession();
                if (ImGui::InputInt("map height", &map_height))
                    EventListener::UpdateSession();
                if (ImGui::InputInt("tile width", &tile_width))
                    EventListener::UpdateSession();
                if (ImGui::InputInt("tile height", &tile_height))
                    EventListener::UpdateSession();
                
                if (ImGui::BeginMenu("layers")) 
                {
                    if (map.layers.size())
                    {
                        ImGui::Text("layers: %d", map.layers.size()); 

                        for (int i = 0; i < map.layers.size(); ++i)
                        {
                            ImGui::Separator();
                        
                            ImGui::Text("layer: %d", i);
                            
                            ImGui::PushID(i);
                        
                            if (ImGui::BeginMenu("data file: ")) 
                            {
                                for (const auto& asset : AssetManager::Get()->loadedAssets)
                                {
                                    std::string key = asset.first;
                                    std::string path = asset.second;

                                    key.erase(std::remove(key.begin(), key.end(), '\"'), key.end());
                                    path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());

                                    if (System::Utils::str_endsWith(path, ".csv") || System::Utils::str_endsWith(path, ".json")) 
                                        if (ImGui::MenuItem(key.c_str())) 
                                        {
                                            if (System::Utils::str_endsWith(path, ".csv")) 
                                            {
                                                System::Resources::Manager::LoadFile(key, path);
                                                System::Resources::Manager::LoadTilemapFrames(map.layers[i].textureKey, map.layers[i].spriteWidth, map_width, map_height, tile_width, tile_height);
                                                
                                                map.layers[i] = System::Game::CreateTileLayer(map.layers[i].textureKey.c_str(), map.layers[i].dataKey.c_str(), map_width, map_height, tile_width, tile_height, map.layers[i].depth, i, 0.0f, 0.0f, map.layers[i].scrollFactorX, map.layers[i].scrollFactorY);  
                                            }
                                            else 
                                                InitMapFromJSON(key, path);
                                                
                                            AssetManager::Register(key);
                                            EventListener::UpdateSession();
                                        }      
                                }
                    
                                ImGui::EndMenu();
                            }
                            
                            ImGui::SameLine();

                            std::string data_file_name = "";

                            if (map.layers.size())
                                data_file_name = map.layers[i].dataKey;

                            ImGui::Text(data_file_name.c_str()); 

                            if (ImGui::ImageButton("tex button", (void*)(intptr_t)Graphics::Texture2D::Get(map.layers[i].textureKey).ID, ImVec2(50, 50)) && System::Utils::GetFileType(AssetManager::Get()->selectedAsset) == System::Resources::Manager::IMAGE) 
                            {
                                map.layers[i].textureKey = AssetManager::Get()->selectedAsset;
                                EventListener::UpdateSession();
                            }

                            else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && System::Utils::GetFileType(AssetManager::Get()->selectedAsset) != System::Resources::Manager::IMAGE)
                                ImGui::SetTooltip("cannot set texture because selected asset is not of type image.");

                            ImGui::SliderInt("depth", &map.layers[i].depth, 0, 1000);
                            if (ImGui::IsItemDeactivatedAfterEdit())
                                EventListener::UpdateSession();

                            if (ImGui::InputFloat("scroll factor x", &map.layers[i].scrollFactorX)) 
                                EventListener::UpdateSession();
                            
                            if (ImGui::InputFloat("scroll factor y", &map.layers[i].scrollFactorY)) 
                                EventListener::UpdateSession();

                            if (ImGui::BeginMenu("select shader"))
                            {
                                if (std::filesystem::is_empty(Editor::projectPath + AssetManager::Get()->shader_dir))
                                    ImGui::Text("no shaders in directory.");

                                else
                                    for (const auto &dir : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::Get()->shader_dir))
                                        if (dir.is_directory()) {
                                            std::string name = dir.path().filename().string();
                                            if (ImGui::MenuItem(name.c_str())) {
                                                map.layers[i].shader = name;
                                                EventListener::UpdateSession();
                                            }
                                        }

                                ImGui::EndMenu();
                            }
    
                            ImGui::PopID();
                        } 

                        if (ImGui::Button("add layer")) {
                            AddLayer();                    
                            EventListener::UpdateSession();
                        } 

                        ImGui::SameLine();

                        if (ImGui::Button("remove layer") && map.layers.size()) {
                            System::Game::RemoveTileLayer(map.layers.back().ID);
                            map.layers.pop_back();
                            EventListener::UpdateSession();
                        }  
                    }
                    else
                        ImGui::Text("No layers defined.");

                    ImGui::EndMenu();
                }

                //load map from json

                if (ImGui::BeginMenu("load json map")) 
                {
                    for (const auto& asset : AssetManager::Get()->loadedAssets)
                    {
                        std::string key = asset.first,
                                    path = asset.second;

                        key.erase(std::remove(key.begin(), key.end(), '\"'), key.end());
                        path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());

                        if (System::Utils::str_endsWith(path, ".json")) 
                        {
                            ImGui::SetTooltip("Note: Only CSV Tile format supported."); 

                            if (ImGui::MenuItem(key.c_str())) {
                                InitMapFromJSON(key, path);
                                AssetManager::Register(key);
                                EventListener::UpdateSession();
                            }
                        }    
                    }

                    EventListener::UpdateSession();

                    ImGui::EndMenu();
                }
            }

            ImGui::TreePop();
        }

        ImGui::PopID();

    }
}


//--------------------------------------


void TilemapNode::Render(float _positionX, float _positionY, float _rotation, float _scaleX, float _scaleY)
{
    positionX = _positionX;
    positionY = _positionY; 
    rotation = _rotation;
    scaleX = _scaleX;
    scaleY = _scaleY;

    //update tilesprite

    for (auto it = map.layers.begin(); it != map.layers.end(); ++it) 
        if (it != map.layers.end())
            for (const auto& entity : System::Game::GetScene()->entities) 
            {
                const auto layer = *it; 

                if (entity->GetType() == Entity::TILE && layer.ID == entity->GetData<std::string>("id"))
                    entity->SetDepth(layer.depth); 
            }
}