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
    this->layer = 1;
    this->map_width = 10;
    this->map_height = 10;
    this->tile_width = 64;
    this->tile_height = 64;

    Editor::Log("Tilemap node " + this->name + " created."); 
}


//---------------------------
 

TilemapNode::~TilemapNode() 
{

    if (this->m_mapApplied)
        MapManager::ClearMap();

    if (!this->virtual_node)
        Editor::Log("Tilemap node " + this->name + " deleted.");
}


//---------------------------


void TilemapNode::Reset(const char* component_type)
{

    bool passAll = strcmp(component_type, "") == 0;

    if (strcmp(component_type, "Physics") == 0 || passAll) 
    {
        for (const auto& body : this->bodies)
            Physics::DestroyBody(body);

        this->bodyX.clear();
        this->bodyY.clear();
        this->body_width.clear(); 
        this->body_height.clear();

        this->bodies.clear();
    }

    this->m_mapApplied = false;

    if (passAll)
        MapManager::ClearMap();
}


//---------------------------


void TilemapNode::ApplyTilemap(bool clearPrevious, bool renderReversed)
{

    if (clearPrevious)
        MapManager::ClearMap();

    //for every layer, iterate over widths and heights by index, decrementing height each iteration by 1 until 0

    for (int i = 0; i < this->layer; i++) 
    { 

        int w = 0,  
            h = renderReversed ? this->spr_sheet_height[i] - 1 : 0; 

        for (int y = 0; y < this->map_height; ++y)
            for (int x = 0; x < this->map_width; ++x)
            {
                if (w == this->spr_sheet_width[i]) 
                { 
                    w = 0;

                    if (renderReversed)
                        h--;  
                    else 
                        h++; 
                }   

                if (w < this->map_width) {

                    this->offset.push_back({ w, h, this->tile_width, this->tile_height, 1, 1 }); 
                    w++;
                }
            }
            
        if (!clearPrevious)
            return;

        //load atlas frames from csv offsets

        std::string key = this->layers[i][0],
                    path = this->layers[i][1],
                    texture = this->layers[i][2];

        System::Resources::Manager::LoadFrames(texture, this->offset);
        System::Resources::Manager::LoadFile(key.c_str(), path.c_str());

        //parse csv data and load map layer

        std::vector<std::string> data = System::Resources::Manager::ParseCSV(key);

        if (data.size()) {

            System::Resources::Manager::LoadTilemap(key, data);

            MapManager::CreateLayer(key.c_str(), texture.c_str(), this->map_width, this->map_height, this->tile_width, this->tile_height, this->depth[i]);

            this->m_layersApplied = true;
            this->m_mapApplied = true;
        }
    }
}


//---------------------------


void TilemapNode::CreateBody(float x, float y, float width, float height) 
{

    this->bodyX.push_back(x);
    this->bodyY.push_back(y);
    this->body_width.push_back(width);
    this->body_height.push_back(height);

    auto body = Physics::CreateStaticBody(x, y, width, height);

    this->bodies.push_back(body);
}


//---------------------------


void TilemapNode::UpdateBody(int index) 
{

    b2PolygonShape body; 
    body.SetAsBox(this->body_width[index] / 2, this->body_height[index] / 2);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &body; 

    this->bodies[index]->DestroyFixture(this->bodies[index]->GetFixtureList());
    this->bodies[index]->CreateFixture(&fixtureDef);
    this->bodies[index]->SetTransform(b2Vec2(this->bodyX[index] + this->body_width[index] / 2, this->bodyY[index] + this->body_height[index] / 2), 0);
}


//---------------------------


void TilemapNode::Render(std::shared_ptr<Node> node)
{

    ImGui::Separator(); 

    {

        ImGui::PushID(("(Tilemap) " + this->name).c_str());

        if (ImGui::TreeNode(("(Tilemap) " + this->name).c_str()))
        {
        
            static char buf1[32] = ""; ImGui::InputText("name", buf1, 32, ImGuiInputTextFlags_CallbackCompletion, ChangeName, &this->ID);

            if (ImGui::BeginMenu("Add Component"))
            {
                if (ImGui::MenuItem("Physics"))
                    this->AddComponent("Physics");
            
                ImGui::EndMenu();
            }

            //components

            if (this->HasComponent("Physics") && ImGui::BeginMenu("Physics"))
            { 

                auto physics_component = this->GetComponent("Physics", this->ID);

                if (physics_component)
                {

                    if (ImGui::BeginMenu("bodies"))
                    {
                        ImGui::Text("manual input: CTRL + click");

                        for (int i = 0; i < this->bodies.size(); i++)
                        {

                            ImGui::PushID(i);

                            ImGui::Text("body: %d", i);

                            ImGui::SameLine();

                            if (ImGui::Button("remove") && this->bodies.size() > 1) 
                            {
                                auto it = std::find(this->bodies.begin(), this->bodies.end(), this->bodies[i]);
                                if (it != this->bodies.end()) {
                                    Physics::DestroyBody(*it); 
                                    this->bodies.erase(it);
                                }
                            }

                            ImGui::SliderFloat("width", &this->body_width[i], 0.0f, this->map_width * this->tile_width, NULL, ImGuiSliderFlags_AlwaysClamp); 
                            ImGui::SliderFloat("height", &this->body_height[i], 0.0f, this->map_height * this->tile_height, NULL, ImGuiSliderFlags_AlwaysClamp); 

                            ImGui::SliderFloat("x", &this->bodyX[i], 0.0f, this->map_width * this->tile_width, NULL, ImGuiSliderFlags_AlwaysClamp);  
                            ImGui::SliderFloat("y", &this->bodyY[i], 0.0f, this->map_height * this->tile_height, NULL, ImGuiSliderFlags_AlwaysClamp); 

                            ImGui::Separator();             
                            
                            this->UpdateBody(i);
                            
                            ImGui::PopID();

                        }

                        ImGui::EndMenu();
                    }

                    //load collision data from file

                    if (ImGui::BeginMenu("load collision data"))
                    {
                        for (const auto& asset : AssetManager::loadedAssets)
                        {

                            std::string key = asset.first;
                            std::string path = asset.second;

                            key.erase(std::remove(key.begin(), key.end(), '\"'), key.end());
                            path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());

                            if (System::Utils::str_endsWith(path, ".json")) 
                            {

                                if (ImGui::MenuItem(key.c_str())) 
                                {

                                    for (const auto& body : this->bodies)
                                        Physics::DestroyBody(body);

                                    this->bodyX.clear();
                                    this->bodyY.clear();
                                    this->body_width.clear();
                                    this->body_height.clear();

                                    this->bodies.clear();

                                    //parse json to extract body data

                                    std::ifstream JSON(path);

                                    json data = json::parse(JSON);

                                    auto createBodies = [&](float x, float y, float w, float h) {

                                        this->CreateBody(x, y, w, h);
                                        
                                        for (int i = 0; i < this->bodies.size(); i++)
                                            this->UpdateBody(i);
                                    };

                                    if (data["objects"].size())
                                        for (const auto& body : data["objects"])
                                            createBodies(body["x"], body["y"], body["width"], body["height"]);

                                    else if (data["layers"].size())
                                        for (const auto& layer : data["layers"])
                                            if (layer["objects"].size())
                                                for (const auto& body : layer["objects"])
                                                    createBodies(body["x"], body["y"], body["width"], body["height"]);
             
                                    this->m_layersApplied = false;
                                }
                            }
                        }
                        ImGui::EndMenu();
                    }
                    
                    if (ImGui::Button("add")) 
                        this->CreateBody();

                    ImGui::SameLine();

                    if (ImGui::BeginMenu("remove physics?"))
                    {
                        if (ImGui::MenuItem("yes"))
                            this->RemoveComponent(physics_component); 

                        ImGui::EndMenu();
                    }                
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Delete"))
            {
                if (ImGui::MenuItem("Are You Sure?")) 
                    DeleteNode(node);

                ImGui::EndMenu();
            }

            ImGui::Checkbox("Edit", &this->show_options);

            if (this->show_options)
            {
                
                if (ImGui::BeginMenu("layers")) 
                {

                    ImGui::Text("layers: %d", this->layer); 

                    for (int i = 0; i < this->layer; ++i)
                    {
                        ImGui::Separator();
                    
                        ImGui::Text("layer: %d", i);
                        
                        ImGui::PushID(i);
                       
                        this->layers.push_back({ "", "", "" });

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

                                        this->layers[i][0] = key;
                                        this->layers[i][1] = path;

                                        this->m_layersApplied = false;
                                    }
                            }
                  
                            ImGui::EndMenu();
                        }
                        
                        ImGui::SameLine();

                        std::string csv_name = "";

                        if (this->layers.size())
                            csv_name = this->layers[i][0];

                        ImGui::Text(csv_name.c_str()); 

                        if (ImGui::ImageButton("tex button", (void*)(intptr_t)Graphics::Texture2D::GetTexture(this->layers[i][2]).ID, ImVec2(50, 50)) && System::Utils::GetFileType(Editor::selectedAsset) == "image") 
                        {
                            this->layers[i][2] = Editor::selectedAsset;
                            this->m_layersApplied = false;
                        }

                        else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && System::Utils::GetFileType(Editor::selectedAsset) != "image")
                            ImGui::SetTooltip("cannot set texture because selected asset is not of type image.");

                        this->spr_sheet_width.push_back(i);
                        this->spr_sheet_height.push_back(i);
                        this->depth.push_back(i);

                        if (
                            ImGui::InputInt("frames x", &this->spr_sheet_width[i]) ||
                            ImGui::InputInt("frames y", &this->spr_sheet_height[i]) ||
                            ImGui::SliderInt("depth", &this->depth[i], 0, 1000)
                        )
                            this->m_layersApplied = false;
 
                        ImGui::PopID();
                    } 

                    if (ImGui::Button("apply") && !this->m_layersApplied)
                        this->ApplyTilemap();

                    if (this->m_layersApplied) { 
                        ImGui::SameLine(); 
                        ImGui::Text("applied");
                    }

                    if (ImGui::Button("add layer"))
                        this->layer++;

                    ImGui::SameLine();

                    if (ImGui::Button("remove layer") && this->layer > 1) {

                        if (this->layers.size())
                            System::Game::maps->RemoveLayer(this->layers[this->layer - 1][0]);

                        this->layers.pop_back();
                        this->layer--;
                    }

                    ImGui::EndMenu();
                }

                ImGui::InputInt("map width", &this->map_width);
                ImGui::InputInt("map height", &this->map_height);
                ImGui::InputInt("tile width", &this->tile_width);
                ImGui::InputInt("tile height", &this->tile_height);
            }

            ImGui::TreePop();
        }


        ImGui::PopID();

    }
}

