#include "../../../../build/include/app.h"
#include "../editor.h"
#include "./gui.h"


void GUI::ShowSettings()
{

    //scenes

    if (ImGui::BeginMenu("Scenes")) {

        if (ImGui::BeginMenu("scenes in queue"))
        {
            if (!Editor::scenes.size())
                ImGui::Text("none selected.");

            else
                for (int i = 0; i < Editor::scenes.size(); i++) 
                    ImGui::Text(("scene " + std::to_string(i) + ": " + Editor::scenes[i]).c_str());

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("add")) 
        {
            for (const auto& filename : std::filesystem::directory_iterator(Editor::projectPath)) 
                if (System::Utils::str_endsWith(filename.path().string(), ".SPAGHYETI") || System::Utils::str_endsWith(filename.path().string(), ".spaghyeti"))
                {
                    std::string scene = " " + System::Utils::ReplaceFrom(filename.path().filename().string(), ".", ""); 
                  
                    if (ImGui::MenuItem((scene).c_str())) 
                    {
                        if (std::find_if(Editor::scenes.begin(), Editor::scenes.end(), [&](const std::string& s ) { return s == scene; }) == Editor::scenes.end()) 
                            Editor::scenes.push_back(scene);
                        
                        else 
                            ImGui::Text("scene already selected.");
                    }
                }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("remove")) 
        {
            for (const auto& filename : std::filesystem::directory_iterator(Editor::projectPath)) 
                if (System::Utils::str_endsWith(filename.path().string(), ".SPAGHYETI") || System::Utils::str_endsWith(filename.path().string(), ".spaghyeti"))
                {
                    std::string scene = " " + System::Utils::ReplaceFrom(filename.path().filename().string(), ".", ""); 
                    if (ImGui::MenuItem((scene).c_str())) {
                        auto it = std::find_if(Editor::scenes.begin(), Editor::scenes.end(), [&](const std::string& s ) { return s == scene; });
                        if (it != Editor::scenes.end())
                            Editor::scenes.erase(it);
                    } 
                }

            ImGui::EndMenu();
        }
    
        ImGui::EndMenu();
    }

    //scene global vars

    if (ImGui::BeginMenu("Globals"))
    {

        ImGui::Text("Global Variables");

        ImGui::Separator();

        for (int i = 0; i < Editor::globals.size(); i++)
        {

            ImGui::PushID(i);

            ImGui::Text("var: %d", i);

            if (ImGui::InputText("name", &Editor::globals[i].first))
                Editor::globals_applied = false;

            ImGui::SameLine();

            static const char* items[] = { "int", "float", "bool", "string", "int[]", "float[]", "string[]" };

            if (ImGui::BeginCombo("type", Editor::globals[i].second.c_str()))
            {
                for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                {
                    bool is_sel = (Editor::globals[i].second == items[n]);

                    if (ImGui::Selectable(items[n], is_sel)) {
                        Editor::globals[i].second = items[n];
                        Editor::globals_applied = false;
                    }

                    if (is_sel)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            ImGui::Separator();

            ImGui::PopID();
        }

        if (ImGui::Button("add"))
            Editor::globals.push_back({"", ""});

        ImGui::SameLine();

        if (ImGui::Button("delete"))
        {

            std::string type = Editor::globals.back().second,
                        var = Editor::globals.back().first;

            Editor::globals.pop_back();

            Editor::globals_applied = false;

            Editor::Log("global variable: " + type + " " + var + " removed.");
        }

        ImGui::SameLine();

        if (ImGui::Button("apply"))
        {
            for (const auto &var : Editor::globals) {

                if ((!var.first.length() || !var.second.length()) || std::adjacent_find(Editor::globals.begin(), Editor::globals.end()) != Editor::globals.end())
                    break;

                Editor::Log("global variable: " + var.second + " " + var.first + " added.");

                Editor::globals_applied = true;
            }
        }

        ImGui::SameLine();

        if (Editor::globals_applied)
            ImGui::Text("variables applied.");

        ImGui::EndMenu();
    }

    //world physics

    if (ImGui::BeginMenu("Physics"))
    {
        ImGui::InputFloat("gravity x", &Editor::gravityX);
        ImGui::InputFloat("gravity y", &Editor::gravityY);
        ImGui::Checkbox("continuous", &Editor::gravity_continuous);
        ImGui::Checkbox("sleeping", &Editor::gravity_sleeping);

        ImGui::EndMenu();
    }

    //world bounds

    if (ImGui::BeginMenu("World Bounds"))
    {
        if (ImGui::BeginMenu("width")) {
            ImGui::InputFloat("begin", &Editor::game->camera->currentBoundsWidthBegin);
            ImGui::InputFloat("end", &Editor::game->camera->currentBoundsWidthEnd);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("height")) {
            ImGui::InputFloat("begin", &Editor::game->camera->currentBoundsHeightBegin);
            ImGui::InputFloat("end", &Editor::game->camera->currentBoundsHeightEnd);

            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }
}

//----------------------------------------

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

