#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <any>
#include <fstream>
#include <memory>

#if USE_JSON == 1
	#include "../../../../../vendors/nlohmann/json.hpp"
	using json = nlohmann::json;
#endif

#include "../../../../../vendors/glm/gtc/matrix_transform.hpp"  

#include "../../resources/manager/manager.h"

#include "../../inputs/inputs.h"

//base gameobject entity class

class Entity {


	public: 

		static inline int DEPTH, 
						  g_ID = 0;

		int m_depth;

		float   
			m_rotation, 
			m_alpha; 
		bool 
			m_isSpritesheet, 
			m_flipX, 
			m_flipY, 
			m_active, 
			m_renderable, 
			m_alive;

		const char* type;

		glm::vec3 m_tint; 
		glm::mat4 m_model; 
		glm::vec2 m_position, m_scale;

		std::string ID;

		inline bool IsSprite() {
			return strcmp(this->type, "sprite") == 0 || 
				   strcmp(this->type, "tile") == 0;
		}

		inline void SetDepth(int depth) { this->m_depth = depth; }
		inline void SetAlpha(float alpha) { this->m_alpha = alpha; }
		inline void SetRotation(float rotation) { this->m_rotation = rotation; }
		inline void SetPosition(const glm::vec2 &position) { this->m_position = position; }

		inline void SetPosition(float x, float y) { 
			this->m_position.x = x;
			this->m_position.y = y; 
		}
		
		inline void SetTint(const glm::vec3 &tint) { this->m_tint = tint; }
		inline void SetFlipX(bool flipX) { this->m_flipX = flipX; };
		inline void SetFlipY(bool flipY) { this->m_flipY = flipY; };

		inline void SetFlip(bool flipX, bool flipY) { 
			this->m_flipX = flipX; 
			this->m_flipY = flipY; 
		}
		
		inline void SetScale(float scaleX, float scaleY = 1.0f) { 
			this->m_scale.x = scaleX;
			this->m_scale.y = scaleY != 1.0f ? 
				scaleY : scaleX; 
		}

		inline void SetEnabled(bool isEnabled) {
			this->m_active = isEnabled;
			this->m_renderable = isEnabled;
		}

		inline void StartFollow(Camera* camera, float offset) {
			camera->SetPosition(glm::vec2(-this->m_position.x + offset, camera->m_position.y));
		}
		
		virtual void Render() = 0;
		virtual void Update(Inputs* inputs, Camera* camera){};

		Entity() = default;
		Entity(const char* type, const glm::vec2 &position):
			type(type),
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

//----------------------------- container for shapes 


class Geometry : public Entity {

    public:

	  	Shader m_shader;

		float width, height;

		inline void SetColor(const glm::vec3 &color) { this->m_color = color; } 
		inline void SetDrawStyle(int style) { this->drawStyle = style; } 

		//quad

		Geometry(float x, float y, float width, float height);

		//line

		Geometry(float x, float y, const glm::vec2 &start, const glm::vec2 &end);

		~Geometry() = default;

		void Render();

	private:

		glm::vec3 m_color = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec2 start, end;

		int drawStyle = 0;

		const char* m_type;
		
		std::shared_ptr<Graphics::Primitive> primitive;
};

//----------------------------- text

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


//----------------------------- base sprite class

class Sprite : public Entity {

	public:  

		int m_frames, m_currentFrame;

		float velocityX, velocityY;

		const char* currentAnim = nullptr;

		Shader m_shader; 
			
		Graphics::Texture2D m_texture; 

		std::string m_key;
		std::map<std::string, std::pair<int, int>> m_anims;

		//key val data to be assigned to sprite object

		std::map<std::string, std::any> data;

		//physics body

		std::vector<std::pair<b2Body*, glm::vec2>> bodies;

		int GetBodyDataType() const { 
			for (const auto &body : this->bodies)
				return body.first->GetFixtureList()->GetBody()->GetUserData().pointer; 
		}

		template<typename T>
		inline T GetData(const std::string &key) { return std::any_cast<T>(this->data.at(key)); }

		inline void SetData(const std::string &key, const std::any &value) { this->data.insert({key, value}); }
		inline void SetTexture(unsigned int id) { this->m_texture.ID = id; }
		inline void SetFrame(int frame) { this->m_currentFrame = frame; }
		inline void SetAnimation(const char* key) { this->currentAnim = key; }
		inline void StopAnimation() { this->currentAnim = nullptr; }

		inline void BeginContact() { this->m_contacting = true; }
		inline void EndContact() { this->m_contacting = false; }
		inline bool IsContacting() { this->m_contacting; }
		inline bool IsSpritesheet() { return this->m_isSpritesheet; }

		void ReadSpritesheetData();

		void SetVelocity(float velX, float velY);
		void SetVelocityX(float velX);
		void SetVelocityY(float velY);
		
		void SetImpulse(float x, float y);
		void SetImpulseX(float x);
		void SetImpulseY(float y);
		
		void Animate(const std::string &animKey, bool yoyo = false, int rate = 2); 
		void Render();

		Sprite(const std::string &key, const glm::vec2 &position = glm::vec2(0.0f, 0.0f), int frame = 0);
		Sprite(const std::string &key, const glm::vec2 &position, const char* type);
	   
	   ~Sprite() { std::cout << "Sprite: " + this->m_key + " Destroyed.\n"; };


	private:

		bool m_contacting, 
			 m_isSpritesheet = false, 
			 anim_yoyo = false;

		glm::vec2 m_velocity;
		
		//internal spritesheet data

		std::vector<std::array<int, 4>> m_resourceData; 
};





