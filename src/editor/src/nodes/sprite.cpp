#include "./node.h"
#include "../gui/gui.h"
#include "../editor.h"
#include "../../../../build/sdk/include/app.h"

using namespace editor;


SpriteNode::SpriteNode(const std::string& id): 
    Node(id, "Sprite"),
        key(""),
        tint(glm::vec3(1.0f)),
        show_sprite_texture(false),
        framesApplied(false),
        filter_nearest(true),
        flippedX(false),
        flippedY(false),
        lock_in_place(false),
        make_UI(false),
        frame(1),
        anim(1),
        depth(1),
        restitution(0.0f),
        density(0.0f), 
        friction(0.0f),
        alpha(1.0f),
        U1(0.0f),
        V1(0.0f),
        U2(1.0f),
        V2(1.0f)

{

    this->spriteHandle = nullptr;

    Editor::Log("sprite node " + this->m_name + " created.");   
}

         
//---------------------------


SpriteNode::~SpriteNode()
{
    this->currentTexture = NULL;

    if (this->spriteHandle != nullptr)
        System::Game::DestroyEntity(this->spriteHandle);
    
    if (!this->virtual_node)
        Editor::Log("Sprite node " + this->m_name + " deleted.");
}


//---------------------------


void SpriteNode::Reset(const char* component_type)
{

    bool passAll = strcmp(component_type, "") == 0;

    if (strcmp(component_type, "Shader") == 0 || passAll)
        if (this->spriteHandle.get())
            this->spriteHandle->m_shader = Shader::GetShader("sprite");

    if (strcmp(component_type, "Script") == 0 || passAll)
        this->behaviors.clear();

    if (strcmp(component_type, "Animator") == 0 || passAll)
    {
        this->animBuf1.clear();
        this->animBuf2.clear();
        this->animBuf3.clear();
        this->animBuf4.clear();
        this->anim = 0; 
    }

    if (strcmp(component_type, "Physics") == 0 || passAll)
    {

        for (const auto& body : this->bodies)
            Physics::DestroyBody(body);
        
        this->bodyX.clear();
        this->bodyY.clear(); 
        this->body_width.clear();
        this->body_height.clear();
        this->body_pointer.clear();
        this->bodies.clear();
        this->is_sensor.clear();

        this->restitution = 0.0f;
        this->density = 0.0f;
        this->friction = 0.0f;

    }
    
}


//---------------------------------


void SpriteNode::CreateBody(float x, float y, float width, float height, bool isSensor, int pointerType) 
{

    BoolContainer bc;

    bc.b = isSensor;

    this->bodyX.push_back(x);
    this->bodyY.push_back(y);
    this->body_width.push_back(width);
    this->body_height.push_back(height);
    this->is_sensor.push_back(bc);
    this->body_pointer.push_back(pointerType);

    b2Body* body = Physics::CreateDynamicBody("box", x, y, width, height); 

    this->bodies.push_back(body);
    
}


//---------------------------------


void SpriteNode::RegisterFrames()
{
    std::vector<std::array<int, 6>> framesToPush;

    for (const auto& frame : this->frames)
        framesToPush.push_back({ frame.x, frame.y, frame.width, frame.height, frame.factorX, frame.factorY });

    System::Resources::Manager::LoadFrames(this->key, framesToPush);
}


//--------------------------------- applies texture to current seleted node 


void SpriteNode::ApplyTexture(const std::string& asset)
{  

    if (this->spriteHandle == nullptr) { 

        this->spriteHandle = System::Game::CreateSprite(asset, 0.0f, 0.0f);
        this->spriteHandle->ID = this->m_ID;
    }

    else 
       this->spriteHandle->SetTexture(asset); 

    this->currentTexture = this->spriteHandle->m_texture.GetTexture(asset).ID;  
    this->key = asset;

}


//--------------------------------- applies texture to current seleted node 


void SpriteNode::ApplyAnimation(const std::string& key, int start, int end)
{

    try {

        std::map<std::string, std::pair<int, int>> animsToLoad;

        this->animations.insert({ key, { key, start, end } });

        for (const auto& anim : this->animations)
            animsToLoad.insert({ { anim.second.key, { anim.second.start, anim.second.end } } });
        
        System::Resources::Manager::UnLoadAnims(this->key);
        System::Resources::Manager::LoadAnims(this->key, animsToLoad);

        if (this->spriteHandle) {
            this->spriteHandle->m_anims = System::Resources::Manager::GetAnimations(this->key);
            this->spriteHandle->ReadSpritesheetData();   
        }
  
    }

    catch (std::runtime_error& err) { 
        std::cout << "there was a problem applying animation: " << err.what() << "\n"; 
    }
}


//---------------------------


void SpriteNode::Render(std::shared_ptr<Node> node)
{
	
    ImGui::Separator(); 

    {

        assert(this->m_active);

        ImGui::PushID(("(Sprite) " + this->m_name).c_str());

        if (ImGui::TreeNode(("(Sprite) " + this->m_name).c_str()))
        {
        
            static char name_buf[32] = ""; ImGui::InputText("name", name_buf, 32, ImGuiInputTextFlags_CallbackCompletion, ChangeName, &this->m_ID);

            //save prefab

            if (ImGui::Button("Save prefab")) 
                this->SavePrefab(); 
            
            if (ImGui::BeginMenu("Add Component"))
            {
                if (this->spriteHandle) 
                {

                    if (ImGui::MenuItem("Animator"))
                        this->AddComponent("Animator");

                    if (ImGui::MenuItem("Physics"))
                        this->AddComponent("Physics");
                
                    if (ImGui::MenuItem("Scripts"))
                        this->AddComponent("Script");

                    if (ImGui::MenuItem("Shader"))
                        this->AddComponent("Shader");
   
                }
                else
                    ImGui::Text("Please apply texture before creating a component.");
                
                ImGui::EndMenu();
            }

            //component options

            //-------------------------------- animator

            if (this->HasComponent("Animator") && ImGui::BeginMenu("Animator"))
            {

                auto anim_component = this->GetComponent("Animator", this->m_ID);

                if (anim_component)
                {
                    for (int i = 0; i < this->anim; i++)
                    {

                        ImGui::Text("animation: %d", i);

                        ImGui::PushID(i);
        
                        StringContainer sc;
                        BoolContainer bc;

                        this->do_yoyo.push_back(bc);
                        this->animBuf1.push_back(sc);
                        this->animBuf2.push_back(i);
                        this->animBuf3.push_back(i);
                        this->animBuf4.push_back(2);

                        if (this->spriteHandle && this->spriteHandle->IsSpritesheet())
                        {

                            if (ImGui::Button("play")) 
                                this->currentAnim = { this->animBuf1[i].s, { this->do_yoyo[i].b, this->animBuf4[i] } };
                                
                            ImGui::SameLine(); 

                            if (ImGui::Button("stop")) 
                                this->currentAnim = { "", {} };

                        }

                        if (this->animBuf1.size() && this->animBuf1[i].s.length()) {

                            ImGui::SameLine();

                            if (ImGui::Button("apply")) 
                                this->ApplyAnimation(this->animBuf1[i].s, this->animBuf2[i], this->animBuf3[i]);
                            
                        }

                        if (i != 0 && this->anim > 1)
                        {

                            ImGui::SameLine(); 

                            if (ImGui::Button("remove")) {

                                std::map<std::string, Anims>::iterator it = this->animations.find(this->animBuf1[i].s);

                                if (it != this->animations.end())
                                    this->animations.erase(it);

                                this->anim--;
                            }

                        }

                        ImGui::InputText("key", &this->animBuf1[i].s);
                        ImGui::InputInt("start", &this->animBuf2[i]); 
                        ImGui::InputInt("end", &this->animBuf3[i]);
                        ImGui::InputInt("rate", &this->animBuf4[i]);
                        ImGui::Checkbox("yoyo", &this->do_yoyo[i].b);

                        ImGui::Separator();

                        ImGui::PopID();
                        
                    }

                    if (ImGui::Button("add animation"))
                        this->anim++;
            
                    ImGui::SameLine();

                    if (ImGui::BeginMenu("remove animator?")) {
                        
                        if (ImGui::MenuItem("yes")) 
                            this->RemoveComponent(anim_component); 

                        ImGui::EndMenu();
                    }

                }
              
                ImGui::EndMenu();
            }


            //------------------------------ shader


            if (this->HasComponent("Shader") && ImGui::BeginMenu("Shader")) {

                GUI::RenderShaderOptions(this->m_ID);
                
                ImGui::EndMenu();
            }


            //------------------------------ script


            if (this->HasComponent("Script") && ImGui::BeginMenu("Script")) {

                GUI::RenderScriptOptions(this->m_ID);
                
                ImGui::EndMenu();
            }


            //------------------------------ physics


            if (this->HasComponent("Physics") && ImGui::BeginMenu("Physics"))
            {

                auto physics_component = this->GetComponent("Physics", this->m_ID);

                if (physics_component)
                {
                    for (int i = 0; i < this->bodies.size(); i++)
                    {

                        ImGui::PushID(i);

                        ImGui::Text((i == 0) ? "anchor: %d" : "body: %d", i);

                        ImGui::SliderFloat("offset x", &this->bodyX[i], 0.0f, System::Window::m_width); 
                        ImGui::SliderFloat("offset y", &this->bodyY[i], 0.0f, System::Window::m_height);
                        ImGui::SliderFloat("width", &this->body_width[i], 0.0f, System::Window::m_width); 
                        ImGui::SliderFloat("height", &this->body_height[i], 0.0f, System::Window::m_height);   
                        ImGui::InputInt("type", &this->body_pointer[i]); 

                        //sensor available for static body only

                        if (i > 0)
                            ImGui::Checkbox("sensor", &this->is_sensor[i].b);

                        ImGui::Separator();     

                        if (this->bodies[i] != nullptr)
                        {
                            b2PolygonShape body;
                            body.SetAsBox(this->body_width[i], this->body_height[i]);
                            b2FixtureDef fixtureDef;
                            fixtureDef.shape = &body;

                            this->bodies[i]->DestroyFixture(this->bodies[i]->GetFixtureList());
                            this->bodies[i]->CreateFixture(&fixtureDef);    

                        }

                        ImGui::PopID();

                    }

                    ImGui::Text("settings");

                    ImGui::SliderFloat("density", &this->density, 0.0f, 1000.0f);
                    ImGui::SliderFloat("friction", &this->friction, 0.0f, 1.0f);
                    ImGui::SliderFloat("restitution", &this->restitution, 0.0f, 1.0f);

                    ImGui::Separator();     

                    if (ImGui::Button("add")) 
                        this->CreateBody();

                    ImGui::SameLine();

                    if (ImGui::Button("remove") && this->bodies.size() > 1) {
                        Physics::DestroyBody(this->bodies.back());
                        this->bodies.pop_back();
                    }

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
            
                ImGui::Checkbox("texture", &this->show_sprite_texture); 

                if (this->show_sprite_texture)
                {

                    //texture
        
                    if (ImGui::ImageButton("texture button", (void*)(intptr_t)this->currentTexture, ImVec2(50, 50), ImVec2(0, 1), ImVec2(1, 0)) && System::Utils::GetFileType(Editor::selectedAsset) == "image")
                        this->ApplyTexture(Editor::selectedAsset);

                    else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && Editor::selectedAsset.length() && System::Utils::GetFileType(Editor::selectedAsset) != "image")
                        ImGui::SetTooltip("cannot set texture because selected asset is not of type image.");

                    //if texture applied to sprite
                        
                    if (this->currentTexture)
                    {

                    //spritesheet / atlas

                        if (ImGui::BeginMenu("frames")) 
                        {
                            //apply frames

                            if (ImGui::Button("apply") && !this->framesApplied)
                            {
                                this->framesApplied = true;
                                this->frames.clear();

                                for (int i = 0; i < this->frame; i++) 
                                    this->frames.push_back({ this->frameBuf1[i], this->frameBuf2[i], this->frameBuf3[i], this->frameBuf4[i], this->frameBuf5[i], this->frameBuf6[i]}); 

                                this->RegisterFrames(); 
                                this->spriteHandle->ReadSpritesheetData();
                                this->spriteHandle->SetFrame(0);

                            }

                            if (this->framesApplied) {  
                                ImGui::SameLine(); 
                                ImGui::Text("applied");
                            }

                            ImGui::SameLine();

                            if (this->spriteHandle)
                            {

                                if (ImGui::BeginMenu("frames:"))
                                {

                                    for (int i = 0; i < this->frame; ++i)
                                    {
                                        ImGui::Separator(); 
                                    
                                        ImGui::Text("frame: %d", i); 
                                        
                                        ImGui::PushID(i);
                                        
                                        this->frameBuf1.push_back(0);
                                        this->frameBuf2.push_back(0);
                                        this->frameBuf3.push_back(0);
                                        this->frameBuf4.push_back(0);
                                        this->frameBuf5.push_back(1);
                                        this->frameBuf6.push_back(1);

                                        if (ImGui::Button("-x") && this->frameBuf5[i] > 1) {
                                            this->framesApplied = false;
                                            this->frameBuf5[i]--;
                                        }

                                        ImGui::SameLine();

                                        if (ImGui::Button("+x") && this->frameBuf5[i]) {
                                            this->framesApplied = false;
                                            this->frameBuf5[i]++;
                                        }

                                        ImGui::SameLine();

                                        ImGui::Text("factor x: %d", this->frameBuf5[i]);

                                        if (ImGui::Button("-y") && this->frameBuf6[i] > 1) {
                                            this->framesApplied = false;
                                            this->frameBuf6[i]--;
                                        }

                                        ImGui::SameLine();

                                        if (ImGui::Button("+y") && this->frameBuf6[i]) {
                                            this->framesApplied = false;
                                            this->frameBuf6[i]++;
                                        }

                                        ImGui::SameLine();

                                        ImGui::Text("factor y: %d", this->frameBuf6[i]);

                                        if (
                                            ImGui::InputInt("position x", &this->frameBuf1[i]) ||
                                            ImGui::InputInt("position y", &this->frameBuf2[i]) || 
                                            ImGui::InputFloat("width", &this->frameBuf3[i]) || 
                                            ImGui::InputFloat("height", &this->frameBuf4[i]) 
                                        ) 
                                            this->framesApplied = false;

                                        ImGui::PopID();

                                    } 

                                    ImGui::EndMenu();
                                }

                                if (ImGui::Button("-") && this->spriteHandle->m_currentFrame > 0) 
                                    this->spriteHandle->m_currentFrame--;                    
                                
                                ImGui::SameLine();

                                if (ImGui::Button("+") && this->spriteHandle->m_currentFrame < this->spriteHandle->m_frames - 1) 
                                    this->spriteHandle->m_currentFrame++; 

                                ImGui::SameLine();

                                ImGui::Text("frame: %d", this->spriteHandle->m_currentFrame);

                                ImGui::SameLine();

                                ImGui::Text("total: %d", this->frame); 

                            }

                            if (ImGui::Button("add frame")) 
                                this->frame++;

                            ImGui::SameLine();

                            if (ImGui::Button("remove frame") && this->frame > 1) 
                            {
                                this->frames.pop_back();

                                this->frameBuf1.pop_back();
                                this->frameBuf2.pop_back();
                                this->frameBuf3.pop_back();
                                this->frameBuf4.pop_back();
                                this->frameBuf5.pop_back();
                                this->frameBuf6.pop_back();
                                this->frame--;
                            }

                            ImGui::EndMenu();
                        } 

                        if (ImGui::BeginMenu("UVs"))
                        {
                            ImGui::SliderFloat("U1", &this->U1, 0.0f, 1.0f); 
                            ImGui::SliderFloat("V1", &this->V1, 0.0f, 1.0f);
                            ImGui::SliderFloat("U2", &this->U2, 0.0f, 1.0f);
                            ImGui::SliderFloat("V2", &this->V2, 0.0f, 1.0f);

                            ImGui::EndMenu();
                        }

                        ImGui::Checkbox("filter nearest", &this->filter_nearest);
                        ImGui::Checkbox("lock image", &this->lock_in_place);
                        ImGui::Checkbox("make UI", &this->make_UI);

                        ImGui::Checkbox("flipX", &this->flippedX); 
                        ImGui::Checkbox("flipY", &this->flippedY);
 
                        if (this->filter_nearest) {
                            this->spriteHandle->m_texture.Filter_Min = GL_NEAREST;
                            this->spriteHandle->m_texture.Filter_Max = GL_NEAREST;
                        }
                        
                        else {
                            this->spriteHandle->m_texture.Filter_Min = GL_LINEAR;
                            this->spriteHandle->m_texture.Filter_Max = GL_LINEAR;
                        }

                        this->spriteHandle->m_texture.SetFiltering();

                        ImGui::ColorEdit3("tint", (float*)&this->tint); 
                        ImGui::SliderFloat("alpha", &this->alpha, 0.0f, 1.0f); 

                    }

                }

                ImGui::SliderInt("depth", &this->depth, 0, 1000);

                ImGui::SliderFloat("position x", &this->positionX, -System::Window::m_width, System::Window::m_width); 
                ImGui::SliderFloat("position y", &this->positionY, -System::Window::m_height, System::Window::m_height); 

                ImGui::SliderFloat("rotation", &this->rotation, 0.0f, 360.0f); 

                ImGui::SliderFloat("scale x", &this->scaleX, -100.0f, 100.0f); 
                ImGui::SliderFloat("scale y", &this->scaleY, -100.0f, 100.0f); 


            }

            ImGui::TreePop();
        }

        //entity handle update
        
        if (this->spriteHandle)
        {

            this->spriteHandle->m_texture.U1 = this->U1;
            this->spriteHandle->m_texture.V1 = this->V1;
            this->spriteHandle->m_texture.U2 = this->U2;
            this->spriteHandle->m_texture.V2 = this->V2;
            
            this->spriteHandle->SetScale(this->scaleX, this->scaleY);
            this->spriteHandle->SetPosition(this->positionX, this->positionY);
            this->spriteHandle->SetRotation(this->rotation); 
            this->spriteHandle->SetDepth(this->depth);
            this->spriteHandle->SetFlip(this->flippedX, this->flippedY);
            this->spriteHandle->SetAlpha(this->alpha);
            this->spriteHandle->SetTint(this->tint);

            if (this->currentAnim.first.length())   
                this->spriteHandle->Animate(this->currentAnim.first, this->currentAnim.second.first, this->currentAnim.second.second);

            else
                this->spriteHandle->StopAnimation();

            //entity physics body transform
            
            if (this->bodies.size())
                for (int i = 0; i < this->bodies.size(); i++)   
                {
                    this->bodies[i]->SetTransform(
                        b2Vec2(
                            this->spriteHandle->m_position.x + this->bodyX[i], 
                            this->spriteHandle->m_position.y + this->bodyY[i]
                        ), 0);
                           
                    this->spriteHandle->SetPosition(this->spriteHandle->m_position.x + this->bodyX[i],this->spriteHandle->m_position.y + this->bodyY[i]/* this->positionX, this->positionY */);
                }
        }

        ImGui::PopID();

    }

}


