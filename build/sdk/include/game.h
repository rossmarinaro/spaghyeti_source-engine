#pragma once

#include "./particles.h"
#include "./entity.h"
#include "./physics.h"
#include "./time.h"
#include "./utils.h"
#include "./camera.h"
#include "./scene.h"
#include "./inputs.h"


namespace System {
    
    //game state and factory functions
    class Game {

        public:  

            Time* time;
            Text* text;
            Camera* camera;
            Physics* physics; 
            Inputs* inputs;

            std::vector<Scene*> scenes;

            Game();
            virtual ~Game() = default; 

            template <typename T>
            inline void LoadScene() {
                T* scene = new T(m_context);
                scenes.push_back(scene);
            }
           
            //game lifecycle

            void Boot();
            void UpdateFrame();
            void Exit();
 
            //create objects 

            static std::shared_ptr<Entity> CreateEntity(int type = Entity::GENERIC, int layer = 1);
            static std::shared_ptr<Sprite> CreateUISprite(const std::string& key, float x, float y, int frame = 0);
            static std::shared_ptr<Sprite> CreateSprite(const std::string& key, float x, float y, int frame = 0, float scale = 1.0f, int layer = 1, bool isSpawn = false);
            static std::shared_ptr<Sprite> CreateTileSprite(const std::string& key, float x, float y, int frame);
            static std::shared_ptr<Text> CreateText(const std::string& content, float x, float y, const std::string& font = "", int layer = 2);
            static std::shared_ptr<Geometry> CreateGeom(float x, float y, float width, float height, int layer = 1, bool isStatic = false, bool isSpawn = false);

            //create tilemap layer from csv or json
            static const std::string CreateTileLayer (
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
                float rotation = 0.0f,
                float scaleX = 1.0f,
                float scaleY = 1.0f,
                float scrollFactorX = 1.0f,
                float scrollFactorY = 1.0f,
                const std::string& shaderKey = ""
            );
        
            static void CreateSpawn(
                int type, 
                const std::string& filename, 
                float x, 
                float y, 
                float width, 
                float height, 
                float spawn_width, 
                float spawn_height, 
                const Math::Vector3& tint, 
                float alpha, 
                bool loop, 
                const std::string& behaviorName,
                const Scene::Spawn::Body& body
            );

            static void RemoveTileLayer(const std::string& ID);
            static void DestroyEntity(std::shared_ptr<Entity> entity);
            static void SetCullPosition(Math::Vector2* position);
            static void StartScene(const std::string& key, bool loadMap);
            static Scene* GetScene(const std::string& key = "");
            
            template <typename T>
            static inline std::shared_ptr<T> CreateBehavior(const std::shared_ptr<Entity>& entity, Scene* scene) {
                static_assert(std::is_base_of<entity_behaviors::Behavior, T>::value, "T must be a value of type Behavior!");
                const auto behavior = std::make_shared<T>(entity);
                scene->behaviors.emplace_back(behavior); 
                return behavior;
            }

            template <typename T>
            static inline const std::shared_ptr<T> GetBehavior() { 
                auto it = std::find_if(GetScene()->behaviors.begin(), GetScene()->behaviors.end(), 
                    [&](const auto behavior) { return behavior->key == typeid(T).name(); });

                if (it != GetScene()->behaviors.end())
                    return std::dynamic_pointer_cast<T>(*it);
                    
                return nullptr;
            }
            
        private:

            static inline int s_spawn_count;
            static inline std::vector<std::string> cachedScenes;
            std::atomic_bool m_gameState;
            Process::Context m_context;
            Scene* currentScene; 

            bool CheckEntityRenderable(std::shared_ptr<Entity>& entity);
            void RenderEntities();
            void RenderUI();
            void Reset(bool removeBehaviors = true);
    }; 

}


