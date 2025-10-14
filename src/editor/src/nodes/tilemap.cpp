#include "./node.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../../../../build/sdk/include/app.h"

#include "../../../vendors/box2d/include/box2d/box2d.h"


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

    if (m_init)
        Editor::Log("Tilemap node " + name + " created."); 
}


//---------------------------
 

TilemapNode::~TilemapNode() {
    
    if (m_mapApplied)
        MapManager::ClearMap();

    if (m_init)
        Editor::Log("Tilemap node " + name + " deleted.");
}


//---------------------------


void TilemapNode::AddLayer() {
    layers.push_back({ "", "" });
    layer++;
    spr_sheet_width.push_back(0);
    spr_sheet_height.push_back(0);
    depth.push_back(0);
}


//---------------------------


void TilemapNode::Reset(int component_type)
{

    bool passAll = component_type == Component::NONE;

    if (component_type == Component::PHYSICS || passAll) {
        for (const auto& body : bodies)
            Physics::DestroyBody(body.pb);

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
                    std::array<int, 6> off = { w, h, tile_width, tile_height, 1, 1 };
                    offset.emplace_back(off); 
                    w++;
                }
            }

        std::string csv = layers[i][0];
        const std::string path = layers[i][1], 
                          texture = layers[i][2]; 

        AssetManager::Register(texture); 
        AssetManager::Register(csv); 
        
        if (clearPrev)
        {
            if (isJSON)
                csv += "_" + std::to_string(i);

            System::Resources::Manager::LoadFrames(texture, offset); //image texture with frame offsets
            System::Resources::Manager::LoadFile(csv.c_str(), path.c_str()); //csv 
            
            //parse csv data and load map layer

            if (MapManager::CreateLayer(texture.c_str(), csv.c_str(), map_width, map_height, tile_width, tile_height, depth[i])) {

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
    const auto body = Physics::CreateBody(Physics::Body::Type::STATIC, Physics::Body::Shape::BOX, x, y, width, height);
    const Body b = { body, x, y, width, height };

    bodies.emplace_back(b);
}


//---------------------------


void TilemapNode::UpdateBody(int index) 
{
    b2PolygonShape body;  

    body.SetAsBox(bodies[index].width / 2, bodies[index].height / 2);
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &body; 

    if (bodies[index].pb) {
        bodies[index].pb->DestroyFixture();
        bodies[index].pb->CreateFixture(&fixtureDef);
        bodies[index].pb->SetTransform(bodies[index].x + bodies[index].width / 2, bodies[index].y + bodies[index].height / 2);
    }
}


//---------------------------


void TilemapNode::Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr)
{
    ImGui::Separator(); 

    {

        ImGui::PushID(("(Tilemap) " + name).c_str());

        if (ImGui::TreeNode(("(Tilemap) " + name).c_str()))
        {
        
            Node::Update(node, arr);

            if (ImGui::BeginMenu("Add Component"))
            {
                if (ImGui::MenuItem("Physics"))
                    AddComponent(Component::PHYSICS);
            
                ImGui::EndMenu();
            }

            //components

            if (HasComponent(Component::PHYSICS) && ImGui::BeginMenu("Physics"))
            { 

                auto physics_component = GetComponent(Component::PHYSICS, ID);

                if (physics_component)
                {

                    if (ImGui::BeginMenu("bodies"))
                    {
                        ImGui::Text("manual input: CTRL + click");
                        
                        int i = 0;

                        for (auto& body : bodies)
                        {

                            ImGui::PushID(i);

                            ImGui::Text("body: %d", i);

                            ImGui::SliderFloat("width", &body.width, 0.0f, Editor::Get()->game->camera->currentBoundsWidthEnd * 2, NULL, ImGuiSliderFlags_AlwaysClamp); 
                            ImGui::SliderFloat("height", &body.height, 0.0f, map_height * tile_height, NULL, ImGuiSliderFlags_AlwaysClamp); 

                            ImGui::SliderFloat("x", &body.x, 0.0f, Editor::Get()->game->camera->currentBoundsWidthEnd * 2, NULL, ImGuiSliderFlags_AlwaysClamp);  
                            ImGui::SliderFloat("y", &body.y, 0.0f, Editor::Get()->worldHeight, NULL, ImGuiSliderFlags_AlwaysClamp); 
                            
                            UpdateBody(i);

                            if (ImGui::Button("remove") && bodies.size() > 1) 
                            {
                                auto it = std::find_if(bodies.begin(), bodies.end(), [&](const Body& b) { return b.pb->id == body.pb->id; });
                            
                                if (it != bodies.end()) 
                                {
                                    auto bod = *it;   

                                    it = bodies.erase(std::move(it));
                                    --it;

                                    if (bod.pb)
                                        Physics::DestroyBody(bod.pb); 
                                }
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
                                    for (const auto& body : bodies)
                                        Physics::DestroyBody(body.pb);

                                    bodies.clear();

                                    //parse json to extract body data

                                    std::ifstream JSON(Editor::projectPath + path);

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

                            for (const auto& asset : AssetManager::Get()->loadedAssets)
                            {

                                std::string key = asset.first;
                                std::string path = asset.second;

                                key.erase(std::remove(key.begin(), key.end(), '\"'), key.end());
                                path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());

                                if (System::Utils::str_endsWith(path, ".csv")) 
                                    if (ImGui::MenuItem(key.c_str())) {
                                        layers[i][1] = path;
                                        layers[i][0] = key;
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

                        if (ImGui::ImageButton("tex button", (void*)(intptr_t)Graphics::Texture2D::Get(layers[i][2]).ID, ImVec2(50, 50)) && System::Utils::GetFileType(AssetManager::Get()->selectedAsset) == System::Resources::Manager::IMAGE) 
                        {
                            layers[i][2] = AssetManager::Get()->selectedAsset;
                            m_layersApplied = false;
                        }

                        else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && System::Utils::GetFileType(AssetManager::Get()->selectedAsset) != System::Resources::Manager::IMAGE)
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

                                //parse json to extract body data

                                std::ifstream JSON(Editor::projectPath + path);

                                json data = json::parse(JSON);

                                auto createBodies = [&](float x, float y, float w, float h) {

                                    CreateBody(x, y, w, h);
                                    
                                    for (int i = 0; i < bodies.size(); i++)
                                        UpdateBody(i);

                                };

                                if (!data.contains("width") || !data.contains("height") || !data.contains("tilewidth") || !data.contains("tileheight") || !data.contains("layers"))
                                    Editor::Log("Bad JSON!");

                                else 
                                {
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

                                                    AddComponent(Component::PHYSICS);

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

                    }

                    ImGui::EndMenu();

                }
                
            }

            ImGui::TreePop();
        }


        ImGui::PopID();

    }
}

