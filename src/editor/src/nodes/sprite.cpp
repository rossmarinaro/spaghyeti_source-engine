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
    currentFrame = 0;
    depth = 1;
    alpha = 1.0f;
    U1 = 0.0f;
    V1 = 0.0f;
    U2 = 1.0f;
    V2 = 1.0f;
    scrollFactorX = 1.0f;
    scrollFactorY = 1.0f;
    spriteHandle = nullptr;
    anim_to_play_on_start = { "", 2, -1, false };

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
            Physics::DestroyBody(body.second);
        
        bodies.clear();
    }
}


//---------------------------------


std::shared_ptr<Physics::Body> SpriteNode::CreateBody(
    int type,
    int shape, 
    float x, 
    float y, 
    float width, 
    float height, 
    float radius, 
    float restitution,
    float density,
    float friction,
    bool isSensor, 
    int pointerType
) 
{
    const auto pb = shape == Physics::Body::Shape::BOX ? 
        Physics::CreateBody(type, x, y, width, height, isSensor, pointerType, density, friction, restitution) : //box
        Physics::CreateBody(type, x, y, radius, isSensor, pointerType, density, friction, restitution); //circle

    std::pair body = { "", pb };

    bodies.emplace_back(body);

    return pb;
}



//---------------------------------


void SpriteNode::UpdateBody(const std::shared_ptr<Physics::Body>& body) 
{
    if (body->shape == Physics::Body::Shape::BOX)
        body->UpdateFixture(body->width, body->height);   

    if (body->shape == Physics::Body::Shape::CIRCLE)
        body->UpdateFixture(body->radius);

    body->SetTransform(body->x + body->width / 2, body->y + body->height / 2);
}


//---------------------------------


const std::string SpriteNode::BodyTypeToString(int type) {
    switch (type) {
        case Physics::Body::Type::DYNAMIC: return "dynamic";
        case Physics::Body::Type::KINEMATIC: return "kinematic";
        case Physics::Body::Type::STATIC: 
        default: return "static";
    }

}

//--------------------------------- load frames, overwrite previously cached key


void SpriteNode::RegisterFrames() 
{
    std::vector<std::array<int, 6>> framesToPush;

    for (const auto& frame : frames) {
        std::array<int, 6> fr = { 
            frame.x, frame.y, 
            frame.width, frame.height, 
            frame.factorX, frame.factorY 
        };

        framesToPush.emplace_back(fr);
    }

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

        ImGui::PushID(("(Sprite) " + ID).c_str());

        std::string selText = (Editor::selectedEntity && spriteHandle) && Editor::selectedEntity->ID == spriteHandle->ID ? "<SELECTED> " : "";
   
        if (GUI::Get()->collapseFolders)
            ImGui::SetNextItemOpen(false, ImGuiCond_Always);

        if (ImGui::TreeNodeEx((void*)(intptr_t)0, ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick, (selText + "(Sprite) " + name).c_str()))
        {
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                Editor::Get()->events->selected_nodes = &nodes;

            Node::Update(arr);

            if (ImGui::Button("Select") && spriteHandle) 
                Editor::FocusEntity(spriteHandle);
            
            if (ImGui::BeginMenu("Add Component"))
            {
                if (spriteHandle) 
                {
                    if (ImGui::MenuItem("Animator")) {
                        AddComponent(Component::ANIMATOR);
                        EventListener::UpdateSession();
                    }

                    if (ImGui::MenuItem("Physics")) {
                        AddComponent(Component::PHYSICS);
                        EventListener::UpdateSession();
                    }
                
                    if (ImGui::MenuItem("Scripts")) {
                        AddComponent(Component::SCRIPT);
                        EventListener::UpdateSession();
                    }

                    if (ImGui::MenuItem("Shader")) {
                        AddComponent(Component::SHADER);
                        EventListener::UpdateSession();
                    }
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
                    
                    if (ImGui::Button("add animation")) {
                        animations.push_back({ "", 0, 0, 2, -1, false }); 
                        EventListener::UpdateSession();
                    }
            
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

                                if (animations[i].key.size()) {
                                    ImGui::SameLine();
                                    if (ImGui::Button("apply")) {
                                        ApplyAnimation(animations[i].key);
                                        EventListener::UpdateSession();
                                    }
                                }

                                if (i != 0 && animations.size()) {
                                    ImGui::SameLine(); 
                                    if (ImGui::Button("remove")) {
                                        auto it = std::find_if(animations.begin(), animations.end(), [&](const Sprite::Anim& anim) { return anim.key == animations[i].key; });
                                        if (it != animations.end()) {
                                            it = animations.erase(it);
                                            --it;
                                            EventListener::UpdateSession();
                                        }
                                    }
                                }

                                if (ImGui::InputText("key", &animations[i].key))
                                    EventListener::UpdateSession();
                                if (ImGui::InputInt("start", &animations[i].start))
                                    EventListener::UpdateSession(); 
                                if (ImGui::InputInt("end", &animations[i].end))
                                    EventListener::UpdateSession();
                                if (ImGui::InputInt("rate", &animations[i].rate))
                                    EventListener::UpdateSession();
                                if (ImGui::InputInt("repeat", &animations[i].repeat))
                                    EventListener::UpdateSession();

                                if (animations[i].repeat < -1)
                                    animations[i].repeat = -1;
                                
                                if (ImGui::Checkbox("yoyo", &animations[i].yoyo))
                                    EventListener::UpdateSession();

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
                                EventListener::UpdateSession();
                            } 
                        }
                            
                        ImGui::EndCombo();
                    }

                    if (anim_to_play_on_start.key.length() && ImGui::BeginMenu("remove default animation")) {
                        
                        if (ImGui::MenuItem("yes")) {
                            anim_to_play_on_start = { "", 2, -1, false };
                            EventListener::UpdateSession();
                        }

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

                        if (i >= 1)
                            ImGui::Separator();

                        ImGui::Text((i == 0) ? "anchor: %d" : "body: %d", i);
                        
                        if (ImGui::Checkbox("is sensor", &body.second->isSensor))
                            EventListener::UpdateSession();

                        if (ImGui::InputInt("int pointer", (int*)&body.second->pointer))
                            EventListener::UpdateSession(); 

                        if (body.second->pointer <= -1)
                            body.second->pointer = -1;  
                        
                        static const char* items[] = { "kinematic", "static", "dynamic" };

                        if (ImGui::BeginCombo("body type", body.first.c_str()))
                        {
                            for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                            {
                                bool is_sel = (body.first == items[n]);

                                if (ImGui::Selectable(items[n], is_sel)) 
                                {
                                    switch (n) {
                                        case 0: body.second->SetType(Physics::Body::Type::KINEMATIC); break;
                                        case 1: body.second->SetType(Physics::Body::Type::STATIC); break;
                                        case 2: default: body.second->SetType(Physics::Body::Type::DYNAMIC); break;
                                    }

                                    EventListener::UpdateSession();
                                }

                                if (is_sel)
                                    ImGui::SetItemDefaultFocus();
                            }

                            ImGui::EndCombo();
                        }

                        if (ImGui::SliderFloat("offset x", &body.second->x, 0.0f, System::Window::s_width))
                            UpdateBody(body.second);

                        if (ImGui::IsItemDeactivatedAfterEdit())
                            EventListener::UpdateSession();

                        if (ImGui::SliderFloat("offset y", &body.second->y, 0.0f, System::Window::s_height))
                            UpdateBody(body.second);

                        if (ImGui::IsItemDeactivatedAfterEdit()) 
                            EventListener::UpdateSession();

                        //shapes

                        if (body.second->shape == Physics::Body::Shape::BOX)
                        {
                            if (ImGui::SliderFloat("width", &body.second->width, 0.0f, System::Window::s_width))
                                UpdateBody(body.second);

                            if (ImGui::IsItemDeactivatedAfterEdit()) 
                                EventListener::UpdateSession();

                            if (ImGui::SliderFloat("height", &body.second->height, 0.0f, System::Window::s_height))
                                UpdateBody(body.second);

                            if (ImGui::IsItemDeactivatedAfterEdit()) 
                                EventListener::UpdateSession();
                        }

                        if (body.second->shape == Physics::Body::Shape::CIRCLE) 
                        {
                            if (ImGui::SliderFloat("radius", &body.second->radius, 0.0f, 2000.0f))
                                UpdateBody(body.second);

                            if (ImGui::IsItemDeactivatedAfterEdit()) 
                                EventListener::UpdateSession();
                        }

                        //settings

                        if (body.second->type == Physics::Body::Type::DYNAMIC) 
                        {
                            ImGui::SliderFloat("density", &body.second->density, 0.0f, 1000.0f);
                            if (ImGui::IsItemDeactivatedAfterEdit())
                                EventListener::UpdateSession();

                            ImGui::SliderFloat("friction", &body.second->friction, 0.0f, 1.0f);
                            if (ImGui::IsItemDeactivatedAfterEdit())
                                EventListener::UpdateSession();

                            ImGui::SliderFloat("restitution", &body.second->restitution, 0.0f, 1.0f);
                            if (ImGui::IsItemDeactivatedAfterEdit())
                                EventListener::UpdateSession();
                        }
 
                        ImGui::PopID();

                        i++;
                    } 

                    if (i >= 1)
                        ImGui::Separator();

                    if (ImGui::BeginMenu("add")) 
                    {
                        if (ImGui::MenuItem("box")) {
                            CreateBody(Physics::Body::Type::DYNAMIC, Physics::Body::Shape::BOX);
                            EventListener::UpdateSession();
                        }
                        
                        if (ImGui::MenuItem("circle")) {
                            CreateBody(Physics::Body::Type::DYNAMIC, Physics::Body::Shape::CIRCLE);
                            EventListener::UpdateSession();
                        }
 
                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu("remove"))
                    {
                        if (ImGui::MenuItem("last") && bodies.size() > 0) 
                        {
                            Physics::DestroyBody(bodies.back().second);
                            bodies.pop_back();
                            EventListener::UpdateSession();       
                        }

                        if (ImGui::MenuItem("all") && bodies.size() > 0) {
                            for (const auto& body : bodies)
                                Physics::DestroyBody(body.second);
        
                            bodies.clear();
                        }
                        
                        ImGui::EndMenu();
                    }

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
        
                    if (ImGui::ImageButton("texture button", (void*)(intptr_t)m_currentTexture, ImVec2(50, 50)) && System::Utils::GetFileType(AssetManager::Get()->selectedAsset) == System::Resources::Manager::IMAGE) {
                        ApplyTexture(AssetManager::Get()->selectedAsset);
                        EventListener::UpdateSession();
                    }
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
 
                            if (ImGui::Button("apply") && !framesApplied) 
                            {
                                RegisterFrames();  
                                
                                framesApplied = true;
                                spriteHandle->ReadSpritesheetData(); 
                                spriteHandle->SetFrame(0);

                                EventListener::UpdateSession();
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
                                    if (!frames.size()) 
                                        ImGui::Text("no frames defined"); 
                                    else 
                                    {
                                        int i = 0;
                                        
                                        for (auto& frame : frames)
                                        {
                                            ImGui::Separator(); 
                                        
                                            ImGui::Text("frame: %d", i); 
                                            
                                            ImGui::PushID(i);
                                            
                                            if (ImGui::Button("-x") && frame.factorX > 1) {
                                                framesApplied = false;
                                                frame.factorX--;
                                                EventListener::UpdateSession();
                                            }

                                            ImGui::SameLine();

                                            if (ImGui::Button("+x") && frame.factorX <= frames.size()) {
                                                framesApplied = false;
                                                frame.factorX++;
                                                EventListener::UpdateSession();
                                            }

                                            ImGui::SameLine();

                                            ImGui::Text("factor x: %d", frame.factorX);

                                            if (ImGui::Button("-y") && frame.factorY > 1) {
                                                framesApplied = false;
                                                frame.factorY--;
                                                EventListener::UpdateSession();
                                            }

                                            ImGui::SameLine();

                                            if (ImGui::Button("+y") && frame.factorY <= frames.size()) {
                                                framesApplied = false;
                                                frame.factorY++;
                                                EventListener::UpdateSession();
                                            }

                                            ImGui::SameLine();

                                            ImGui::Text("factor y: %d", frame.factorY);

                                            if (
                                                ImGui::InputInt("position x", &frame.x) ||
                                                ImGui::InputInt("position y", &frame.y) || 
                                                ImGui::InputInt("width", &frame.width) || 
                                                ImGui::InputInt("height", &frame.height) 
                                            ) {
                                                framesApplied = false;
                                                EventListener::UpdateSession();
                                            }

                                            ImGui::PopID();

                                            i++;
                                        } 
                                    }

                                    ImGui::EndMenu();
                                }

                                if (ImGui::Button("-") && currentFrame > 0) {
                                    currentFrame--;                    
                                    EventListener::UpdateSession();
                                }
                                
                                ImGui::SameLine();

                                if (ImGui::Button("+") && currentFrame < spriteHandle->frames - 1) {
                                    currentFrame++; 
                                    EventListener::UpdateSession();
                                }

                                ImGui::SameLine();

                                ImGui::Text("frame: %d", currentFrame);

                                ImGui::SameLine();

                                ImGui::Text("total: %d", frames.size()); 

                            }

                            if (ImGui::Button("add frame")) 
                            {
                                Frame fr = { 0, 0, 0, 0, 1, 1 };
                                frames.emplace_back(fr);
                                
                                EventListener::UpdateSession();
                            }

                            ImGui::SameLine();

                            if (ImGui::Button("remove frame") && frames.size()) {
                                frames.pop_back();
                                EventListener::UpdateSession();
                            }

                            if (ImGui::Button("reset")) {
                                frames.clear();
                                EventListener::UpdateSession();
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

                                                frames.clear();

                                                if (data["frames"].size())
                                                    for (const auto& index : data["frames"]) 
                                                    {
                                                        int x = index["frame"]["x"],
                                                            y = index["frame"]["y"];

                                                        float width = index["frame"]["w"],
                                                              height = index["frame"]["h"];

                                                        Frame frame;

                                                        frame.x = x;
                                                        frame.y = y;
                                                        frame.width = width;
                                                        frame.height = height;

                                                        frames.emplace_back(frame);
                                                    };

                                                EventListener::UpdateSession();
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
                            if (ImGui::IsItemDeactivatedAfterEdit())
                                EventListener::UpdateSession();

                            ImGui::SliderFloat("V1", &V1, 0.0f, 1.0f);
                            if (ImGui::IsItemDeactivatedAfterEdit())
                                EventListener::UpdateSession();

                            ImGui::SliderFloat("U2", &U2, 0.0f, 1.0f);
                            if (ImGui::IsItemDeactivatedAfterEdit())
                                EventListener::UpdateSession();

                            ImGui::SliderFloat("V2", &V2, 0.0f, 1.0f);
                            if (ImGui::IsItemDeactivatedAfterEdit())
                                EventListener::UpdateSession();

                            ImGui::EndMenu();
                        }

                        if (ImGui::Checkbox("filter nearest", &filter_nearest))
                            EventListener::UpdateSession();

                        if (ImGui::Checkbox("lock image", &lock_in_place))
                            EventListener::UpdateSession();

                        if (ImGui::Checkbox("make UI", &make_UI))
                            EventListener::UpdateSession();

                        if (ImGui::IsItemClicked())
                            spriteHandle->SetAsUI(!make_UI);

                        if (ImGui::Checkbox("flipX", &flippedX))
                            EventListener::UpdateSession(); 
                        if (ImGui::Checkbox("flipY", &flippedY))
                            EventListener::UpdateSession();
 
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
                        if (ImGui::IsItemDeactivatedAfterEdit())
                            EventListener::UpdateSession();

                        ImGui::SliderFloat("alpha", &alpha, 0.0f, 1.0f); 
                        if (ImGui::IsItemDeactivatedAfterEdit())
                            EventListener::UpdateSession();

                    }

                }

                ImGui::SliderInt("depth", &depth, 0, 1000);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();
                 
                if (ImGui::Checkbox("cull", &cull))
                    EventListener::UpdateSession();
       
                ImGui::InputFloat("scroll factor x", &scrollFactorX);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();
     
                ImGui::InputFloat("scroll factor y", &scrollFactorY);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();
            
                ImGui::SliderFloat("position x", &positionX, -Editor::Get()->worldWidth, Editor::Get()->worldWidth);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();
             
                ImGui::SliderFloat("position y", &positionY, -Editor::Get()->worldHeight, Editor::Get()->worldHeight);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();
           
                ImGui::SliderFloat("rotation", &rotation, 0.0f, 360.0f);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();
             
                ImGui::SliderFloat("scale x", &scaleX, -100.0f, 100.0f);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();
         
                ImGui::SliderFloat("scale y", &scaleY, -100.0f, 100.0f);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();

                if (ImGui::Checkbox("stroke", &isStroked))
                    EventListener::UpdateSession(); 

                ImGui::ColorEdit3("stroke color", (float*)&strokeColor);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();
        
                ImGui::SliderFloat("stroke width", &strokeWidth, 0.0f, 100.0f);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();
        
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
            spriteHandle->SetAnimation(m_currentAnim.key, m_currentAnim.yoyo, m_currentAnim.rate, m_currentAnim.repeat);
        else {
            spriteHandle->StopAnimation();
            spriteHandle->SetFrame(currentFrame);
        }
             
        //entity physics body transform
        
        if (bodies.size())
            for (auto& body : bodies) {
                body.first = BodyTypeToString(body.second->type);
                body.second->SetTransform(spriteHandle->position.x + body.second->x, spriteHandle->position.y + body.second->y);
            }

        if (System::Game::GetScene()->ListenForInteraction(spriteHandle) && ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse)
            Editor::FocusEntity(spriteHandle);
    }   
}


