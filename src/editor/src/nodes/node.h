#pragma once

#include <type_traits>
#include <vector>
#include <string>

#include "../gui/gui.h"
#include "../components/component.h"
#include "../../../vendors/nlohmann/json.hpp"
#include "../../../../build/sdk/include/math.h"
#include "../../../../build/sdk/include/app.h"

using json = nlohmann::json;

namespace editor {

    //base node 
    class Node {

        private:

            struct NodeInfo {
                std::string ID;
                std::vector<std::shared_ptr<Node>> arr = nodes;
            };

            std::vector<std::shared_ptr<Component>> components; 

            static inline int s_MAX_NODES = 256; 
            static inline std::vector<std::string> s_names;
            static std::string CheckName(const std::string& name, std::vector<std::string>& arr = s_names);
            static bool CheckCanAddNode(bool init, const std::vector<std::shared_ptr<Node>>& arr);

        public:

            enum { SPRITE, TILEMAP, TEXT, AUDIO, EMPTY, GROUP, SPAWNER };

            int type, body_type, body_shape;

            bool created, 
                 active, 
                 show_options, 
                 isShadow,
                 isStroked;

            float scaleX, 
                  scaleY,
                  positionX, 
                  positionY,
                  rotation,
                  strokeWidth,
                  shadowDistanceX,
                  shadowDistanceY;

            std::string ID, name;

            Math::Vector3 strokeColor, shadowColor;

            struct StringContainer { std::string s = ""; };
            struct BoolContainer { bool b = false; };

            std::pair<std::string, std::pair<std::string, std::string>> shader;
            std::map<std::string, std::string> behaviors;
            
            Node(bool init, int type, const std::string& name = "Untitled");

            virtual ~Node() {}
            
            virtual void Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr);
            virtual void Render(float _positionX = 0.0f, float _positionY = 0.0f, float _rotation = 0.0f, float _scaleX = 1.0f, float _scaleY = 1.0f) {}
            virtual void Reset(const int component = Component::NONE) = 0;

            static inline std::vector<std::shared_ptr<Node>> nodes;

            template <typename T>
                static inline std::shared_ptr<T> Make(bool init = true, std::vector<std::shared_ptr<Node>>& arr = nodes) 
                {
                    static_assert(std::is_base_of<Node, T>::value, "T must be a value of type Node!");

                    if (!CheckCanAddNode(init, arr))
                        return nullptr;                            
                    
                    const auto node = std::make_shared<T>(init);
                    arr.emplace_back(node);
                    return node;
                }

            static void ClearAll();
            static void DeleteNode (const std::string& id, std::vector<std::shared_ptr<Node>>& arr = nodes);
            static void ApplyShader(std::shared_ptr<Node>& node, const std::string& name);
            static void LoadShader(std::shared_ptr<Node> node, const std::string& name, const std::string& vertPath, const std::string& fragPath);
            static std::shared_ptr<Node> ReadData(json& data, bool makeNode, void* scene, std::vector<std::shared_ptr<Node>>& arr = nodes);
            static json WriteData(const std::shared_ptr<Node>& node);
            static std::shared_ptr<Node> Get(const std::string& id, std::vector<std::shared_ptr<Node>>& arr = nodes);
            static const std::string GetType(int type);
            
            void AddComponent(int type, bool init = true); 
            void RemoveComponent(std::shared_ptr<Component>& component);
            
            const std::shared_ptr<Component> GetComponent(int type, const std::string& id);
            const bool HasComponent(int type);

        protected:

            struct Body {
                std::shared_ptr<Physics::Body> pb;
                float x, y, width, height;
            };

            bool m_init;
                        
            void SavePrefab(std::vector<std::shared_ptr<Node>>& arr);
            void ShowOptions(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr);

            static int ChangeName(ImGuiInputTextCallbackData* data);
    };

    //--------------------------------- sprite


    class SpriteNode : public Node {

        struct Frames { int x, y, width, height, factorX, factorY; };

        struct Anims { 
            
            std::string key = ""; 

            int start = 0, 
                end = 0, 
                rate = 2, 
                repeat = -1; 

            bool yoyo = false; 
        };

        public:

            int frame, 
                depth,
                currentFrame;

            bool framesApplied,       
                 filter_nearest,
                 flippedX,
                 flippedY,
                 lock_in_place,
                 cull,
                 make_UI;

            float U1,
                  V1,
                  U2,
                  V2,
                  alpha,
                  restitution,
                  density, 
                  friction,
                  scrollFactorX,
                  scrollFactorY;

            std::string key;
            Math::Vector3 tint;

            std::vector<Frames> frames; 
            std::vector<Anims> animations; 

            Anims anim_to_play_on_start; 

            std::shared_ptr<Sprite> spriteHandle;
            std::vector<BoolContainer> is_sensor, anim_yoyo;

            std::vector<int> frame_x,
                             frame_y,
                             body_pointer;

            std::vector<float> frame_width,
                               frame_height,
                               frame_fX,
                               frame_fY;

            std::vector<Body> bodies;  

            SpriteNode(bool init);
            ~SpriteNode();      

            void Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr) override;
            void Reset(const int component_type = Component::NONE) override;
            void Render(float _positionX, float _positionY, float _rotation, float _scaleX, float _scaleY) override;

            void RegisterFrames();
            void ApplyTexture(const std::string& key);
            void ApplyAnimation(const std::string& key);

            void CreateBody(
                float x = 0.0f, 
                float y = 0.0f, 
                float width = 0.0f, 
                float height = 0.0f,
                bool isSensor = false,
                int pointerType = -1
            );

        private:

            bool m_show_sprite_texture;
            std::string m_bodyType;
            Anims m_currentAnim;
            unsigned int m_currentTexture = NULL;

    };

    //--------------------------------- tilemap


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
            std::vector<Body> bodies;

            TilemapNode(bool init);
            ~TilemapNode();

            void Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr) override;
            void Reset(const int component_type = Component::NONE) override; 
            void Render(float _positionX, float _positionY, float _rotation, float _scaleX, float _scaleY) override;

            void ApplyTilemap(bool clearPrevious = true, bool renderReversed = false, bool isJSON = false);
            void CreateBody(float x = 0.0f, float y = 0.0f, float width = 0.0f, float height = 0.0f);
            void UpdateBody(int index);

        private: 

            bool m_layersApplied, m_mapApplied;
            void AddLayer();
            void ParseJSONData(const std::string& key, const std::string& path);


    };

    //--------------------------------- text, font


    class TextNode : public Node {

        public:

            int depth;
            float size, alpha, charOffsetX, charOffsetY;
            bool UIFlag; 

            Math::Vector3 tint;
            std::shared_ptr<Text> textHandle;
            std::string textBuf, currentFont;

            TextNode(bool init);
            ~TextNode();     

            void ChangeFont(const std::string& font = "");
            void Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr) override;
            void Reset(const int component_type = Component::NONE) override;
            void Render(float _positionX, float _positionY, float _rotation, float _scaleX, float _scaleY) override;
    };


    //--------------------------------- audio, sfx


    class AudioNode : public Node {

        public:
            
            float volume;
            bool loop;
            
            std::string audio_source_name;
          
            AudioNode(bool init);
            ~AudioNode();

            void Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr) override;
            void Reset(const int component_type = Component::NONE) override;
            void Load();

        private:

            Graphics::Texture2D m_audioTexture;
    };



    //--------------------------------- empty, geometry


    class EmptyNode : public Node {

        public:

            bool show_debug, debug_fill;

            int depth;
    
            float rectWidth, 
                  rectHeight,
                  radius,
                  line_weight;

            std::string currentShape;
            std::shared_ptr<Geometry> debugGraphic;

            EmptyNode(bool init);
            ~EmptyNode();      

            void CreateShape(const std::string& shape);

            void Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr) override;
            void Reset(const int component_type = Component::NONE) override;
            void Render(float _positionX, float _positionY, float _rotation, float _scaleX, float _scaleY) override;

    };


    //--------------------------------------------- group of nodes


    class GroupNode : public Node {

        public:

            std::vector<std::shared_ptr<Node>> _nodes;

            GroupNode(bool init);
            ~GroupNode();      

            void Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr) override;
            void Reset(const int component_type = Component::NONE) override;
            void Render(float _positionX, float _positionY, float _rotation, float _scaleX, float _scaleY) override;
    };


    //--------------------------------------------- spawn point


    class SpawnerNode : public Node {

        public:

            int typeOf, category;
            float width, height, spawnWidth, spawnHeight, alpha; 
            bool loop;
            System::Scene::Spawn::Body body;

            std::string animationKey, textureKey, behaviorKey;
            std::pair<std::string, std::string> spriteSheetKey;
            Math::Vector3 tint;

            std::shared_ptr<Geometry> rectHandle;

            SpawnerNode(bool init);
            ~SpawnerNode();      

            void Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr) override;
            void Reset(const int component_type = Component::NONE) override;
            void Render(float _positionX, float _positionY, float _rotation, float _scaleX, float _scaleY) override;
            void ApplyTexture(const std::string& asset);
            void CreateMarker();

        private:
         
            unsigned int m_currentTexture = NULL;
            std::string m_spawnType, m_bodyType, m_category;
            std::shared_ptr<Text> m_textHandle;
    };
}




