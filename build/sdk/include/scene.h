#pragma once

#include "./context.h"
#include "./behaviors.h"

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

            const bool IsPaused();
            const Math::Vector2 GetWorldDimensions();
            const Process::Context& GetContext();

            void SetPause(bool isPaused);

            //assign entity to react to input
            void SetInteractive(std::shared_ptr<Entity> entity, bool interactive = true);
            void SetWorldDimensions(float width, float height);

            //check if cursor is hovering entity
            const bool ListenForInteraction(std::shared_ptr<Entity> entity); 

            template <typename T>
            const inline std::shared_ptr<T> GetEntity(const std::string& nameOrID, bool isID = false) 
            {  
                auto entity_it = std::find_if(entities.begin(), entities.end(), [&](auto entity) { return isID ? entity->ID == nameOrID : entity->name == nameOrID; });
                auto UI_it = std::find_if(UI.begin(), UI.end(), [&](auto UI) { return isID ? UI->ID == nameOrID : UI->name == nameOrID; });

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
   
            Process::Context m_context;

            void Init(const std::string& key);

    };
}
