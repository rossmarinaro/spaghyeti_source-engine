#pragma once

#include <set> 

#include "./particles.h"
#include "./tilemap.h"
#include "./entity.h"
#include "./physics.h"
#include "./behaviors.h"
#include "./time.h"
#include "./utils.h"
#include "./camera.h"
#include "./scene.h"
#include "./inputs.h"

namespace System {
    
    //game base class
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
                T* scene = new T(this->m_context);
                this->scenes.push_back(scene);
            }
           
            //game lifecycle

            void Boot();
            void UpdateFrame();
            void Flush();
            void Exit();

            //create objects

            static std::shared_ptr<Entity> CreateEntity(const std::string& type = "empty", int layer = 1);
            static std::shared_ptr<Sprite> CreateUI(const std::string& key, float x, float y, int frame = 0);
            static std::shared_ptr<Sprite> CreateSprite(const std::string& key, float x, float y, int frame = 0, float scale = 1.0f, int layer = 1);
            static std::shared_ptr<Sprite> CreateTileSprite(const std::string& key, float x, float y, int frame);
            static std::shared_ptr<Text> CreateText(const std::string& content, float x, float y, int layer = 2);
            static std::shared_ptr<Geometry> CreateGeom(float x, float y, float width, float height, int layer = 1);
            static void DestroyEntity(std::shared_ptr<Entity> entity);
            
            static void StartScene(const std::string& key);
            static Scene* GetScene(const std::string& key = "");
            
            template <typename T>
            static inline void CreateBehavior(std::shared_ptr<Entity> entity, Scene* scene) {

                auto behavior = std::make_shared<T>(entity);
                scene->behaviors.push_back(behavior); 
            }

            template <typename T>
            static inline const std::shared_ptr<T> GetBehavior() 
            { 

                auto it = std::find_if(GetScene()->behaviors.begin(), GetScene()->behaviors.end(), 
                    [&](auto behavior) { return behavior->key == typeid(T).name(); });

                if (it != GetScene()->behaviors.end())
                    return std::dynamic_pointer_cast<T>(*it);
                    
                return nullptr;
            }

            //map manager

            static inline MapManager* maps;
            
        private:

            static inline std::vector<std::string> cachedScenes;
            std::atomic_bool m_gameState;
            Process::Context m_context;
            Scene* currentScene; 

            
    }; 

}


