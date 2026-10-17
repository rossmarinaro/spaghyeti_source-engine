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

            static void SetCullPosition(Math::Vector2* position);
            static void StartScene(const std::string& key, bool loadMap);
            static Scene* GetScene(const std::string& key = "");

        private:

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


