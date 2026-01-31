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


void TilemapNode::AddLayer() 
{
    Layer l;

    l.dataKey = "";
    l.textureKey = "";
    l.path = "";
    l.shader = "";
    l.depth = layers.size() ? layers.back().depth + 1 : 0;
    l.scrollFactorX = 1.0f;
    l.scrollFactorY = 1.0f;

    layers.push_back(l);

    layer++;
    spr_sheet_width.push_back(0);
    spr_sheet_height.push_back(0);
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
    if (clearPrev) 
        offset.clear();

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

        std::string dataKey = layers[i].dataKey;

        const std::string texture = layers[i].textureKey,
                          localPath = Editor::projectPath + "resources/assets/data/" + dataKey; 

        AssetManager::Register(texture); 
        AssetManager::Register(dataKey); 
        
        if (clearPrev)
        {
            System::Resources::Manager::LoadFrames(texture, offset); //image texture with frame offsets
            System::Resources::Manager::LoadFile(dataKey.c_str(), localPath.c_str()); //csv or json data relative to current project
     
            //parse data and load map layer

            if (MapManager::CreateLayer(texture.c_str(), dataKey.c_str(), map_width, map_height, tile_width, tile_height, layers[i].depth, i)) 
            {
                m_layersApplied = true;
                m_mapApplied = true;
            }

            else {
                const std::string dataType = isJSON ? "JSON" : "CSV";
                Editor::Log("There was a problem parsing " + dataType + " file.");
            }
        }

    }

    //set tiles initial position for group node origin data

    for (const auto& entity : System::Game::GetScene()->entities) 
        if (entity->GetType() == Entity::TILE) 
            entity->SetData("initial position", entity->position);
}


//--------------------------- parse json to extract tileset and body data


void TilemapNode::ParseJSONData(const std::string& key, const std::string& path)
{
    std::ifstream JSON(Editor::projectPath + path);

    if (!JSON.good()) {
        Editor::Log("Tilemap: unable to parse, invalid json.");
        return;
    }

    const json data = json::parse(JSON);

    if (!data.contains("width") || 
        !data.contains("height") || 
        !data.contains("tilewidth") || 
        !data.contains("tileheight") || 
        !data.contains("layers"))
            Editor::Log("Tilemap: cannot construct from json missing params: [width, height, tilewidth, tileheight, layers].");

    else 
    {            
        map_width = data["width"];
        map_height = data["height"];
        tile_width = data["tilewidth"];
        tile_height = data["tileheight"];

        if (data.contains("tilesets")) 
        {
            if (data.contains("layers") && data["layers"].size())
            {
                int i = 0;

                for (const auto& layer : data["layers"])  
                {
                    if (layer.contains("data")) 
                    {
                        //iterate tileset, bounds check, default to 0 index if over
                        
                        const int tilesetIndex = data["tilesets"].size() > i ? i : 0;

                        AddLayer();

                        layers[i].dataKey = key;
                        layers[i].path = path;
                        layers[i].textureKey = static_cast<std::string>(data["tilesets"][tilesetIndex]["name"]) + ".png";
                        layers[i].depth = i;

                        spr_sheet_width[i] = Graphics::Texture2D::Get(layers[i].textureKey).Width / tile_width;
                        spr_sheet_height[i] = Graphics::Texture2D::Get(layers[i].textureKey).Height / tile_height; 

                        ++i;
                    }

                    if (layer.contains("objects")) 
                    {
                        AddComponent(Component::PHYSICS);

                        for (const auto& body : layer["objects"]) {
                            CreateBody(body["x"], body["y"], body["width"], body["height"]);
        
                            for (int i = 0; i < bodies.size(); i++)
                                UpdateBody(i);
                        }
                    }
                }  

                ApplyTilemap(true, false, true);
            }
            else 
                Editor::Log("Error parsing tilemap. JSON does not contain layers field.");
        }
        else 
            Editor::Log("Error parsing tilemap. JSON does not contain tilesets field.");
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


void TilemapNode::Update(std::vector<std::shared_ptr<Node>>& arr)
{
    ImGui::Separator(); 

    {
        ImGui::PushID(("(Tilemap) " + name).c_str());

        if (ImGui::TreeNode(("(Tilemap) " + name).c_str()))
        {
            Node::Update(arr);

            if (ImGui::BeginMenu("Add Component"))
            {
                if (ImGui::MenuItem("Physics"))
                    AddComponent(Component::PHYSICS);
            
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
                                    if (ImGui::MenuItem(key.c_str())) {
                                        layers[i].dataKey = key;
                                        layers[i].path = path;
                                        layers[i].depth = i;
                                        spr_sheet_width[i] = Graphics::Texture2D::Get(layers[i].textureKey).Width / tile_width;
                                        spr_sheet_height[i] = Graphics::Texture2D::Get(layers[i].textureKey).Height / tile_height; 
                                        m_layersApplied = false;
                                    }
                                }

                               if (System::Utils::str_endsWith(path, ".json")) {
                                    if (ImGui::MenuItem(key.c_str())) 
                                       ParseJSONData(key, path);
                               }
                                       
                            }
                  
                            ImGui::EndMenu();
                        }
                        
                        ImGui::SameLine();

                        std::string data_file_name = "";

                        if (layers.size())
                            data_file_name = layers[i].dataKey;

                        ImGui::Text(data_file_name.c_str()); 

                        if (ImGui::ImageButton("tex button", (void*)(intptr_t)Graphics::Texture2D::Get(layers[i].textureKey).ID, ImVec2(50, 50)) && System::Utils::GetFileType(AssetManager::Get()->selectedAsset) == System::Resources::Manager::IMAGE) 
                        {
                            layers[i].textureKey = AssetManager::Get()->selectedAsset;
                            m_layersApplied = false;
                        }

                        else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && System::Utils::GetFileType(AssetManager::Get()->selectedAsset) != System::Resources::Manager::IMAGE)
                            ImGui::SetTooltip("cannot set texture because selected asset is not of type image.");

                        if (ImGui::SliderInt("depth", &layers[i].depth, 0, 1000))
                            m_layersApplied = false;

                        if (ImGui::InputFloat("scroll factor x", &layers[i].scrollFactorX))
                            m_layersApplied = false;
                        
                        if (ImGui::InputFloat("scroll factor y", &layers[i].scrollFactorY))
                            m_layersApplied = false;

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
                                            layers[i].shader = name;
                                        }
                                    }

                            ImGui::EndMenu();
                        }
 
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

                    if (ImGui::Button("remove layer") && layer > 0) 
                    {
                        if (layers.size())
                            System::Game::maps->RemoveLayer(layers[layer - 1].dataKey);

                        layers.pop_back();
                        layer--;
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
                                ParseJSONData(key, path);
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

            entity->SetPosition(initialPosition.x + positionX, initialPosition.y + positionY);
            entity->SetScale(entity->scale.x * scaleX, entity->scale.y * scaleY);
            entity->SetRotation(entity->rotation + rotation);
        }
}