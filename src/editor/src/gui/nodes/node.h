#pragma once

#include <vector>
#include <string>

#include "../../../../../build/include/entity.h"

#include "../../components/component.h"
#include "../gui.h"

class Node {

    public:

        bool created, 
             m_active, 
             move_physics,
             show_options;

        float scaleX, 
              scaleY,
              positionX, 
              positionY,
              rotation,
              body_width, 
              body_height,  
              body_offsetX, 
              body_offsetY;

        std::string m_ID;

        std::string m_name;
        std::string m_type;

        std::vector<Component*> components;

        struct StringContainer { std::string s = ""; };

        Node(const std::string &id, std::string type, std::string name = "Untitled");

        virtual ~Node() = default;
        virtual void Render() = 0;

        static inline int count = 0, 
                          MAX_NODES = 100; 

        template <typename T>
            static inline T* Create (const std::string &id)
            {
 
                T* n = new T(id);
                nodes.push_back(n);

                return n;
            }

        static inline std::vector<Node*> nodes;

        static Node* MakeNode(const char* type);
        static void DeleteNode (Node* node);
        static void ClearAll();
        static int ChangeName(ImGuiInputTextCallbackData* data);

        Component* AddComponent(const char* type);
        void RemoveComponent(Component* component);
        const Component* GetComponent(const char* type);
        const bool HasComponent(const char* type);

};


//---------------------------------


class AudioNode : public Node {

    public:

        std::string audio_source_name;
        float volume;
        bool loop;

        AudioNode(const std::string &id);
        ~AudioNode();

        void Render() override;

    private:

        Graphics::Texture2D &audioTexture;
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

        typedef struct Frames { int x, y, width, height; };
        typedef struct Anims { std::string key; int start, end; };

        std::vector<Frames> frames; 
        std::map<std::string, Anims> animations; 

        std::shared_ptr<Sprite> spriteHandle;

        std::vector<int> frameBuf1; 
        std::vector<int> frameBuf2; 
        std::vector<int> frameBuf3; 
        std::vector<int> frameBuf4;

        std::vector<StringContainer> animBuf1; 
        std::vector<int> animBuf2; 
        std::vector<int> animBuf3; 
        std::vector<int> animBuf4; 

        SpriteNode(const std::string &id);
        ~SpriteNode();      

        void Render() override;

        void ApplyTexture(const std::pair<std::string, GLuint> &asset);
        void ApplyAnimation(const std::string &key, int start, int end);


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

        std::vector<int> spr_sheet_width; 
        std::vector<int> spr_sheet_height;
        std::vector<int> depth;

        std::vector<std::array<std::string, 3>> layers;
        std::vector<std::array<int, 4>> offset;

        TilemapNode(const std::string &id);
        ~TilemapNode();

        void Render() override;
        void ApplyTilemap();

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

        void Render() override;

};

//---------------------------------


class EmptyNode : public Node {

    public:

        bool show_debug, debug_fill;

        std::shared_ptr<Geometry> m_debugGraphic;

        EmptyNode(const std::string &id);
        ~EmptyNode();      

        void Render() override;

};