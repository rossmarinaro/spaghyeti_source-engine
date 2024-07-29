#include "../../../../build/sdk/include/app.h"
#include "../assets/assets.h"
#include "../editor.h"
#include "./gui.h"


void editor::GUI::ShowSettings()
{

    //scenes

    if (ImGui::BeginMenu("Scenes")) 
    {

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
                    const std::string scene = System::Utils::ReplaceFrom(filename.path().filename().string(), ".", ""); 
                  
                    if (Editor::events.s_currentScene != scene)
                        if (ImGui::MenuItem((scene).c_str())) 
                            if (std::find_if(Editor::scenes.begin(), Editor::scenes.end(), [&](const std::string& s ) { return s == scene; }) == Editor::scenes.end()) 
                                Editor::scenes.push_back(scene);
                }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("remove")) 
        {
            for (auto scene_it = Editor::scenes.begin(); scene_it != Editor::scenes.end(); scene_it++) 
            {
                if (Editor::events.s_currentScene != *scene_it)
                {
                    if (ImGui::MenuItem((*scene_it).c_str())) 
                    {

                        auto it = std::find_if(Editor::scenes.begin(), Editor::scenes.end(), [&](const std::string& s ) { return s == *scene_it; });
  
                        if (it != Editor::scenes.end()) {
                            scene_it = Editor::scenes.erase(it);
                            --scene_it;
                        }
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

    //data loader
        
    if (ImGui::BeginMenu("Load Data"))
    {

        if (ImGui::BeginMenu("spritesheets"))
        {
            int i = 0;

            for (auto& spritesheet : Editor::spritesheets)
            {

                ImGui::PushID(i);

                i++;

                ImGui::Separator();

                std::string alias_key = spritesheet.first;

                ImGui::InputText("key", &alias_key);

                spritesheet.first = alias_key;

                ImGui::SameLine();

                if (ImGui::BeginCombo("path", spritesheet.second.c_str()))
                {

                    for (const auto& asset : AssetManager::loadedAssets)
                    {

                        std::string key = asset.first;
                        std::string path = asset.second;

                        key.erase(std::remove(key.begin(), key.end(), '\"'), key.end());
                        path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());

                        if (System::Utils::str_endsWith(path, ".json")) 
                        {

                            if (ImGui::MenuItem(key.c_str())) 
                            {

                                //parse json to extract frame data

                                std::ifstream JSON(path);

                                if (!JSON.good()) 
                                    Editor::Log("Error parsing. Maybe file is in an inappropriate folder?");

                                else 
                                {

                                    json data = json::parse(JSON);

                                    std::vector<std::array<int, 6>> framesToPush;

                                    if (data["frames"].size() > 1)
                                        for (const auto& index : data["frames"]) 
                                        {
                                            int x = index["frame"]["x"],
                                                y = index["frame"]["y"];

                                            float width = index["frame"]["w"],
                                                  height = index["frame"]["h"];
                                    
                                            framesToPush.push_back({ x, y, width, height, 1, 1 });

                                        };
                    
                                    System::Resources::Manager::LoadFrames(key, framesToPush);
                                    spritesheet.second = path;

                                }
                            }
                        }
                    }

                    ImGui::EndCombo();
                }

                ImGui::Separator();

                ImGui::PopID();
            }

            if (ImGui::Button("add"))
                Editor::spritesheets.push_back({ "", "" });

            ImGui::SameLine();

            if (ImGui::Button("delete"))
                Editor::spritesheets.pop_back();
    
            ImGui::EndMenu();
            
        }


        ImGui::EndMenu();

    }

}


//----------------------------------------


void editor::GUI::ShowMenu()
{

    ImGui::MenuItem(("Platform: " + Editor::platform).c_str(), NULL, false, false);
    ImGui::MenuItem(("Build: " + Editor::buildType).c_str(), NULL, false, false);
    ImGui::MenuItem(("Distribution: " + Editor::releaseType).c_str(), NULL, false, false);

    ImGui::Separator();

    if (ImGui::MenuItem("Build / Run"))
        Editor::events.buildFlag = true;

    if (ImGui::MenuItem("New"))
        Editor::events.NewProject();

    if (ImGui::MenuItem("Open", "Ctrl+O"))
        Editor::events.OpenScene();

    if (ImGui::MenuItem("Save", "Ctrl+S"))
        if(!Editor::events.SaveScene())
            Editor::Log("could not save scene.");
 
    if (ImGui::MenuItem("Save As.."))
        if (!Editor::events.SaveScene(true))
            Editor::Log("could not save scene.");

    ImGui::Separator();

    if (ImGui::MenuItem("Quit", "Alt+F4"))
        s_show_quit = true; 

    if (ImGui::BeginMenu("Options"))
    {

        //build 

        if (ImGui::BeginMenu("Build"))
        {
            if (ImGui::BeginMenu("type")) 
            {
                if (Editor::platform == "WebGL")
                    ImGui::Text("WebGL builds are static by default.");

                else 
                {
                    if (ImGui::MenuItem("static"))
                        Editor::buildType = "static";

                    if (ImGui::MenuItem("dynamic"))
                        Editor::buildType = "dynamic";
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("distribution")) 
            {

                if (Editor::platform == "WebGL")
                    ImGui::Text("WebGL builds are release by default.");

                else
                {
                    if (ImGui::MenuItem("debug"))
                        Editor::releaseType = "debug";

                    if (ImGui::MenuItem("release"))
                        Editor::releaseType = "release";
                }


                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("icon")) 
            {
                const char* icon = AssetManager::projectIcon.length() ? 
                    ("icon path: " + AssetManager::projectIcon).c_str() : "No icon assigned.";
                
                ImGui::Text(icon);

                ImGui::Separator();

                if (ImGui::MenuItem("select icon")) 
                    Editor::events.OpenFile(); 

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("configurations")) 
            {

                ImGui::Text(Editor::platform.c_str());

                ImGui::Separator();

                if (Editor::platform == "WebGL")
                {
                    ImGui::Checkbox("use pthreads", &Editor::use_pthreads);
                    ImGui::Checkbox("shared memory", &Editor::shared_memory);
                    ImGui::Checkbox("allow memory growth", &Editor::allow_memory_growth);
                    ImGui::Checkbox("allow exception catching", &Editor::allow_exception_catching);
                    ImGui::Checkbox("export all", &Editor::export_all);
                    ImGui::Checkbox("WASM", &Editor::wasm);
                    ImGui::Checkbox("GL assertions", &Editor::gl_assertions);
                    ImGui::Checkbox("use WebGL2", &Editor::use_webgl2);
                    ImGui::Checkbox("full ES3", &Editor::full_es3);
                }

                else 
                    ImGui::Text("N/A");
                

                ImGui::EndMenu();
            }


            ImGui::EndMenu();
        }

        //platform

        if (ImGui::BeginMenu("Switch Platform"))
        {
            if (ImGui::MenuItem("Windows"))
                Editor::platform = "Windows";

            if (ImGui::MenuItem("WebGL")) 
            {
                Editor::platform = "WebGL";
                Editor::releaseType = "release";
                Editor::buildType = "static";
            }

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

