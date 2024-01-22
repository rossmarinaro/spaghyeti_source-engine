#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <any>
#include <fstream>
#include <memory>

#if USE_JSON == 1 
	#include "../../vendors/nlohmann/json.hpp"
	using json = nlohmann::json;
#endif

#include "../../vendors/glm/gtc/matrix_transform.hpp"  

#include "./manager.h"
#include "./inputs.h"
 
//base gameobject entity class

class Entity {

	public: 

		static inline int DEPTH = 0, g_ID = 0;

		int m_depth;
		float m_rotation, m_alpha; 
		bool m_isSpritesheet, m_flipX, m_flipY, m_active, m_renderable, m_alive;

		glm::vec3 m_tint; 
		glm::mat4 m_model; 
		glm::vec2 m_position, m_scale;

		std::string ID;

		inline void SetDepth(int depth) { m_depth = depth; }
		inline void SetAlpha(float alpha) { m_alpha = alpha; }
		inline void SetRotation(float rotation) { m_rotation = rotation; }
		inline void SetPosition(const glm::vec2 &position) { m_position = position; }
		inline void SetPosition(float x, float y) 
		{ 
			m_position.x = x;
			m_position.y = y; 
		}
		inline void SetTint(const glm::vec3 &tint) { m_tint = tint; }
		inline void SetFlipX(bool flipX) { m_flipX = flipX; };
		inline void SetFlipY(bool flipY) { m_flipY = flipY; };

		inline void SetFlip(bool flipX, bool flipY) 
		{ 
			m_flipX = flipX; 
			m_flipY = flipY; 
		};
		
		inline void SetScale(float scaleX, float scaleY = 1.0f) 
		{ 
			m_scale.x = scaleX;
			m_scale.y = scaleY != 1.0f ? 
				scaleY : scaleX; 
		}

		inline void SetEnabled(bool isEnabled)
		{
			m_active = isEnabled;
			m_renderable = isEnabled;
		}

		virtual void Render() = 0;
		virtual void Update(Inputs* inputs){};

		Entity() = default;
		Entity(const glm::vec2 &position):
			m_model(glm::mat4(1.0f)),
			m_position(position),
			m_scale(glm::vec2(1.0f)), 
			m_rotation(0.0f),  
			m_alpha(1.0f),
			m_tint(glm::vec3(1.0f, 1.0f, 1.0f)),
			m_active(true),
			m_alive(true),
			m_renderable(true),
			m_flipX(false),
			m_flipY(false), 
			m_depth(DEPTH + 1){ g_ID++; };

		virtual ~Entity() { g_ID--; };
};

//text

class Text : public Entity {

    public:

        GLTtext* buffer;
        std::string content;

        static void Init();
        static void ShutDown();

        void Render();
		void SetText(const std::string &content);
 
        Text (
			std::string content, 
			float x, 
			float y, 
			float scale = 1, 
			glm::vec3 tint = glm::vec3(1.0f)
		);
		
        Text() = default;
        ~Text();

};

//base sprite class

class Sprite : public Entity {

	public:  

		Shader &m_shader; 
			
		Graphics::Texture2D &m_texture; 

		std::string m_key;
		std::string m_type = "generic";
		std::map<std::string, std::pair<int, int>> m_anims;

		glm::vec2 m_velocity;

		//key val data to be assigned to sprite object

		std::map<std::string, std::any> data;

		//physics body

		struct {

			b2Body* self = nullptr;
			b2PolygonShape shape;
			glm::vec2 offset;
			int GetBodyDataType() const { return this->self->GetFixtureList()->GetBody()->GetUserData().pointer; }

		} m_body;

		int m_frames, m_currentFrame;

		const char* currentAnim = nullptr;

		bool 
			m_isSpritesheet, 
			m_contacting, 
			anim_yoyo = false;

		template<typename T>
		inline T GetData(const std::string &key) { return std::any_cast<T>(this->data.at(key)); }

		inline void SetData(const std::string &key, const std::any &value) { this->data.insert({key, value}); }
		inline void SetTexture(unsigned int id) { this->m_texture.ID = id; };
		inline void SetFrame(int frame) { m_currentFrame = frame; }
		inline void SetAnimation(const char* key) { currentAnim = key; }
		inline void StopAnimation() { currentAnim = nullptr; }

		inline void BeginContact() { m_contacting = true; }
		inline void EndContact() { m_contacting = false; }

		void ReadSpritesheetData();

		void SetVelocity(float velX, float velY);
		void SetVelocityX(float velX);
		void SetVelocityY(float velY);
		
		void Animate(const std::string &animKey, bool yoyo = false, int rate = 2);   
		void Render();

		Sprite(const std::string &key, const glm::vec2 &position = glm::vec2(0.0f, 0.0f), int frame = 0);
		Sprite(const std::string &key, const glm::vec2 &position, const char* type);
	
		~Sprite() { std::cout << "Sprite: " + this->m_key + " Destroyed.\n"; };


	private:

		unsigned int m_id;
		
		//internal spritesheet data

		std::vector<std::array<int, 4>> m_resourceData; 
};

//player

class Player : public Sprite { 

	public:

		virtual void Update() = 0;

		Player(const std::string &key, const glm::vec2 &position): 
			Sprite(key, position)
				{ std::cout << "Sprite: player spawned.\n"; };
		
		~Player() { std::cout << "Sprite: player destroyed.\n"; };

};



