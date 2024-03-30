#include "../../../../build/include/app.h"
#include "../editor.h"
#include "./gui.h"


void editor::GUI::ShowSettings()
{

    //scenes

    if (ImGui::BeginMenu("Scenes")) {

        if (ImGui::BeginMenu("scenes in queue"))
        {
            for (int i = 0; i < Editor::scenes.size(); i++) 
                ImGui::Text(("scene " + std::to_string(i) + ": " + Editor::scenes[i]).c_str());

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("add")) 
        {
            for (const auto& filename : std::filesystem::directory_iterator(Editor::projectPath)) 
                if (System::Utils::str_endsWith(filename.path().string(), ".spaghyeti"))
                {
                    std::string scene = System::Utils::ReplaceFrom(filename.path().filename().string(), ".", ""); 
                  
                  if (Editor::events.currentProject != scene)
                    if (ImGui::MenuItem((scene).c_str())) 
                        if (std::find_if(Editor::scenes.begin(), Editor::scenes.end(), [&](const std::string& s ) { return s == scene; }) == Editor::scenes.end()) 
                            Editor::scenes.push_back(scene);
                }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("remove")) 
        {
            for (const auto& scene : Editor::scenes) 
            {
                if (Editor::events.currentProject != scene)
                {
                    if (ImGui::MenuItem((scene).c_str())) {

                        auto it = std::find_if(Editor::scenes.begin(), Editor::scenes.end(), [&](const std::string& s ) { return s == scene; });

                        if (it != Editor::scenes.end() && Editor::scenes.size() > 1)
                            Editor::scenes.erase(it);
                    }
                } 

                else if (Editor::scenes.size() == 1)
                    ImGui::Text("cannot remove base scene.");
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


void editor::GUI::ShowMenu()
{

    ImGui::MenuItem("File Select", NULL, false, false); 
    
    ImGui::SameLine();

    ImGui::Text(("Platform: " + Editor::platform).c_str());

    if (ImGui::MenuItem("Build / Run"))
        Editor::events.BuildAndRun();

    if (ImGui::MenuItem("New"))
        Editor::events.NewProject();

    if (ImGui::MenuItem("Open", "Ctrl+O"))
        Editor::events.OpenProject();

    if (ImGui::MenuItem("Save", "Ctrl+S"))
        if(Editor::events.SaveProject())
            Editor::events.exitFlag = true;

    if (ImGui::MenuItem("Save As.."))
        if (Editor::events.SaveProject(true))
            Editor::events.exitFlag = true;

    ImGui::Separator();

    if (ImGui::MenuItem("Quit", "Alt+F4"))
        show_quit = true;

    if (ImGui::BeginMenu("Options"))
    {

        //platform

        if (ImGui::BeginMenu("Switch Platform"))
        {
            if (ImGui::MenuItem("Windows"))
                Editor::platform = "Windows";

            if (ImGui::MenuItem("WebGL"))
                Editor::platform = "WebGL";

            ImGui::EndMenu();
        }

        //editor theme

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


        ImGui::EndMenu();
    }


}

