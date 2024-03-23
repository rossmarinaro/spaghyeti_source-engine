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

#include "../../vendors/miniaudio.h"


//game base class
class Game {

    public:  

        const char* m_currentStage; 
        
        Time* time;
        Camera* camera;
        Physics* physics;
        Text* text; 

        Process::Context context;

        Game() = default;
        virtual ~Game() = default;

        virtual void Preload() {}
        virtual void Run() {}
        virtual void Update() {}

        static inline MapManager* maps;

        static void Boot();
        static void Exit();
        static void UpdateFrame();

        static std::shared_ptr<Sprite> CreateUI(const std::string& key, float x, float y, int frame = 0);
        static std::shared_ptr<Sprite> CreateSprite(const std::string& key, float x, float y, int frame = 0, float scale = 1.0f);
        static std::shared_ptr<Sprite> CreateTileSprite(const std::string& key, float x, float y, int frame);
        static std::shared_ptr<Text> CreateText(const std::string& content, float x, float y);
        static std::shared_ptr<Geometry> CreateGeom(float x, float y, float width, float height);
        static std::shared_ptr<Geometry> CreateGeom(float x, float y, const glm::vec2& start, const glm::vec2& end);
        
        template <typename T>
        static inline void CreateBehavior(std::shared_ptr<Entity> entity, Game* game) {

            auto behavior = std::make_shared<T>(entity);
            game->behaviorManager->behaviors.push_back(behavior); 
        }
                
        static void DestroyEntity(std::shared_ptr<Entity> entity);
        static void DestroyUI();

        //render queues

        std::vector<std::shared_ptr<Entity>> entities;

        //ui
 
        std::vector<std::shared_ptr<Sprite>> virtual_buttons; 
        std::shared_ptr<Geometry> cursor = nullptr;

    protected:

        int worldWidth, worldHeight = 0; 

        std::map<const char*, std::any> data;
        std::vector<std::shared_ptr<entity_behaviors::Behavior>> behaviors;

        template <typename T>
        inline std::shared_ptr<T> GetBehavior(const std::string& key) {
            return std::dynamic_pointer_cast<T>(*std::find_if(behaviors.begin(), behaviors.end(), [&](std::shared_ptr<entity_behaviors::Behavior> b) { 
                return b->key == key; 
            }));
        }

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

        inline bool UIListenForInput(int index) {
            return this->virtual_buttons[index]->m_tint != glm::vec3(1.0f);
        }

        static inline ma_device music;
        static inline bool gameState = false;

        void RemoveFromVector(std::vector<std::shared_ptr<Sprite>>& vector, std::shared_ptr<Sprite> sprite);
        void RemoveFromVector(std::vector<std::shared_ptr<Text>>& vector, std::shared_ptr<Text> text);
        

}; 

