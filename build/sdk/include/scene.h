#pragma once

#include "./context.h"
#include "./vendors/miniaudio.h"

namespace System {
    
    //container for gameplay instance
    class Scene {

        public: 

            std::string key;

            std::unique_ptr<Geometry> vignette;
            
            std::vector<std::shared_ptr<Entity>> entities;
            std::vector<std::shared_ptr<Entity>> UI;
            std::vector<std::shared_ptr<entity_behaviors::Behavior>> behaviors;
            std::vector<std::pair<int, std::shared_ptr<Entity>>> virtual_buttons; 

            Scene(const Process::Context& context): 
                m_context(context)
                    { Init("Untitled" + std::to_string(s_ID)); }

            Scene(const Process::Context& context, const std::string& key): 
                m_context(context)
                    { Init(key); }

            virtual ~Scene() { s_ID--; };
            virtual void Preload() {}
            virtual void Run(bool onSceneLoad) {}

            inline bool IsPaused() {
                return m_paused;
            }

            template<typename T>
            inline T GetData(const char* key) const { 
                return std::any_cast<T>(m_data.at(key));
            }

            inline void SetData(const char* key, std::any value) { 
                m_data.insert({ key, value }); 
            }

            inline const glm::vec2 GetWorldDimensions() { 
                return glm::vec2(m_worldWidth, m_worldHeight);
            }

            inline void SetWorldDimensions(float width, float height) { 
                m_worldWidth = width;
                m_worldHeight = height;
            }

            inline void SetPause(bool isPaused) {
                m_paused = isPaused;
            }

            //assign entity to react to input

            inline void SetInteractive(std::shared_ptr<Entity> entity, bool interactive = true) 
            {

                auto it = std::find_if(virtual_buttons.begin(), virtual_buttons.end(), [&](auto e) { return e.second == entity; });

                if (interactive && it == virtual_buttons.end()) {
                    virtual_buttons.push_back({ 0, entity });
                    return;
                }

                if (it != virtual_buttons.end())
                    virtual_buttons.erase(it);
                
            }

            //check if cursor is hovering entity

            inline bool ListenForInteraction(std::shared_ptr<Entity> entity) 
            {
                auto it = std::find_if(virtual_buttons.begin(), virtual_buttons.end(), [&](auto e) { return e.second == entity; });

                if (it != virtual_buttons.end()) {
                    auto element = *it;
                    return element.first;
                }

                return false;
            }

            inline const Process::Context& GetContext() { return m_context; }

            template <typename T>
            const inline std::shared_ptr<T> GetEntity(const std::string& key) 
            {

                auto entity_it = std::find_if(entities.begin(), entities.end(), [&](auto entity) { return entity->name == key; });
                auto UI_it = std::find_if(UI.begin(), UI.end(), [&](auto UI) { return UI->name == key; });

                if (entity_it != entities.end())
                    return std::static_pointer_cast<T>(*entity_it);

                if (UI_it != UI.end())
                    return std::static_pointer_cast<T>(*UI_it);

                return nullptr;
            }

        private:
 
            static inline int s_ID = 0;

            int m_worldWidth = 0, 
                m_worldHeight = 0;

            bool m_paused;

            ma_device m_music;     
            Process::Context m_context;

            std::map<const char*, std::any> m_data;

            inline void Init(const std::string& key) {
                s_ID++;
                this->key = key;
            }

    };
}
