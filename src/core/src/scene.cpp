#include "../../../build/sdk/include/scene.h"

System::Scene::Scene(const Process::Context& context): 
    m_context(context)
        { Init("Untitled" + std::to_string(s_ID)); }

//---------------------------------

System::Scene::Scene(const Process::Context& context, const std::string& key): 
    m_context(context)
        { Init(key); }

//---------------------------------

void System::Scene::Init(const std::string& key) {
    s_ID++;
    m_worldWidth = 0; 
    m_worldHeight = 0;
    this->key = key;
}

//---------------------------------

void System::Scene::SetWorldDimensions(float width, float height) { 
    m_worldWidth = width;
    m_worldHeight = height;
}

//--------------------------------- assign entity to react to input

void System::Scene::SetInteractive(std::shared_ptr<Entity> entity, bool interactive) {

    auto it = std::find_if(virtual_buttons.begin(), virtual_buttons.end(), [&](auto e) { return e.second == entity; });

    if (interactive && it == virtual_buttons.end()) {
        virtual_buttons.push_back({ 0, entity });
        return;
    }

    if (it != virtual_buttons.end())
        virtual_buttons.erase(it);
    
}

//--------------------------------- check if cursor is hovering entity

bool System::Scene::ListenForInteraction(std::shared_ptr<Entity> entity) {
    
    auto it = std::find_if(virtual_buttons.begin(), virtual_buttons.end(), [&](auto e) { return e.second == entity; });

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

bool System::Scene::IsPaused() {
    return m_paused;
}

//---------------------------------

void System::Scene::SetData(const char* key, std::any value) { 
    m_data.insert({ key, value }); 
}

//---------------------------------

const glm::vec2 System::Scene::GetWorldDimensions() { 
    return glm::vec2(m_worldWidth, m_worldHeight);
}

//---------------------------------

void System::Scene::SetPause(bool isPaused) {
    m_paused = isPaused;
}







