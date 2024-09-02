#pragma once

#include <vector>
#include <string>

#include "../gui/gui.h"
#include "../components/component.h"

namespace editor {

    //base node 
    class Node {

        public:

            bool created, 
                 active, 
                 show_options;

            float scaleX, 
                  scaleY,
                  positionX, 
                  positionY,
                  rotation;

            std::string ID,
                        name,
                        type;

            std::vector<float> body_width, 
                               body_height,  
                               bodyX, 
                               bodyY;

            std::pair<std::string, std::pair<std::string, std::string>> shader;
            std::map<std::string, std::string> behaviors;
            
            Node(const std::string& type, const std::string& name = "Untitled", std::vector<std::shared_ptr<Node>>& arr = nodes);
            Node(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr);

            virtual ~Node() {}
            
            virtual void Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr) = 0;
            virtual void Reset(const char* component = "") = 0;
            virtual void Render() {}

            static inline std::vector<std::shared_ptr<Node>> nodes;

            template <typename T>
            static inline std::shared_ptr<T> Make(std::vector<std::shared_ptr<Node>>& arr = nodes) 
            {

                auto node = std::make_shared<T>();

                arr != nodes ? arr.push_back(node) : nodes.push_back(node);

                return node;

            }

            static void ClearAll();
            static void DeleteNode (std::shared_ptr<Node>& node);
            static void ApplyShader(std::shared_ptr<Node> node, const std::string& name);
            static void LoadShader(std::shared_ptr<Node> node, const std::string& name, const std::string& vertPath, const std::string& fragPath);
            static std::shared_ptr<Node> ReadData(json& data, bool makeNode, void* scene, std::vector<std::shared_ptr<Node>>& arr = nodes, bool isChild = false);
            static json WriteData(std::shared_ptr<Node>& node);
            static std::shared_ptr<Node> Get(const std::string& id);
            
            void AddComponent(const char* type, bool init = true);
            void RemoveComponent(std::shared_ptr<Component>& component);
            
            const std::shared_ptr<Component> GetComponent(const std::string& type, const std::string& id);
            const bool HasComponent(const char* type);

            struct StringContainer { std::string s = ""; };
            struct BoolContainer { bool b = false; };

        protected:

            bool virtual_node = false;
            
            static int ChangeName(ImGuiInputTextCallbackData* data);
            
            void SavePrefab();
            void ShowOptions(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr);

        private:
        
            std::vector<std::shared_ptr<Component>> components; 

            static inline int s_MAX_NODES = 100; 

            static const char* s_Assign();
            
            static inline std::string CheckName(const std::string& key, const std::vector<std::shared_ptr<Node>>& arr, int count) 
            {
                if (std::find_if(arr.begin(), arr.end(), [&](auto node) { return node->name == key; }) != arr.end())
                    return key + "_" + std::to_string(count + 1); 
                    
                return key;
            }

    };

    //---------------------------------


    class SpriteNode : public Node {

        public:

            int frame, 
                anim, 
                depth;

            bool framesApplied,       
                 filter_nearest,
                 flippedX,
                 flippedY,
                 lock_in_place,
                 make_UI;

            float U1,
                  V1,
                  U2,
                  V2,
                  alpha,
                  restitution,
                  density, 
                  friction;

            std::string key;
            glm::vec3 tint;

            typedef struct Frames { int x, y, width, height, factorX, factorY; };
            typedef struct Anims { std::string key; int start, end; };

            std::vector<Frames> frames; 
            std::map<std::string, Anims> animations; 

            std::shared_ptr<Sprite> spriteHandle;
            std::vector<StringContainer> animBuf1; 
            std::vector<BoolContainer> is_sensor;

            std::vector<int> frameBuf1,
                             frameBuf2,
                             animBuf2,
                             animBuf3,
                             animBuf4,
                             body_pointer;

            std::vector<float> frameBuf3,
                               frameBuf4,
                               frameBuf5,
                               frameBuf6;

            std::vector<b2Body*> bodies; 

            SpriteNode();
            ~SpriteNode();      

            void Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr) override;
            void Reset(const char* component_type = "") override;
            void Render() override;

            void RegisterFrames();
            void ApplyTexture(const std::string& key);
            void ApplyAnimation(const std::string& key, int start, int end);

            void CreateBody(
                float x = 0.0f, 
                float y = 0.0f, 
                float width = 0.0f, 
                float height = 0.0f,
                bool isSensor = false,
                int pointerType = 0
            );

        private:

            bool m_show_sprite_texture;

            std::pair<std::string, std::pair<bool, int>> m_currentAnim;
            std::vector<BoolContainer> m_do_yoyo;

            GLuint m_currentTexture = NULL;

    };

    //---------------------------------


    class TilemapNode : public Node {

        public:

            int layer, 
                map_width, 
                map_height,
                tile_width, 
                tile_height;

            std::vector<int> spr_sheet_width,
                             spr_sheet_height,
                             depth;

            std::vector<std::array<std::string, 3>> layers;
            std::vector<std::array<int, 6>> offset;
            std::vector<b2Body*> bodies;

            TilemapNode();
            ~TilemapNode();

            void Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr) override;
            void Reset(const char* component_type = "") override; 

            void ApplyTilemap(bool clearPrevious = true, bool renderReversed = false, bool isJSON = false);
            void CreateBody(float x = 0.0f, float y = 0.0f, float width = 0.0f, float height = 0.0f);
            void UpdateBody(int index);

        private: 

            bool m_layersApplied, m_mapApplied;
            void AddLayer();


    };

    //---------------------------------


    class TextNode : public Node {

        public:

            int depth, isUI;

            float size, alpha;

            bool UIFlag;

            glm::vec3 tint;

            std::shared_ptr<Text> textHandle;

            std::string textBuf;

            TextNode();
            ~TextNode();     

            void Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr) override;
            void Reset(const char* component_type = "") override;
            void Render() override;

    };


    //---------------------------------


    class AudioNode : public Node {

        public:
            
            float volume;
            bool loop;
            
            std::string audio_source_name;
          
            AudioNode();
            ~AudioNode();

            void Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr) override;
            void Reset(const char* component_type = "") override;
            void Load();

        private:

            Graphics::Texture2D m_audioTexture;
    };



    //---------------------------------


    class EmptyNode : public Node {

        public:

            bool show_debug, debug_fill;

            int depth;
    
            float rectWidth, 
                  rectHeight,
                  radius,
                  line_weight;

            std::string currentShape;
            std::shared_ptr<Geometry> m_debugGraphic;

            EmptyNode();
            ~EmptyNode();      

            void CreateShape(const std::string& shape);

            void Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr) override;
            void Reset(const char* component_type = "") override;
            void Render() override;

    };


    //---------------------------------------------


    class GroupNode : public Node {

        public:

            std::vector<std::shared_ptr<Node>> _nodes;

            GroupNode();
            ~GroupNode();      

            void Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr) override;
            void Reset(const char* component_type = "") override;
            void Render() override;

    };
}




