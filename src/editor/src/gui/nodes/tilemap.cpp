#include "./node.h"
#include "../../editor.h"
#include "../../assets/assets.h"
#include "../../../../../build/include/app.h"


TilemapNode::TilemapNode(const std::string &id): 
    Node(id, "Tilemap"),
        layersApplied(false),
        layer(1),
        map_width(10),
        map_height(10),
        tile_width(64),
        tile_height(64)
{ Editor::Log("Tilemap node " + this->m_name + " created."); }


//---------------------------
 

TilemapNode::~TilemapNode() {
    
    MapManager::ClearMap();

    Editor::Log("Tilemap node " + this->m_name + " deleted.");
}


//---------------------------


void TilemapNode::ApplyTilemap()
{

    MapManager::ClearMap(); 

    //for every layer, iterate over widths and heights, decrementing height each iteration by 1 until 0

    for (int i = 0; i < this->layer; i++) 
    { 

        int w = 0, 
            h = this->spr_sheet_height[i] - 1; 

        for (int y = 0; y < this->map_height; ++y)
            for (int x = 0; x < this->map_width; ++x)
            { 
                if (w == this->spr_sheet_width[i]) { 
                    w = 0;
                    h--;   
                }   

                if (w < this->map_width) {

                    this->offset.push_back({ w, h, this->tile_width, this->tile_height }); 
                    
                    w++;
                }
            }

        //load atlas frames from csv offsets

        std::string key = this->layers[i][0];
        std::string path = this->layers[i][1];
        std::string texture = this->layers[i][2];

        System::Resources::Manager::LoadFrames(texture, this->offset);
        System::Resources::Manager::LoadFile(key.c_str(), path.c_str());

        //parse csv data and load map layer

        std::vector<std::string> data = System::Resources::Manager::ParseCSV(key);

        if (data.size()) {

            System::Resources::Manager::LoadTilemap(key, data);

            MapManager::CreateLayer(
                key.c_str(), 
                texture.c_str(), 
                this->map_width, 
                this->map_height, 
                this->tile_width,  
                this->tile_height, 
                this->depth[i]
            );

           this->layersApplied = true;
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

    auto body = Game::physics->CreateStaticBody(x, y, width, height);

    this->bodies.push_back(body);
}


//---------------------------


void TilemapNode::Render()
{

    ImGui::Separator(); 

    {

        ImGui::PushID(("(Tilemap) " + this->m_name).c_str());

        if (ImGui::TreeNode(("(Tilemap) " + this->m_name).c_str()))
        {
        
            static char buf1[32] = ""; ImGui::InputText("name", buf1, 32, ImGuiInputTextFlags_CallbackCompletion, ChangeName, &this->m_ID);

            if (ImGui::BeginMenu("Add Component"))
            {
                if (ImGui::MenuItem("Physics"))
                    this->AddComponent("Physics");
            
                ImGui::EndMenu();
            }

            for (const auto &component : this->components)
                if (strcmp(component->m_type, "Physics") == 0 && ImGui::BeginMenu("Physics"))
                { 

                    for (int i = 0; i < this->bodies.size(); i++)
                    {

                        ImGui::PushID(i);
                        
                        ImGui::SliderFloat("x", &this->bodyX[i], 0.0f, System::Window::m_width); 
                        ImGui::SliderFloat("y", &this->bodyY[i], 0.0f, System::Window::m_height); 
                        ImGui::SliderFloat("width", &this->body_width[i], 0.0f, System::Window::m_width); 
                        ImGui::SliderFloat("height", &this->body_height[i], 0.0f, System::Window::m_height); 

                        ImGui::Separator();             
                        
                        b2PolygonShape body; 
                        body.SetAsBox(this->body_width[i], this->body_height[i]);
                        b2FixtureDef fixtureDef;
                        fixtureDef.shape = &body; 

                        this->bodies[i]->DestroyFixture(this->bodies[i]->GetFixtureList());
                        this->bodies[i]->CreateFixture(&fixtureDef);
                        this->bodies[i]->SetTransform(b2Vec2(this->bodyX[i], this->bodyY[i]), 0);
                        
                        ImGui::PopID();

                    }

                    if (ImGui::Button("add")) 
                        this->CreateBody();

                    if (ImGui::Button("remove") && this->bodies.size() > 1) { 
                        Game::physics->DestroyBody(this->bodies.back()); 
                        this->bodies.pop_back();
                    }

                    if (ImGui::BeginMenu("remove physics?"))
                    {
                        if (ImGui::MenuItem("yes"))
                            this->RemoveComponent(component); 

                        ImGui::EndMenu();
                    }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Delete"))
            {
                if (ImGui::MenuItem("Are You Sure?")) 
                    DeleteNode(this);

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
                       
                        this->layers.push_back({});

                        if (ImGui::BeginMenu("CSV: ")) 
                        {

                            for (const auto &asset : AssetManager::loadedAssets)
                            {

                                std::string key = asset.first;
                                std::string path = asset.second;

                                key.erase(std::remove(key.begin(), key.end(), '\"'), key.end());
                                path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());

                                if (System::Utils::str_endsWith(path, ".csv")) {

                                    if (ImGui::MenuItem(key.c_str())) {

                                        this->layers[i][0] = key;
                                        this->layers[i][1] = path;
                                        this->layersApplied = false;
                                    }
                                }
                            }
                  
                            ImGui::EndMenu();
                        }
                        
                        ImGui::SameLine();

                        std::string csv_name = "";

                        if (this->layers.size())
                            csv_name = this->layers[i][0];

                        ImGui::Text(csv_name.c_str()); 

                        if (ImGui::ImageButton("tex button", (void*)(intptr_t) System::Resources::Manager::texture2D->GetTexture(this->layers[i][2]).ID, ImVec2(50, 50), ImVec2(0, 1), ImVec2(1, 0)) && System::Utils::GetFileType(Editor::selectedAsset.first) == "image") 
                        {
                            this->layers[i][2] = Editor::selectedAsset.first;
                            this->layersApplied = false;
                        }

                        else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && System::Utils::GetFileType(Editor::selectedAsset.first) != "image")
                            ImGui::SetTooltip("cannot set texture because selected asset is not of type image.");

                        this->spr_sheet_width.push_back(i);
                        this->spr_sheet_height.push_back(i);
                        this->depth.push_back(i);

                        if (
                            ImGui::InputInt("frames x", &this->spr_sheet_width[i]) ||
                            ImGui::InputInt("frames y", &this->spr_sheet_height[i]) ||
                            ImGui::SliderInt("depth", &this->depth[i], 0, 1000)
                        )
                            this->layersApplied = false;
 
                        ImGui::PopID();
                    } 

                    if (ImGui::Button("apply") && !this->layersApplied)
                        this->ApplyTilemap();

                    if (this->layersApplied) { 
                        ImGui::SameLine(); 
                        ImGui::Text("applied");
                    }

                    if (ImGui::Button("add layer"))
                        this->layer++;

                    ImGui::SameLine();

                    if (ImGui::Button("remove layer") && this->layer > 1) {

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

