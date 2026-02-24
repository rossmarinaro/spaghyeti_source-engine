#include "./node.h"
#include "../gui/gui.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../../../../build/sdk/include/app.h"

#include "../../../vendors/box2d/include/box2d/box2d.h"

using namespace editor;


SpriteNode::SpriteNode(bool init):  
    Node(init, SPRITE),
        m_show_sprite_texture(false)
{
    key = "";
    tint = { 1.0f, 1.0f, 1.0f };
    framesApplied = false;
    filter_nearest = true;
    flippedX = false;
    flippedY = false;
    lock_in_place = false;
    cull = false; 
    make_UI = false;
    frame = 0;
    currentFrame = 0;
    depth = 1;
    restitution = 0.0f;
    density = 0.0f;
    friction = 0.0f;
    alpha = 1.0f;
    U1 = 0.0f;
    V1 = 0.0f;
    U2 = 1.0f;
    V2 = 1.0f;
    scrollFactorX = 1.0f;
    scrollFactorY = 1.0f;
    frame_x.push_back(0);
    frame_y.push_back(0); 
    frame_width.push_back(0);
    frame_height.push_back(0);
    frame_fX.push_back(1);
    frame_fY.push_back(1);
    spriteHandle = nullptr;
    anim_to_play_on_start = { "", 2, -1, false };
    body_type = Physics::Body::Type::DYNAMIC;
    body_shape = Physics::Body::Shape::BOX;

    if (m_init)
        Editor::Log("Sprite node " + name + " created.");   
}


         
//---------------------------


SpriteNode::~SpriteNode() {

    m_currentTexture = NULL;

    if (spriteHandle != nullptr)
        System::Game::DestroyEntity(spriteHandle);
    
    if (m_init)
        Editor::Log("Sprite node " + name + " deleted.");
}


//---------------------------


void SpriteNode::Reset(int component_type)
{

    bool passAll = component_type == Component::NONE;

    if (component_type == Component::SHADER || passAll)
        if (spriteHandle.get())
            spriteHandle->SetShader("sprite");

    if (component_type == Component::SCRIPT || passAll)
        behaviors.clear();

    if (component_type == Component::ANIMATOR || passAll) 
        animations.clear();

    if (component_type == Component::PHYSICS || passAll)
    {
        for (const auto& body : bodies)
            Physics::DestroyBody(body.pb);
        
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

    is_sensor.push_back(bc);
    body_pointer.push_back(pointerType);

    const auto body = Physics::CreateBody(body_type, body_shape, x, y, width, height);
    const Body b = { body, x, y, width, height };

    bodies.emplace_back(b);
}


//--------------------------------- load frames, overwrite previously cached key


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

    if (!spriteHandle) { 
        spriteHandle = System::Game::CreateSprite(asset, 0.0f, 0.0f);
        spriteHandle->name = name;
        System::Game::GetScene()->SetInteractive(spriteHandle);
    }
 
    else 
       spriteHandle->SetTexture(asset); 

    m_currentTexture = Graphics::Texture2D::Get(asset).ID;  
    key = asset;

    AssetManager::Register(key);

}


//--------------------------------- applies texture to current seleted node 


void SpriteNode::ApplyAnimation(const std::string& key)
{
    try {

        std::map<const std::string, std::pair<int, int>> animsToLoad;

        for (const auto& anim : animations) 
            animsToLoad.insert({ { anim.key, { anim.start, anim.end } } });
        
        System::Resources::Manager::LoadAnims(key, animsToLoad);

        if (spriteHandle) 
        {
            const auto anims = System::Resources::Manager::GetAnimations(key);
            
            if (anims) {
                spriteHandle->anims = *anims;
                spriteHandle->ReadSpritesheetData();   
            } 
        }
    }

    catch (std::runtime_error& err) { 
        Editor::Log("there was a problem applying animation: " + (std::string)err.what()); 
    }
}


//---------------------------  


void SpriteNode::Update(std::vector<std::shared_ptr<Node>>& arr)
{

    ImGui::Separator(); 

    {
        assert(active);

        ImGui::PushID(("(Sprite) " + name).c_str());

        std::string selText = (Editor::selectedEntity && spriteHandle) && Editor::selectedEntity->ID == spriteHandle->ID ? "<SELECTED> " : "";

        if (ImGui::TreeNode((selText + "(Sprite) " + name).c_str()))
        {
            Node::Update(arr);

            if (ImGui::Button("Select") && spriteHandle)
                Editor::selectedEntity = spriteHandle;
            
            if (ImGui::BeginMenu("Add Component"))
            {
                if (spriteHandle) 
                {
                    if (ImGui::MenuItem("Animator"))
                        AddComponent(Component::ANIMATOR);

                    if (ImGui::MenuItem("Physics"))
                        AddComponent(Component::PHYSICS);
                
                    if (ImGui::MenuItem("Scripts"))
                        AddComponent(Component::SCRIPT);

                    if (ImGui::MenuItem("Shader"))
                        AddComponent(Component::SHADER);
   
                }
                else
                    ImGui::Text("Please apply texture before creating a component.");
                
                ImGui::EndMenu();
            }

            //component options

            //-------------------------------- animator

            if (HasComponent(Component::ANIMATOR) && ImGui::BeginMenu("Animator"))
            {
                auto anim_component = GetComponent(Component::ANIMATOR, ID);

                if (anim_component)
                {
                    ImGui::Text(("animations: " + std::to_string(animations.size())).c_str()); 
                    
                    if (ImGui::Button("add animation")) 
                        animations.push_back({ "", 0, 0, 2, -1, false }); 
            
                    ImGui::SameLine();

                    if (ImGui::BeginMenu("remove animator?")) {
                        
                        if (ImGui::MenuItem("yes")) 
                            RemoveComponent(anim_component); 

                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("animations"))
                    {
                        if (animations.size() <= 0) 
                            ImGui::Text("no animations defined"); 
                                   
                        else 
                            for (int i = 0; i < animations.size(); i++)
                            {
                                ImGui::Text("animation: %d", i);

                                ImGui::PushID(i); 

                                if (spriteHandle && spriteHandle->IsSpritesheet()) {

                                    if (ImGui::Button("play")) 
                                        m_currentAnim = { animations[i].key, animations[i].start, animations[i].end, animations[i].rate, animations[i].repeat, animations[i].yoyo };
                                        
                                    ImGui::SameLine(); 

                                    if (ImGui::Button("stop")) 
                                        m_currentAnim = { "", 0, 0, 2, -1, false };
                                }

                                if (animations[i].key.size() && animations[i].key.length()) {
                                    ImGui::SameLine();
                                    if (ImGui::Button("apply")) 
                                        ApplyAnimation(animations[i].key);
                                }

                                if (i != 0 && animations.size() > 1) {
                                    ImGui::SameLine(); 
                                    if (ImGui::Button("remove")) {
                                        auto it = std::find_if(animations.begin(), animations.end(), [&](const Anims& anim) { return anim.key == animations[i].key; });
                                        if (it != animations.end()) {
                                            it = animations.erase(it);
                                            --it;
                                        }
                                    }
                                }

                                ImGui::InputText("key", &animations[i].key);
                                ImGui::InputInt("start", &animations[i].start); 
                                ImGui::InputInt("end", &animations[i].end);
                                ImGui::InputInt("rate", &animations[i].rate);
                                ImGui::InputInt("repeat", &animations[i].repeat);

                                if (animations[i].repeat < -1)
                                    animations[i].repeat = -1;
                                
                                ImGui::Checkbox("yoyo", &animations[i].yoyo);

                                ImGui::Separator();

                                ImGui::PopID();
                                
                            }

                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginCombo("set default animation", anim_to_play_on_start.key.c_str())) {
                        for (const auto& anim : animations) 
                        {
                            if (!anim.key.length())
                                continue;

                            if (ImGui::Selectable(anim.key.c_str())) {
                                anim_to_play_on_start.key = anim.key;
                                anim_to_play_on_start.rate = anim.rate;
                                anim_to_play_on_start.repeat = anim.repeat;
                                anim_to_play_on_start.yoyo = anim.yoyo;
                            } 
                        }
                            
                        ImGui::EndCombo();
                    }

                    if (anim_to_play_on_start.key.length() && ImGui::BeginMenu("remove default animation")) {
                        
                        if (ImGui::MenuItem("yes")) 
                            anim_to_play_on_start = { "", 2, -1, false };

                        ImGui::EndMenu();
                    }
                }

                ImGui::EndMenu();
            }


            //------------------------------ shader


            if (HasComponent(Component::SHADER) && ImGui::BeginMenu("Shader")) {
                RenderShaderOptions(ID, arr);
                ImGui::EndMenu();
            }


            //------------------------------ script


            if (HasComponent(Component::SCRIPT) && ImGui::BeginMenu("Script")) {
                RenderScriptOptions(ID, arr);  
                ImGui::EndMenu();
            }


            //------------------------------ physics


            if (HasComponent(Component::PHYSICS) && ImGui::BeginMenu("Physics"))
            {
                const auto physics_component = GetComponent(Component::PHYSICS, ID);

                if (physics_component)
                {
                    int i = 0;

                    for (auto& body : bodies)
                    {
                        ImGui::PushID(i);

                        ImGui::Text((i == 0) ? "anchor: %d" : "body: %d", i);

                        ImGui::SliderFloat("offset x", &body.x, 0.0f, System::Window::s_width); 
                        ImGui::SliderFloat("offset y", &body.y, 0.0f, System::Window::s_height);
                        ImGui::SliderFloat("width", &body.width, 0.0f, System::Window::s_width); 
                        ImGui::SliderFloat("height", &body.height, 0.0f, System::Window::s_height);
 
                        ImGui::InputInt("type", &body_pointer[i]); 

                        if (body_pointer[i] <= -1)
                            body_pointer[i] = -1;

                        ImGui::Checkbox("sensor", &is_sensor[i].b);

                        ImGui::Separator();     

                        b2PolygonShape bod;
                        bod.SetAsBox(body.width, body.height);
                        b2FixtureDef fixtureDef;
                        fixtureDef.shape = &bod;

                        bodies[i].pb->DestroyFixture();
                        bodies[i].pb->CreateFixture(&fixtureDef);    
 
                        ImGui::PopID();

                        i++;
                    } 

                    ImGui::Text("settings");

                    static const char* items[] = { "kinematic", "static", "dynamic" };

                    if (ImGui::BeginCombo("type", m_bodyType.c_str()))
                    {
                        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                        {
                            bool is_sel = (m_bodyType == items[n]);

                            if (ImGui::Selectable(items[n], is_sel)) 
                                switch (n) {
                                    case 0: body_type = Physics::Body::Type::KINEMATIC; break;
                                    case 1: body_type = Physics::Body::Type::STATIC; break;
                                    case 2: default: body_type = Physics::Body::Type::DYNAMIC; break;
                                }

                            if (is_sel)
                                ImGui::SetItemDefaultFocus();
                        }

                        ImGui::EndCombo();
                    }

                    if (body_type == Physics::Body::Type::DYNAMIC) {
                        ImGui::SliderFloat("density", &density, 0.0f, 1000.0f);
                        ImGui::SliderFloat("friction", &friction, 0.0f, 1.0f);
                        ImGui::SliderFloat("restitution", &restitution, 0.0f, 1.0f);
                    }

                    ImGui::Separator();     

                    if (ImGui::Button("add")) 
                        CreateBody();

                    ImGui::SameLine();

                    if (ImGui::Button("remove") && bodies.size() > 1) {
                        Physics::DestroyBody(bodies.back().pb);
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

            if (show_options)
            {
                ImGui::Checkbox("texture", &m_show_sprite_texture); 

                if (m_show_sprite_texture)
                {
                    //texture
        
                    if (ImGui::ImageButton("texture button", (void*)(intptr_t)m_currentTexture, ImVec2(50, 50)) && System::Utils::GetFileType(AssetManager::Get()->selectedAsset) == System::Resources::Manager::IMAGE)
                        ApplyTexture(AssetManager::Get()->selectedAsset);

                    else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && AssetManager::Get()->selectedAsset.length() && System::Utils::GetFileType(AssetManager::Get()->selectedAsset) != System::Resources::Manager::IMAGE)
                        ImGui::SetTooltip("cannot set texture because selected asset is not of type image.");

                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && m_currentTexture)
                        ImGui::SetTooltip(key.c_str());

                    //if texture applied to sprite
                        
                    if (m_currentTexture)
                    {
                        //spritesheet / atlas

                        if (ImGui::BeginMenu("frames")) 
                        {
                            //apply frames
 
                            if (ImGui::Button("apply") && !framesApplied) {
                          
                                frames.clear(); Editor::Log("1");

                                for (int i = 0; i < frame; i++) 
                                    frames.push_back({ frame_x[i], frame_y[i], frame_width[i], frame_height[i], frame_fX[i], frame_fY[i]}); 

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
                                            
                                            if (ImGui::Button("-x") && frame_fX[i] > 1.0f) {
                                                framesApplied = false;
                                                frame_fX[i]--;
                                            }

                                            ImGui::SameLine();

                                            if (ImGui::Button("+x") && frame_fX[i] <= frame) {
                                                framesApplied = false;
                                                frame_fX[i]++;
                                            }

                                            ImGui::SameLine();

                                            ImGui::Text("factor x: %d", frame_fX[i]);

                                            if (ImGui::Button("-y") && frame_fY[i] > 1) {
                                                framesApplied = false;
                                                frame_fY[i]--;
                                            }

                                            ImGui::SameLine();

                                            if (ImGui::Button("+y") && frame_fY[i]) {
                                                framesApplied = false;
                                                frame_fY[i]++;
                                            }

                                            ImGui::SameLine();

                                            ImGui::Text("factor y: %d", frame_fY[i]);

                                            if (
                                                ImGui::InputInt("position x", &frame_x[i]) ||
                                                ImGui::InputInt("position y", &frame_y[i]) || 
                                                ImGui::InputFloat("width", &frame_width[i]) || 
                                                ImGui::InputFloat("height", &frame_height[i]) 
                                            ) 
                                                framesApplied = false;

                                            ImGui::PopID();

                                        } 

                                    ImGui::EndMenu();
                                }

                                if (ImGui::Button("-") && currentFrame > 0) 
                                    currentFrame--;                    
                                
                                ImGui::SameLine();

                                if (ImGui::Button("+") && currentFrame < spriteHandle->frames - 1) 
                                    currentFrame++; 

                                ImGui::SameLine();

                                ImGui::Text("frame: %d", currentFrame);

                                ImGui::SameLine();

                                ImGui::Text("total: %d", frame); 

                            }

                            if (ImGui::Button("add frame")) 
                            {
                                frame_x.push_back(0);
                                frame_y.push_back(0);
                                frame_width.push_back(0);
                                frame_height.push_back(0);
                                frame_fX.push_back(1);
                                frame_fY.push_back(1);

                                frame++;
                            }

                            ImGui::SameLine();

                            if (ImGui::Button("remove frame") && frame > 0) 
                            {
                                frames.pop_back();

                                frame_x.pop_back();
                                frame_y.pop_back();
                                frame_width.pop_back();
                                frame_height.pop_back();
                                frame_fX.pop_back();
                                frame_fY.pop_back();
                                
                                frame--;
                            }

                            if (ImGui::Button("reset")) 
                            {
                                frames.clear();

                                frame_x.clear();
                                frame_y.clear();
                                frame_width.clear();
                                frame_height.clear();
                                frame_fX.clear();
                                frame_fY.clear();
                                
                                frame = 0;
                            }

                            //load frame data from file

                            if (ImGui::BeginMenu("load frame data"))
                            {
                                for (const auto& asset : AssetManager::Get()->loadedAssets)
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

                                            std::ifstream JSON(Editor::projectPath + path);

                                            if (!JSON.good()) 
                                                Editor::Log("Error parsing. Maybe file is in an inappropriate folder?");

                                            else 
                                            {
                                                json data = json::parse(JSON);

                                                //clear previous frames

                                                frame = 1;
                                                frames.clear();

                                                frame_x.clear();
                                                frame_y.clear();
                                                frame_width.clear();
                                                frame_height.clear();
                                                frame_fX.clear();
                                                frame_fY.clear();

                                                if (data["frames"].size() > 1)
                                                    for (const auto& index : data["frames"]) 
                                                    {
                                                        int x = index["frame"]["x"],
                                                            y = index["frame"]["y"];

                                                        float width = index["frame"]["w"],
                                                            height = index["frame"]["h"];

                                                        frame_x.push_back(x);
                                                        frame_y.push_back(y);
                                                        frame_width.push_back(width);
                                                        frame_height.push_back(height);
                                                        frame_fX.push_back(1);
                                                        frame_fY.push_back(1);

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
                ImGui::Checkbox("cull", &cull);
                ImGui::InputFloat("scroll factor x", &scrollFactorX);
                ImGui::InputFloat("scroll factor y", &scrollFactorY);
                ImGui::SliderFloat("position x", &positionX, -System::Window::s_width, System::Window::s_width); 
                ImGui::SliderFloat("position y", &positionY, -System::Window::s_height, System::Window::s_height); 
                ImGui::SliderFloat("rotation", &rotation, 0.0f, 360.0f); 
                ImGui::SliderFloat("scale x", &scaleX, -100.0f, 100.0f); 
                ImGui::SliderFloat("scale y", &scaleY, -100.0f, 100.0f); 

                ImGui::Checkbox("stroke", &isStroked); 
                ImGui::ColorEdit3("stroke color", (float*)&strokeColor); 
                ImGui::SliderFloat("stroke width", &strokeWidth, 0.0f, 100.0f);
            }

            ImGui::TreePop();
        }


        ImGui::PopID();

    }

}


//------------------------------------


void SpriteNode::Render(float _positionX, float _positionY, float _rotation, float _scaleX, float _scaleY)
{
    //entity handle update
        
    if (spriteHandle)
    {
        spriteHandle->texture.U1 = U1;
        spriteHandle->texture.V1 = V1;
        spriteHandle->texture.U2 = U2;
        spriteHandle->texture.V2 = V2;

        spriteHandle->SetFrame(currentFrame);
        spriteHandle->SetScale(scaleX * _scaleX, scaleY * _scaleY);  
        spriteHandle->SetPosition(positionX + _positionX, positionY + _positionY);   
        spriteHandle->SetRotation(rotation + _rotation); 
        spriteHandle->SetDepth(depth);
        spriteHandle->SetFlip(flippedX, flippedY);
        spriteHandle->SetAlpha(alpha);
        spriteHandle->SetTint(tint);
        spriteHandle->SetStroke(isStroked, strokeColor, strokeWidth);
        spriteHandle->SetCull(cull);

        if (m_currentAnim.key.length())   
            spriteHandle->SetAnimation(m_currentAnim.key.c_str(), m_currentAnim.yoyo, m_currentAnim.rate, m_currentAnim.repeat);

        else
            spriteHandle->StopAnimation();
             
        //entity physics body transform
        
        if (bodies.size())
            for (const auto& body : bodies)   
                body.pb->SetTransform(spriteHandle->position.x + body.x, spriteHandle->position.y + body.y);

        switch (body_type) {
            case Physics::Body::Type::DYNAMIC: m_bodyType = "dynamic"; break;
            case Physics::Body::Type::KINEMATIC: m_bodyType = "kinematic"; break;
            case Physics::Body::Type::STATIC: 
            default: m_bodyType = "static"; break;
        }

        //if (System::Game::GetScene()->ListenForInteraction(spriteHandle) && ImGui::IsMouseDown(ImGuiMouseButton_Left) && (ImGui::IsMouseDown(ImGuiKey_RightShift) || ImGui::IsMouseDown(ImGuiKey_LeftShift)))
            //Editor::selectedEntity = spriteHandle;
    
    }
}


