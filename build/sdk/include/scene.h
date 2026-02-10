#pragma once

#include "./context.h"
#include "./behaviors.h"
#include "./geometry.h"
namespace System {
    
    //container for gameplay instance
    class Scene {

	        std::map<std::string, std::any> m_globals;

        public:  std::vector<Graphics::Vertex> batchSprites_verts; 
            struct Spawn 
            {
                int type;
                float posX, posY, width, height, spawn_width, spawn_height, alpha;
                bool loop, can_create;
                std::string filename, index;
                std::vector<std::pair<std::string, bool>> behaviors_attached;
                Math::Vector3 tint;
                
                struct Body { 
                    int type, shape; 
                    bool exist, is_sensor; 
                    float xOff, yOff, w, h, density, friction, restitution; 
                } body;

                //check if spawn has behavior, attach and return false to verify next frame
                bool hasBehavior(const std::string& behaviorName);
            };

            std::string key;
 
            Math::Vector2* cameraTarget;

            std::unique_ptr<Geometry> vignette;
            
            std::vector<Spawn> spawns; 
            std::vector<std::shared_ptr<Entity>> entities;
            std::vector<std::shared_ptr<Entity>> UI;
            std::vector<std::shared_ptr<entity_behaviors::Behavior>> behaviors;
            std::vector<std::pair<bool, std::string>> virtual_buttons; 

            Scene(const Process::Context& context);
            Scene(const Process::Context& context, const std::string& key);

            virtual ~Scene() { s_ID--; };
            virtual void Preload() {}
            virtual void Update() {}
            virtual void Run(bool onSceneLoad) {}

            const bool IsPaused();
            const Math::Vector2 GetWorldDimensions();
            const Process::Context& GetContext();

            void SetPause(bool isPaused);

            //assign entity to react to input
            void SetInteractive(const std::shared_ptr<Entity>& entity, bool interactive = true);
            void SetWorldDimensions(float width, float height);
            void SetGlobal(const std::string& key, const std::any& value);

            //check if cursor is hovering entity
            const bool ListenForInteraction(const std::shared_ptr<Entity>& entity); 

            //get scene entity by name or key (passing true as an argument treats input string as "ID")
            template <typename T>
            const inline std::shared_ptr<T> GetEntity(const std::string& nameOrID, bool isID = false) 
            {  
                static_assert(std::is_base_of<Entity, T>::value, "T must be a value of type Entity!");

                if (!isID) //duplicate names return nullptr
                {
                    int i = 0;
                    for (const auto entity : entities)
                        if (entity->name == nameOrID)
                            i++;
                    if (i >= 2)
                        return nullptr;
                }
                
                const auto entity_it = std::find_if(entities.begin(), entities.end(), [&](const auto entity) { return isID ? entity->ID == nameOrID : entity->name == nameOrID; });
                const auto UI_it = std::find_if(UI.begin(), UI.end(), [&](const auto UI) { return isID ? UI->ID == nameOrID : UI->name == nameOrID; });

                if (entity_it != entities.end())
                    return std::static_pointer_cast<T>(*entity_it);

                if (UI_it != UI.end())
                    return std::static_pointer_cast<T>(*UI_it);

                return nullptr;
            }

            //get custom variables from extended scene
            template<typename T>
            inline const T GetGlobal(const std::string& key) { 
                if (m_globals.find(key) != m_globals.end())
                    return std::any_cast<T>(m_globals.at(key)); 
                return T();
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
