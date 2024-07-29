#pragma once

#include "../../../build/sdk/include/renderer.h"
#include "./nodes/node.h"

namespace editor {

    //scene copy template
    class Scene {

        public:

            ~Scene() {
                nodes.clear();
                globals.clear();
            }

            int worldWidth = 2000,
                worldHeight = 2000;

            float gravityX = 0.0f,
                  gravityY = 500.0f,
                  vignetteVisibility = 0.0f,
                  cameraZoom = 1,
                  currentBoundsWidthBegin,
                  currentBoundsWidthEnd,
                  currentBoundsHeightBegin,
                  currentBoundsHeightEnd;

            bool globals_applied = false, 
                gravity_continuous = true,
                gravity_sleeping = true;
    
            glm::vec2 cameraPosition;
            glm::vec4 cameraBackgroundColor;

            std::vector<std::pair<std::string, std::string>> globals;
            std::vector<std::pair<std::string, std::string>> spritesheets;
            std::vector<std::shared_ptr<editor::Node>> nodes;

            template <typename T>
            static inline std::shared_ptr<T> CreateObject(Scene* scene) 
            {

                auto node = std::make_shared<T>();

                scene->nodes.push_back(node);

                return node;

            }

    };
}