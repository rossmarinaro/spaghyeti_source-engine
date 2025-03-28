#if USE_JSON == 1
    #include <fstream>
	#include "../../vendors/nlohmann/json.hpp"
	using json = nlohmann::json;
#endif

#include "../../shared/renderer.h"
#include "../../vendors/glm/gtc/matrix_transform.hpp" 
#include "../../vendors/UUID.hpp"
#include "../../vendors/box2d/include/box2d/box2d.h"

#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/window.h"



//---------------------------------- empty entity

Entity::Entity(int type) {
    this->type = type;
    name = "Untitled_" + std::to_string(s_count);
    ID = UUID::generate_uuid();
    s_count++;
    s_depth++;
}


//------------------------------------ active entity 


Entity::Entity(int type, float x, float y)
{ 
    this->type = type;

    position = { x, y };
    scrollFactor = { 1.0f, 1.0f };
    scale = { 1.0f, 1.0f }; 
    tint = { 1.0f, 1.0f, 1.0f };
    rotation = 0.0f;  
    alpha = 1.0f;
    active = true;
    alive = true;
    renderable = true;
    cull = false;
    flipX = false;
    flipY = false;
    depth = s_depth + 1;
    ID = UUID::generate_uuid();

    name = "Untitled_" + std::to_string(s_count);
    s_count++;
    s_depth++;
}


//------------------------------------


void Entity::SetData(const std::string& key, const std::any& value) { 

    auto it = m_data.find(key);

    if (it != m_data.end())
        m_data.erase(it);

    m_data.insert({ key, value }); 
}


//------------------------------------ 


const bool Entity::IsSprite() {
    return type == SPRITE || type == TILE;
}


//------------------------------------ 


void Entity::SetFlip(bool flipX, bool flipY) { 
    this->flipX = flipX; 
    this->flipY = flipY; 
}


//------------------------------------ 		


void Entity::SetScale(float scaleX, float scaleY) { 
    scale.x = scaleX;
    scale.y = scaleY != 1.0f ? 
        scaleY : scaleX; 
}


//------------------------------------ 


void Entity::SetEnabled(bool isEnabled) {
    active = isEnabled;
    renderable = isEnabled;
}


//------------------------------------ 


void Entity::SetPosition(float x, float y) { 
    position.x = x;
    position.y = y; 
}


/* GEOMETRY */


//quad
Geometry::Geometry(float x, float y, float width, float height): 
    Entity(GEOMETRY, x, y),
        m_type(QUAD)
{ 
    this->width = width;
    this->height = height;

    #ifndef __EMSCRIPTEN__
        m_drawStyle = GL_FILL;
    #else 
        m_drawStyle = 1;
    #endif
    
    tint = { 0.0f, 0.0f, 1.0f };
    texture = Graphics::Texture2D::Get("base");
    shader = Graphics::Shader::Get("graphics");
    renderable = true;
    isStatic = false;

    LOG("Geometry: quad created."); 

}


//-------------------------------------------


Geometry::~Geometry() { 
    if (m_type == QUAD)
        LOG("Geometry: quad destroyed."); 

    //...more shapes?
}


//------------------------------------- 
 

void Geometry::Render()
{

    glm::mat4 model = glm::mat4(1.0f); 

    if (m_type == QUAD)
    {
 
        texture.FrameWidth = width;
        texture.FrameHeight = height;
  
        model = glm::translate(model, { 0.5f * width + position.x, 0.5f * height + position.y, 0.0f }); 
        model = glm::rotate(model, glm::radians(rotation), { 0.0f, 0.0f, 1.0f }); 
        model = glm::translate(model, { -0.5f * width - position.x, -0.5f * height - position.y, 0.0f });

        const Math::Vector4 pm = System::Application::game->camera->GetProjectionMatrix(System::Window::s_scaleWidth, System::Window::s_scaleHeight);
        
        const glm::mat4 proj = (glm::highp_mat4)glm::ortho(pm.x, pm.y, pm.z, pm.w, -1.0f, 1.0f), 
                        view = isStatic ? glm::mat4(1.0f) : glm::translate(model, glm::vec3(System::Application::game->camera->GetPosition().x * scrollFactor.x, System::Application::game->camera->GetPosition().y * scrollFactor.y, 0.0f)),
                        mvp = proj * view * model;
                                
        shader.SetVec3f("tint", tint);
        shader.SetFloat("alphaVal", alpha);

        shader.SetMat4("mvp", { 
            { mvp[0][0], mvp[0][1], mvp[0][2], mvp[0][3] }, 
            { mvp[1][0], mvp[1][1], mvp[1][2], mvp[1][3] },   
            { mvp[2][0], mvp[2][1], mvp[2][2], mvp[2][3] },  
            { mvp[3][0], mvp[3][1], mvp[3][2], mvp[3][3] }
        });  

        texture.Update(position, false, false, m_drawStyle, m_thickness); 

    }

    //render other shapes...
}


/* SPRITE */


//-------------------------------------- standard sprite / tile


Sprite::Sprite(const std::string& key, float x, float y, int frame, bool isTile): 
    Entity(SPRITE, x, y)
{   
    this->key = key;
    currentFrame = frame;   
    velocityX = 0.0f;
    velocityY = 0.0f; 

    anims = System::Resources::Manager::GetAnimations(key);
    texture = Graphics::Texture2D::Get(key);
    shader = Graphics::Shader::Get("sprite");          
    
    if (isTile) {
        type = TILE; 
        //shader = Graphics::Shader::Get("batch");
        return;
    }

    LOG("Sprite: \"" + key + "\" created. (generic)");
}


//------------------------------ clone


Sprite::Sprite(const Sprite& sprite):
    Entity(SPRITE, sprite.position.x, sprite.position.y)
{
    key = sprite.key;
    currentFrame = sprite.currentFrame; 
    velocityX = sprite.velocityX;
    velocityY = sprite.velocityY;   

    anims = System::Resources::Manager::GetAnimations(sprite.key);
    texture = Graphics::Texture2D::Get(sprite.key);
    shader = Graphics::Shader::Get("sprite");     

    LOG("Sprite: \"" + key + "\" cloned."); 

}

 
//-------------------------------------- UI sprite

 
Sprite::Sprite(const std::string& key, const Math::Vector2& position): 
    Entity(UI, position.x, position.y)
{
    this->key = key; 
    texture = Graphics::Texture2D::Get(key);
    shader = Graphics::Shader::Get("sprite");  
    
    LOG("Sprite: \"" + key + "\" created. (UI)"); 
}


  
//-------------------------------------------


Sprite::~Sprite() {
    if (type != TILE) 
        LOG("Sprite: \"" + key + "\" destroyed."); 
}



//-------------------------------------------


std::shared_ptr<Sprite> Sprite::Clone() 
{
    const auto clone = std::make_shared<Sprite>(*this);

    clone->ReadSpritesheetData();

    System::Game::GetScene()->entities.push_back(clone); 

    if (bodies.size())
        for (int i = 0; i < bodies.size(); i++)
            clone->bodies.push_back({ 
                Physics::CreateDynamicBody(Physics::BOX, 
                    bodies[0].second.x, 
                    bodies[0].second.y, 
                    bodies[0].second.z, 
                    bodies[0].second.w, 
                    bodies[0].first->isSensor, 
                    bodies[0].first->pointer, 
                    bodies[0].first->density, 
                    bodies[0].first->friction, 
                    bodies[0].first->restitution), { 
                        bodies[0].second.z, 
                        bodies[0].second.w, 
                        bodies[0].second.z, 
                        bodies[0].second.w 
                    } 
                });
      
    return clone;
}


//------------------------------------ velocity, use physics if available / else default to position update


void Sprite::SetVelocity(float velX, float velY) 
{ 

    if (!active)
        return;

    velocityX = velX;
    velocityY = velY;

    if (bodies.size()) 
        bodies[0].first->SetLinearVelocity(velocityX, velocityY);

    else {
        position.x += velocityX /* * System::Application::game->time->GetSeconds() */; 
        position.y += velocityY /* * System::Application::game->time->GetSeconds() */; 
    }

}


//--------------------------------


void Sprite::SetVelocityX(float velX) 
{ 

    if (!active)
        return;

    velocityX = velX; 

    if (bodies.size() && IsContacting()) 
        bodies[0].first->SetLinearVelocity(velocityX, bodies[0].first->GetLinearVelocity().y);

    else
        position.x += velocityX; // System::Application::game->time->GetSeconds();     
}


//---------------------------------


void Sprite::SetVelocityY(float velY) { 

    if (!active)
        return;

    velocityY = velY;

    if (bodies.size()) 
        bodies[0].first->SetLinearVelocity(bodies[0].first->GetLinearVelocity().x, velocityY);
    
    else
        position.y += velocityY; // System::Application::game->time->GetSeconds(); 
}


//----------------------------- set impulse x


void Sprite::SetImpulse(float x, float y) {

    if (active && bodies.size())
        bodies[0].first->ApplyLinearImpulse(x * 10000, y * 10000);
}


//----------------------------- set impulse x


void Sprite::SetImpulseX(float x) {

    if (active && bodies.size())
        bodies[0].first->ApplyLinearImpulse(x * 10000, bodies[0].first->GetLinearVelocity().y);
}


//----------------------------- set impulse y


void Sprite::SetImpulseY(float y) {

    if (active && bodies.size())
        bodies[0].first->ApplyLinearImpulse(bodies[0].first->GetLinearVelocity().x, y * 10000);
}


//----------------------------- remove bodies


void Sprite::RemoveBodies() 
{

    //reset texture position to normal coords

    const float x = bodies[0].first->GetPosition().x + bodies[0].second.x,
                y = bodies[0].first->GetPosition().y + bodies[0].second.y;

    for (auto it = bodies.begin(); it != bodies.end(); ++it) 
        Physics::DestroyBody((*it).first);

    bodies.clear();

    SetPosition(x, y);
}


//----------------------------- set texture


void Sprite::SetTexture(const std::string& key)
{  

    const auto& tex = Graphics::Texture2D::Get(key); 
        
    this->key = key; 

    texture.ID = tex.ID;
    texture.Width = tex.Width; 
    texture.Height = tex.Height;
    texture.FrameWidth = tex.FrameWidth; 
    texture.FrameHeight = tex.FrameHeight;
    texture.U1 = tex.U1;
    texture.U2 = tex.U2;
    texture.V1 = tex.V1;
    texture.V2 = tex.V2;
    texture.Repeat = tex.Repeat;
    texture.Wrap_S = tex.Wrap_S;
    texture.Wrap_T = tex.Wrap_T;
    texture.Filter_Min = tex.Filter_Min;
    texture.Filter_Max = tex.Filter_Max; 

    m_currentAnim = { "", false, 0, 0, 0 }; 
}


//----------------------------- read frame data


void Sprite::ReadSpritesheetData()
{    
    
    const std::string& spritesheet = System::Resources::Manager::GetSpritesheetPath(key);
 
    //json file

    if (System::Utils::str_endsWith(spritesheet, ".json")) 
    {

        #if USE_JSON == 1 

            std::ifstream JSON(spritesheet);

            const json data = json::parse(JSON);

                for (const auto& frame : data["frames"])
                    if (frame.contains("frame")) 
                    {
                        int x = frame["frame"]["x"],
                            y = frame["frame"]["y"],
                            w = frame["frame"]["w"],
                            h = frame["frame"]["h"];

                        m_resourceData.push_back({ x, y, w, h });
                    }
        #else 

            LOG("Sprite: an attempt to load JSON failed, because JSON has not been enabled.");
            
        #endif

    }

    //int array

    else if (System::Resources::Manager::GetRawSpritesheetData(key).size())
        m_resourceData = System::Resources::Manager::GetRawSpritesheetData(key);  

    //not a spritesheet

    else 
        return;
    
    m_isSpritesheet = true;
    frames = m_resourceData.size();

}


//------------------------------------------ animations


void Sprite::SetAnimation(const std::string& key, bool yoyo, int rate, int repeat) { 
    m_animComplete = false;
    m_currentAnim = { key, rate, repeat, yoyo, true, false };
}
		

//------------------------------------------ 

void Sprite::StopAnimation() { 
    m_animComplete = false;
    m_currentAnim = { "", 0, 0, false, false, false }; 
}


//------------------------------------------ render sprite / update transformations


void Sprite::Render()
{  

    if (!alive)
        return;

    //update spritesheet UV subtexturing if applicable

    if (m_isSpritesheet) 
    {
        
        const float currentFrameX = (float)m_resourceData[currentFrame][0],
                    currentFrameY = (float)m_resourceData[currentFrame][1],
                    currentFrameWidth = (float)m_resourceData[currentFrame][2],
                    currentFrameHeight = (float)m_resourceData[currentFrame][3],
                    factorX = (float)m_resourceData[currentFrame][4],
                    factorY = (float)m_resourceData[currentFrame][5];

        texture.FrameWidth = currentFrameWidth;
        texture.FrameHeight = currentFrameHeight;

        //tilemap

        if (type == TILE) {
            texture.U1 = (currentFrameX * currentFrameWidth) / texture.Width;      
            texture.U2 = ((currentFrameX + 1) * currentFrameWidth) / texture.Width;

            texture.V1 = (currentFrameY * currentFrameHeight) / texture.Height; 
            texture.V2 = ((currentFrameY + 1) * currentFrameHeight) / texture.Height; 
        }

        //generic

        else {
            texture.U1 = (currentFrameX * factorX) / texture.Width;      
            texture.U2 = ((currentFrameX + currentFrameWidth) * factorX) / texture.Width;

            texture.V1 = (currentFrameY * factorY) / texture.Height; 
            texture.V2 = ((currentFrameY + currentFrameHeight) * factorY) / texture.Height; 
        }
 
    }

    //sprite model transformations
 
    const Math::Vector4& pm = System::Application::game->camera->GetProjectionMatrix(System::Window::s_scaleWidth, System::Window::s_scaleHeight);
    const Math::Matrix4& vm = System::Application::game->camera->GetViewMatrix((System::Application::game->camera->GetPosition().x * scrollFactor.x * scale.x), (System::Application::game->camera->GetPosition().y * scrollFactor.y * scale.y));
    
    const glm::mat4 view = !IsSprite() ? glm::mat4(1.0f) : glm::mat4({ vm.a.x, vm.a.y, vm.a.z, vm.a.w }, { vm.b.x, vm.b.y, vm.b.z, vm.b.w }, { vm.c.x, vm.c.y, vm.c.z, vm.c.w }, { vm.d.x, vm.d.y, vm.d.z, vm.d.w }), 
                    proj = (glm::highp_mat4)glm::ortho(pm.x, pm.y, pm.z, pm.w, -1.0f, 1.0f);

    glm::mat4 model = glm::mat4(1.0f); 

    model = glm::translate(model, { 0.5f * texture.FrameWidth + position.x * scale.x, 0.5f * texture.FrameHeight + position.y * scale.y, 0.0f }); 
    model = glm::rotate(model, glm::radians(rotation), { 0.0f, 0.0f, 1.0f }); 
    model = glm::translate(model, { -0.5f * texture.FrameWidth - position.x * scale.x, -0.5f * texture.FrameHeight - position.y * scale.y, 0.0f });

    //update shaders and textures 

    if (active)
    {

        //render texture

        if (renderable)
        {
            //if (strcmp(type, "tile") == 0)
            //shader.SetInt("images", 1, true);
            //else
                shader.SetInt("image", 0);    

            #ifndef __EMSCRIPTEN__
                shader.SetInt("repeat", texture.Repeat);
            #endif

            //set shader uniforms

            shader.SetVec2f("scale", { scale.x, scale.y });
            shader.SetFloat("alphaVal", alpha); 
            shader.SetVec3f("tint", tint);

            const glm::mat4 mvp = proj * view * model;

            shader.SetMat4("mvp", { 
                { mvp[0][0], mvp[0][1], mvp[0][2], mvp[0][3] }, 
                { mvp[1][0], mvp[1][1], mvp[1][2], mvp[1][3] },   
                { mvp[2][0], mvp[2][1], mvp[2][2], mvp[2][3] },  
                { mvp[3][0], mvp[3][1], mvp[3][2], mvp[3][3] }
            });  

            unsigned int fill = 1;

            #ifndef __EMSCRIPTEN__
                fill = GL_FILL;
            #endif

            texture.Update(position, flipX, flipY, fill);   
 
        }

        //update physics bodies if exists

        if (bodies.size())
            for (int i = 0; i < bodies.size(); i++)
                if (bodies[i].first->IsEnabled()) {
                    if (i == 0 && bodies[i].first->GetType() == b2_dynamicBody) { 
                        Math::Vector2 pos = bodies[0].first->GetPosition(); 
                        pos.y += bodies[0].second.w; //apply y offset
                        SetPosition(pos.x - (bodies[0].second.x * scale.x), (pos.y - (bodies[0].second.y * scale.y)) - bodies[0].second.w);
                    }
                    else
                        bodies[i].first->SetTransform((position.x - (bodies[0].second.x * scale.x)), ((position.y - (bodies[0].second.y * scale.y)) - bodies[0].second.w));
                }

        //play current animation

        if (m_isSpritesheet && m_currentAnim.key.length())
        {
            const std::string& animKey = m_currentAnim.key;
            const bool yoyo = m_currentAnim.yoyo;
            const int rate = m_currentAnim.rate;

            const uint32_t seconds = System::Application::game->time->GetSeconds() * rate;
           
            try {

                if (m_isSpritesheet && active)
                {
                    const auto anim = anims.find(animKey.c_str());

                    if (anim == anims.end() || System::Game::GetScene()->IsPaused() || ((m_currentAnim.repeat <= 0 && m_currentAnim.repeat != -1))) 
                        return;

                    std::vector<int> frames; //frames to populate  
                    
                    int startFrame = anims.find(m_currentAnim.key.c_str())->second.first,
                        endFrame = anims.find(m_currentAnim.key.c_str())->second.second,
                        frame = yoyo ? startFrame : endFrame;

                    m_animComplete = frame == currentFrame && m_currentAnim.can_complete;
                    m_currentAnim.can_complete = false;

                    if (yoyo)
                    {

                        for (int i = anim->second.first; i < anim->second.second + 1; i++) 
                            frames.emplace_back(i);
                        
                        const uint32_t elapsed = seconds % frames.size();

                        std::vector<int> frames_reversed;

                        for (int i = anim->second.second; i > anim->second.first - 1; i--) 
                            frames_reversed.emplace_back(i);

                        const uint32_t elapsed_reversed = seconds % frames_reversed.size();

                        if (!m_anim_yoyo && currentFrame == anim->second.second && frames[elapsed] != anim->second.second) 
                            m_anim_yoyo = true;

                        if (m_anim_yoyo && currentFrame == anim->second.first && frames_reversed[elapsed_reversed] != anim->second.first)    
                            m_anim_yoyo = false;

                        if (m_anim_yoyo) 
                            SetFrame(frames_reversed[elapsed_reversed]);

                        else  
                            SetFrame(frames[elapsed]);
                    }

                    else {

                        for (int i = anim->second.first; i < anim->second.second + 1; i++) 
                            frames.emplace_back(i);
                        
                        const uint32_t elapsed = seconds % frames.size();
                   
                        SetFrame(frames[elapsed]);

                    }

                    //animation complete

                    m_currentAnim.can_complete = true;

                    if (currentFrame == anim->second.second) {
                        if (m_currentAnim.can_decrement) {
                            m_currentAnim.can_decrement = false;

                            if (m_currentAnim.repeat > -1) 
                                m_currentAnim.repeat--; 
                        }
                    }

                    else 
                        m_currentAnim.can_decrement = true;

                }
            }

            catch (std::runtime_error& err) { 
                LOG("Sprite: error playing animation: " << err.what()); 
            }
        }
         
    }

}
 


//----------------------------------- (sprite overlap not box2d related)


const bool Sprite::CheckOverlap(const std::shared_ptr<Sprite>& spriteA, const std::shared_ptr<Sprite>& spriteB)
{
    //x axis

    const bool collisionX = spriteA->position.x + spriteA->texture.FrameWidth / 2 >= spriteB->position.x &&
                    spriteB->position.x + spriteB->texture.FrameWidth / 2 >= spriteA->position.x,

    //y axis

        collisionY = spriteA->position.y + spriteA->texture.FrameHeight / 2 >= spriteB->position.y &&
                    spriteB->position.y + spriteB->texture.FrameHeight / 2 >= spriteA->position.y;

    return collisionX && collisionY;

}
 
