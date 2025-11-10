#pragma once

#include <any>
#include <memory>
#include <atomic>
#include "./manager.h"
#include "./inputs.h"
#include "./physics.h"

//base gameobject entity class
class Entity {

    //key val data to be assigned to entity object

	std::map<std::string, std::any> m_data;

	public: 

        Graphics::Shader shader;

        enum { GENERIC, UI, SPRITE, GEOMETRY, TEXT, TILE };

		int depth;

		float rotation, alpha, outlineWidth;  
		
        bool flipX, 
             flipY, 
             active, 
             renderable, 
             cull,
             alive,
             outlineEnabled,
             shadowEnabled;

		Math::Vector3 tint, outlineColor, shadowColor; 
		Math::Vector2 position, scale, scrollFactor;
		std::string ID, name;

		template<typename T>
		inline const T GetData(const std::string& key) { 
            if (m_data.find(key) != m_data.end())
                return std::any_cast<T>(m_data.at(key)); 
            return T();
        }

        inline const int GetType() { return m_type; }
		inline void SetDepth(int depth) { this->depth = depth; }
		inline void SetAlpha(float alpha) { this->alpha = alpha; }
		inline void SetRotation(float rotation) { this->rotation = rotation; }
		inline void SetPosition(const Math::Vector2& position) { this->position = position; }
		inline void ClearTint() { tint = { 1.0f, 1.0f, 1.0f }; }
		inline void SetTint(const Math::Vector3& tint) { this->tint = tint; }
		inline void SetFlipX(bool flipX) { this->flipX = flipX; }
		inline void SetFlipY(bool flipY) { this->flipY = flipY; }
        inline void SetName(const std::string& name) { this->name = name; }
        inline void SetCull(bool cull) { this->cull = cull; }
        inline void SetScrollFactor(const Math::Vector2& scrollFactor) { this->scrollFactor = scrollFactor; }
        inline void SetStatic(bool is_static) { m_isStatic = is_static; }
		 
		virtual void Render() {}
        virtual ~Entity() { s_count--; }
  
		Entity(int type);
		Entity(int type, float x, float y);
        
        const bool IsSprite();
        void SetShader(const std::string& key);
        void SetData(const std::string& key, const std::any& value);
		void SetFlip(bool flipX, bool flipY);
		void SetScale(float scaleX, float scaleY = 1.0f);
		void SetEnabled(bool isEnabled);
        void SetPosition(float x, float y);

        static const std::string GenerateID();

        static inline int s_depth = 0, 
                          s_count = 0, 
                          s_rendered = 0;

    protected:

        int m_type;
        bool m_isStatic;
};


//container for shapes 
class Geometry : public Entity {

    public:

        Graphics::Texture2D texture;

		float width, height, radius;

        inline void SetThickness(float thickness) { m_thickness = thickness; }
		inline void SetSize(float radius) { this->radius = radius; } 

		//quad

		Geometry(float x, float y, float width, float height);

        //TODO: circle

		~Geometry();

		void Render() override;
        void SetSize(float width, float height);
        void SetDrawStyle(int style);

	private:

        enum { QUAD, CIRCLE };

    	int m_type;
        float m_thickness, m_drawStyle;
		
};



//text (fonts, embedded fallback)
class Text : public Entity {

    public:	
    
        enum { DEFAULT, FONT };

        static void Init(); 
        static void ShutDown();

        int textType;

        float shadowOffsetX, 
              shadowOffsetY, 
              charoffsetX, 
              charoffsetY;

        std::string content, font; 
       
        void Render() override;
		void SetText(const std::string& content);
        void SetStroke(bool isOutlined, const Math::Vector3& color = { 1.0f, 1.0f, 1.0f }, float width = 1.0f);
        void SetShadow(bool isShadow, const Math::Vector3& color = { 1.0f, 1.0f, 1.0f }, float offsetX = 0.0f, float offsetY = 0.0f);
        void SetSlant(float offsetX, float offsetY);

        const Math::Vector2 GetTextDimensions();
 
       Text(
            const std::string& content, 
            float x, 
            float y, 
            const std::string& font = "", 
            float scale = 1.0f, 
            const Math::Vector3& tint = { 1.0f, 1.0f, 1.0f }
        );

       ~Text();

    private:

        struct Character {
            unsigned int TextureID; // ID handle of the glyph texture
            Math::Vector2   Size;      // Size of glyph
            Math::Vector2   Bearing;   // Offset from baseline to left/top of glyph
            unsigned int Advance;   // Horizontal offset to advance to next glyph
        };

        unsigned int m_VAO, m_VBO, m_pixel_height; 
        std::map<char, Character> m_chars;

        void* GetGLTPointer();

};


//sprite container
class Sprite : public Entity {

    struct Anim { 
        std::string key; 
        int rate, repeat; 
        bool yoyo, can_decrement, can_complete; 
    } m_currentAnim;

    std::vector<std::pair<std::shared_ptr<Physics::Body>, Math::Vector4>> m_bodies;  

	public:  

        Graphics::Texture2D texture;

		int frames, 
			currentFrame, 
			num_contacts = 0;

		float velocityX, velocityY;

		std::string key;
		std::map<const std::string, std::pair<int, int>> anims;

		inline const int GetBodyDataType() { 
			for (const auto& body : m_bodies)
				return body.first->pointer; 
			return 0;
		}

        inline const Anim& GetCurrentAnimation() { return m_currentAnim; }

		inline void SetContact(bool isContact) { m_contacting = isContact; }
        inline void SetAsUI(bool isUI) { m_type = isUI ? UI : SPRITE; }
        
		inline const bool IsContacting() { return m_contacting; }
		inline const bool IsSpritesheet() { return m_isSpritesheet; } 
		inline const bool IsAnimComplete() { return m_animComplete; }
        inline void SetFrame(int frame) { currentFrame = frame; }

        const bool CheckOverlap(const std::shared_ptr<Sprite>& spriteA, const std::shared_ptr<Sprite>& spriteB);
		
		void ReadSpritesheetData();
        void RemoveBodies(); 
        void AddBody(const std::shared_ptr<Physics::Body>& body, const Math::Vector4& offsets);

        std::shared_ptr<Physics::Body> GetBody(int index = 0);
        std::vector<std::pair<std::shared_ptr<Physics::Body>, Math::Vector4>>& GetBodies();  
        
		void SetTexture(const std::string& key);
        void SetStroke(bool isOutlined, const Math::Vector3& color = { 1.0f, 1.0f, 1.0f }, float width = 1.0f);
        
        void StopAnimation();
		void SetAnimation(const std::string& key, bool yoyo = false, int rate = 2, int repeat = -1);

		void SetVelocity(float velX, float velY);
		void SetVelocityX(float velX);
		void SetVelocityY(float velY);

        //physics bodies only, defaults to SetVelocity()
		void SetImpulse(float x, float y);

        //physics bodies only, defaults to SetVelocityX()
		void SetImpulseX(float x);

        //physics bodies only, defaults to SetVelocityY()
		void SetImpulseY(float y);

        std::shared_ptr<Sprite> Clone();
        
        Sprite(const std::string& key, float x = 0.0f, float y = 0.0f, bool isTile = false);
		Sprite(const std::string& key, const Math::Vector2& position);
        Sprite(const Sprite& sprite);
	   
	    ~Sprite();

       	void Render() override;

	private:

		bool m_contacting, 
			 m_isSpritesheet = false, 
			 m_animComplete = true,
             m_isAnimPlaying = false,
			 m_anim_yoyo = false;

		Math::Vector2 m_velocity;
		
		//internal spritesheet data 

		std::vector<std::array<int, 6>> m_resourceData; 
};



