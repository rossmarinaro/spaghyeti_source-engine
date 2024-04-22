#pragma once

#include "./context.h"
#include "./vendors/miniaudio.h"

namespace System {
    
    //container for gameplay instance
    class Scene {

        public: 

            std::string key;
            
            std::vector<std::shared_ptr<Entity>> entities;
            std::vector<std::shared_ptr<Entity>> UI;
            std::vector<std::shared_ptr<entity_behaviors::Behavior>> behaviors;

            Scene(const Process::Context& context): 
                context(context),
                key("Untitled" + std::to_string(s_ID)) { s_ID++; }

            Scene(const Process::Context& context, const std::string& key): 
                context(context),
                key(key) { s_ID++; }

            virtual ~Scene() { s_ID--; };

            virtual void Preload() {}
            virtual void Run() {}

            inline bool IsPaused() {
                return this->m_paused;
            }

        protected:
            
            ma_device music;     
            Process::Context context;   

            template<typename T>
            inline T GetData(const char* key) const { 
                return std::any_cast<T>(this->m_data.at(key));
            }

            inline void SetData(const char* key, std::any value) { 
                this->m_data.insert({key, value}); 
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

    };
}

