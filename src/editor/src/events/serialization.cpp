#include <filesystem>
#include <iterator>

#include "./events.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../nodes/node.h"
#include "../../../../build/sdk/include/app.h"


using namespace editor;


//-----------------------------------


std::string EventListener::GetScriptName(const std::string& path)
{

    std::ifstream src(path);
    std::string line, scriptName;

    while (src >> line)
        if (line == "class")  
            if (src >> line)
                scriptName = line;
    
    return scriptName;  
}


//-----------------------------------


void EventListener::EncodeFile(const std::string& path, bool newScene)
{
    std::ofstream src(path);

    json data;

    Serialize(data, newScene);

    std::string JSON = data.dump();

    for (int i = 0; i < JSON.length(); i++)
        if (JSON[i] == '{')
            JSON[i] = '%';
        else if (JSON[i] == '}')
            JSON[i] = '|';
        else if (JSON[i] == '[')
            JSON[i] = '?';
        else if (JSON[i] == ']')
            JSON[i] = '!';
        else if (JSON[i] == ',')
            JSON[i] = '&';
        else if (JSON[i] == ',')
            JSON[i] = '&';
        else if (JSON[i] == ':')
            JSON[i] = '$';

    src << JSON;

    src.close();
}


//-----------------------------------


void EventListener::DecodeFile(const std::string& outPath, const std::filesystem::path& currentPath)
{

    std::string line;

    std::ifstream ini_file(currentPath.string());
    std::ofstream out_file(outPath);

    while (getline(ini_file, line))
    {

        //decode spaghyet file format to json

        for (int i = 0; i < line.length(); i++)
            if (line[i] == '%')
                line[i] = '{';
            else if (line[i] == '|')
                line[i] = '}';
            else if (line[i] == '?')
                line[i] = '[';
            else if (line[i] == '!')
                line[i] = ']';
            else if (line[i] == '&')
                line[i] = ',';
            else if (line[i] == '$')
                line[i] = ':';

        out_file << line << "\n";

    }

    ini_file.close();
    out_file.close();

}


//-----------------------------project serialization


void EventListener::Serialize(json& data, bool newScene)
{

    json scenes = json::array();
    json nodes = json::array();

    json sprites;
    json tilemaps;
    json audio;
    json text;
    json empty;
    json groups;
    
    data["icon"] = newScene ? "" : AssetManager::projectIcon;

    //camera

    data["camera"]["vignetteVisibility"] = newScene ? 0.0f : Editor::vignetteVisibility;
    data["camera"]["x"] = newScene ? 0.0f : Editor::game->camera->GetPosition().x;
    data["camera"]["y"] = newScene ? 0.0f : Editor::game->camera->GetPosition().y;
    data["camera"]["width"] = newScene ? 2000.0f : Editor::worldWidth;
    data["camera"]["height"] = newScene ? 2000.0f : Editor::worldHeight;
    data["camera"]["zoom"] = newScene ? 1.0f : Editor::game->camera->GetZoom(); 
    data["camera"]["color"]["x"] = newScene ? 0.5f : Editor::game->camera->GetBackgroundColor().x;
    data["camera"]["color"]["y"] = newScene ? 0.5f : Editor::game->camera->GetBackgroundColor().y;
    data["camera"]["color"]["z"] = newScene ? 0.5f : Editor::game->camera->GetBackgroundColor().z;
    data["camera"]["color"]["w"] = newScene ? 1.0f : Editor::game->camera->GetBackgroundColor().w;
    data["camera"]["alpha"] = newScene ? 1.0f : GUI::s_grid->alpha;
    data["camera"]["pitch"] = newScene ? 20.0f : GUI::s_grid_quantity;
    data["camera"]["bounds"]["width"]["begin"] = newScene ? 0.0f : Editor::game->camera->currentBoundsWidthBegin;
    data["camera"]["bounds"]["width"]["end"] = newScene ? 0.0f : Editor::game->camera->currentBoundsWidthEnd;
    data["camera"]["bounds"]["height"]["begin"] = newScene ? 0.0f : Editor::game->camera->currentBoundsHeightBegin;
    data["camera"]["bounds"]["height"]["end"] = newScene ? 0.0f : Editor::game->camera->currentBoundsHeightEnd;

    //settings

    data["settings"]["physics"]["gravity"]["x"] = newScene ? 0.0f : Editor::gravityX;
    data["settings"]["physics"]["gravity"]["y"] = newScene ? 500.0f : Editor::gravityY;
    data["settings"]["physics"]["continuous"] = newScene ? true : Editor::gravity_continuous;
    data["settings"]["physics"]["sleeping"] = newScene ? true : Editor::gravity_sleeping;

    data["globals_applied"] = newScene ? false : Editor::globals_applied;

    //loaded data

    json spritesheets = json::array(),
         assets = json::array(),
         shaders = json::array(),
         globals = json::array();

    if (!newScene)
    {
        if (Editor::globals_applied)
            for (const auto& global : Editor::globals)
                globals.push_back({ { "key", global.first }, { "type", global.second } });

        if (AssetManager::assets_preload.size())
            for (const auto& asset : AssetManager::assets_preload)
                assets.push_back(asset);

        if (Editor::shaders_applied)
            for (const auto& shader : Editor::shaders)
                shaders.push_back({ { "key", shader.first }, { "vertex", shader.second.first }, { "fragment", shader.second.second } });

        if (Editor::spritesheets.size())
            for (const auto& spritesheet : Editor::spritesheets)
                spritesheets.push_back({ { "key", spritesheet.first }, { "path", spritesheet.second } });

        if (Editor::scenes.size() > 1) 
        {
            for (int i = 0; i < Editor::scenes.size(); i++)
                scenes.push_back({ { "key", Editor::scenes[i] } });

            data["scenes"] = scenes;
        }

        for (auto& node : Node::nodes)
        {

            if (node->type == "Sprite")
                sprites.push_back(Node::WriteData(node));

            if (node->type == "Tilemap")
                tilemaps.push_back(Node::WriteData(node));

            if (node->type == "Audio")
                audio.push_back(Node::WriteData(node));

            if (node->type == "Empty")
                empty.push_back(Node::WriteData(node));

            if (node->type == "Text")
                text.push_back(Node::WriteData(node));
                
            if (node->type == "Group")
                groups.push_back(Node::WriteData(node));
        
        }
    }

    data["spritesheets"] = spritesheets;
    data["assets"] = assets;
    data["shaders"] = shaders;
    data["globals"] = globals;
    data["nodes"]["sprites"] = sprites;
    data["nodes"]["tilemaps"] = tilemaps;
    data["nodes"]["audio"] = audio;
    data["nodes"]["empty"] = empty;
    data["nodes"]["text"] = text;
    data["nodes"]["groups"] = groups;

}


//--------------------------------------- Save / Load Projects


//project de-serialization
void EventListener::Deserialize(std::ifstream& JSON)
{

    if (!JSON.good()) {
        Editor::Log("There was a problem reading the file.");
        return;
    }

    json data = json::parse(JSON);

    //scenes 

    if (data["scenes"].size() > 1) //saved in queue
        for (const auto& scene : data["scenes"]) 
            Editor::scenes.push_back(scene["key"]);
    
    else //register opened scene
        Editor::scenes.push_back(Editor::events.s_currentScene);

    AssetManager::projectIcon = data["icon"];

    //camera 

    Editor::vignetteVisibility = data["camera"]["vignetteVisibility"];
    Editor::game->camera->SetPosition({ data["camera"]["x"], data["camera"]["y"] });
    Editor::game->camera->SetZoom(data["camera"]["zoom"]);
    Editor::game->camera->SetBackgroundColor({ data["camera"]["color"]["x"], data["camera"]["color"]["y"], data["camera"]["color"]["z"], data["camera"]["color"]["w"] }); 

    Editor::game->camera->SetBounds(
        data["camera"]["bounds"]["width"]["begin"], data["camera"]["bounds"]["width"]["end"],
        data["camera"]["bounds"]["height"]["begin"], data["camera"]["bounds"]["height"]["end"]
    );

    GUI::s_grid->alpha = data["camera"]["alpha"];
    GUI::s_grid_quantity = data["camera"]["pitch"];
    
    Editor::worldWidth = data["camera"]["width"];
    Editor::worldHeight = data["camera"]["height"];

    Editor::gravityX = data["settings"]["physics"]["gravity"]["x"];
    Editor::gravityY = data["settings"]["physics"]["gravity"]["y"];
    Editor::gravity_continuous = data["settings"]["physics"]["continuous"];
    Editor::gravity_sleeping = data["settings"]["physics"]["sleeping"];

    for (auto& sprite : data["nodes"]["sprites"])
        Node::ReadData(sprite, true, nullptr);

    for (auto& tilemap : data["nodes"]["tilemaps"])
        Node::ReadData(tilemap, true, nullptr);

    for (auto& audio : data["nodes"]["audio"])
        Node::ReadData(audio, true, nullptr);

    for (auto& empty : data["nodes"]["empty"])
        Node::ReadData(empty, true, nullptr);

    for (auto& text : data["nodes"]["text"])
        Node::ReadData(text, true, nullptr);

    for (auto& group : data["nodes"]["groups"])
        Node::ReadData(group, true, nullptr);

    //loaded data

    if (data.contains("spritesheets"))
        for (const auto& spritesheet : data["spritesheets"])
            Editor::spritesheets.push_back({ spritesheet["key"], spritesheet["path"] });

    if (data.contains("assets"))
        for (const auto& asset : data["assets"]) {
            const std::string key = static_cast<std::string>(asset);
            AssetManager::Register(asset, true, true);
        }

    if (data.contains("shaders"))
        for (const auto& shader : data["shaders"]) 
            Editor::shaders.push_back({ shader["key"], { shader["vertex"], shader["fragment"] } });
        
    //global variables
    
    if (data.contains("globals"))
    {
        for (const auto& global : data["globals"])
            Editor::globals.push_back({ global["key"], global["type"] });

        if (data["globals_applied"]) 
            Editor::globals_applied = true;
    }

}



//-------------------------------------------------


//extracts saved data from scene to compile
void EventListener::ParseScene(const std::string& sceneKey, std::ifstream& JSON)
{

    auto scene = new editor::Scene;

    json data = json::parse(JSON);

    //camera

    scene->vignetteVisibility = data["camera"]["vignetteVisibility"];
    scene->cameraPosition.x = data["camera"]["x"];
    scene->cameraPosition.y = data["camera"]["y"];
    scene->cameraZoom = data["camera"]["zoom"];
    scene->cameraBackgroundColor.x = data["camera"]["color"]["x"];
    scene->cameraBackgroundColor.y = data["camera"]["color"]["y"];
    scene->cameraBackgroundColor.z = data["camera"]["color"]["z"];
    scene->cameraBackgroundColor.w = data["camera"]["color"]["w"]; 

    scene->currentBoundsWidthBegin = data["camera"]["bounds"]["width"]["begin"];
    scene->currentBoundsWidthEnd = data["camera"]["bounds"]["width"]["end"];
    scene->currentBoundsHeightBegin = data["camera"]["bounds"]["height"]["begin"];
    scene->currentBoundsHeightEnd = data["camera"]["bounds"]["height"]["end"];
    
    scene->worldWidth = data["camera"]["width"];
    scene->worldHeight = data["camera"]["height"];

    scene->gravityX = data["settings"]["physics"]["gravity"]["x"];
    scene->gravityY = data["settings"]["physics"]["gravity"]["y"];
    scene->gravity_continuous = data["settings"]["physics"]["continuous"];
    scene->gravity_sleeping = data["settings"]["physics"]["sleeping"];

    for (auto& sprite : data["nodes"]["sprites"])
        Node::ReadData(sprite, false, scene);

    for (auto& tilemap : data["nodes"]["tilemaps"])
        Node::ReadData(tilemap, false, scene);

    for (auto& audio : data["nodes"]["audio"])
        Node::ReadData(audio, false, scene);

    for (auto& empty : data["nodes"]["empty"])
        Node::ReadData(empty, false, scene);

    for (auto& text : data["nodes"]["text"])
        Node::ReadData(text, false, scene);

    for (auto& group : data["nodes"]["groups"])
        Node::ReadData(group, false, scene);

    //loaded data

    if (data.contains("spritesheets")) 
        for (const auto& spritesheet : data["spritesheets"])
            scene->spritesheets.push_back({ spritesheet["key"], spritesheet["path"] });

    //copy preloaded assets / shaders into scene

    if (data.contains("assets"))
        for (const auto& asset : data["assets"])
            scene->assets.push_back(asset);

    if (AssetManager::assets_to_build.size())
        for (const auto& asset : AssetManager::assets_to_build)
            scene->assets.push_back(asset);

    if (data.contains("shaders")) {

        for (const auto& shader : data["shaders"])
            scene->shaders.push_back(shader);

        scene->shaders_applied = true;
    }
        
    AssetManager::assets_to_build.clear();

    //global variables
    
    if (data.contains("globals")) {
        
        for (const auto& global : data["globals"])
            scene->globals.push_back({ global["key"], global["type"] });

        scene->globals_applied = true;
    }

    //scene ready for compilation

    compileQueue.push_back({ sceneKey, scene });

}



