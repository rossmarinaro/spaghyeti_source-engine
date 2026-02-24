#pragma once

#include <memory>
#include <fstream>

#include "../../../vendors/imgui/imgui.h"
#include "../../../vendors/imgui/imgui_impl_glfw.h"
#include "../../../vendors/imgui/imgui_impl_opengl3.h"
#include "../../../vendors/imgui/imgui_internal.h"
#include "../../../vendors/imgui/imgui_stdlib.h"

#include "../../../../build/sdk/include/entity.h"

namespace editor {

    class GUI { 

        public:

            bool running,
                 show_init,
                 show_quit,
                 show_grid;

            float grid_quantity;

            std::unique_ptr<Geometry> grid;
            
            GUI();
           ~GUI();

            void Render();
            
            static inline GUI* Get() {
                return s_self;
            }

        private:

            unsigned int s_currentTexture = NULL;
 
            void RenderNodes();
            void RenderDockSpace();
            void RenderLogs();
            void RenderAssets();
            void ShowSettings();
            void ShowMenu();
            void ShowViewport();
            void ShowOptionsInit();
            void ShowOptionsQuit();
            void ShowOptionsSave(bool quit);
            void AlignForWidth(float width, float alignment = 0.5f);
            void displayThumbnail(const std::vector<std::pair<std::string, unsigned int>>& vec);

            static inline GUI* s_self;
            
            static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
    };

}