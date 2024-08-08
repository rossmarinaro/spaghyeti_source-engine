#include "./node.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../../../../build/sdk/include/app.h"

using namespace editor;


TilemapNode::TilemapNode(): 
    Node("Tilemap"),
        m_layersApplied(false),
        m_mapApplied(false)
{ 
    layer = 0;
    map_width = 10;
    map_height = 10;
    tile_width = 64;
    tile_height = 64;

    Editor::Log("Tilemap node " + name + " created."); 
}


//---------------------------
 

TilemapNode::~TilemapNode() 
{

    if (m_mapApplied)
        MapManager::ClearMap();

    if (!virtual_node)
        Editor::Log("Tilemap node " + name + " deleted.");
}


//---------------------------


void TilemapNode::AddLayer()
{
    layers.push_back({ "", "" });
    layer++;
    spr_sheet_width.push_back(0);
    spr_sheet_height.push_back(0);
    depth.push_back(0);
}


//---------------------------


void TilemapNode::Reset(const char* component_type)
{

    bool passAll = strcmp(component_type, "") == 0;

    if (strcmp(component_type, "Physics") == 0 || passAll) 
    {
        for (const auto& body : bodies)
            Physics::DestroyBody(body);

        bodyX.clear();
        bodyY.clear();
        body_width.clear(); 
        body_height.clear();

        bodies.clear();
    }

    m_mapApplied = false;

    if (passAll)
        MapManager::ClearMap();
}


//---------------------------


void TilemapNode::ApplyTilemap(bool clearPrev, bool renderReversed, bool isJSON)
{

    if (clearPrev) {
        MapManager::ClearMap();
        offset.clear();
    }

    //for every layer, iterate over widths and heights by index, decrementing height each iteration by 1 until 0

    for (int i = 0; i < layer; i++) 
    { 

        int w = 0,  
            h = renderReversed ? spr_sheet_height[i] - 1 : 0; 

        for (int y = 0; y < map_height; ++y)
            for (int x = 0; x < map_width; ++x)
            {
                if (w == spr_sheet_width[i]) 
                { 
                    w = 0;

                    if (renderReversed)
                        h--;  
                    else 
                        h++; 
                }   

                if (w < map_width) {

                    offset.push_back({ w, h, tile_width, tile_height, 1, 1 }); 
                    w++;
                }
            }

        std::string key = layers[i][0], //csv
                    path = layers[i][1], //path
                    texture = layers[i][2]; //image

        AssetManager::Register(texture, clearPrev); //image
        AssetManager::Register(key, clearPrev); //csv
        
        if (clearPrev)
        {
            if (isJSON)
                key += "_" + std::to_string(i);

            System::Resources::Manager::LoadFrames(texture, offset); //image texture with frame offsets
            System::Resources::Manager::LoadFile(key.c_str(), path.c_str()); //csv 
            
            //parse csv data and load map layer

            const std::vector<std::string>& data = System::Resources::Manager::ParseCSV(key, i);

            if (data.size()) {

                MapManager::CreateLayer(texture.c_str(), key.c_str(), data, map_width, map_height, tile_width, tile_height, depth[i]);

                m_layersApplied = true;
                m_mapApplied = true;
            }

            else 
                Editor::Log("There was a problem parsing CSV file.");
        }

    }
}


//---------------------------


void TilemapNode::CreateBody(float x, float y, float width, float height) 
{

    bodyX.push_back(x);
    bodyY.push_back(y);
    body_width.push_back(width);
    body_height.push_back(height);

    auto body = Physics::CreateStaticBody(x, y, width, height);

    bodies.push_back(body);
}


//---------------------------


void TilemapNode::UpdateBody(int index) 
{

    b2PolygonShape body; 
    body.SetAsBox(body_width[index] / 2, body_height[index] / 2);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &body; 

    bodies[index]->DestroyFixture(bodies[index]->GetFixtureList());
    bodies[index]->CreateFixture(&fixtureDef);
    bodies[index]->SetTransform(b2Vec2(bodyX[index] + body_width[index] / 2, bodyY[index] + body_height[index] / 2), 0);
}


//---------------------------


void TilemapNode::Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr)
{

    ImGui::Separator(); 

    {

        ImGui::PushID(("(Tilemap) " + name).c_str());

        if (ImGui::TreeNode(("(Tilemap) " + name).c_str()))
        {
        
            static char buf1[32] = ""; ImGui::InputText("name", buf1, 32, ImGuiInputTextFlags_CallbackCompletion, ChangeName, &ID);

            if (ImGui::BeginMenu("Add Component"))
            {
                if (ImGui::MenuItem("Physics"))
                    AddComponent("Physics");
            
                ImGui::EndMenu();
            }

            //components

            if (HasComponent("Physics") && ImGui::BeginMenu("Physics"))
            { 

                auto physics_component = GetComponent("Physics", ID);

                if (physics_component)
                {

                    if (ImGui::BeginMenu("bodies"))
                    {
                        ImGui::Text("manual input: CTRL + click");

                        for (int i = 0; i < bodies.size(); i++)
                        {

                            ImGui::PushID(i);

                            ImGui::Text("body: %d", i);

                            ImGui::SameLine();

                            if (ImGui::Button("remove") && bodies.size() > 1) 
                            {
                                auto it = std::find(bodies.begin(), bodies.end(), bodies[i]);
                                if (it != bodies.end()) {
                                    Physics::DestroyBody(*it); 
                                    bodies.erase(it);
                                }
                            }

                            ImGui::SliderFloat("width", &body_width[i], 0.0f, map_width * tile_width, NULL, ImGuiSliderFlags_AlwaysClamp); 
                            ImGui::SliderFloat("height", &body_height[i], 0.0f, map_height * tile_height, NULL, ImGuiSliderFlags_AlwaysClamp); 

                            ImGui::SliderFloat("x", &bodyX[i], 0.0f, map_width * tile_width, NULL, ImGuiSliderFlags_AlwaysClamp);  
                            ImGui::SliderFloat("y", &bodyY[i], 0.0f, map_height * tile_height, NULL, ImGuiSliderFlags_AlwaysClamp); 

                            ImGui::Separator();             
                            
                            UpdateBody(i);
                            
                            ImGui::PopID();

                        }

                        ImGui::EndMenu();
                    }

                    //load collision data from file

                    if (ImGui::BeginMenu("load collision data")) 
                    {
                        for (const auto& asset : AssetManager::loadedAssets)
                        {

                            std::string key = asset.first,
                                        path = asset.second;

                            key.erase(std::remove(key.begin(), key.end(), '\"'), key.end());
                            path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());

                            if (System::Utils::str_endsWith(path, ".json")) 
                            {

                                if (ImGui::MenuItem(key.c_str())) 
                                {

                                    for (const auto& body : bodies)
                                        Physics::DestroyBody(body);

                                    bodyX.clear();
                                    bodyY.clear();
                                    body_width.clear();
                                    body_height.clear();

                                    bodies.clear();

                                    //parse json to extract body data

                                    std::ifstream JSON(path);

                                    json data = json::parse(JSON);

                                    auto createBodies = [&](float x, float y, float w, float h) {

                                        CreateBody(x, y, w, h);
                                        
                                        for (int i = 0; i < bodies.size(); i++)
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
                                }
                            }
                        }
                        
                        ImGui::EndMenu();
                    }
                    
                    if (ImGui::Button("add")) 
                        CreateBody();

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

            if (ImGui::BeginMenu("Options")) {
                ShowOptions(node, arr);
                ImGui::EndMenu();
            }

            ImGui::Checkbox("Edit", &show_options);

            if (show_options)
            {

                //map dimensions
                
                ImGui::InputInt("map width", &map_width);
                ImGui::InputInt("map height", &map_height);
                ImGui::InputInt("tile width", &tile_width);
                ImGui::InputInt("tile height", &tile_height);
                
                if (ImGui::BeginMenu("layers")) 
                {

                    ImGui::Text("layers: %d", layer); 

                    for (int i = 0; i < layer; ++i)
                    {
                        ImGui::Separator();
                    
                        ImGui::Text("layer: %d", i);
                        
                        ImGui::PushID(i);
                       
                        if (ImGui::BeginMenu("CSV: ")) 
                        {

                            for (const auto& asset : AssetManager::loadedAssets)
                            {

                                std::string key = asset.first;
                                std::string path = asset.second;

                                key.erase(std::remove(key.begin(), key.end(), '\"'), key.end());
                                path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());

                                if (System::Utils::str_endsWith(path, ".csv")) 
                                    if (ImGui::MenuItem(key.c_str())) {

                                        layers[i][1] = path;

                                        m_layersApplied = false;
                                    }
                            }
                  
                            ImGui::EndMenu();
                        }
                        
                        ImGui::SameLine();

                        std::string csv_name = "";

                        if (layers.size())
                            csv_name = layers[i][0];

                        ImGui::Text(csv_name.c_str()); 

                        if (ImGui::ImageButton("tex button", (void*)(intptr_t)Graphics::Texture2D::Get(layers[i][1]).ID, ImVec2(50, 50)) && System::Utils::GetFileType(AssetManager::selectedAsset) == "image") 
                        {
                            layers[i][1] = AssetManager::selectedAsset;
                            m_layersApplied = false;
                        }

                        else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && System::Utils::GetFileType(AssetManager::selectedAsset) != "image")
                            ImGui::SetTooltip("cannot set texture because selected asset is not of type image.");

                        if (
                            ImGui::InputInt("frames x", &spr_sheet_width[i]) ||
                            ImGui::InputInt("frames y", &spr_sheet_height[i]) ||
                            ImGui::SliderInt("depth", &depth[i], 0, 1000)
                        )
                            m_layersApplied = false;
 
                        ImGui::PopID();
                    } 

                    if (ImGui::Button("apply") && !m_layersApplied)
                        ApplyTilemap();

                    if (m_layersApplied) { 
                        ImGui::SameLine(); 
                        ImGui::Text("applied");
                    }

                    if (ImGui::Button("add layer")) 
                        AddLayer();                    

                    ImGui::SameLine();

                    if (ImGui::Button("remove layer") && layer > 1) {

                        if (layers.size())
                            System::Game::maps->RemoveLayer(layers[layer - 1][0]);

                        layers.pop_back();
                        layer--;
                    }

                    ImGui::EndMenu();
                }

                //load map from json

                if (ImGui::BeginMenu("load map")) 
                {


                    for (const auto& asset : AssetManager::loadedAssets)
                    {

                        std::string key = asset.first,
                                    path = asset.second;

                        key.erase(std::remove(key.begin(), key.end(), '\"'), key.end());
                        path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());

                        if (System::Utils::str_endsWith(path, ".json")) 
                        {

                            if (ImGui::MenuItem(key.c_str())) 
                            {

                                //parse json to extract body data

                                std::ifstream JSON(path);

                                json data = json::parse(JSON);

                                auto createBodies = [&](float x, float y, float w, float h) {

                                    CreateBody(x, y, w, h);
                                    
                                    for (int i = 0; i < bodies.size(); i++)
                                        UpdateBody(i);

                                };

                                map_width = data["width"];
                                map_height = data["height"];
                                tile_width = data["tilewidth"];
                                tile_height = data["tileheight"];

                                int i = 0, 
                                    d = 0;

                                if (data["layers"].size())
                                    for (const auto& layer : data["layers"])    
                                        if (layer.contains("data")) 
                                        {
                                            AddLayer();

                                            layers[i][1] = path;
                                            layers[i][2] = static_cast<std::string>(data["tilesets"][0]["name"]) + ".png";
                                            depth[i] = d;

                                            spr_sheet_width[i] = Graphics::Texture2D::Get(layers[2][1]).Width / tile_width;
                                            spr_sheet_height[i] = Graphics::Texture2D::Get(layers[2][1]).Height / tile_height;

                                            Reset();
                                            
                                            if (layer.contains("objects")) {

                                                AddComponent("Physics");

                                                for (const auto& body : layer["objects"])
                                                    createBodies(body["x"], body["y"], body["width"], body["height"]);
                                            }

                                            i++;
                                            d++;
                                        }

                                ApplyTilemap(true, false, true);

                            }

                        }

                    }

                    ImGui::EndMenu();

                }
                
            }

            ImGui::TreePop();
        }


        ImGui::PopID();

    }
}

