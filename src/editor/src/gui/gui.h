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

            bool running = true,
                 show_init = true,
                 show_quit = false,
                 show_grid = false;

            std::unique_ptr<Geometry> grid;
            std::shared_ptr<Geometry> cursor;
            
            float grid_quantity;

            GUI() = default;
           ~GUI();

            void Render();
            void RenderShaderOptions(const std::string& nodeId);
            void RenderScriptOptions(const std::string& nodeId);

            static void Launch(GUI* gui);
            
            static inline GUI* Get() {
                return s_self;
            }

        private:
 
            GLuint s_currentTexture = NULL;

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

            static inline GUI* s_self;
            
            static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
    };

}