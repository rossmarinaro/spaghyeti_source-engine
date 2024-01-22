#include "../../../../build/include/app.h"
#include "../editor.h"
#include "./gui.h"


void GUI::ShowMenu()
{

    ImGui::MenuItem("File Select", NULL, false, false); ImGui::SameLine(); 
    
    ImGui::Text(("Platform: " + Editor::platform).c_str());

    if (ImGui::MenuItem("Build / Run")) 
        Editor::events.BuildAndRun();

    if (ImGui::MenuItem("New")) 
        Editor::events.NewProject();

    if (ImGui::MenuItem("Open", "Ctrl+O")) 
        Editor::events.OpenProject();

    // if (ImGui::BeginMenu("Open Recent"))
    // {
    //     ImGui::MenuItem("fish_hat.c");
    //     ImGui::MenuItem("fish_hat.inl");
    //     ImGui::MenuItem("fish_hat.h");

    //     if (ImGui::BeginMenu("More.."))
    //     {
    //         ImGui::MenuItem("Hello");

    //         if (ImGui::BeginMenu("Recurse.."))
    //         {
    //             ShowMenu();
    //             ImGui::EndMenu();
    //         }
    //         ImGui::EndMenu();
    //     }
    //     ImGui::EndMenu();
    // }

    if (ImGui::MenuItem("Save", "Ctrl+S")) 
        Editor::events.SaveProject();

    if (ImGui::MenuItem("Save As..")) 
        Editor::events.SaveProject(true);

    ImGui::Separator();

    if (ImGui::MenuItem("Quit", "Alt+F4")) 
        show_quit = true;


    if (ImGui::BeginMenu("Options"))
    {

        if (ImGui::BeginMenu("Switch Platform"))
        {
            if (ImGui::MenuItem("Windows"))
                Editor::platform = "Windows";

            if (ImGui::MenuItem("WebGL"))
                Editor::platform = "WebGL";

            ImGui::EndMenu();
        } 

        if (ImGui::BeginMenu("Change Theme"))
        {
            if (ImGui::MenuItem("classic"))
                ImGui::StyleColorsClassic();

            if (ImGui::MenuItem("light"))
                ImGui::StyleColorsLight();

            if (ImGui::MenuItem("dark"))
                ImGui::StyleColorsDark();

            ImGui::EndMenu();
        } 

        // ImGui::Checkbox("Options", &show_demo_window);      // Edit bools storing our window open/close state
        // static bool enabled = true;
        // ImGui::MenuItem("Enabled", "", &enabled);
        // ImGui::BeginChild("child", ImVec2(0, 60), ImGuiChildFlags_Border);
        // for (int i = 0; i < 10; i++)
        //     ImGui::Text("Scrolling Text %d", i);
        // ImGui::EndChild();
        // static float f = 0.5f;
        // static int n = 0;
        // ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        // ImGui::InputFloat("Input", &f, 0.1f);
        // ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
         ImGui::EndMenu();
    }

    // if (ImGui::BeginMenu("Colors"))
    // {
    //     float sz = ImGui::GetTextLineHeight();
    //     for (int i = 0; i < ImGuiCol_COUNT; i++)
    //     {
    //         const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
    //         ImVec2 p = ImGui::GetCursorScreenPos();
    //         ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
    //         ImGui::Dummy(ImVec2(sz, sz));
    //         ImGui::SameLine();
    //         ImGui::MenuItem(name);
    //     }
    //     ImGui::EndMenu();
    // }

    // // Here we demonstrate appending again to the "Options" menu (which we already created above)
    // // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
    // // In a real code-base using it would make senses to use this feature from very different code locations.
    // if (ImGui::BeginMenu("Options")) // <-- Append!
    // {
    //    // IMGUI_DEMO_MARKER("Examples/Menu/Append to an existing menu");
    //     static bool b = true;
    //     ImGui::Checkbox("SomeOption", &b);
    //     ImGui::EndMenu();
    // }

    // if (ImGui::BeginMenu("Disabled", false)) // Disabled
    // {
    //     IM_ASSERT(0);
    // }

    // if (ImGui::MenuItem("Checked", NULL, true)) {}
    //     ImGui::Separator();


    //options

   // if (show_grid)
       // RenderGrid();

    //if (show_sprite_options)
       // RenderEntityEditor();

//test project generation

    //char* buf;

    //ImGui::InputText("Project name:", buf, sizeof(buf));

}

