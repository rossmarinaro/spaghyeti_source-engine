#include "../../../../build/sdk/include/app.h"
#include "../assets/assets.h"
#include "../editor.h"
#include "./gui.h"


void editor::GUI::ShowSettings()
{

    auto session = Editor::Get();
    auto am = AssetManager::Get();

    //scenes

    if (ImGui::BeginMenu("Scenes")) 
    {

        if (ImGui::BeginMenu("scenes in queue")) {
            for (int i = 0; i < session->scenes.size(); i++) 
                ImGui::Text(("scene " + std::to_string(i) + ": " + session->scenes[i]).c_str());

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("add"))  
        {
            for (const auto& filename : std::filesystem::directory_iterator(Editor::projectPath)) 
            {
                const auto IterateScenes = [&](const std::filesystem::directory_entry& path) -> void 
                {
                    std::string name = path.path().filename().string();
                    const std::string scene = System::Utils::ReplaceFrom(name, ".", ""); 
                    auto session = Editor::Get();

                    if (session->events->s_currentScene != scene)
                        if (ImGui::MenuItem((scene).c_str())) 
                            if (std::find_if(session->scenes.begin(), session->scenes.end(), [&](const std::string& s) { return s == scene; }) == session->scenes.end()) 
                                session->scenes.push_back(scene); 
                };

                std::string sceneDir = Editor::projectPath + "scenes";

                std::replace(sceneDir.begin(), sceneDir.end(), '\\', '/');

                if (filename.exists() && System::Utils::str_endsWith(filename.path().string(), ".spaghyeti")) 
                    IterateScenes(filename);

                else if (filename.is_directory() && filename.path().string() == sceneDir)
                    for (const auto& f : std::filesystem::directory_iterator(sceneDir)) 
                        if (f.exists() && System::Utils::str_endsWith(f.path().string(), ".spaghyeti"))
                            IterateScenes(f);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("remove")) 
        {
            for (auto scene_it = session->scenes.begin(); scene_it != session->scenes.end(); scene_it++) 
            {
                if (session->events->s_currentScene != *scene_it)
                {
                    if (ImGui::MenuItem((*scene_it).c_str())) {

                        auto it = std::find_if(session->scenes.begin(), session->scenes.end(), [&](const std::string& s ) { return s == *scene_it; });
  
                        if (it != session->scenes.end()) {
                            scene_it = session->scenes.erase(it); 
                            --scene_it;
                        }
                    }
                } 

                else if (session->scenes.size() == 1)
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

        for (int i = 0; i < session->globals.size(); i++)
        {

            ImGui::PushID(i);

            ImGui::Text("var: %d", i);

            if (ImGui::InputText("name", &session->globals[i].first))
                session->globals_applied = false;

            ImGui::SameLine();

            static const char* items[] = { "int", "float", "bool", "string", "int[]", "float[]", "string[]" };

            if (ImGui::BeginCombo("type", session->globals[i].second.c_str()))
            {
                for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                {
                    bool is_sel = (session->globals[i].second == items[n]);

                    if (ImGui::Selectable(items[n], is_sel)) {
                        session->globals[i].second = items[n];
                        session->globals_applied = false;
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
            session->globals.push_back({"", ""});

        ImGui::SameLine();

        if (ImGui::Button("delete"))
        {

            std::string type = session->globals.back().second,
                        var = session->globals.back().first;

            session->globals.pop_back();

            session->globals_applied = false;

            Editor::Log("global variable: " + type + " " + var + " removed.");
        }

        ImGui::SameLine();

        if (ImGui::Button("apply"))
        {
            for (const auto &var : session->globals) {

                if ((!var.first.length() || !var.second.length()) || std::adjacent_find(session->globals.begin(), session->globals.end()) != session->globals.end())
                    break;

                Editor::Log("global variable: " + var.second + " " + var.first + " added.");

                session->globals_applied = true;
            }
        }

        ImGui::SameLine();

        if (session->globals_applied)
            ImGui::Text("variables applied.");


        ImGui::EndMenu();
    }
    

    //world physics

    if (ImGui::BeginMenu("Physics"))
    {
        ImGui::InputFloat("gravity x", &session->gravityX);
        ImGui::InputFloat("gravity y", &session->gravityY);
        ImGui::Checkbox("continuous", &session->gravity_continuous);
        ImGui::Checkbox("sleeping", &session->gravity_sleeping);

        ImGui::EndMenu();
    }

    //world bounds

    if (ImGui::BeginMenu("World Bounds"))
    {
        if (ImGui::BeginMenu("width")) {
            ImGui::InputFloat("begin", &session->game->camera->currentBoundsWidthBegin);
            ImGui::InputFloat("end", &session->game->camera->currentBoundsWidthEnd);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("height")) {
            ImGui::InputFloat("begin", &session->game->camera->currentBoundsHeightBegin);
            ImGui::InputFloat("end", &session->game->camera->currentBoundsHeightEnd);

            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    //scene data loader
        
    if (ImGui::BeginMenu("Load Data"))
    {

        //preload assets

        if (ImGui::BeginMenu("preload assets"))
        {

            if (ImGui::BeginMenu("add"))
            {
                for (auto& asset : am->loadedAssets) 
                {
                    std::string key = asset.first,
                                path = asset.second;

                    key.erase(std::remove(key.begin(), key.end(), '\"'), key.end());
                    path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());
                    
                    if (System::Utils::GetFileType(path) == System::Resources::Manager::DATA) 
                        continue;

                    if (ImGui::MenuItem(path.c_str()))  
                        AssetManager::Register(key);
                }

                ImGui::EndMenu();
            }

            ImGui::Separator();
            
            int i = 0;

            if (am->assets.size())
                for (auto& asset : am->assets) 
                {
                    i++; 

                    std::string path = "build/assets/" + asset;

                    if (Editor::platform != "WebGL")
                        path = "assets/" + asset;

                    if (ImGui::BeginMenu((asset + ": " + path).c_str()))
                    {
                        if (ImGui::MenuItem("delete")) 
                        {
                            auto it = std::find(am->assets.begin(), am->assets.end(), asset);

                            if (it != am->assets.end())
                                am->assets.erase(it);
                        }

                        ImGui::EndMenu();
                    }

                    if (i < am->assets.size())
                        ImGui::Separator();
                }

            else
                ImGui::MenuItem("no assets loaded.");


            ImGui::EndMenu();
        }

        //preload shaders

        if (ImGui::BeginMenu("preload shaders"))
        {

            const auto iterate = [&] (const std::string& type, std::string& p, int index) -> void 
            {

                if (System::Utils::str_endsWith(p, type)) 
                    if (ImGui::MenuItem(p.c_str())) {
                        if (type == ".vert")
                            Editor::Get()->shaders[index].second.first = p;
                        if (type == ".frag")
                            Editor::Get()->shaders[index].second.second = p;
                    }
            };

            const auto searchShaderFolders = [&am, &iterate](int index, const std::string& type) -> void 
            {
                for (const auto& shader : std::filesystem::directory_iterator(Editor::projectPath + am->shader_dir)) 
                {
                    std::string path = shader.path().string();
                    std::replace(path.begin(), path.end(), '\\', '/');

                    const std::string name = shader.path().filename().string();

                    if (shader.is_directory()) { 
                        for (const auto& folder : std::filesystem::directory_iterator(shader)) {
                            if (!folder.is_directory()) {
                                std::string p = folder.path().string();
                                std::replace(p.begin(), p.end(), '\\', '/'); 
                                iterate(type, p, index);
                            }
                        }
                    }
                    else
                        iterate(type, path, index);
                }
            };

            for (int i = 0; i < session->shaders.size(); i++)
            {

                ImGui::PushID(i);

                ImGui::InputText("key", &session->shaders[i].first);

                ImGui::SameLine();

                if (ImGui::Button("remove"))
                {
                    auto it = std::find_if(session->shaders.begin(), session->shaders.end(), [&session, &i](const auto& sh) { return sh.first == session->shaders[i].first; });

                    if (it != session->shaders.end()) {
                        it = session->shaders.erase(it);
                        --it;
                    }
                }

                ImGui::Text(("vertex: " + session->shaders[i].second.first).c_str());

                ImGui::SameLine();
                
                if (ImGui::BeginMenu(".vert")) {
                    searchShaderFolders(i, ".vert");
                    ImGui::EndMenu();
                }

                ImGui::Text(("fragment: " + session->shaders[i].second.second).c_str());

                ImGui::SameLine();

                if (ImGui::BeginMenu(".frag")) {
                    searchShaderFolders(i, ".frag");
                    ImGui::EndMenu();
                }

                ImGui::Separator();

                ImGui::PopID();

            }

            if (ImGui::Button("add"))
                session->shaders.push_back({ "", { "none selected", "none selected" }});

            ImGui::SameLine();

            if (ImGui::Button("delete")) {
                std::string key = session->shaders.back().first;
                session->shaders.pop_back();
                session->shaders_applied = false;
                Editor::Log("shader: " + key + " removed.");
            }

            if (ImGui::Button("apply"))
            {
                for (const auto& shader : session->shaders) {

                    if ((!shader.first.length() || shader.second.first == "none selected" || shader.second.second == "none selected") || 
                        std::adjacent_find(session->shaders.begin(), session->shaders.end()) != session->shaders.end())
                        break;

                    session->shaders_applied = true;

                    Editor::Log("shader: " + shader.first + " added.");
                }
            }

            ImGui::EndMenu();
        }

        //preload spritesheets

        if (ImGui::BeginMenu("preload spritesheets"))
        {
            int i = 0;

            for (auto& spritesheet : session->spritesheets)
            {

                ImGui::PushID(i);

                i++;

                ImGui::Separator();

                std::string alias_key = spritesheet.first;

                ImGui::InputText("texture key", &alias_key);

                spritesheet.first = alias_key;

                ImGui::SameLine();

                if (ImGui::BeginCombo("path", spritesheet.second.c_str()))
                {
                    for (const auto& asset : am->loadedAssets)
                    {
                        std::string key = asset.first,
                                    path = asset.second;

                        key.erase(std::remove(key.begin(), key.end(), '\"'), key.end());
                        path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());

                        if (System::Utils::str_endsWith(path, ".json")) 
                            if (ImGui::MenuItem(key.c_str())) 
                                spritesheet.second = path;
                    }

                    ImGui::EndCombo();
                }

                ImGui::SameLine();

                if (ImGui::Button("remove"))
                {
                    auto it = std::find_if(session->spritesheets.begin(), session->spritesheets.end(), [&spritesheet](const auto& sh) { return sh.first == spritesheet.first; });

                    if (it != session->spritesheets.end()) {
                        it = session->spritesheets.erase(it);
                        --it;
                    }
                }

                ImGui::Separator();

                ImGui::PopID();
            }

            if (ImGui::Button("add"))
                session->spritesheets.push_back({ "", "" });

            ImGui::SameLine();

            if (ImGui::Button("delete"))
                session->spritesheets.pop_back();
    
            ImGui::EndMenu();
            
        }

        //preload animations

        if (ImGui::BeginMenu("preload animations"))
        {
            for (int i = 0; i < session->animations.size(); i++)
            {
                ImGui::PushID(i);

                ImGui::InputText("texture key", &session->animations[i].first);

                if (ImGui::BeginMenu("animations")) 
                {
                    int j = 0;

                    if (session->animations[i].second.size())
                        for (auto& anim : session->animations[i].second) 
                        {
                            ImGui::PushID(j);

                            ImGui::InputText("key", &anim.first);
                            ImGui::InputInt("start", &anim.second.first);
                            ImGui::InputInt("end", &anim.second.second);

                            ImGui::PopID();

                            if (session->animations[i].second.size() > 1)
                                ImGui::Separator();

                            j++;
                        }
                    else
                        ImGui::Text("none applied.");

                    ImGui::EndMenu();
                }

                if (ImGui::Button("add key")) 
                    session->animations[i].second.push_back({ "", { 0, 0 }});

                ImGui::SameLine();

                if (ImGui::Button("delete key"))
                    session->animations[i].second.pop_back();

                if (ImGui::Button("remove"))
                {
                    auto it = std::find_if(session->animations.begin(), session->animations.end(), [&session, &i](const auto& anim) { return anim.first == session->animations[i].first; });

                    if (it != session->animations.end()) {
                        it = session->animations.erase(it);
                        --it;
                    }
                }

                ImGui::Separator();

                ImGui::PopID();

            }

            if (ImGui::Button("add")) 
                session->animations.push_back({ "", {}});

            ImGui::SameLine();

            if (ImGui::Button("delete")) {
                std::string key = session->animations.back().first;
                session->animations.pop_back();
                session->animations_applied = false;
                Editor::Log("animation: " + key + " removed.");
            }

            if (ImGui::Button("apply"))
            {
                for (const auto& animation : session->animations) {

                    if ((!animation.first.length()) || 
                        std::adjacent_find(session->animations.begin(), session->animations.end()) != session->animations.end())
                        break;

                    session->animations_applied = true;

                    Editor::Log("animation: " + animation.first + " added.");
                }
            }

            ImGui::EndMenu();
        }


        ImGui::EndMenu();

    }

}


//----------------------------------------


void editor::GUI::ShowMenu()
{
    auto session = Editor::Get();

    ImGui::MenuItem(("Platform: " + Editor::platform).c_str(), NULL, false, false);
    ImGui::MenuItem(("Build: " + Editor::buildType).c_str(), NULL, false, false);
    ImGui::MenuItem(("Distribution: " + Editor::releaseType).c_str(), NULL, false, false);

    ImGui::Separator();

    if (ImGui::MenuItem("Build / Run"))
        session->events->buildFlag = true;

    if (ImGui::BeginMenu("New")) 
    {
        if (ImGui::MenuItem("project"))
            if (!session->events->NewProject())
                Editor::Log("could not create project.");

        if (ImGui::MenuItem("scene"))
            if (!session->events->NewScene())
                Editor::Log("could not create scene.");

        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Open", "Ctrl+O"))
        session->events->OpenProject();

    if (ImGui::MenuItem("Save", "Ctrl+S"))
        if(!session->events->SaveScene())
            Editor::Log("could not save scene.");
 
    if (ImGui::MenuItem("Save As.."))
        if (!session->events->SaveScene(true))
            Editor::Log("could not save scene.");

    ImGui::Separator();

    if (ImGui::MenuItem("Quit", "Alt+F4"))
        show_quit = true; 

    if (ImGui::BeginMenu("Options"))
    {

        //build 

        if (ImGui::BeginMenu("Build"))
        {  
            //std::ostringstream oss;
            //oss << std::fixed << std::setprecision(1) << session->maxVersion << std::fixed << std::setprecision(1) << session->minVersion;
            //ImGui::Text(("version: " + oss.str()).c_str());
            ImGui::Text(("version: " + std::to_string(session->maxVersion) + "." + std::to_string(session->midVersion) + "." + std::to_string(session->minVersion)/* oss.str() */).c_str());
            ImGui::InputInt("maxVersion", &session->maxVersion);
            ImGui::InputInt("midVersion", &session->midVersion);
            ImGui::InputInt("minVersion", &session->minVersion);

            if (ImGui::BeginMenu("type")) 
            {
                if (Editor::platform == "WebGL")
                    ImGui::Text("WebGL builds are static by default.");

                else 
                {
                    if (ImGui::MenuItem("static")) {
                        Editor::buildType = "static";
                        Editor::releaseType = "release";
                    }

                    if (ImGui::MenuItem("dynamic"))
                        Editor::buildType = "dynamic";
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("distribution")) 
            {

                if (Editor::platform == "WebGL")
                    ImGui::Text("WebGL builds are release by default.");

                else if (Editor::buildType == "static")
                    ImGui::Text("static builds are release by default.");

                else {
                    if (ImGui::MenuItem("debug"))
                        Editor::releaseType = "debug";

                    if (ImGui::MenuItem("release"))
                        Editor::releaseType = "release";
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("icon")) 
            {
                const char* icon = AssetManager::Get()->projectIcon.length() ? 
                    ("icon path: " + AssetManager::Get()->projectIcon).c_str() : "No icon assigned.";
                
                ImGui::Text(icon);

                ImGui::Separator();

                if (ImGui::MenuItem("select icon")) 
                    session->events->OpenFile(); 

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("configurations")) 
            {
                ImGui::Checkbox("multi threaded", &session->isMultiThreaded);
                ImGui::Checkbox("preserve source file", &session->preserveSrc);
                
                ImGui::Text(("platform: " + Editor::platform).c_str());

                ImGui::Separator();

                if (Editor::platform == "WebGL") {
                    ImGui::Checkbox("use pthreads", &session->use_pthreads);
                    ImGui::Checkbox("shared memory", &session->shared_memory);
                    ImGui::Checkbox("allow memory growth", &session->allow_memory_growth);
                    ImGui::Checkbox("allow exception catching", &session->allow_exception_catching);
                    ImGui::Checkbox("export all", &session->export_all);
                    ImGui::Checkbox("WASM", &session->wasm);
                    ImGui::Checkbox("GL assertions", &session->gl_assertions);
                    ImGui::Checkbox("use WebGL2", &session->use_webgl2);
                    ImGui::Checkbox("full ES3", &session->full_es3);
                    ImGui::Checkbox("embed assets", &session->webgl_embed_files);
                }

                else 
                    ImGui::Checkbox("embed assets", &session->embed_files);
                
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

//-----------------------------------------


void editor::GUI::ShowViewport()
{
    auto session = Editor::Get();

    ImGui::Text("viewport");
    ImGui::Separator();

    ImGui::InputInt("width", &session->worldWidth);
    ImGui::InputInt("height", &session->worldHeight); 
    ImGui::SliderFloat("x", (float*)&session->game->camera->GetPosition()->x, session->worldWidth, -session->worldWidth);
    ImGui::SliderFloat("y", (float*)&session->game->camera->GetPosition()->y, session->worldHeight, -session->worldHeight); 
    ImGui::SliderFloat("zoom", session->game->camera->GetZoom(), -10.0f, 10.0f);
    ImGui::SliderFloat("rotation", session->game->camera->GetRotation(), 0.0f, 360.0f);
    ImGui::SliderFloat("vignette", &session->vignetteVisibility, 0.0f, 1.0f);
    ImGui::ColorEdit4("color", (float*)session->game->camera->GetBackgroundColor()); 

    //cull target

    if (ImGui::BeginCombo("cull target", session->cullTarget.first.c_str()))
    {
        auto iterateSprites = [](auto node) -> void {
            auto sn = std::dynamic_pointer_cast<SpriteNode>(Node::Get(node->ID));
            if (ImGui::Selectable(sn->name.c_str())) 
                Editor::Get()->cullTarget = { sn->name, { sn->positionX, sn->positionY } };
        };

        for (const auto& node : Node::nodes) {
            if (node->type == Node::SPRITE)
                iterateSprites(node);

            if (node->type == Node::GROUP) {
                auto gn = std::dynamic_pointer_cast<GroupNode>(Node::Get(node->ID));
                for (const auto& n : gn->_nodes)
                    if (n->type == Node::SPRITE)
                        iterateSprites(n);
            }
        }

        ImGui::EndCombo();
    }

    //vignette visibility

    session->game->GetScene()->vignette->SetAlpha(session->vignetteVisibility);
    ImGui::Text("grid");
    ImGui::Separator();

    ImGui::SliderFloat("grid alpha", (float*)&grid->alpha, 0.0f, 1.0f);
    ImGui::SliderFloat("pitch", (float*)&grid_quantity, 0.0f, 200.0f);

    //cursor

    ImGui::Text("cursor");
    ImGui::Separator();

    ImGui::SliderFloat("cursor alpha", (float*)&cursor->alpha, 0.0f, 1.0f);
    ImGui::ColorEdit3("tint", (float*)&cursor->tint);
}