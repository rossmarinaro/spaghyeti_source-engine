#pragma once

#include "../../../build/include/renderer.h"
#include "./nodes/node.h"

namespace editor {

    //scene copy template
    class Scene {

        public:

			int worldWidth = 2000,
				worldHeight = 2000,
                GUIGridQuantity;

			float gravityX = 0.0f,
				  gravityY = 500.0f,
                  cameraZoom = 1,
                  GUIGridAlpha = 1.0f,
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
            std::vector<std::shared_ptr<editor::Node>> nodes;

            Scene() = default;

            template <typename T>
            static inline std::shared_ptr<T> MakeNode(Scene* scene) {

                auto node = std::make_shared<T>();

                scene->nodes.push_back(node);

                return node;

            }

    };
}