#pragma once

#include <vector>
#include <string>


#include "../../components/component.h"
#include "../gui.h"

class Node {

    public:

        bool created, 
             m_active, 
             show_options;

        float scaleX, 
              scaleY,
              positionX, 
              positionY,
              rotation;

        std::string m_ID,
                    m_name,
                    m_type;

        std::vector<float> body_width, 
                           body_height,  
                           bodyX, 
                           bodyY;

        std::vector<std::shared_ptr<Component>> components;
        std::pair<std::string, std::pair<std::string, std::string>> shader;
        std::map<std::string, std::string> behaviors;

        Node(const std::string &id, std::string type, std::string name = "Untitled");

        virtual ~Node() = default;
        virtual void Render(std::shared_ptr<Node> node) = 0;
        virtual void Reset(const char* component = "") = 0;

        static inline int count = 0, 
                          MAX_NODES = 100; 

        static inline std::vector<std::shared_ptr<Node>> nodes;

        static const char* Assign();

        static inline std::shared_ptr<Node> GetNode(const std::string &id) {
            return *std::find_if(nodes.begin(), nodes.end(), [&](std::shared_ptr<Node> n) { 
                return n->m_ID == id; 
            });
        }

        template <typename T>
            static inline std::shared_ptr<T> MakeNode() {

                auto uuid = Assign();
            
                if (uuid) {
                 
                    auto node = std::make_shared<T>(uuid);
                    nodes.push_back(node);
                    return node;
                }
                
                return nullptr;

            }

        static void DeleteNode (std::shared_ptr<Node> node);
        static int ChangeName(ImGuiInputTextCallbackData* data);
        
        static void LoadShader(
            std::shared_ptr<Node> node, 
            const std::string &name, 
            const std::string &vertPath, 
            const std::string &fragPath
        );

        static void ClearAll();

        void AddComponent(const char* type, bool init = true);
        void RemoveComponent(std::shared_ptr<Component> component);

        const std::shared_ptr<Component> GetComponent(const std::string &type, const std::string &id);
        const bool HasComponent(const char* type);

        struct StringContainer { std::string s = ""; };
        struct BoolContainer { bool b = false; };

};

//---------------------------------


class SpriteNode : public Node {

    public:

        int 
            frame, 
            anim, 
            depth;

        bool framesApplied,       
             filter_nearest,
             do_yoyo;

        float restitution,
              density, 
              friction;

        typedef struct Frames { int x, y, width, height; };
        typedef struct Anims { std::string key; int start, end; };

        std::vector<Frames> frames; 
        std::map<std::string, Anims> animations; 

        std::shared_ptr<Sprite> spriteHandle;
        std::vector<StringContainer> animBuf1; 
        std::vector<BoolContainer> is_sensor;

        std::vector<int> frameBuf1,
                         frameBuf2,
                         frameBuf3,
                         frameBuf4,
                         animBuf2,
                         animBuf3,
                         animBuf4,
                         body_pointer;

        std::vector<std::pair<b2Body*, std::string>> bodies;

        SpriteNode(const std::string &id);
        ~SpriteNode();      

        void Render(std::shared_ptr<Node> node) override;
        void Reset(const char* component_type = "") override;

        void ApplyTexture(const std::pair<std::string, GLuint> &asset);
        void ApplyAnimation(const std::string &key, int start, int end);

        void CreateBody(
            const char* type, 
            float x = 0.0f, 
            float y = 0.0f, 
            float width = 0.0f, 
            float height = 0.0f,
            bool isSensor = false,
            int pointerType = 0
        );

    private:

        bool show_sprite_options,
             show_sprite_texture,
             show_sprite_atlas,
             show_sprite_uv,
             do_animate;

        GLuint currentTexture = NULL;

};

//---------------------------------


class TilemapNode : public Node {

    public:

        int 
            layer, 
            map_width, 
            map_height,
            tile_width, 
            tile_height;

        std::vector<int> spr_sheet_width,
                         spr_sheet_height,
                         depth;

        std::vector<std::array<std::string, 3>> layers;
        std::vector<std::array<int, 4>> offset;
        std::vector<b2Body*> bodies;

        TilemapNode(const std::string &id);
        ~TilemapNode();

        void Render(std::shared_ptr<Node> node) override;
        void Reset(const char* component_type = "") override;

        void ApplyTilemap();
        void CreateBody(
            float x = 0.0f, 
            float y = 0.0f, 
            float width = 0.0f, 
            float height = 0.0f
        );

    private: 

        bool layersApplied;


};

//---------------------------------


class TextNode : public Node {

    public:

        float size, alpha;

        glm::vec3 tint;

        std::shared_ptr<Text> textHandle;

        std::string textBuf;

        TextNode(const std::string &id);
        ~TextNode();     

        void Render(std::shared_ptr<Node> node) override;
        void Reset(const char* component_type = "") override;

};


//---------------------------------


class AudioNode : public Node {

    public:

        std::string audio_source_name;
        float volume;
        bool loop;

        AudioNode(const std::string &id);
        ~AudioNode();

        void Render(std::shared_ptr<Node> node) override;
        void Reset(const char* component_type = "") override;

    private:

        Graphics::Texture2D &audioTexture;
};



//---------------------------------


class EmptyNode : public Node {

    public:

        bool 
            show_debug, 
            debug_fill;
 
        float 
            rectWidth, 
            rectHeight,
            radius;

        std::string currentShape;
        std::shared_ptr<Geometry> m_debugGraphic;

        EmptyNode(const std::string &id);
        ~EmptyNode();      

        void CreateShape(const std::string &shape);

        void Render(std::shared_ptr<Node> node) override;
        void Reset(const char* component_type = "") override;

};