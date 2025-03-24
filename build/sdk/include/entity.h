#pragma once

#include <any>
#include <memory>
#include <atomic>
#include "./vendors/box2d/include/box2d/box2d.h"

#include "./manager.h"
#include "./inputs.h"


//base gameobject entity class

class Entity {

    //key val data to be assigned to entity object

	std::map<std::string, std::any> m_data;

	public: 

        enum { GENERIC, UI, SPRITE, GEOMETRY, TEXT, TILE };

		int depth, type;

		float rotation, alpha;  
		
        bool flipX, 
             flipY, 
             active, 
             renderable, 
             cull,
             alive;

		Math::Vector3 tint; 
		Math::Vector2 position, scale, scrollFactor;
		std::string ID, name;

		template<typename T>
		inline const T GetData(const std::string& key) { 
            if (m_data.find(key) != m_data.end())
                return std::any_cast<T>(m_data.at(key)); 
            return T();
        }

		inline void SetDepth(int depth) { this->depth = depth; }
		inline void SetAlpha(float alpha) { this->alpha = alpha; }
		inline void SetRotation(float rotation) { this->rotation = rotation; }
		inline void SetPosition(const Math::Vector2& position) { this->position = position; }
		inline void ClearTint() { tint = { 1.0f, 1.0f, 1.0f }; }
		inline void SetTint(const Math::Vector3& tint) { this->tint = tint; }
		inline void SetFlipX(bool flipX) { this->flipX = flipX; }
		inline void SetFlipY(bool flipY) { this->flipY = flipY; }
        inline void SetScrollFactor(const Math::Vector2& scrollFactor) { this->scrollFactor = scrollFactor; }
		 
		virtual void Render(float projWidth, float projHeight) {}
        virtual ~Entity() { s_count--; }
  
		Entity(int type);
		Entity(int type, float x, float y);
        
        const bool IsSprite();
        void SetData(const std::string& key, const std::any& value);
		void SetFlip(bool flipX, bool flipY);
		void SetScale(float scaleX, float scaleY = 1.0f);
		void SetEnabled(bool isEnabled);
        void SetPosition(float x, float y);

        static inline int s_depth = 0, s_count = 0;
        static inline Math::Vector2* s_cullPosition;

        static inline void SetCullPosition(Math::Vector2* position) { s_cullPosition = position; }

};


//----------------------------- container for shapes 


class Geometry : public Entity {

    public:

        Shader shader;
        Graphics::Texture2D texture;

		float width, height, radius;
        bool isStatic;

		inline void SetDrawStyle(int style) { m_drawStyle = style; } 
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

        enum { QUAD, CIRCLE };

    	int m_type;
        float m_thickness, m_drawStyle;
		
};



//----------------------------- text


class Text : public Entity {

    public:	
    
        enum { DEFAULT, FONT };

        //include 95 charecters
        static inline const uint32_t charsToIncludeInFontAtlas = 95; 

        static void Init();
        static void ShutDown();

        int textType;

        std::string content, font; 
       
        void Render(float projWidth, float projHeight);
		void SetText(const std::string& content);
        const Math::Vector2 GetTextDimensions();
 
       Text(const std::string& content, float x, float y, const std::string& font = "", float scale = 1, const Math::Vector3& tint = { 1.0f, 1.0f, 1.0f });
       ~Text();

    private:

        struct Vertex {
            Math::Vector3 position;
            Math::Vector4 color;
            Math::Vector2 texCoord;
        };

        //ASCII ' ' space
        static const uint32_t s_codePointOfFirstChar = 32,
                              s_fontAtlasWidth = 512,
                              s_fontAtlasHeight = 512;
        
        //VBO size in bytes - enough for 600000 vertices (100000 quads)
        static const size_t s_VBO_SIZE = 600000 * sizeof(Vertex); 

        const float m_fontSize = 64.0f; 

        std::vector<Vertex> m_vertices;
        
        //maximum pixel height of the text.
        uint32_t m_textHeight,
                 m_vertexIndex,
                 m_vaoID, m_vboID,
                 m_shaderProgramID,
                 m_fontTextureID;

        void* GetGLTPointer();

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

		std::vector<std::pair<b2Body*, Math::Vector4>> bodies; 

		inline const int GetBodyDataType() const { 
			for (const auto& body : bodies)
				return body.first->GetFixtureList()->GetBody()->GetUserData().pointer; 
			return 0;
		}

		inline void SetFrame(int frame) { currentFrame = frame; }
		inline void SetContact(bool isContact) { m_contacting = isContact; }
		inline const bool IsContacting() { return m_contacting; }
		inline const bool IsSpritesheet() { return m_isSpritesheet; } 
		inline const bool IsAnimComplete() { return m_animComplete; }

        void StopAnimation();
		void SetAnimation(const char* key, bool yoyo = false, int rate = 2, int repeat = -1);
		
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
        const std::shared_ptr<Sprite> Clone();

        Sprite(
            const std::string& key, 
            float x = 0.0f, 
            float y = 0.0f, 
            int frame = 0, 
            bool isTile = false
        );

		Sprite(const std::string& key, const Math::Vector2& position);
        Sprite(Sprite& sprite);
	   
	   ~Sprite();

	private:

		bool m_contacting, 
			 m_isSpritesheet = false, 
			 m_animComplete = true,
             m_isAnimPlaying = false,
			 m_anim_yoyo = false;

		Math::Vector2 m_velocity;

        struct Anim { 
            std::string key; 
            int rate, repeat; 
            bool yoyo, can_decrement, can_complete; 
        } m_currentAnim;
		
		//internal spritesheet data 

		std::vector<std::array<int, 6>> m_resourceData; 
};



