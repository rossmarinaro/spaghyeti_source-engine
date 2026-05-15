#include "./node.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../../../../build/sdk/include/app.h"
#include "../../../vendors/box2d/include/box2d/box2d.h"
#include "../../../vendors/UUID.hpp"

using namespace editor;


TilemapNode::TilemapNode(bool init): 
    Node(init, TILEMAP),
        m_layersApplied(false),
        m_mapApplied(false)
{ 
    layer = 0;
    map_width = 10;
    map_height = 10;
    tile_width = 64;
    tile_height = 64;
    layers = nullptr; 
    bodies = nullptr;
    
    if (m_init)
        Editor::Log("Tilemap node " + name + " created."); 
}


//---------------------------
 

TilemapNode::~TilemapNode() {
    
 //   if (m_mapApplied) 
      //  for (const auto& _layer : layers)
         //   System::Game::RemoveTileLayer(_layer.ID);

    if (m_init)
        Editor::Log("Tilemap node " + name + " deleted.");
}


//---------------------------


void TilemapNode::AddLayer() 
{
    System::Scene::TilemapLayer _layer;

    _layer.ID = layers->size();
    _layer.dataKey = "";
    _layer.textureKey = "";
    _layer.path = "";
    _layer.shader = "";
    _layer.depth = layers->size() ? layers->back().depth + 1 : 0;
    _layer.scrollFactorX = 1.0f;
    _layer.scrollFactorY = 1.0f;
    _layer.spriteWidth = 0.0f; 

    layers->push_back(_layer);

    layer++;
    //spr_sheet_width.push_back(0);
    //spr_sheet_height.push_back(0);
}


//---------------------------


void TilemapNode::Reset(int component_type)
{
    bool passAll = component_type == Component::NONE;

    if (component_type == Component::PHYSICS || passAll && bodies) {
        for (const auto& body : *bodies)
            Physics::DestroyBody(body);

        bodies->clear();
        delete bodies;
        bodies = nullptr;
    }

    m_mapApplied = false;

    if (passAll && layers) 
    {
        for (const auto& _layer : *layers) 
            System::Game::RemoveTileLayer(_layer.ID); 
        delete layers;
        layers = nullptr;
    }
  
}


//---------------------------


void TilemapNode::ApplyTilemap(const std::string& dataKey)
{ 
    // for (int i = 0; i < layer; i++)
    // {
    //     const std::string dataKey = layers[i].dataKey,
    //                       texture = layers[i].textureKey;

    //     AssetManager::Register(texture); 
    //     AssetManager::Register(dataKey);

    //     const std::string layerID = System::Game::CreateTileLayer(
    //         texture.c_str(), 
    //         dataKey.c_str(), 
    //         map_width, 
    //         map_height, 
    //         tile_width, 
    //         tile_height, 
    //         layers[i].depth, 
    //         i
    //     );

    //    if (layerID.size()) { 
    //         layers[i].ID = layerID;
    //         m_layersApplied = true;
    //         m_mapApplied = true;
    //    }

    //    else {
    //        Editor::Log("There was a problem parsing tilemap file.");
    //        return;
    //    }
    // }

    // for (const auto& map : System::Game::GetScene()->tilemaps)
    //     if (map.second.second.size()) {
    //         AddComponent(Component::PHYSICS);
    //         break;
    //     }

    const std::string path = *System::Resources::Manager::GetFilePath(dataKey);
    std::ifstream JSON(path);

    if (!JSON.good()) {
        Editor::Log("problem");//LOG("Tilemap: unable to parse, invalid json.");
        return;
    }

    const json data = json::parse(JSON);

    if (data.contains("layers") && data["layers"].size()) {
        layer = data["layers"].size();
        for (const auto& layer : data["layers"]) {
            if (layer.contains("objects")) Editor::Log("Okkk");
        }
    }

    AssetManager::Register(dataKey);

    m_layersApplied = true;
    m_mapApplied = true;
    
    //set tiles initial position for group node origin data

    for (const auto& entity : System::Game::GetScene()->entities) 
        if (entity->GetType() == Entity::TILE) 
            entity->SetData("initial position", entity->position);      
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
             
                                    m_layersApplied = false;
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
                    ImGui::Text("layers: %d", layer); 

                    for (int i = 0; i < layer; ++i)
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

                                if (System::Utils::str_endsWith(path, ".csv")) 
                                {
                                    // if (ImGui::MenuItem(key.c_str())) {
                                    //     layers[i].dataKey = key;
                                    //     layers[i].path = path;
                                    //     layers[i].depth = i;
                                    //     spr_sheet_width[i] = Graphics::Texture2D::Get(layers[i].textureKey).Width / tile_width;
                                    //     spr_sheet_height[i] = Graphics::Texture2D::Get(layers[i].textureKey).Height / tile_height; 
                                    //     m_layersApplied = false;
                                    //     EventListener::UpdateSession();
                                    // }
                                }

                               if (System::Utils::str_endsWith(path, ".json")) {
                                    if (ImGui::MenuItem(key.c_str())) {
                                        System::Resources::Manager::LoadTilemapFromJSON(key, path);//ParseJSONData(key, path);
                                        ApplyTilemap(key);
                                    }
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
                            m_layersApplied = false;
                            EventListener::UpdateSession();
                        }

                        else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && System::Utils::GetFileType(AssetManager::Get()->selectedAsset) != System::Resources::Manager::IMAGE)
                            ImGui::SetTooltip("cannot set texture because selected asset is not of type image.");

                        if (ImGui::SliderInt("depth", &(*layers)[i].depth, 0, 1000)) 
                            m_layersApplied = false;
                        
                        if (ImGui::IsItemDeactivatedAfterEdit())
                            EventListener::UpdateSession();

                        if (ImGui::InputFloat("scroll factor x", &(*layers)[i].scrollFactorX)) {
                            m_layersApplied = false;
                            EventListener::UpdateSession();
                        }
                        
                        if (ImGui::InputFloat("scroll factor y", &(*layers)[i].scrollFactorY)) {
                            m_layersApplied = false;
                            EventListener::UpdateSession();
                        }

                        if (ImGui::BeginMenu("select shader"))
                        {
                            if (std::filesystem::is_empty(Editor::projectPath + AssetManager::Get()->shader_dir))
                                ImGui::Text("no shaders in directory.");

                            else
                                for (const auto &dir : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::Get()->shader_dir))
                                    if (dir.is_directory()) {
                                        std::string name = dir.path().filename().string();
                                        if (ImGui::MenuItem(name.c_str())) {
                                            m_layersApplied = false;
                                            (*layers)[i].shader = name;
                                            EventListener::UpdateSession();
                                        }
                                    }

                            ImGui::EndMenu();
                        }
 
                        ImGui::PopID();
                    } 

                    if (ImGui::Button("apply") && !m_layersApplied && layers->size()) {
                        //System::Game::RemoveTileLayer(layers[layer - 1].ID);
                        //ApplyTilemap();
                        EventListener::UpdateSession();
                    }

                    if (m_layersApplied) { 
                        ImGui::SameLine(); 
                        ImGui::Text("applied");
                    }

                    if (ImGui::Button("add layer")) {
                        AddLayer();                    
                        EventListener::UpdateSession();
                    } 

                    ImGui::SameLine();

                    if (ImGui::Button("remove layer") && layer > 0) 
                    {
                       // if (layers.size()) 
                            //System::Game::RemoveTileLayer(layers[layer - 1].ID);

                        layers->pop_back();
                        layer--;
                        EventListener::UpdateSession();
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

                        if (System::Utils::str_endsWith(path, ".json")) {
                            ImGui::SetTooltip("Note: Only CSV Tile format supported."); 
                            if (ImGui::MenuItem(key.c_str())) 
                            {
                                System::Resources::Manager::LoadTilemapFromJSON(key, (Editor::projectPath + path));

                                auto map = System::Game::CreateTilemapFromJSON(key);

                                //for (const auto& layer : map.first)
                                    //layers.emplace_back(layer);

                                //for (const auto& body : map.second)
                                    //bodies.emplace_back(body);

                                layers = new std::vector<System::Scene::TilemapLayer>();//&map.first;
                                bodies = new std::vector<std::shared_ptr<Physics::Body>>();//&map.second;

                                if (bodies && bodies->size()) 
                                    AddComponent(Component::PHYSICS, false);
                               // ApplyTilemap(key);
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

    for (const auto& entity : System::Game::GetScene()->entities) 
        if (entity->GetType() == Entity::TILE) 
        {
            const auto initialPosition = entity->GetData<Math::Vector2>("initial position");

            //entity->SetPosition(initialPosition.x + positionX, initialPosition.y + positionY);
            //entity->SetScale(entity->scale.x * scaleX, entity->scale.y * scaleY);
        }
}