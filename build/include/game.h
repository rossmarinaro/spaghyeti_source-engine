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

        Game() = default;
        virtual ~Game() = default;

        virtual void Preload() {}
        virtual void Run(Inputs* inputs, Camera* camera, Physics* physics) {}
        virtual void Update(Inputs* inputs, Camera* camera, Physics* physics) {}

        static inline std::string name = "";

		static inline Time* time;
        static inline Camera* camera;
        static inline Physics* physics;
        static inline Text* text; 

        static void Boot();
        static void Exit();
        static void UpdateFrame();
        
        static std::shared_ptr<Sprite> CreateUI(const std::string &key, float x, float y, int frame = 0);
        static std::shared_ptr<Sprite> CreateSprite(const std::string &key, float x, float y, int frame = 0, float scale = 1.0f);
        static std::shared_ptr<Sprite> CreateTileSprite(const std::string &key, float x, float y, int frame);
        static std::shared_ptr<Text> CreateText(const std::string &content, float x, float y);
        static std::shared_ptr<Geometry> CreateGeom(float x, float y, float width, float height);
        static std::shared_ptr<Geometry> CreateGeom(float x, float y, const glm::vec2 &start, const glm::vec2 &end);
        
        template <typename T>
        static inline void CreateBehavior(std::shared_ptr<Entity> entity, Game* game) {

            auto behavior = std::make_shared<T>(entity);
            game->behaviors.push_back(behavior);
        }
                
        static void DestroyEntity(std::shared_ptr<Entity> entity);
        static void DestroyUI();

        static void RemoveFromVector(std::vector<std::shared_ptr<Sprite>>& vector, std::shared_ptr<Sprite> sprite);
        static void RemoveFromVector(std::vector<std::shared_ptr<Text>>& vector, std::shared_ptr<Text> text);

        //render queues

        static inline std::vector<std::shared_ptr<Entity>> entities;

        //ui
 
        std::vector<std::shared_ptr<Sprite>> virtual_buttons; 
        std::shared_ptr<Geometry> cursor = nullptr;

    protected:

        std::vector<std::shared_ptr<Behavior>> behaviors;
        int worldWidth, worldHeight = 0; 

        std::map<const char*, std::any> data;

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

        template <typename T>
        inline std::shared_ptr<T> GetBehavior(const std::string &key) {
            return std::dynamic_pointer_cast<T>(*std::find_if(behaviors.begin(), behaviors.end(), [&](std::shared_ptr<Behavior> b) { 
                return b->key == key; 
            }));
        }

        static inline ma_device music;
        static inline bool gameState = false;
        

}; 

