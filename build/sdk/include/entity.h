#pragma once

#include <any>
#include <memory>
#include <atomic>

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

		std::string ID, name;

		//key val data to be assigned to entity object

		std::map<std::string, std::any> data;

		template<typename T>
		inline T GetData(const std::string& key) 
        { 
            if (this->data.find(key) != this->data.end())
                return std::any_cast<T>(this->data.at(key)); 

            return T();
        }

		inline void SetDepth(int depth) { this->depth = depth; }
		inline void SetAlpha(float alpha) { this->alpha = alpha; }
		inline void SetRotation(float rotation) { this->rotation = rotation; }
		inline void SetPosition(const glm::vec2& position) { this->position = position; }
		inline void ClearTint() { tint = glm::vec3(1.0f); }
		inline void SetTint(const glm::vec3& tint) { this->tint = tint; }
		inline void SetFlipX(bool flipX) { this->flipX = flipX; }
		inline void SetFlipY(bool flipY) { this->flipY = flipY; }
        inline void SetScrollFactor(const glm::vec2& factor) { m_scrollFactor = factor; }
     
        inline bool IsSprite() {
			return strcmp(this->type, "sprite") == 0 || 
				   strcmp(this->type, "tile") == 0;
		}

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

        inline void SetPosition(float x, float y) { 
			this->position.x = x;
			this->position.y = y; 
		}
		 
		virtual void Render(float projWidth, float projHeight) {}
        virtual ~Entity() { s_count--; }
 
		Entity(const char* type);
		Entity(const char* type, float x, float y);
        
        void Cull(const glm::vec2& targetPosition);
        void SetData(const std::string& key, const std::any& value);

    protected:

        glm::vec2 m_scrollFactor;

};


//----------------------------- container for shapes 


class Geometry : public Entity {

    public:

        Shader shader;
        Graphics::Texture2D texture;

		float width, height, radius;
        bool isStatic;

		inline void SetDrawStyle(GLint style) { m_drawStyle = style; } 
        inline void SetThickness(float thickness) { m_thickness = thickness; } 

		inline void SetSize(float width, float height) { 
			this->width = width; 
			this->height = height;
		} 

		inline void SetSize(float radius) { this->radius = radius; } 

		//quad

		Geometry(float x, float y, float width, float height);

        //TODO: circle

		~Geometry() = default;

		void Render(float projWidth, float projHeight);

	private:

        float m_thickness;
        
        GLint m_drawStyle;

		const char* m_type;
		
};


//----------------------------- text


class Text : public Entity {

    public:	

        std::string content; 

        static void Init();
        static void ShutDown();
 
        void Render(float projWidth, float projHeight);
		void SetText(const std::string& content);
        const glm::vec2 GetTextDimensions();
 
        Text (const std::string& content, float x, float y, float scale = 1, glm::vec3 tint = glm::vec3(1.0f));
		
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
			for (const auto &body : bodies)
				return body.first->GetFixtureList()->GetBody()->GetUserData().pointer; 
			return 0;
		}

		inline void SetFrame(int frame) { currentFrame = frame; }
		inline void SetAnimation(const char* key, bool yoyo = false, int rate = 2) { m_currentAnim = { key, { yoyo, rate } }; }
		inline void StopAnimation() { m_currentAnim = {}; }
		inline void SetContact(bool isContact) { m_contacting = isContact; }
		inline bool IsContacting() { return m_contacting; }
		inline bool IsSpritesheet() { return m_isSpritesheet; } 
		inline bool IsAnimComplete() { return m_animComplete; }

		void ReadSpritesheetData();
		void RemoveBodies();
		void SetTexture(const std::string& key);

		void SetVelocity(float velX, float velY);
		void SetVelocityX(float velX);
		void SetVelocityY(float velY);

		void SetImpulse(float x, float y);
		void SetImpulseX(float x);
		void SetImpulseY(float y);
		void Render(float projWidth, float projHeight);
        std::shared_ptr<Sprite> Clone();

		Sprite(const std::string& key, const glm::vec2& position);
		Sprite(const std::string& key, float x = 0.0f, float y = 0.0f, int frame = 0, bool isTile = false);
        Sprite(Sprite& sprite);
	   
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



