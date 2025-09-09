#include "../../../build/sdk/include/scene.h"
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
    m_worldWidth = 0; 
    m_worldHeight = 0;
    cullPosition = GetContext().camera->GetPosition(); 
}

//---------------------------------

void Scene::SetWorldDimensions(float width, float height) { 
    m_worldWidth = width;
    m_worldHeight = height;
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

const Process::Context& Scene::GetContext() { 
    return m_context; 
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


//---------------------------------

bool Scene::Spawn::hasBehavior(const std::string& behaviorName) 
{ 
    //match behavior name with spawn behavior name

    for (auto& b : behaviors_attached)
        if (b.first == behaviorName && !b.second) {
            b.second = true;
            return true;
        }

    return false;
}


