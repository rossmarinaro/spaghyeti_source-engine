#pragma once

#include "./context.h"
#include "../../vendors/miniaudio.h"

namespace System {
    
    //container for gameplay instance
    class Scene {

        public: 

            std::string key;
            
            std::vector<std::shared_ptr<Entity>> entities;
            std::vector<std::shared_ptr<entity_behaviors::Behavior>> behaviors;

            Scene(const Process::Context& context): 
                context(context),
                key("Untitled" + std::to_string(g_ID)) { g_ID++; }

            Scene(const Process::Context& context, const std::string& key): 
                context(context),
                key(key) { g_ID++; }

            virtual ~Scene() { g_ID--; };

            virtual void Preload() {}
            virtual void Run() {}
            virtual void Update() {}

        protected:
            
            ma_device music;     
            Process::Context context;   

            template<typename T>
            inline T GetData(const char* key) const { 
                return std::any_cast<T>(this->data.at(key));
            }

            inline void SetData(const char* key, std::any value) { 
                this->data.insert({key, value}); 
            }

            inline const glm::vec2 GetWorldDimensions() { 
                return glm::vec2(this->worldWidth, this->worldHeight);
            }

            inline void SetWorldDimensions(float width, float height) { 
                this->worldWidth = width;
                this->worldHeight = height;
            }

        private:

            static inline int g_ID = 0;

            int worldWidth = 0, 
                worldHeight = 0;

            std::map<const char*, std::any> data;

    };
}

