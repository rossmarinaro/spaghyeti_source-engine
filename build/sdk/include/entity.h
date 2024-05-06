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

		static inline int s_depth = 0, s_count = 0;

		int depth;

		float rotation, alpha;  
		
        bool flipX, 
			 flipY, 
			 active, 
			 renderable, 
			 alive;

		const char* type;

		glm::vec3 tint; 

		glm::vec2 position, scale;

		std::string name;

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

		inline void SetDepth(int depth) { this->depth = depth; }
		inline void SetAlpha(float alpha) { this->alpha = alpha; }
		inline void SetRotation(float rotation) { this->rotation = rotation; }
		inline void SetPosition(const glm::vec2& position) { this->position = position; }

		inline void SetPosition(float x, float y) { 
			this->position.x = x;
			this->position.y = y; 
		}
		
		inline void ClearTint() { this->tint = glm::vec3(1.0f); }
		inline void SetTint(const glm::vec3& tint) { this->tint = tint; }
		inline void SetFlipX(bool flipX) { this->flipX = flipX; };
		inline void SetFlipY(bool flipY) { this->flipY = flipY; };

		inline void SetFlip(bool flipX, bool flipY) { 
			this->flipX = flipX; 
			this->flipY = flipY; 
		}
		
		inline void SetScale(float scaleX, float scaleY = 1.0f) { 
			this->scale.x = scaleX;
			this->scale.y = scaleY != 1.0f ? 
				scaleY : scaleX; 
		}

		inline void SetEnabled(bool isEnabled) {
			this->active = isEnabled;
			this->renderable = isEnabled;
		}

		inline void StartFollow(Camera* camera, float offset) {

			camera->targetX = this->position.x;
			camera->targetY = this->position.y;

			if (camera->InBounds())
				camera->position.x = (-this->position.x + offset) / 2;  
		}
		 
		virtual void Render() {}
 
		Entity(const char* type) {
            this->type = type;
            this->name = "Untitled_" + std::to_string(s_count);
            s_count++;
        }

		Entity(const char* type, float x, float y):
        	m_model(glm::mat4(1.0f))
		{ 
            this->type = type;
			this->position = glm::vec2(x, y);
			this->scale = glm::vec2(1.0f); 
			this->rotation = 0.0f;  
			this->alpha = 1.0f;
			this->tint = glm::vec3(1.0f, 1.0f, 1.0f);
			this->active = true;
			this->alive = true;
			this->renderable = true;
			this->flipX = false;
			this->flipY = false;
			this->depth = s_depth + 1;

            this->name = "Untitled_" + std::to_string(s_count);
            s_count++; 
        };

		virtual ~Entity() { s_count--; };

    protected:

        glm::mat4 m_model; 

};


//----------------------------- container for shapes 


class Geometry : public Entity {

    public:

        Shader shader;
        Graphics::Texture2D texture;

		float width, height, radius;

		inline void SetDrawStyle(int style) { this->m_drawStyle = style; } 

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

		GLint m_drawStyle = GL_FILL;

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
        std::array<GLfloat, 2> GetTextDimensions();
 
        Text (const std::string& content, float x, float y, float scale = 1, glm::vec3 tint = glm::vec3(1.0f));
		
        Text() = default;
       ~Text();

    private:

        static inline GLTtext* s_buffer;
        GLTtext* m_handle;

};



//----------------------------- base sprite class


class Sprite : public Entity {

	public:  

        Shader shader;
        Graphics::Texture2D texture;

		int frames, 
			currentFrame, 
			num_contacts = 0;

		float velocityX, velocityY;

		std::string key;
		std::map<std::string, std::pair<int, int>> anims;

		//physics body

		std::vector<std::pair<b2Body*, glm::vec4>> bodies; 

		inline int GetBodyDataType() const { 
			for (const auto &body : this->bodies)
				return body.first->GetFixtureList()->GetBody()->GetUserData().pointer; 
			return 0;
		}

		inline void SetFrame(int frame) { this->currentFrame = frame; }
		inline void SetAnimation(const char* key, bool yoyo = false, int rate = 2) { this->m_currentAnim = { key, { yoyo, rate } }; }
		inline void StopAnimation() { this->m_currentAnim = {}; }

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

		Sprite(const std::string& key, const glm::vec2& position);
		Sprite(const std::string& key, float x = 0.0f, float y = 0.0f, int frame = 0, bool isTile = false);
	   
	   ~Sprite();


	private:

		bool m_contacting, 
			 m_isSpritesheet = false, 
			 m_animComplete = true,
			 m_anim_yoyo = false;

		glm::vec2 m_velocity;

		std::pair<std::string, std::pair<bool, int>> m_currentAnim;
		
		//internal spritesheet data

		std::vector<std::array<int, 6>> m_resourceData; 
};



