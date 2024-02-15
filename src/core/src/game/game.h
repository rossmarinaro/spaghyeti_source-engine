#pragma once

#include <set>

#include "./particles/particles.h"
#include "./maps/tilemap.h"
#include "./entities/entity.h"
#include "./physics/physics.h"

#include "../misc/time.h"
#include "../misc/utils.h"

#include "../camera/camera.h"
#include "../inputs/inputs.h"

#include "../../../../vendors/miniaudio.h"


//game base class
class Game {

    public:  

    
        const char* m_currentStage; 

        static inline std::string name = "";

        static inline int worldWidth, worldHeight = 0;

		static inline Time* time;
        static inline Camera* camera;
        static inline Physics* physics;
        static inline Text* text; 
        static inline Player* player = nullptr;

        std::map<const char*, std::any> data;

        template<typename T>
		inline T GetData(const char* key) const { return std::any_cast<T>(this->data.at(key)); }

		inline void SetData(const char* key, std::any value) { this->data.insert({key, value}); }

        Game() = default;
        virtual ~Game() = default;

        virtual void Preload() {}
        virtual void Run(Camera* camera) {}
        virtual void Update(Inputs* inputs, Camera* camera) {}

        static void Boot();
        static void Exit();
        static void UpdateFrame();

        template<typename T>
        static inline std::shared_ptr<T> CreateCustomSprite(const std::string &key, float x, float y) {
            auto e = std::make_shared<T>(key, x, y);

            entities.insert(e);

            return e;
        }

        template<typename T>
        static inline std::shared_ptr<T> CreatePlayer(
            const std::string &key, 
            float x, 
            float y, 
            float scale = 1.0f,
            bool hasBody = false, 
            glm::vec2 bodyDimensions = glm::vec2(1.0f, 1.0f),  
            glm::vec2 bodyOffset = glm::vec2(0.0f, 0.0f),
            float density = 0.0f,
            float friction = 0.0f,
            float restitution = 0.0f
        )
        {

            auto player = std::make_shared<T>(key, glm::vec2(x, y));

            if (scale)
                player->SetScale(scale); 

            if (hasBody)
            {

                auto body = physics->CreateDynamicBody(
                    glm::vec2(player->m_position.x, player->m_position.y), 
                    glm::vec2(bodyDimensions.x * scale, bodyDimensions.y * scale),
                    false,
                    2,
                    density,
                    friction,
                    restitution
                );
                
                player->bodies.push_back({ body, bodyOffset });
            }

            entities.insert(player);

            return player;
        }
        
        static std::shared_ptr<Sprite> CreateUI(const std::string &key, float x, float y, int frame = 0);
        static std::shared_ptr<Sprite> CreateSprite(const std::string &key, float x, float y, int frame = 0, float scale = 1.0f);
        static std::shared_ptr<Sprite> CreateTileSprite(const std::string &key, float x, float y, int frame);
        static std::shared_ptr<Text> CreateText(const std::string &content, float x, float y);
        static std::shared_ptr<Geometry> CreateGeom(float x, float y, float width, float height);
        static std::shared_ptr<Geometry> CreateGeom(float x, float y, const glm::vec2 &start, const glm::vec2 &end);
        
        static void DestroyEntity(std::shared_ptr<Entity> entity);
        static void DestroyUI();

        static void RemoveFromVector(std::vector<std::shared_ptr<Sprite>>& vector, std::shared_ptr<Sprite> sprite);
        static void RemoveFromVector(std::vector<std::shared_ptr<Text>>& vector, std::shared_ptr<Text> text);

        //render queues

        static inline std::vector<std::shared_ptr<Entity>> entities;

        std::vector<std::shared_ptr<Sprite>> virtual_buttons; 
        std::shared_ptr<Geometry> cursor = nullptr;


    private:

        static inline ma_device music;
        static inline bool gameState = false;
        

}; 


