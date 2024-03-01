#include "./node.h"
#include "../gui.h"
#include "../../editor.h"
#include "../../../../../build/include/app.h"


SpriteNode::SpriteNode(const std::string &id): 
    Node(id, "Sprite"),
        show_sprite_uv(false),
        show_sprite_texture(false),
        show_sprite_atlas(false),
        framesApplied(false),
        do_animate(false),
        do_yoyo(false),
        filter_nearest(true),
        frame(1),
        anim(1),
        depth(1),
        restitution(0.0f),
        density(0.0f), 
        friction(0.0f)

{

    this->spriteHandle = nullptr;

    Editor::Log("sprite node " + this->m_name + " created.");   
}


//---------------------------


SpriteNode::~SpriteNode()
{
    currentTexture = NULL;

    if (this->spriteHandle != nullptr)
        Game::DestroyEntity(this->spriteHandle);
    
    Editor::Log("Sprite node " + this->m_name + " deleted.");
}


//---------------------------------


void SpriteNode::CreateBody(
    const char* type, 
    float x, 
    float y, 
    float width, 
    float height,
    bool isSensor,
    int pointerType
) 
{

    BoolContainer bc;

    bc.b = isSensor;

    this->bodyX.push_back(x);
    this->bodyY.push_back(y);
    this->body_width.push_back(width);
    this->body_height.push_back(height);
    this->is_sensor.push_back(bc);
    this->body_pointer.push_back(pointerType);

    b2Body* body;

    if (strcmp("static", type) == 0) 
        body = Game::physics->CreateStaticBody(x, y, width, height); 

    if (strcmp("dynamic", type) == 0) 
        body = Game::physics->CreateDynamicBody(glm::vec2(x, y), glm::vec2(width, height)); 

    this->bodies.push_back({ body, type });
    
}


//--------------------------------- applies texture to current seleted node 


void SpriteNode::ApplyTexture(const std::pair<std::string, GLuint> &asset)
{  

    if (this->spriteHandle == nullptr) { 

        this->spriteHandle = Game::CreateSprite(asset.first, 0.0f, 0.0f);
        this->spriteHandle->ID = this->m_ID;
    }

    else {
        this->spriteHandle->SetTexture(asset.second); 
        this->spriteHandle->m_key = asset.first;
    }

    this->currentTexture = asset.second; 

}


//--------------------------------- applies texture to current seleted node 


void SpriteNode::ApplyAnimation(const std::string &key, int start, int end)
{

    try {

        std::map<std::string, std::pair<int, int>> animsToLoad;

        this->animations.insert({ key, { key, start, end }  });

        for (const auto& anim : this->animations)
            animsToLoad.insert({ { anim.second.key, { anim.second.start, anim.second.end } } });

        System::Resources::Manager::LoadAnims(this->spriteHandle->m_key, animsToLoad);

        this->spriteHandle->m_anims = System::Resources::Manager::GetAnimations(this->spriteHandle->m_key);

        this->spriteHandle->ReadSpritesheetData();     
    }

    catch (std::runtime_error& err) { 
        std::cout << "there was a problem applying animation: " << err.what() << "\n"; 
    }

    
}


//---------------------------


void SpriteNode::Render()
{
	
    ImGui::Separator(); 

    {

        assert(this->m_active);

        ImGui::PushID(("(Sprite) " + this->m_name).c_str());

        if (ImGui::TreeNode(("(Sprite) " + this->m_name).c_str()))
        {
        
            static char name_buf[32] = ""; ImGui::InputText("name", name_buf, 32, ImGuiInputTextFlags_CallbackCompletion, ChangeName, &this->m_ID);

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

            for (const auto &component : this->components)
            {

                //-------------------------------- animator

                if (strcmp(component->m_type, "Animator") == 0 && ImGui::BeginMenu("Animator"))
                {
                    for (int i = 0; i < this->anim; ++i)
                    {

                        ImGui::Separator();

                        ImGui::Text("animation: %d", i);

                        ImGui::PushID(i);

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
                    
                        if (this->animBuf1.size() && this->animBuf1[i].s.length()) {

                            ImGui::SameLine();

                            if (ImGui::Button("apply"))
                                this->ApplyAnimation(this->animBuf1[i].s, this->animBuf2[i], this->animBuf3[i]);
                        }

                        StringContainer sc;

                        this->animBuf1.push_back(sc);
                        this->animBuf2.push_back(i);
                        this->animBuf3.push_back(i);
                        this->animBuf4.push_back(2);

                        ImGui::InputText("key", &this->animBuf1[i].s);
                        ImGui::InputInt("start", &this->animBuf2[i]); 
                        ImGui::InputInt("end", &this->animBuf3[i]);

                        if (this->spriteHandle && this->spriteHandle->IsSpritesheet())
                        {

                            if (ImGui::Button("play") && this->animBuf1[i].s.length()) 
                                this->do_animate = true;
                                
                            ImGui::SameLine(); 

                            if (ImGui::Button("stop") && this->animBuf1[i].s.length()) 
                                this->do_animate = false;

                            if (this->do_animate)   
                                this->spriteHandle->Animate(this->animBuf1[i].s, this->do_yoyo, this->animBuf4[i]);

                            ImGui::SameLine();
                           
                            if (ImGui::Button("+") && this->spriteHandle->m_currentFrame < this->spriteHandle->m_frames - 1)                       
                                this->spriteHandle->m_currentFrame++; 

                            ImGui::SameLine();

                            if (ImGui::Button("-") && this->spriteHandle->m_currentFrame > 0)                         
                                this->spriteHandle->m_currentFrame--;

                            ImGui::SameLine();
                                
                            ImGui::Text("frame: %d", this->spriteHandle->m_currentFrame);

                            ImGui::InputInt("rate", &this->animBuf4[i]);

                            ImGui::Checkbox("yoyo", &this->do_yoyo); 

                        }

                        ImGui::PopID();
                        
                    }

                    if (ImGui::Button("add"))
                        this->anim++;

                    if (ImGui::BeginMenu("remove animator?")) {
                        
                        if (ImGui::MenuItem("yes")) 
                            this->RemoveComponent(component); 

                        ImGui::EndMenu();
                    }

                    ImGui::Separator();

                    ImGui::EndMenu();
                }


                //------------------------------ shader


                if (strcmp(component->m_type, "Shader") == 0 && ImGui::BeginMenu("Shader")) {

                    GUI::RenderShaderOptions(this->m_ID);
                    
                    ImGui::EndMenu();
                }


                //------------------------------ script


                if (strcmp(component->m_type, "Script") == 0 && ImGui::BeginMenu("Script")) {

                    GUI::RenderScriptOptions(this->m_ID);
                    
                    ImGui::EndMenu();
                }


                //------------------------------ physics


                if (strcmp(component->m_type, "Physics") == 0 && ImGui::BeginMenu("Physics"))
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

                        if (this->bodies[i].first != nullptr)
                        {
                            b2PolygonShape body ;//= this->spriteHandle->shape;
                            body.SetAsBox(this->body_width[i], this->body_height[i]);
                            b2FixtureDef fixtureDef;
                            fixtureDef.shape = &body;

                            this->bodies[i].first->DestroyFixture(this->bodies[i].first->GetFixtureList());
                            this->bodies[i].first->CreateFixture(&fixtureDef);    

                        }

                        ImGui::PopID();

                    }

                    ImGui::Text("settings");

                    ImGui::SliderFloat("density", &this->density, 0.0f, 10.0f);
                    ImGui::SliderFloat("friction", &this->friction, 0.0f, 1.0f);
                    ImGui::SliderFloat("restitution", &this->restitution, 0.0f, 1.0f);

                    ImGui::Separator();     

                    if (ImGui::Button("add")) 
                        this->CreateBody("static");

                    ImGui::SameLine();

                    if (ImGui::Button("remove") && this->bodies.size() > 1) {
                        Game::physics->DestroyBody(this->bodies.back().first);
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
            
                ImGui::Checkbox("texture", &this->show_sprite_texture); 

                if (this->show_sprite_texture)
                {

                    //texture
        
                    if (ImGui::ImageButton("texture button", (void*)(intptr_t)this->currentTexture, ImVec2(50, 50), ImVec2(0, 1), ImVec2(1, 0)) && System::Utils::GetFileType(Editor::selectedAsset.first) == "image")
                        this->ApplyTexture(Editor::selectedAsset);

                    else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && Editor::selectedAsset.first.length() && System::Utils::GetFileType(Editor::selectedAsset.first) != "image")
                        ImGui::SetTooltip("cannot set texture because selected asset is not of type image.");
                        
                    if (this->currentTexture)
                    {

                        ImGui::Checkbox("UVs", &this->show_sprite_uv); ImGui::SameLine();

                        ImGui::Checkbox("atlas", &this->show_sprite_atlas); ImGui::SameLine();

                        ImGui::Checkbox("filter nearest", &this->filter_nearest);
 
                        if (this->filter_nearest)
                        {
                            this->spriteHandle->m_texture.Filter_Min = GL_NEAREST;
                            this->spriteHandle->m_texture.Filter_Max = GL_NEAREST;
                        }
                        else
                        {
                            this->spriteHandle->m_texture.Filter_Min = GL_LINEAR;
                            this->spriteHandle->m_texture.Filter_Max = GL_LINEAR;
                        }

                        this->spriteHandle->m_texture.SetFiltering();

                        if (this->show_sprite_uv)
                        {
                            ImGui::SliderFloat("U1", &this->spriteHandle->m_texture.U1, 0.0f, 1.0f); 
                            ImGui::SliderFloat("V1", &this->spriteHandle->m_texture.V1, 0.0f, 1.0f);
                            ImGui::SliderFloat("U2", &this->spriteHandle->m_texture.U2, 0.0f, 1.0f);
                            ImGui::SliderFloat("V2", &this->spriteHandle->m_texture.V2, 0.0f, 1.0f);
                        }

                        //spritesheet / atlas

                        if (this->show_sprite_atlas)
                        {
                            if (ImGui::BeginMenu("frames")) 
                            {

                                ImGui::Text("frames: %d", this->frame); 

                                for (int i = 0; i < this->frame; ++i)
                                {
                                    ImGui::Separator(); 
                              
                                    ImGui::Text("frame: %d", i);
                                    
                                    ImGui::PushID(i);

                                    this->frameBuf1.push_back(i);
                                    this->frameBuf2.push_back(i);
                                    this->frameBuf3.push_back(i);
                                    this->frameBuf4.push_back(i);

                                    if (
                                        ImGui::InputInt("frame x", &this->frameBuf1[i]) ||
                                        ImGui::InputInt("frame y", &this->frameBuf2[i]) || 
                                        ImGui::InputInt("frame width", &this->frameBuf3[i]) || 
                                        ImGui::InputInt("frame height", &this->frameBuf4[i])

                                    ) 
                                        this->framesApplied = false;

                                    ImGui::PopID();

                                } 

                                if (ImGui::Button("apply"))
                                {
                                    
                                    for (int i = 0; i < this->frame; ++i) {
                                        this->frames.push_back({ this->frameBuf1[i], this->frameBuf2[i], this->frameBuf3[i], this->frameBuf4[i] }); 
                                        this->framesApplied = true; this->spriteHandle->SetFrame(0);
                                    }

                                    std::vector<std::array<int, 4>> framesToPush;

                                    for (const auto& fr : this->frames) 
                                        framesToPush.push_back({ fr.x, fr.y, fr.width, fr.height });
                                    
                                    System::Resources::Manager::LoadFrames(this->spriteHandle->m_key, framesToPush);

                                }

                                if (this->framesApplied) {  
                                    ImGui::SameLine(); 
                                    ImGui::Text("applied");
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

                                    this->frame--;
                                }

                                ImGui::EndMenu();
                            }

                        }

                        ImGui::ColorEdit3("tint", (float*)&this->spriteHandle->m_tint); 
                        ImGui::SliderFloat("alpha", &this->spriteHandle->m_alpha, 0.0f, 1.0f); 

                    }

                }

                ImGui::SliderInt("depth", &this->depth, 0, 1000);

                ImGui::SliderFloat("position x", &this->positionX, 0.0f, System::Window::m_width); 
                ImGui::SliderFloat("position y", &this->positionY, 0.0f, System::Window::m_height); 

                ImGui::SliderFloat("rotation", &this->rotation, 0.0f, 360.0f); 

                ImGui::SliderFloat("scale x", &this->scaleX, -2.0f, 100.0f); 
                ImGui::SliderFloat("scale y", &this->scaleY, -2.0f, 100.0f); 


            }

            ImGui::TreePop();
        }

        //entity handle update
        
        if (this->spriteHandle)
        {

            this->spriteHandle->SetScale(this->scaleX, this->scaleY);
            this->spriteHandle->SetPosition(this->positionX, this->positionY);
            this->spriteHandle->SetRotation(this->rotation); 
            this->spriteHandle->SetDepth(this->depth);

            //entity physics body transform
            
            if (this->bodies.size())
                for (int i = 0; i < this->bodies.size(); i++)   
                    this->bodies[i].first->SetTransform(
                        b2Vec2(
                            this->spriteHandle->m_position.x + this->bodyX[i], 
                            this->spriteHandle->m_position.y + this->bodyY[i]
                        ), 0);

        }

        ImGui::PopID();

    }

}


