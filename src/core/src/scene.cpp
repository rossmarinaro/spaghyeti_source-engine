#include "../../../build/sdk/include/scene.h"
#include "../../../build/sdk/include/game.h"
#include "../../../build/sdk/include/utils.h"

using namespace System;

Scene::Scene(const Process::Context& context): 
    m_context(context)
        { Init("Untitled" + std::to_string(s_ID)); }

//---------------------------------

Scene::Scene(const Process::Context& context, const std::string& key): 
    m_context(context)
        { Init(key); }

//---------------------------------

void Scene::Init(const std::string& key) 
{
    this->key = key;
    s_ID++;
    m_paused = false;
    m_depthSort = false;
    m_worldWidth = 0; 
    m_worldHeight = 0;
    cameraTarget = GetContext()->camera->GetPosition(); 
}
 
//---------------------------------

void Scene::SetWorldDimensions(float width, float height) { 
    m_worldWidth = width;
    m_worldHeight = height;
}

//---------------------------------

void Scene::SetDepthSort(bool depthSort) { 
    m_depthSort = depthSort;
}

//--------------------------------- assign entity to react to input

void Scene::SetInteractive(const std::shared_ptr<Entity>& entity, bool interactive) {

    const auto it = std::find_if(virtual_buttons.begin(), virtual_buttons.end(), [&](auto e) { return e.second == entity->ID; });

    if (interactive && it == virtual_buttons.end()) {
        virtual_buttons.push_back({ false, entity->ID });
        return;
    }

    if (it != virtual_buttons.end())
        virtual_buttons.erase(it);
}

//--------------------------------- check if cursor is hovering entity

const bool Scene::ListenForInteraction(const std::shared_ptr<Entity>& entity) {
    
    auto it = std::find_if(virtual_buttons.begin(), virtual_buttons.end(), [&](auto e) { return e.second == entity->ID; });

    if (it != virtual_buttons.end()) {
        const auto element = *it;
        return element.first;
    }

    return false;
}

//---------------------------------

Process::Context* Scene::GetContext() { 
    return &m_context; 
}

//---------------------------------

const bool Scene::IsPaused() {
    return m_paused;
}

//---------------------------------

const Math::Vector2 Scene::GetWorldDimensions() { 
    return { 
        static_cast<float>(m_worldWidth), 
        static_cast<float>(m_worldHeight) 
    };
}

//---------------------------------

void Scene::SetPause(bool isPaused) {
    m_paused = isPaused;
}

//---------------------------------

void Scene::SetGlobal(const std::string& key, const std::any& value) { 

    auto it = m_globals.find(key);

    if (it != m_globals.end())
        m_globals.erase(it);

    m_globals.insert({ key, value }); 
}


//----------------------------------


void Scene::DestroyEntity(std::shared_ptr<Entity> entity)
{
    entity->renderable = false;
    entity->active = false;
    entity->alive = false;

    //reset spawn if applied
    
    auto spawn_it = std::find_if(spawns.begin(), spawns.end(), [&entity](const std::shared_ptr<Spawn>& s) { return s->index == entity->name; });

    if (spawn_it != spawns.end()) {
        const auto spawn = *spawn_it;
       if (!spawn->loop)
           spawn_it = spawns.erase(spawn_it);
    }

    const auto it = std::find(entities.begin(), entities.end(), entity);

    if (it != entities.end())  
        entitiesToRemove.emplace_back(*it);

    else {
        const auto UI_it = std::find(UI.begin(), UI.end(), entity);
        if (UI_it != UI.end())
            entitiesToRemove.emplace_back(*UI_it);
    }

    //remove from vector and disappear into the void
 
    if (entity.unique())
        entity.reset();
}


//-----------------------------


void Scene::DestroyBehavior(const std::string& id)
{
    auto behavior_it = std::find_if(behaviors.begin(), behaviors.end(), [&id](const std::shared_ptr<entity_behaviors::Behavior>& b)
        { return b->ID == id; });

    if (behavior_it != behaviors.end()) {
        auto behavior = (*behavior_it);
        behavior->active = false;
        behavior.reset();
    }
}



//----------------------------------


void Scene::CreateSpawn(
    int type, 
    const std::string& filename, 
    float x, 
    float y, 
    float width, 
    float height, 
    float spawn_width, 
    float spawn_height, 
    const Math::Vector3& tint, 
    int depth,
    float alpha, 
    bool loop, 
    const std::string& behaviorName,
    const Spawn::Body& body
) 
{
    Spawn data;

    data.can_create = true; 
    data.initialized = false;
    data.behavior_initialized = false;
    data.type = type;
    data.filename = filename;
    data.posX = x;
    data.posY = y; 
    data.width = width;
    data.height = height;
    data.spawn_width = spawn_width;
    data.spawn_height = spawn_height;
    data.tint = tint;
    data.depth = depth;
    data.alpha = alpha;
    data.loop = loop;
    data.body = { body.exist, body.self };
    data.index = System::Utils::ReplaceFrom(data.filename, ".", "") + std::to_string(s_spawn_count); 

    //append behaviors if name matches valid loaded behavior name

    if (behaviorName.length())
        data.behavior_attached = behaviorName;

    const auto spawn = std::make_shared<Spawn>(data); 

    spawns.emplace_back(spawn);

    s_spawn_count++;
}



//----------------------------- base entity wrapper


std::shared_ptr<Entity> Scene::CreateEntity(int type, int layer)
{
    const auto entity = std::make_shared<Entity>(type);

    if (layer == 0)
        entities.emplace_back(entity);

    if (layer == 1 || layer == 2)
        UI.emplace_back(entity);

    entity->render_layer = layer;

    return entity;
}



//-----------------------------


std::shared_ptr<Sprite> Scene::CreateSprite(const std::string& key, float x, float y, int frame, float scale, int layer, bool isSpawn)
{
    const auto sprite = std::make_shared<Sprite>(key, x, y, isSpawn);

    #if STANDALONE == 1
        sprite->ReadSpritesheetData();
        sprite->SetFrame(frame);
    #endif

    sprite->SetScale(scale);

    if (layer == 1)
        entities.emplace_back(sprite);

    if (layer == 2)
        UI.emplace_back(sprite);

    return sprite;
}


//----------------------------- ui sprite default layer 1


std::shared_ptr<Sprite> Scene::CreateUISprite(const std::string& key, float x, float y, int frame, float scale)
{
    const Math::Vector2 pos = { x, y };

    const auto element = std::make_shared<Sprite>(key, pos);

    #if STANDALONE == 1
        element->ReadSpritesheetData();
        element->SetFrame(frame);
    #endif

    element->SetScale(scale);
    element->render_layer = 1;

    UI.emplace_back(element);

    return element;
}


//----------------------------- quad


std::shared_ptr<Geometry> Scene::CreateGeom(float x, float y, float width, float height, int layer, bool isStatic, bool isSpawn)
{
    const auto geom = std::make_shared<Geometry>(x, y, width, height, isSpawn);

    if (isStatic)
        geom->SetStatic(true);

    geom->render_layer = layer;

    if (layer == 0)
        entities.emplace_back(geom);

    if (layer == 1 || layer == 2)
        UI.emplace_back(geom);

    return geom;
}


//----------------------------- text


std::shared_ptr<Text> Scene::CreateText(const std::string& content, float x, float y, const std::string& font, int layer)
{
    const auto text = std::make_shared<Text>(content, x, y, font);

    if (layer == 0)
        entities.emplace_back(text);

    if (layer == 1 || layer == 2)
        UI.emplace_back(text);

    text->render_layer = layer;

    return text;
}


