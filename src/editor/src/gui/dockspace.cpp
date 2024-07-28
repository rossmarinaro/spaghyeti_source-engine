#include "./gui.h"
#include "../editor.h"
#include "../../../../build/sdk/include/app.h"

using namespace System;
using namespace editor;

static void HelpMarker(const char* desc)
{

    ImGui::TextDisabled("(?)");

    if (ImGui::BeginItemTooltip())
    {

        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);

        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}


//--------------------------


static void ShowDockingDisabledMessage()
{
    
    ImGuiIO& io = ImGui::GetIO();

    ImGui::Text("ERROR: Docking is not enabled! See Demo > Configuration.");
    ImGui::Text("Set io.ConfigFlags |= ImGuiConfigFlags_DockingEnable in your code, or ");

    ImGui::SameLine(0.0f, 0.0f);
    
    if (ImGui::SmallButton("click here to enable docking."))
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
} 



//--------------------------

bool enabled = false;

void GUI::RenderDockSpace()
{

    //tmp
    bool test = true;
    bool* p_open = &test;
    
    //end tmp

    // READ THIS !!!
    // TL;DR; this demo is more complicated than what most users you would normally use.
    // If we remove all options we are showcasing, this demo would become:
    //     void ShowExampleAppDockSpace()
    //     {
    //         ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    //     }
    // In most cases you should be able to just call DockSpaceOverViewport() and ignore all the code below!
    // In this specific demo, we are not using DockSpaceOverViewport() because:
    // - (1) we allow the host window to be floating/moveable instead of filling the viewport (when opt_fullscreen == false)
    // - (2) we allow the host window to have padding (when opt_padding == true)
    // - (3) we expose many flags and need a way to have them visible.
    // - (4) we have a local menu bar in the host window (vs. you could use BeginMainMenuBar() + DockSpaceOverViewport()
    //      in your code, but we don't here because we allow the window to be floating)

    static bool opt_fullscreen = true;
    static bool opt_padding = false;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.

    ImGuiWindowFlags window_flags = ImGuiConfigFlags_DockingEnable;//ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;//ImGuiDockNodeFlags_None;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    if (opt_fullscreen)
    {
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;


    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.

    if (!opt_padding)
       ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("GlobalDockspace", p_open, window_flags);


    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();

        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("GlobalDockspace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }
        else
            ShowDockingDisabledMessage();
    

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("::SpagYeti Engine::"))
        {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
            ImGui::MenuItem("Padding", NULL, &opt_padding);
            ImGui::Separator();

            if (ImGui::MenuItem("Flag: NoDockingOverCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingOverCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingOverCentralNode; }
            if (ImGui::MenuItem("Flag: NoDockingSplit",         "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingSplit) != 0))             { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingSplit; }
            if (ImGui::MenuItem("Flag: NoUndocking",            "", (dockspace_flags & ImGuiDockNodeFlags_NoUndocking) != 0))                { dockspace_flags ^= ImGuiDockNodeFlags_NoUndocking; }
            if (ImGui::MenuItem("Flag: NoResize",               "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))                   { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
            if (ImGui::MenuItem("Flag: AutoHideTabBar",         "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))             { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
            if (ImGui::MenuItem("Flag: PassthruCentralNode",    "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
            
            ImGui::Separator();

            if (ImGui::MenuItem("Close", NULL, false, p_open != NULL))
                *p_open = false;

            ImGui::EndMenu();
        }

        HelpMarker(
            "When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!" "\n"
            "- Drag from window title bar or their tab to dock/undock." "\n"
            "- Drag from window menu button (upper-left button) to undock an entire node (all windows)." "\n"
            "- Hold SHIFT to disable docking (if io.ConfigDockingWithShift == false, default)" "\n"
            "- Hold SHIFT to enable docking (if io.ConfigDockingWithShift == true)" "\n"
            "This demo app has nothing to do with enabling docking!" "\n\n"
            "This demo app only demonstrate the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window." "\n\n"
            "Read comments in ShowExampleAppDockSpace() for more details.");

        ImGui::EndMenuBar();
    }


    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    //ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;


    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;


    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    // DockSpace
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        
        ImGuiID dockspace_id = ImGui::GetID("GlobalDockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        static auto first_time = true;

        if (first_time)
        {
            first_time = false;

            ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            // split the dockspace into 2 nodes -- DockBuilderSplitNode takes in the following args in the following order
            //   window ID to split, direction, fraction (between 0 and 1), the final two setting let's us choose which id we want (which ever one we DON'T set as NULL, will be returned by the function)
            //                                                   
            auto dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id);          
            auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.24f, nullptr, &dockspace_id);
            auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.35f, nullptr, &dockspace_id);
            auto dock_id_up = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Up, 0.13f, nullptr, &dockspace_id);

            // we now dock our windows into the docking node we made above
            ImGui::DockBuilderDockWindow("Session", dock_id_right);
            ImGui::DockBuilderDockWindow("Workspace", dock_id_left);
            ImGui::DockBuilderDockWindow("Assets", dock_id_down);
            ImGui::DockBuilderDockWindow("Toolbar", dock_id_up);
            ImGui::DockBuilderFinish(dockspace_id);
        }
        
    }

    if (enabled)
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    enabled = true;
    
    ImGui::End();


    //-------------toolbar


    ImGui::Begin("Toolbar");

        if (ImGui::BeginMenu(("Project: " + Editor::events.s_currentProject).c_str()))
        {
            ShowMenu();
            ImGui::EndMenu();
        }

        ImGui::SameLine();

        ImGui::Text(("Scene: " + Editor::events.s_currentScene).c_str());

    ImGui::End();


    //--------------workspace


    ImGui::Begin("Workspace", p_open, window_flags);

        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {

            ImGuiID dockspace_id_ws = ImGui::GetID("Workspace");

            ImGui::DockSpace(dockspace_id_ws, ImVec2(0.0f, 0.0f), dockspace_flags);

            static auto first_time = true;

            if (first_time)
            {
                first_time = false;

                ImGui::DockBuilderRemoveNode(dockspace_id_ws); // clear any previous layout
                ImGui::DockBuilderAddNode(dockspace_id_ws, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspace_id_ws, viewport->Size);

                //split the dockspace into 2 nodes -- DockBuilderSplitNode takes in the following args in the following order
                //window ID to split, direction, fraction (between 0 and 1), the final two setting let's us choose which id we want (which ever one we DON'T set as NULL, will be returned by the function)
                                                                  
                auto dock_id_up_ws = ImGui::DockBuilderSplitNode(dockspace_id_ws, ImGuiDir_Up, 0.18f, nullptr, &dockspace_id_ws);
                auto dock_id_down_ws = ImGui::DockBuilderSplitNode(dockspace_id_ws, ImGuiDir_Down, 1.0f, nullptr, &dockspace_id_ws);

                //we now dock our windows into the docking node we made above
                ImGui::DockBuilderDockWindow("Settings", dock_id_up_ws);
                ImGui::DockBuilderDockWindow("Scene Heirarchy", dock_id_down_ws);
                ImGui::DockBuilderFinish(dockspace_id_ws);
            }
        }

        
        ImGui::Begin("Settings");
            ShowSettings();
        ImGui::End();

        ImGui::Begin("Scene Heirarchy");
            RenderNodes();
        ImGui::End();


    ImGui::End();


     //-------------- Session


    ImGui::Begin("Session", p_open, window_flags);

        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {

            ImGuiID dockspace_id_ws = ImGui::GetID("Session");

            ImGui::DockSpace(dockspace_id_ws, ImVec2(0.0f, 0.0f), dockspace_flags);

            static auto first_time = true;

            if (first_time)
            {
                first_time = false;

                ImGui::DockBuilderRemoveNode(dockspace_id_ws); // clear any previous layout
                ImGui::DockBuilderAddNode(dockspace_id_ws, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspace_id_ws, viewport->Size);

                //split the dockspace into 2 nodes -- DockBuilderSplitNode takes in the following args in the following order
                //window ID to split, direction, fraction (between 0 and 1), the final two setting let's us choose which id we want (which ever one we DON'T set as NULL, will be returned by the function)
                                                                  
                auto dock_id_up_ws = ImGui::DockBuilderSplitNode(dockspace_id_ws, ImGuiDir_Up, 0.48f, nullptr, &dockspace_id_ws);
                auto dock_id_down_ws = ImGui::DockBuilderSplitNode(dockspace_id_ws, ImGuiDir_Down, 1.8f, nullptr, &dockspace_id_ws);

                //we now dock our windows into the docking node we made above
                ImGui::DockBuilderDockWindow("Camera", dock_id_up_ws);
                ImGui::DockBuilderDockWindow("Logs", dock_id_down_ws);
                ImGui::DockBuilderFinish(dockspace_id_ws);
            }
        }

        ImGui::Begin("Camera");
            RenderCamera();
        ImGui::End();

        ImGui::Begin("Logs");

            RenderLogs();

            //show demo example
                //bool show = true;
                //ImGui::ShowDemoWindow(&show); 

        ImGui::End();


    ImGui::End();


    //--------------assets

    ImGui::Begin("Assets");
        RenderAssets();
    ImGui::End();

    
    ImGui::End();
}

