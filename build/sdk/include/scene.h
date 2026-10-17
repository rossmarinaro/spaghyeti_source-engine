#pragma once

#include "./context.h"
#include "./behaviors.h"

namespace System {
    
    //container for gameplay instance 
    class Scene {

	    std::map<std::string, std::any> m_globals;

        public: 

            struct TilemapLayer {
                std::string key, dataKey, dataPath, textureKey, shader;
                int ID, depth;
                float scrollFactorX, scrollFactorY, alpha; 
                Math::Vector3 tint;
            };

            struct Tilemap {
                std::string key, path;
                unsigned int width, height, tileWidth, tileHeight;
                std::vector<Scene::TilemapLayer> layers;
                std::vector<std::shared_ptr<Physics::Body>> bodies;
            };

            struct Spawn 
            {
                int type, depth;
                float posX, posY, width, height, spawn_width, spawn_height, alpha;
                bool loop, can_create, initialized, behavior_initialized;
                std::string filename, index, behavior_attached;
                Math::Vector3 tint;

                struct Body { 
                    bool exist; 
                    Physics::Body self;
                } body; 

                template <typename T>
                void InitBehavior(
                    const std::string& classKey, 
                    std::shared_ptr<Entity> entity,
                    Scene* scene
                ) 
                {
                    if (behavior_attached != classKey || behavior_initialized) 
                        return;

                    const auto behavior = scene->CreateBehavior<T>(entity);
                    behavior->Init();
                    behavior_initialized = true;
                }
            };

            std::string key;
            Math::Vector2* cameraTarget;
            std::unique_ptr<Geometry> vignette;

            std::vector<std::shared_ptr<Spawn>> spawns; 
            std::vector<std::shared_ptr<Entity>> entities, entitiesToRemove;
            std::vector<std::shared_ptr<Entity>> UI;             
            //std::vector<std::pair<const std::string, const std::string>> tilemaps; //data key, texture key
            std::vector<std::shared_ptr<entity_behaviors::Behavior>> behaviors;
            std::vector<std::pair<bool, std::string>> virtual_buttons; 

            Scene(const Process::Context& context);
            Scene(const Process::Context& context, const std::string& key);

            virtual ~Scene() { s_ID--; };
            virtual void Preload() {}
            virtual void Update() {} 
            virtual void Run(bool onSceneLoad) {}

            //create objects 

            std::shared_ptr<Entity> CreateEntity(int type = Entity::GENERIC, int layer = 1);
            std::shared_ptr<Sprite> CreateUISprite(const std::string& key, float x, float y, int frame = 0, float scale = 1.0f);
            std::shared_ptr<Sprite> CreateSprite(const std::string& key, float x, float y, int frame = 0, float scale = 1.0f, int layer = 1, bool isSpawn = false);
            std::shared_ptr<Text> CreateText(const std::string& content, float x, float y, const std::string& font = "", int layer = 2);
            std::shared_ptr<Geometry> CreateGeom(float x, float y, float width, float height, int layer = 1, bool isStatic = false, bool isSpawn = false);

            void CreateSpawn (
                int type, 
                const std::string& filename, 
                float x, 
                float y, 
                float width, 
                float height, 
                float spawn_width, 
                float spawn_height, 
                const Math::Vector3& tint, 
                int depth,
                float alpha, 
                bool loop, 
                const std::string& behaviorName,
                const Spawn::Body& body
            );

            //tilemap

            //construct tilemap on the fly from a loaded map json file and return map
            Tilemap CreateTilemapFromJSON(const std::string& key); 

            //create tilemap layer from csv or json
            TilemapLayer CreateTileLayer (
                int ID,
                const char* texture_key, 
                const char* data_key,
                uint32_t mapWidth, 
                uint32_t mapHeight, 
                uint32_t tileWidth, 
                uint32_t tileHeight,
                uint32_t depth,
                int index,
                float posX = 0.0f,
                float posY = 0.0f,
                float scrollFactorX = 1.0f,
                float scrollFactorY = 1.0f,
                const std::string& shaderKey = ""
            );            
            
            std::vector<std::shared_ptr<Sprite>> GetTileMapSprites(const std::string& key); 
            std::vector<std::shared_ptr<Sprite>> GetTileLayerSprites(const std::string& mapKey, int layerID); 

            void RemoveTilemap(const std::string& key);
            void RemoveTileLayer(const std::string& mapKey, int layerID);

            void DestroyEntity(std::shared_ptr<Entity> entity);
            void DestroyBehavior(const std::string& id);

            const bool IsPaused();
            const Math::Vector2 GetWorldDimensions();
            Process::Context* GetContext();

            void SetPause(bool isPaused);
            void SetDepthSort(bool depthSort);

            //assign entity to react to input
            void SetInteractive(const std::shared_ptr<Entity>& entity, bool interactive = true);
            void SetWorldDimensions(float width, float height);
            void SetGlobal(const std::string& key, const std::any& value);

            //check if cursor is hovering entity
            const bool ListenForInteraction(const std::shared_ptr<Entity>& entity); 

            const inline bool GetDepthSort() {
                return m_depthSort;
            } 

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

            //behaviors
            
            template <typename T>
            inline std::shared_ptr<T> CreateBehavior(const std::shared_ptr<Entity> entity) {
                static_assert(std::is_base_of<entity_behaviors::Behavior, T>::value, "T must be a value of type Behavior!");
                const auto behavior = std::make_shared<T>(entity);
                behaviors.emplace_back(behavior); 
                return behavior;
            }

            template <typename T>
            inline const std::shared_ptr<T> GetBehavior(const std::string& entityID = "") 
            { 
                const auto it = std::find_if(behaviors.begin(), behaviors.end(), [&](const auto behavior) { 
                    if (entityID.size())
                        return behavior->ID == entityID; //specific behavior

                    return behavior && dynamic_cast<T*>(behavior.get()) != nullptr; //first behavior matching
                    /* return behavior->key == typeid(T).name(); */
                });

                if (it != behaviors.end())
                    return std::dynamic_pointer_cast<T>(*it);
                    
                return nullptr;
            }
            
            static inline int s_spawn_count;
                
        private:
 
            int m_worldWidth, 
                m_worldHeight;

            bool m_paused, m_depthSort;
            Process::Context m_context;

            void Init(const std::string& key);

            static inline int s_ID = 0;
    };
}
