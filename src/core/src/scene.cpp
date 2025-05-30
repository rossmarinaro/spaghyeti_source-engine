#include "../../../build/sdk/include/scene.h"

System::Scene::Scene(const Process::Context& context): 
    m_context(context)
        { Init("Untitled" + std::to_string(s_ID)); }

//---------------------------------

System::Scene::Scene(const Process::Context& context, const std::string& key): 
    m_context(context)
        { Init(key); }

//---------------------------------

void System::Scene::Init(const std::string& key) 
{
    this->key = key;
    s_ID++;
    m_paused = false;
    m_worldWidth = 0; 
    m_worldHeight = 0;
    cullPosition = nullptr;
}

//---------------------------------

void System::Scene::SetWorldDimensions(float width, float height) { 
    m_worldWidth = width;
    m_worldHeight = height;
}

//--------------------------------- assign entity to react to input

void System::Scene::SetInteractive(std::shared_ptr<Entity> entity, bool interactive) {

    auto it = std::find_if(virtual_buttons.begin(), virtual_buttons.end(), [&](auto e) { return e.second == entity->ID; });

    if (interactive && it == virtual_buttons.end()) {
        virtual_buttons.push_back({ false, entity->ID });
        return;
    }

    if (it != virtual_buttons.end())
        virtual_buttons.erase(it);
    
}

//--------------------------------- check if cursor is hovering entity

const bool System::Scene::ListenForInteraction(std::shared_ptr<Entity> entity) {
    
    auto it = std::find_if(virtual_buttons.begin(), virtual_buttons.end(), [&](auto e) { return e.second == entity->ID; });

    if (it != virtual_buttons.end()) {
        auto element = *it;
        return element.first;
    }

    return false;
}

//---------------------------------

const Process::Context& System::Scene::GetContext() { 
    return m_context; 
}

//---------------------------------

const bool System::Scene::IsPaused() {
    return m_paused;
}

//---------------------------------

const Math::Vector2 System::Scene::GetWorldDimensions() { 
    return { 
        static_cast<float>(m_worldWidth), 
        static_cast<float>(m_worldHeight) 
    };
}

//---------------------------------

void System::Scene::SetPause(bool isPaused) {
    m_paused = isPaused;
}

//---------------------------------

void System::Scene::SetGlobal(const std::string& key, const std::any& value) { 

    auto it = m_globals.find(key);

    if (it != m_globals.end())
        m_globals.erase(it);

    m_globals.insert({ key, value }); 
}




