#pragma once

#include "../../shared/renderer.h"
#include "./nodes/node.h"

namespace editor {

    //scene copy template
    class Scene {

        public:

            int worldWidth,
                worldHeight;

            float gravityX,
                  gravityY,
                  vignetteVisibility,
                  cameraZoom,
                  currentBoundsWidthBegin,
                  currentBoundsWidthEnd,
                  currentBoundsHeightBegin,
                  currentBoundsHeightEnd;

            bool shaders_applied,
                 animations_applied,
                 globals_applied, 
                 gravity_continuous,
                 gravity_sleeping;
    
            Math::Vector2 cameraPosition;
            Math::Vector4 cameraBackgroundColor;

            std::pair<std::string, Math::Vector2> cullTarget;

            std::vector<std::pair<std::string, std::string>> globals;
            std::vector<std::pair<std::string, std::string>> spritesheets;
            std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::pair<int, int>>>>> animations;
            std::vector<std::pair<std::string, std::pair<std::string, std::string>>> shaders;
            std::vector<std::shared_ptr<editor::Node>> nodes;
            std::vector<std::string> assets;

            Scene():
                worldWidth(2000),
                worldHeight(2000),
                gravityX(0.0f),
                gravityY(500.0f),
                vignetteVisibility(0.0f),
                cameraZoom(1),
                shaders_applied(false),
                animations_applied(false),
                globals_applied(false), 
                gravity_continuous(true),
                gravity_sleeping(true),
                cullTarget({ "", { 0.0f, 0.0f } })
            {}

            ~Scene() {
                nodes.clear();
                globals.clear();
            }

            template <typename T>
            static inline std::shared_ptr<T> CreateObject(Scene* scene) {
                const auto node = std::make_shared<T>(false);
                scene->nodes.emplace_back(node);
                return node;
            }

    };
}