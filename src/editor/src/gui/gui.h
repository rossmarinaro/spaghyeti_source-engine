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

            static inline bool s_running = true,
                               s_show_init = true,
                               s_show_quit = false,
                               s_show_grid = false;
        
            static void Launch();
            static void Render();
            static void Close();

            static void RenderShaderOptions(const std::string& nodeId);
            static void RenderScriptOptions(const std::string& nodeId);

            static inline std::unique_ptr<Geometry> s_grid;
            static inline std::shared_ptr<Geometry> s_cursor;
            
            static inline float s_grid_quantity = 20.0f;

        private:

            static void RenderNodes();
            static void RenderDockSpace();
            static void RenderLogs();
            static void RenderAssets();
            static void ShowSettings();
            static void ShowMenu();
            static void ShowViewport();
            static void ShowOptionsInit();
            static void ShowOptionsQuit();
            static void ShowOptionsSave(bool quit);
            static void AlignForWidth(float width, float alignment = 0.5f);
            static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);

            static inline GLuint s_currentTexture = NULL;
    };

}