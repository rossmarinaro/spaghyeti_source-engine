#include "../../vendors/UUID.hpp"
#include "../../../build/sdk/include/app.h"


//---------------------------------- empty entity

Entity::Entity(int type, bool isSpawn): 
    m_type(type),
    m_is_spawn(isSpawn),
    m_isStatic(false)
{
    name = "Untitled_" + std::to_string(s_count);          
    ID = GenerateID();  
    render_layer = 0;

    SetShader("sprite");
    
    s_count++;
    s_depth++;
}


//------------------------------------ active entity 


Entity::Entity(int type, float x, float y, bool isSpawn):
    m_type(type),
    m_is_spawn(isSpawn),
    m_isStatic(false)
{ 
    position = { x, y };                         
    scrollFactor = { 1.0f, 1.0f };
    scale = { 1.0f, 1.0f }; 
    tint = { 1.0f, 1.0f, 1.0f };
    outlineColor = { 1.0f, 1.0f, 1.0f };
    rotation = 0.0f;  
    alpha = 1.0f;
    outlineWidth = 1.0f;
    active = true;
    alive = true;
    renderable = true;
    outlineEnabled = false;
    cull = false;
    flipX = false;
    flipY = false;
    depth = s_depth + 1;
    ID = GenerateID(); 
    render_layer = 0;

    SetShader("sprite");

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



//----------------------------- set shader


void Entity::SetShader(const std::string& key) {  
    if (System::Application::resources->shaders.find(key) != System::Application::resources->shaders.end())
        shader = Graphics::Shader::Get(key); 
}


//------------------------------------ 


const bool Entity::IsSprite() {
    return m_type == SPRITE || m_type == TILE;
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


//------------------------------------ 


const std::string Entity::GenerateID() 
{
  std::string uuid = UUID::generate_uuid();

    for (int i = 0; i < uuid.length(); i++)
        if (uuid[i] == '-')
            uuid[i] = '_'; 
    
    return uuid;
}


//----------------------------- base entity wrapper


std::shared_ptr<Entity> System::Game::CreateEntity(int type, int layer)
{
    const auto entity = std::make_shared<Entity>(type);

    if (layer == 0)
        GetScene()->entities.emplace_back(entity);

    if (layer == 1 || layer == 2)
        GetScene()->UI.emplace_back(entity);

    entity->render_layer = layer;

    return entity;
}


