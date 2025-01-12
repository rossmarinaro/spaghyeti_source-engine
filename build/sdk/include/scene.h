#pragma once

#include "./context.h"
#include "./behaviors.h"
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

            Scene(const Process::Context& context);
            Scene(const Process::Context& context, const std::string& key);

            virtual ~Scene() { s_ID--; };
            virtual void Preload() {}
            virtual void Run(bool onSceneLoad) {}

            bool IsPaused();
            void SetData(const char* key, std::any value);
            const glm::vec2 GetWorldDimensions();
            void SetPause(bool isPaused);
            const Process::Context& GetContext();

            //assign entity to react to input

            void SetInteractive(std::shared_ptr<Entity> entity, bool interactive = true);
            void SetWorldDimensions(float width, float height);

            //check if cursor is hovering entity

            bool ListenForInteraction(std::shared_ptr<Entity> entity);

            template<typename T>
            inline T GetData(const char* key) const { 
                return std::any_cast<T>(m_data.at(key));
            }

            template <typename T>
            const inline std::shared_ptr<T> GetEntity(const std::string& name) 
            {

                auto entity_it = std::find_if(entities.begin(), entities.end(), [&](auto entity) { return entity->name == name; });
                auto UI_it = std::find_if(UI.begin(), UI.end(), [&](auto UI) { return UI->name == name; });

                if (entity_it != entities.end())
                    return std::static_pointer_cast<T>(*entity_it);

                if (UI_it != UI.end())
                    return std::static_pointer_cast<T>(*UI_it);

                return nullptr;
            }

        private:
 
            static inline int s_ID = 0;

            int m_worldWidth, 
                m_worldHeight;

            bool m_paused;

            ma_device m_music;     
            Process::Context m_context;

            std::map<const char*, std::any> m_data;

            void Init(const std::string& key);

    };
}
