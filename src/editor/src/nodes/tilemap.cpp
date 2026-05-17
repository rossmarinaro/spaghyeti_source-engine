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

    layers = new std::vector<System::Scene::TilemapLayer>();
    bodies = new std::vector<std::shared_ptr<Physics::Body>>();
    
    if (m_init)
        Editor::Log("Tilemap node " + name + " created."); 
}


//---------------------------
 

TilemapNode::~TilemapNode() 
{//System::Game::RemoveTilemap(const std::string& key);
   // if (m_mapApplied) 
        for (const auto& layer : *layers)
            System::Game::RemoveTileLayer(layer.ID);

    if (layers) {
        delete layers;
        layers = nullptr;
    }
    // if (bodies) {
    //     delete bodies;
    //     bodies = nullptr;
    // }

    if (m_init)
        Editor::Log("Tilemap node " + name + " deleted.");
}


//---------------------------


void TilemapNode::AddLayer() 
{
    System::Scene::TilemapLayer layer;

    layer.ID = layers->size();
    layer.dataKey = "";
    layer.textureKey = "";
    layer.path = "";
    layer.shader = "";
    layer.depth = layers->size() ? layers->back().depth + 1 : 0;
    layer.scrollFactorX = 1.0f;
    layer.scrollFactorY = 1.0f;
    layer.spriteWidth = 0.0f; 

    layers->push_back(layer);
}


//---------------------------


void TilemapNode::Reset(int component_type)
{
    bool passAll = component_type == Component::NONE;

    if (component_type == Component::PHYSICS || passAll && bodies) {
        //for (const auto& body : *bodies)
            //Physics::DestroyBody(body);

        //bodies->clear();
       // delete bodies;
       // bodies = nullptr;
    }

    if (passAll && layers) 
    {
        //for (const auto& _layer : *layers) 
          //  System::Game::RemoveTileLayer(_layer.ID); 
       // delete layers;
       // layers = nullptr;
    }
  
}


//---------------------------


void TilemapNode::InitMapFromJSON(const std::string& key, const std::string& path) 
{
    if (System::Utils::str_endsWith(path, ".json"))
    {
        System::Resources::Manager::LoadTilemapFromJSON(key, (Editor::projectPath + path));

        const auto map = System::Game::CreateTilemapFromJSON(key);

        for (const auto& layer : map.first)
            layers->emplace_back(layer);

        for (const auto& body : map.second)
            bodies->emplace_back(body);

        if (bodies && bodies->size()) 
            AddComponent(Component::PHYSICS, false);
    }
}


//---------------------------


void TilemapNode::CreateBody(float x, float y, float width, float height) {
    const auto pb = Physics::CreateBody(Physics::Body::Type::STATIC, x, y, width, height);
    bodies->emplace_back(pb);
}


//---------------------------


void TilemapNode::UpdateBody(int index) {
    if (bodies->size()) {
        const auto body = (*bodies)[index];
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
                        
                        int i = 0;

                        for (auto& body : *bodies)
                        {
                            ImGui::PushID(i);

                            ImGui::Text("body: %d", i);

                            ImGui::SliderFloat("width", &body->width, 0.0f, Editor::Get()->game->camera->currentBoundsWidthEnd, NULL, ImGuiSliderFlags_AlwaysClamp); 
                            ImGui::SliderFloat("height", &body->height, 0.0f, Editor::Get()->game->camera->currentBoundsHeightEnd, NULL, ImGuiSliderFlags_AlwaysClamp); 

                            ImGui::SliderFloat("x", &body->x, 0.0f, Editor::Get()->game->camera->currentBoundsWidthEnd, NULL, ImGuiSliderFlags_AlwaysClamp);  
                            ImGui::SliderFloat("y", &body->y, 0.0f, Editor::Get()->game->camera->currentBoundsHeightEnd, NULL, ImGuiSliderFlags_AlwaysClamp); 
                            
                            UpdateBody(i);

                            if (ImGui::Button("remove") && bodies->size() > 1) 
                            {
                                auto it = std::find_if(bodies->begin(), bodies->end(), [&](const std::shared_ptr<Physics::Body>& b) { return b->id == body->id; });
                            
                                if (it != bodies->end()) 
                                {
                                    const auto pb = *it;   

                                    it = bodies->erase(std::move(it));
                                    --it;

                                    if (pb)
                                        Physics::DestroyBody(pb); 
                                }

                                EventListener::UpdateSession();
                            }

                            ImGui::Separator();             
                            
                            ImGui::PopID();

                            i++;

                        }

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
                                    for (const auto& body : *bodies)
                                        Physics::DestroyBody(body);

                                    bodies->clear();

                                    //parse json to extract body data

                                    std::ifstream JSON(Editor::projectPath + path);

                                    json data = json::parse(JSON);

                                    const auto createBodies = [&](float x, float y, float w, float h) {

                                        CreateBody(x, y, w, h);
                                        
                                        for (int i = 0; i < bodies->size(); i++)
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
                    if (layers)
                    {
                        ImGui::Text("layers: %d", layers->size()); 

                        for (int i = 0; i < layers->size(); ++i)
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
                                                System::Resources::Manager::LoadTilemapFrames((*layers)[i].textureKey, (*layers)[i].spriteWidth, map_width, map_height, tile_width, tile_height);
                                                
                                                (*layers)[i] = System::Game::CreateTileLayer(layers->size(), (*layers)[i].textureKey.c_str(), (*layers)[i].dataKey.c_str(), map_width, map_height, tile_width, tile_height, (*layers)[i].depth, i, 0.0f, 0.0f, (*layers)[i].scrollFactorX, (*layers)[i].scrollFactorY);  
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

                            if (layers->size())
                                data_file_name = (*layers)[i].dataKey;

                            ImGui::Text(data_file_name.c_str()); 

                            if (ImGui::ImageButton("tex button", (void*)(intptr_t)Graphics::Texture2D::Get((*layers)[i].textureKey).ID, ImVec2(50, 50)) && System::Utils::GetFileType(AssetManager::Get()->selectedAsset) == System::Resources::Manager::IMAGE) 
                            {
                                (*layers)[i].textureKey = AssetManager::Get()->selectedAsset;
                                EventListener::UpdateSession();
                            }

                            else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && System::Utils::GetFileType(AssetManager::Get()->selectedAsset) != System::Resources::Manager::IMAGE)
                                ImGui::SetTooltip("cannot set texture because selected asset is not of type image.");

                            ImGui::SliderInt("depth", &(*layers)[i].depth, 0, 1000);
                            if (ImGui::IsItemDeactivatedAfterEdit())
                                EventListener::UpdateSession();

                            if (ImGui::InputFloat("scroll factor x", &(*layers)[i].scrollFactorX)) 
                                EventListener::UpdateSession();
                            
                            if (ImGui::InputFloat("scroll factor y", &(*layers)[i].scrollFactorY)) 
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
                                                (*layers)[i].shader = name;
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

                        if (ImGui::Button("remove layer") && layers && layers->size()) {
                            System::Game::RemoveTileLayer(layers->back().ID);
                            layers->pop_back();
                            EventListener::UpdateSession();
                        }  
                    }

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
}