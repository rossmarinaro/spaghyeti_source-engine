#include "./node.h"
#include "../gui/gui.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../../../../build/sdk/include/app.h"

using namespace editor;


SpriteNode::SpriteNode():  
    Node("Sprite"),
        m_show_sprite_texture(false)
{
    key = "";
    tint = glm::vec3(1.0f);
    framesApplied = false;
    filter_nearest = true;
    flippedX = false;
    flippedY = false;
    lock_in_place = false;
    make_UI = false;
    frame = 0;
    anim = 1;
    depth = 1;
    restitution = 0.0f;
    density = 0.0f;
    friction = 0.0f;
    alpha = 1.0f;
    U1 = 0.0f;
    V1 = 0.0f;
    U2 = 1.0f;
    V2 = 1.0f;
    frameBuf1.push_back(0);
    frameBuf2.push_back(0);
    frameBuf3.push_back(0);
    frameBuf4.push_back(0);
    frameBuf5.push_back(1);
    frameBuf6.push_back(1);
    spriteHandle = nullptr;

    Editor::Log("Sprite node " + name + " created.");   
}


         
//---------------------------


SpriteNode::~SpriteNode()
{
    m_currentTexture = NULL;

    if (spriteHandle != nullptr)
        System::Game::DestroyEntity(spriteHandle);
    
    if (!virtual_node)
        Editor::Log("Sprite node " + name + " deleted.");
}


//---------------------------


void SpriteNode::Reset(const char* component_type)
{

    bool passAll = strcmp(component_type, "") == 0;

    if (strcmp(component_type, "Shader") == 0 || passAll)
        if (spriteHandle.get())
            spriteHandle->shader = Shader::Get("sprite");

    if (strcmp(component_type, "Script") == 0 || passAll)
        behaviors.clear();

    if (strcmp(component_type, "Animator") == 0 || passAll)
    {
        animBuf1.clear();
        animBuf2.clear();
        animBuf3.clear();
        animBuf4.clear();
        anim = 0; 
    }

    if (strcmp(component_type, "Physics") == 0 || passAll)
    {

        for (const auto& body : bodies)
            Physics::DestroyBody(body);
        
        bodyX.clear();
        bodyY.clear(); 
        body_width.clear();
        body_height.clear();
        body_pointer.clear();
        bodies.clear();
        is_sensor.clear();

        restitution = 0.0f;
        density = 0.0f;
        friction = 0.0f;

    }
    
}


//---------------------------------


void SpriteNode::CreateBody(float x, float y, float width, float height, bool isSensor, int pointerType) 
{

    BoolContainer bc;

    bc.b = isSensor;

    bodyX.push_back(x);
    bodyY.push_back(y);
    body_width.push_back(width);
    body_height.push_back(height);
    is_sensor.push_back(bc);
    body_pointer.push_back(pointerType);

    b2Body* body = Physics::CreateDynamicBody("box", x, y, width, height); 

    bodies.push_back(body);
    
}


//---------------------------------


void SpriteNode::RegisterFrames()
{
    std::vector<std::array<int, 6>> framesToPush;

    for (const auto& frame : frames)
        framesToPush.push_back({ frame.x, frame.y, frame.width, frame.height, frame.factorX, frame.factorY });

    System::Resources::Manager::LoadFrames(key, framesToPush);
}


//--------------------------------- applies texture to current seleted node 


void SpriteNode::ApplyTexture(const std::string& asset)
{  

    if (!spriteHandle) 
    { 
        spriteHandle = System::Game::CreateSprite(asset, 0.0f, 0.0f);
        spriteHandle->name = name;
        System::Game::GetScene()->SetInteractive(spriteHandle);
    }

    else 
       spriteHandle->SetTexture(asset); 

    m_currentTexture = spriteHandle->texture.Get(asset).ID;  
    key = asset;

    AssetManager::Register(key, true);

}


//--------------------------------- applies texture to current seleted node 


void SpriteNode::ApplyAnimation(const std::string& key, int start, int end)
{

    try {

        std::map<std::string, std::pair<int, int>> animsToLoad;

        animations.insert({ key, { key, start, end } });

        for (const auto& anim : animations)
            animsToLoad.insert({ { anim.second.key, { anim.second.start, anim.second.end } } });
        
        System::Resources::Manager::LoadAnims(key, animsToLoad);

        if (spriteHandle) {
            spriteHandle->anims = System::Resources::Manager::GetAnimations(key);
            spriteHandle->ReadSpritesheetData();    
        }
  
    }

    catch (std::runtime_error& err) { 
        Editor::Log("there was a problem applying animation: " + (std::string)err.what()); 
    }
}


//---------------------------


void SpriteNode::Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr)
{
	
    ImGui::Separator(); 

    {

        assert(active);

        ImGui::PushID(("(Sprite) " + name).c_str());

        if (ImGui::TreeNode(("(Sprite) " + name).c_str()))
        {
        
            static char name_buf[32] = ""; ImGui::InputText("name", name_buf, 32, ImGuiInputTextFlags_CallbackCompletion, ChangeName, &ID);

            //save prefab

            if (ImGui::Button("Save prefab")) 
                SavePrefab(); 
            
            if (ImGui::BeginMenu("Add Component"))
            {
                if (spriteHandle) 
                {

                    if (ImGui::MenuItem("Animator"))
                        AddComponent("Animator");

                    if (ImGui::MenuItem("Physics"))
                        AddComponent("Physics");
                
                    if (ImGui::MenuItem("Scripts"))
                        AddComponent("Script");

                    if (ImGui::MenuItem("Shader"))
                        AddComponent("Shader");
   
                }
                else
                    ImGui::Text("Please apply texture before creating a component.");
                
                ImGui::EndMenu();
            }

            //component options

            //-------------------------------- animator

            if (HasComponent("Animator") && ImGui::BeginMenu("Animator"))
            {

                auto anim_component = GetComponent("Animator", ID);

                if (anim_component)
                {
                    for (int i = 0; i < anim; i++)
                    {

                        ImGui::Text("animation: %d", i);

                        ImGui::PushID(i);
        
                        StringContainer sc;
                        BoolContainer bc;

                        m_do_yoyo.push_back(bc);
                        animBuf1.push_back(sc);
                        animBuf2.push_back(i);
                        animBuf3.push_back(i);
                        animBuf4.push_back(2);

                        if (spriteHandle && spriteHandle->IsSpritesheet())
                        {

                            if (ImGui::Button("play")) 
                                m_currentAnim = { animBuf1[i].s, { m_do_yoyo[i].b, animBuf4[i] } };
                                
                            ImGui::SameLine(); 

                            if (ImGui::Button("stop")) 
                                m_currentAnim = { "", {} };

                        }

                        if (animBuf1.size() && animBuf1[i].s.length()) {

                            ImGui::SameLine();

                            if (ImGui::Button("apply")) 
                                ApplyAnimation(animBuf1[i].s, animBuf2[i], animBuf3[i]);
                            
                        }

                        if (i != 0 && anim > 1)
                        {

                            ImGui::SameLine(); 

                            if (ImGui::Button("remove")) {

                                std::map<std::string, Anims>::iterator it = animations.find(animBuf1[i].s);

                                if (it != animations.end())
                                    animations.erase(it);

                                anim--;
                            }

                        }

                        ImGui::InputText("key", &animBuf1[i].s);
                        ImGui::InputInt("start", &animBuf2[i]); 
                        ImGui::InputInt("end", &animBuf3[i]);
                        ImGui::InputInt("rate", &animBuf4[i]);
                        ImGui::Checkbox("yoyo", &m_do_yoyo[i].b);

                        ImGui::Separator();

                        ImGui::PopID();
                        
                    }

                    if (ImGui::Button("add animation"))
                        anim++;
            
                    ImGui::SameLine();

                    if (ImGui::BeginMenu("remove animator?")) {
                        
                        if (ImGui::MenuItem("yes")) 
                            RemoveComponent(anim_component); 

                        ImGui::EndMenu();
                    }

                }
              
                ImGui::EndMenu();
            }


            //------------------------------ shader


            if (HasComponent("Shader") && ImGui::BeginMenu("Shader")) {

                GUI::RenderShaderOptions(ID);
                
                ImGui::EndMenu();
            }


            //------------------------------ script


            if (HasComponent("Script") && ImGui::BeginMenu("Script")) {

                GUI::RenderScriptOptions(ID);
                
                ImGui::EndMenu();
            }


            //------------------------------ physics


            if (HasComponent("Physics") && ImGui::BeginMenu("Physics"))
            {

                auto physics_component = GetComponent("Physics", ID);

                if (physics_component)
                {
                    for (int i = 0; i < bodies.size(); i++)
                    {

                        ImGui::PushID(i);

                        ImGui::Text((i == 0) ? "anchor: %d" : "body: %d", i);

                        ImGui::SliderFloat("offset x", &bodyX[i], 0.0f, System::Window::s_width); 
                        ImGui::SliderFloat("offset y", &bodyY[i], 0.0f, System::Window::s_height);
                        ImGui::SliderFloat("width", &body_width[i], 0.0f, System::Window::s_width); 
                        ImGui::SliderFloat("height", &body_height[i], 0.0f, System::Window::s_height);   
                        ImGui::InputInt("type", &body_pointer[i]); 

                        ImGui::Checkbox("sensor", &is_sensor[i].b);

                        ImGui::Separator();     

                        if (bodies[i] != nullptr)
                        {
                            b2PolygonShape body;
                            body.SetAsBox(body_width[i], body_height[i]);
                            b2FixtureDef fixtureDef;
                            fixtureDef.shape = &body;

                            bodies[i]->DestroyFixture(bodies[i]->GetFixtureList());
                            bodies[i]->CreateFixture(&fixtureDef);    

                        }

                        ImGui::PopID();

                    }

                    ImGui::Text("settings");

                    ImGui::SliderFloat("density", &density, 0.0f, 1000.0f);
                    ImGui::SliderFloat("friction", &friction, 0.0f, 1.0f);
                    ImGui::SliderFloat("restitution", &restitution, 0.0f, 1.0f);

                    ImGui::Separator();     

                    if (ImGui::Button("add")) 
                        CreateBody();

                    ImGui::SameLine();

                    if (ImGui::Button("remove") && bodies.size() > 1) {
                        Physics::DestroyBody(bodies.back());
                        bodies.pop_back();
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

            if (ImGui::BeginMenu("Options")) {
                ShowOptions(node, arr);
                ImGui::EndMenu();
            }

            ImGui::Checkbox("Edit", &show_options);

            if (show_options)
            {
            
                ImGui::Checkbox("texture", &m_show_sprite_texture); 

                if (m_show_sprite_texture)
                {

                    //texture
        
                    if (ImGui::ImageButton("texture button", (void*)(intptr_t)m_currentTexture, ImVec2(50, 50)) && System::Utils::GetFileType(AssetManager::selectedAsset) == "image")
                        ApplyTexture(AssetManager::selectedAsset);

                    else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && AssetManager::selectedAsset.length() && System::Utils::GetFileType(AssetManager::selectedAsset) != "image")
                        ImGui::SetTooltip("cannot set texture because selected asset is not of type image.");

                    //if texture applied to sprite
                        
                    if (m_currentTexture)
                    {

                        //spritesheet / atlas

                        if (ImGui::BeginMenu("frames")) 
                        {
                            //apply frames
 
                            if (ImGui::Button("apply") && !framesApplied)
                            {
                          
                                frames.clear();

                                for (int i = 0; i < frame; i++) 
                                    frames.push_back({ frameBuf1[i], frameBuf2[i], frameBuf3[i], frameBuf4[i], frameBuf5[i], frameBuf6[i]}); 

                                RegisterFrames();  
                                
                                framesApplied = true;
                                spriteHandle->ReadSpritesheetData(); 
                                spriteHandle->SetFrame(0);

                            }

                            if (framesApplied) {  
                                ImGui::SameLine(); 
                                ImGui::Text("applied");
                            }

                            ImGui::SameLine();

                            if (spriteHandle)
                            {

                                if (ImGui::BeginMenu("frames:"))
                                {

                                    if (frame <= 0) 
                                        ImGui::Text("no frames defined"); 
                                   
                                    else 
                                        for (int i = 0; i < frame; i++)
                                        {

                                            ImGui::Separator(); 
                                        
                                            ImGui::Text("frame: %d", i); 
                                            
                                            ImGui::PushID(i);
                                            
                                            if (ImGui::Button("-x") && frameBuf5[i] > 1) {
                                                framesApplied = false;
                                                frameBuf5[i]--;
                                            }

                                            ImGui::SameLine();

                                            if (ImGui::Button("+x") && frameBuf5[i]) {
                                                framesApplied = false;
                                                frameBuf5[i]++;
                                            }

                                            ImGui::SameLine();

                                            ImGui::Text("factor x: %d", frameBuf5[i]);

                                            if (ImGui::Button("-y") && frameBuf6[i] > 1) {
                                                framesApplied = false;
                                                frameBuf6[i]--;
                                            }

                                            ImGui::SameLine();

                                            if (ImGui::Button("+y") && frameBuf6[i]) {
                                                framesApplied = false;
                                                frameBuf6[i]++;
                                            }

                                            ImGui::SameLine();

                                            ImGui::Text("factor y: %d", frameBuf6[i]);

                                            if (
                                                ImGui::InputInt("position x", &frameBuf1[i]) ||
                                                ImGui::InputInt("position y", &frameBuf2[i]) || 
                                                ImGui::InputFloat("width", &frameBuf3[i]) || 
                                                ImGui::InputFloat("height", &frameBuf4[i]) 
                                            ) 
                                                framesApplied = false;

                                            ImGui::PopID();

                                        } 

                                    ImGui::EndMenu();
                                }

                                if (ImGui::Button("-") && spriteHandle->currentFrame > 0) 
                                    spriteHandle->currentFrame--;                    
                                
                                ImGui::SameLine();

                                if (ImGui::Button("+") && spriteHandle->currentFrame < spriteHandle->frames - 1) 
                                    spriteHandle->currentFrame++; 

                                ImGui::SameLine();

                                ImGui::Text("frame: %d", spriteHandle->currentFrame);

                                ImGui::SameLine();

                                ImGui::Text("total: %d", frame); 

                            }

                            if (ImGui::Button("add frame")) 
                            {
                                frameBuf1.push_back(0);
                                frameBuf2.push_back(0);
                                frameBuf3.push_back(0);
                                frameBuf4.push_back(0);
                                frameBuf5.push_back(1);
                                frameBuf6.push_back(1);

                                frame++;
                            }

                            ImGui::SameLine();

                            if (ImGui::Button("remove frame") && frame >= 0) 
                            {
                                frames.pop_back();

                                frameBuf1.pop_back();
                                frameBuf2.pop_back();
                                frameBuf3.pop_back();
                                frameBuf4.pop_back();
                                frameBuf5.pop_back();
                                frameBuf6.pop_back();
                                
                                frame--;
                            }

                            //load frame data from file

                            if (ImGui::BeginMenu("load frame data"))
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

                                            //parse json to extract frame data

                                            std::ifstream JSON(path);

                                            if (!JSON.good()) 
                                                Editor::Log("Error parsing. Maybe file is in an inappropriate folder?");

                                            else 
                                            {

                                                json data = json::parse(JSON);

                                                //clear previous frames

                                                frame = 1;
                                                frames.clear();

                                                frameBuf1.clear();
                                                frameBuf2.clear();
                                                frameBuf3.clear();
                                                frameBuf4.clear();
                                                frameBuf5.clear();
                                                frameBuf6.clear();

                                                if (data["frames"].size() > 1)
                                                    for (const auto& index : data["frames"]) 
                                                    {
                                                        int x = index["frame"]["x"],
                                                            y = index["frame"]["y"];

                                                        float width = index["frame"]["w"],
                                                            height = index["frame"]["h"];

                                                        frameBuf1.push_back(x);
                                                        frameBuf2.push_back(y);
                                                        frameBuf3.push_back(width);
                                                        frameBuf4.push_back(height);
                                                        frameBuf5.push_back(1);
                                                        frameBuf6.push_back(1);

                                                    };
                                                
                                                frame = data["frames"].size();
                                            }
                                        }
                                    }
                                }

                                ImGui::EndMenu();
                            }

                            ImGui::EndMenu();
                        } 

                        if (ImGui::BeginMenu("UVs"))
                        {
                            ImGui::SliderFloat("U1", &U1, 0.0f, 1.0f); 
                            ImGui::SliderFloat("V1", &V1, 0.0f, 1.0f);
                            ImGui::SliderFloat("U2", &U2, 0.0f, 1.0f);
                            ImGui::SliderFloat("V2", &V2, 0.0f, 1.0f);

                            ImGui::EndMenu();
                        }

                        ImGui::Checkbox("filter nearest", &filter_nearest);
                        ImGui::Checkbox("lock image", &lock_in_place);
                        ImGui::Checkbox("make UI", &make_UI);

                        ImGui::Checkbox("flipX", &flippedX); 
                        ImGui::Checkbox("flipY", &flippedY);
 
                        if (filter_nearest) {
                            spriteHandle->texture.Filter_Min = GL_NEAREST;
                            spriteHandle->texture.Filter_Max = GL_NEAREST;
                        }
                        
                        else {
                            spriteHandle->texture.Filter_Min = GL_LINEAR;
                            spriteHandle->texture.Filter_Max = GL_LINEAR;
                        }

                        spriteHandle->texture.SetFiltering();

                        ImGui::ColorEdit3("tint", (float*)&tint); 
                        ImGui::SliderFloat("alpha", &alpha, 0.0f, 1.0f); 

                    }

                }

                ImGui::SliderInt("depth", &depth, 0, 1000);

                ImGui::SliderFloat("position x", &positionX, -System::Window::s_width, System::Window::s_width); 
                ImGui::SliderFloat("position y", &positionY, -System::Window::s_height, System::Window::s_height); 

                ImGui::SliderFloat("rotation", &rotation, 0.0f, 360.0f); 

                ImGui::SliderFloat("scale x", &scaleX, -100.0f, 100.0f); 
                ImGui::SliderFloat("scale y", &scaleY, -100.0f, 100.0f); 


            }

            ImGui::TreePop();
        }


        ImGui::PopID();

    }

}


//------------------------------------


void SpriteNode::Render()
{
    //entity handle update
        
    if (spriteHandle)
    {

        spriteHandle->texture.U1 = U1;
        spriteHandle->texture.V1 = V1;
        spriteHandle->texture.U2 = U2;
        spriteHandle->texture.V2 = V2;
        
        spriteHandle->SetScale(scaleX, scaleY);
        spriteHandle->SetPosition(positionX, positionY);
        spriteHandle->SetRotation(rotation); 
        spriteHandle->SetDepth(depth);
        spriteHandle->SetFlip(flippedX, flippedY);
        spriteHandle->SetAlpha(alpha);
        spriteHandle->SetTint(tint);

        if (m_currentAnim.first.length())   
            spriteHandle->SetAnimation(m_currentAnim.first.c_str(), m_currentAnim.second.first, m_currentAnim.second.second);

        else
            spriteHandle->StopAnimation();
            
        //entity physics body transform
        
        if (bodies.size())
            for (int i = 0; i < bodies.size(); i++)   
                bodies[i]->SetTransform(
                    b2Vec2(
                        spriteHandle->position.x + bodyX[i], 
                        spriteHandle->position.y + bodyY[i]
                    ), 0);

        if (System::Game::GetScene()->ListenForInteraction(spriteHandle) /* && System::Game::GetScene()->GetContext().inputs->LEFT_CLICK */)
            spriteHandle->SetTint({ 1.0f, 0.0f, 0.0f });

    }
}


