#pragma once

#include <memory>
#include <fstream>
#include <iostream>


#include "../../../../vendors/imgui/imgui.h"
#include "../../../../vendors/imgui/imgui_impl_glfw.h"
#include "../../../../vendors/imgui/imgui_impl_opengl3.h"
#include "../../../../vendors/imgui/imgui_internal.h"
#include "../../../../vendors/imgui/imgui_stdlib.h"


class GUI { 

    public:

        static inline bool  m_running = true,
                            show_init = true,
                            show_quit = false,
                            show_toolbar = false,
                            show_grid = false;
    
        static void Launch();
        static void Render();
        static void Close();


    private:

        static inline ImGuiIO io;

        static void RenderGrid();
        static void RenderNodes();
        static void RenderDockSpace();
        static void RenderLogs();
        static void RenderAssets();
        static void RenderCamera();
        static void ShowMenu();
        static void ShowOptionsInit();
        static void ShowOptionsQuit();
        static void AlignForWidth(float width, float alignment = 0.5f);
};

