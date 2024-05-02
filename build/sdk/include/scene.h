#pragma once

#include "./context.h"
#include "./vendors/miniaudio.h"

namespace System {
    
    //container for gameplay instance
    class Scene {

        public: 

            std::string key;

            std::unique_ptr<Sprite> vignette;
            
            std::vector<std::shared_ptr<Entity>> entities;
            std::vector<std::shared_ptr<Entity>> UI;
            std::vector<std::shared_ptr<entity_behaviors::Behavior>> behaviors;

            Scene(const Process::Context& context): 
                m_context(context)
                    { this->Init("Untitled" + std::to_string(s_ID)); }

            Scene(const Process::Context& context, const std::string& key): 
                m_context(context)
                    { this->Init(key); }

            virtual ~Scene() { s_ID--; };

            virtual void Preload() {}
            virtual void Run() {}

            inline bool IsPaused() {
                return this->m_paused;
            }

            inline const Process::Context& GetContext() { return this->m_context; }

            template <typename T>
            const inline std::shared_ptr<T> GetEntity(const std::string& key) 
            {

                auto entity_it = std::find_if(this->entities.begin(), this->entities.end(), [&](auto entity) { return entity->name == key; });
                auto UI_it = std::find_if(this->UI.begin(), this->UI.end(), [&](auto UI) { return UI->name == key; });

                if (entity_it != this->entities.end())
                    return std::static_pointer_cast<T>(*entity_it);

                if (UI_it != this->UI.end())
                    return std::static_pointer_cast<T>(*UI_it);
            }

        protected:
            
            ma_device m_music;     
            Process::Context m_context;   

            template<typename T>
            inline T GetData(const char* key) const { 
                return std::any_cast<T>(this->m_data.at(key));
            }

            inline void SetData(const char* key, std::any value) { 
                this->m_data.insert({ key, value }); 
            }

            inline const glm::vec2 GetWorldDimensions() { 
                return glm::vec2(this->m_worldWidth, this->m_worldHeight);
            }

            inline void SetWorldDimensions(float width, float height) { 
                this->m_worldWidth = width;
                this->m_worldHeight = height;
            }

            inline void SetPause(bool isPaused) {
                this->m_paused = isPaused;
            }

        private:

            static inline int s_ID = 0;

            int m_worldWidth = 0, 
                m_worldHeight = 0;

            bool m_paused;

            std::map<const char*, std::any> m_data;

            inline void Init(const std::string& key) {
                s_ID++;
                this->key = key;
            }

    };
}

