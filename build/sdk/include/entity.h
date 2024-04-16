#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <any>
#include <fstream>
#include <memory>

#if USE_JSON == 1 
	#include "./vendors/nlohmann/json.hpp"
	using json = nlohmann::json;
#endif

#include "./vendors/glm/gtc/matrix_transform.hpp"  

#include "./manager.h"
#include "./inputs.h"

//base gameobject entity class

class Entity {


	public: 

		static inline int DEPTH, 
						  g_count = 0;

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

		//key val data to be assigned to entity object

		std::map<std::string, std::any> data;

		template<typename T>
		inline T GetData(const std::string& key) { return std::any_cast<T>(this->data.at(key)); }

		inline void SetData(const std::string& key, const std::any& value) 
		{ 

			auto it = this->data.find(key);

			if (it != this->data.end())
				this->data.erase(it);

			this->data.insert({ key, value }); 
		}

		inline bool IsSprite() {
			return strcmp(this->type, "sprite") == 0 || 
				   strcmp(this->type, "tile") == 0;
		}

		inline void SetDepth(int depth) { this->m_depth = depth; }
		inline void SetAlpha(float alpha) { this->m_alpha = alpha; }
		inline void SetRotation(float rotation) { this->m_rotation = rotation; }
		inline void SetPosition(const glm::vec2& position) { this->m_position = position; }

		inline void SetPosition(float x, float y) { 
			this->m_position.x = x;
			this->m_position.y = y; 
		}
		
		inline void ClearTint() { this->m_tint = glm::vec3(1.0f); }
		inline void SetTint(const glm::vec3& tint) { this->m_tint = tint; }
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

			camera->targetX = this->m_position.x;
			camera->targetY = this->m_position.y;

			if (camera->InBounds())
				camera->m_position.x = (-this->m_position.x + offset) / 2;  
		}
		 
		virtual void Render() = 0;
 
		Entity() = default;
		Entity(const char* type, float x, float y):
			type(type),
			m_model(glm::mat4(1.0f)),
			m_position(glm::vec2(x, y)),
			m_scale(glm::vec2(1.0f)), 
			m_rotation(0.0f),  
			m_alpha(1.0f),
			m_tint(glm::vec3(1.0f, 1.0f, 1.0f)),
			m_active(true),
			m_alive(true),
			m_renderable(true),
			m_flipX(false),
			m_flipY(false), 
			m_depth(DEPTH + 1){ g_count++; };

		virtual ~Entity() { g_count--; };

};


//----------------------------- container for shapes 


class Geometry : public Entity {

    public:

        Shader m_shader;
        Graphics::Texture2D m_texture;

		float width, height, radius;

		inline void SetDrawStyle(int style) { this->drawStyle = style; } 

		inline void SetSize(float width, float height) { 
			this->width = width; 
			this->height = height;
		} 

		inline void SetSize(float radius) { this->radius = radius; } 

		//quad

		Geometry(float x, float y, float width, float height);

		~Geometry() = default;

		void Render();

	private:

		GLint drawStyle = GL_FILL;

		const char* m_type;
		
};


//----------------------------- text


class Text : public Entity {

    public:	

        std::string content; 

        static void Init();
        static void ShutDown();
 
        void Render();
		void SetText(const std::string& content);
 
        Text (const std::string& content, float x, float y, float scale = 1, glm::vec3 tint = glm::vec3(1.0f));
		
        Text() = default;
       ~Text();

    private:

        static inline GLTtext* buffer;
        GLTtext* handle;

};


//----------------------------- base sprite class


class Sprite : public Entity {

	public:  

        Shader m_shader;
        Graphics::Texture2D m_texture;

		int
			m_frames, 
			m_currentFrame, 
			num_contacts = 0;

		float velocityX, velocityY;

		std::string m_key;
		std::map<std::string, std::pair<int, int>> m_anims;

		//physics body

		std::vector<std::pair<b2Body*, glm::vec2>> bodies;

		inline int GetBodyDataType() const { 
			for (const auto &body : this->bodies)
				return body.first->GetFixtureList()->GetBody()->GetUserData().pointer; 
			return 0;
		}

		inline void SetFrame(int frame) { this->m_currentFrame = frame; }
		inline void SetAnimation(const char* key, bool yoyo = false, int rate = 2) { this->currentAnim = { key, { yoyo, rate } }; }
		inline void StopAnimation() { this->currentAnim = {}; }

		inline void SetContact(bool isContact) { this->m_contacting = isContact; }
		inline bool IsContacting() { return this->m_contacting; }
		inline bool IsSpritesheet() { return this->m_isSpritesheet; } 
		inline bool IsAnimComplete() { return this->m_animComplete; }

		void ReadSpritesheetData();
		void RemoveBodies();
		void SetTexture(const std::string& key);

		void SetVelocity(float velX, float velY);
		void SetVelocityX(float velX);
		void SetVelocityY(float velY);

		void SetImpulse(float x, float y);
		void SetImpulseX(float x);
		void SetImpulseY(float y);
		
		void Animate(const std::string& animKey, bool yoyo = false, int rate = 2); 
		void Render();

		Sprite(const std::string& key, float x, float y, const char* type);

		Sprite(
			const std::string& key, 
			float x = 0.0f, 
			float y = 0.0f, 
			int frame = 0, 
			bool isTile = false
		);
	   
	   ~Sprite();


	private:

		bool m_contacting, 
			 m_isSpritesheet = false, 
			 m_animComplete = true,
			 anim_yoyo = false;

		glm::vec2 m_velocity;

		std::pair<std::string, std::pair<bool, int>> currentAnim;
		
		//internal spritesheet data

		std::vector<std::array<int, 6>> m_resourceData; 
};



