#include <filesystem>
#include <iterator>

#include "./events.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../nodes/node.h"
#include "../../../../build/sdk/include/app.h"


using namespace editor;


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

        //decode spaghyeti file format to json

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


//-----------------------------project serialization (saving scene)


void EventListener::Serialize(json& data, bool newScene)
{

    json scenes = json::array(),
         nodes = json::array(),
         sprites,
         tilemaps,
         audio,
         text,
         empty,
         groups,
         spawns;
    
    data["icon"] = newScene ? "" : AssetManager::Get()->projectIcon;

    auto session = Editor::Get();

    data["minVersion"] = newScene ? 0 : session->minVersion;
    data["midVersion"] = newScene ? 0 : session->midVersion;
    data["maxVersion"] = newScene ? 1 : session->maxVersion;

    //camera

    data["camera"]["vignetteVisibility"] = newScene ? 0.0f : session->vignetteVisibility;
    data["camera"]["x"] = newScene ? 0.0f : session->game->camera->GetPosition()->x;
    data["camera"]["y"] = newScene ? 0.0f : session->game->camera->GetPosition()->y;
    data["camera"]["width"] = newScene ? 2000.0f : session->worldWidth;
    data["camera"]["height"] = newScene ? 2000.0f : session->worldHeight;
    data["camera"]["zoom"] = newScene ? 1.0f : *session->game->camera->GetZoom(); 
    data["camera"]["color"]["x"] = newScene ? 0.5f : session->game->camera->GetBackgroundColor()->x;
    data["camera"]["color"]["y"] = newScene ? 0.5f : session->game->camera->GetBackgroundColor()->y;
    data["camera"]["color"]["z"] = newScene ? 0.5f : session->game->camera->GetBackgroundColor()->z;
    data["camera"]["color"]["w"] = newScene ? 1.0f : session->game->camera->GetBackgroundColor()->w;
    data["camera"]["alpha"] = newScene ? 1.0f : GUI::Get()->grid->alpha;
    data["camera"]["pitch"] = newScene ? 20.0f : GUI::Get()->grid_quantity;
    data["camera"]["bounds"]["width"]["begin"] = newScene ? 0.0f : session->game->camera->currentBoundsWidthBegin;
    data["camera"]["bounds"]["width"]["end"] = newScene ? 0.0f : session->game->camera->currentBoundsWidthEnd;
    data["camera"]["bounds"]["height"]["begin"] = newScene ? 0.0f : session->game->camera->currentBoundsHeightBegin;
    data["camera"]["bounds"]["height"]["end"] = newScene ? 0.0f : session->game->camera->currentBoundsHeightEnd;

    //settings

    data["settings"]["physics"]["gravity"]["x"] = newScene ? 0.0f : session->gravityX;
    data["settings"]["physics"]["gravity"]["y"] = newScene ? 500.0f : session->gravityY;
    data["settings"]["physics"]["continuous"] = newScene ? true : session->gravity_continuous;
    data["settings"]["physics"]["sleeping"] = newScene ? true : session->gravity_sleeping;

    data["globals_applied"] = newScene ? false : session->globals_applied;
    data["shaders_applied"] = newScene ? false : session->shaders_applied;

    data["cull target name"] = newScene ? "" : session->cullTarget.first;
    data["cull target position x"] = newScene ? 0.0f : session->cullTarget.second.x;
    data["cull target position y"] = newScene ? 0.0f : session->cullTarget.second.y;

    //loaded data

    json spritesheets = json::array(),
         assets = json::array(),
         animations = json::array(),
         shaders = json::array(),
         globals = json::array();

    if (!newScene)
    {
        if (session->globals_applied)
            for (const auto& global : session->globals)
                globals.push_back({ { "key", global.first }, { "type", global.second } });

        if (AssetManager::Get()->assets.size())
            for (const auto& asset : AssetManager::Get()->assets)
                assets.push_back(asset);

        if (session->shaders_applied)
            for (const auto& shader : session->shaders)
                shaders.push_back({ { "key", shader.first }, { "vertex", shader.second.first }, { "fragment", shader.second.second } });

        if (session->spritesheets.size())
            for (const auto& spritesheet : session->spritesheets)
                spritesheets.push_back({ { "key", spritesheet.first }, { "path", spritesheet.second } });

        if (session->animations.size())
            for (const auto& animation : session->animations) 
            {
                json anims;

                for (const auto& collection : animation.second) 
                    anims.push_back({{ "key", collection.first }, { "start", collection.second.first }, { "end", collection.second.second }});
                
                animations.push_back({ { "sprite name", animation.first }, { "anims", anims } });
            }
            
        if (session->scenes.size() > 1) {
            for (int i = 0; i < session->scenes.size(); i++)
                scenes.push_back({ { "key", session->scenes[i] } });

            data["scenes"] = scenes;
        }

        for (const auto& node : Node::nodes)
        {
            if (node->type == Node::SPRITE)
                sprites.push_back(Node::WriteData(node));

            if (node->type == Node::TILEMAP)
                tilemaps.push_back(Node::WriteData(node));

            if (node->type == Node::AUDIO)
                audio.push_back(Node::WriteData(node));

            if (node->type == Node::EMPTY)
                empty.push_back(Node::WriteData(node));

            if (node->type == Node::TEXT)
                text.push_back(Node::WriteData(node));
                
            if (node->type == Node::GROUP)
                groups.push_back(Node::WriteData(node));

            if (node->type == Node::SPAWNER)
                spawns.push_back(Node::WriteData(node));
        
        }
    }

    data["spritesheets"] = spritesheets;
    data["animations"] = animations;
    data["assets"] = assets;
    data["shaders"] = shaders;
    data["globals"] = globals;
    data["nodes"]["sprites"] = sprites;
    data["nodes"]["tilemaps"] = tilemaps;
    data["nodes"]["audio"] = audio;
    data["nodes"]["empty"] = empty;
    data["nodes"]["text"] = text;
    data["nodes"]["groups"] = groups;
    data["nodes"]["spawns"] = spawns;

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

    auto session = Editor::Get();

    //version

    if (data.contains("minVersion"))
        session->minVersion = data["minVersion"];

    if (data.contains("midVersion"))
        session->midVersion = data["midVersion"];

    if (data.contains("maxVersion"))
        session->maxVersion = data["maxVersion"];

    //scenes 

    if (data["scenes"].size() > 1) //saved in queue
        for (const auto& scene : data["scenes"]) 
            session->scenes.push_back(scene["key"]);
    
    else //register opened scene
        session->scenes.push_back(session->events->s_currentScene);

    AssetManager::Get()->projectIcon = data["icon"];

    if (data.contains("cull target name") && data.contains("cull target position x") && data.contains("cull target position y")) 
        session->cullTarget = { data["cull target name"], { data["cull target position x"], data["cull target position y"] }};
    else
        session->cullTarget = { "", { 0.0f, 0.0f } };

    //camera 

    session->vignetteVisibility = data["camera"]["vignetteVisibility"];
    session->game->camera->SetPosition({ data["camera"]["x"], data["camera"]["y"] });
    session->game->camera->SetZoom(data["camera"]["zoom"]);
    session->game->camera->SetBackgroundColor({ data["camera"]["color"]["x"], data["camera"]["color"]["y"], data["camera"]["color"]["z"], data["camera"]["color"]["w"] }); 

    session->game->camera->SetBounds(
        data["camera"]["bounds"]["width"]["begin"], data["camera"]["bounds"]["width"]["end"],
        data["camera"]["bounds"]["height"]["begin"], data["camera"]["bounds"]["height"]["end"]
    );

    GUI::Get()->grid->alpha = data["camera"]["alpha"];
    GUI::Get()->grid_quantity = data["camera"]["pitch"];
    
    session->worldWidth = data["camera"]["width"];
    session->worldHeight = data["camera"]["height"];

    session->gravityX = data["settings"]["physics"]["gravity"]["x"];
    session->gravityY = data["settings"]["physics"]["gravity"]["y"];
    session->gravity_continuous = data["settings"]["physics"]["continuous"];
    session->gravity_sleeping = data["settings"]["physics"]["sleeping"];

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

    for (auto& spawn : data["nodes"]["spawns"])
        Node::ReadData(spawn, true, nullptr);

    //loaded data

    if (data.contains("spritesheets"))
        for (const auto& spritesheet : data["spritesheets"])
            session->spritesheets.push_back({ spritesheet["key"], spritesheet["path"] });

    if (data.contains("assets"))
        for (const auto& asset : data["assets"]) 
            AssetManager::Get()->Register(asset);

    if (data.contains("shaders"))
        for (const auto& shader : data["shaders"]) {
            session->shaders.push_back({ shader["key"], { shader["vertex"], shader["fragment"] } });
            session->shaders_applied = true;
        }

    if (data.contains("animations"))
        for (const auto& animation : data["animations"]) 
        { 
            const auto key_it = std::find_if(session->animations.begin(), session->animations.end(), [&](const auto& anim){ return anim.first == animation["sprite name"]; });  
            std::vector<std::pair<std::string, std::pair<int, int>>> anims;
           
            for (const auto& collection : animation["anims"]) 
                anims.push_back({ collection["key"], { collection["start"], collection["end"] }});
            
            if (key_it == session->animations.end()) {
                session->animations.push_back({ animation["sprite name"], anims });
                Component::ApplyAnimations(true);
            } 
        }

    //global variables
    
    if (data.contains("globals")) {
        for (const auto& global : data["globals"])
            session->globals.push_back({ global["key"], global["type"] });

        if (data["globals_applied"]) 
            session->globals_applied = true;
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

    if (data.contains("cull target name") && data.contains("cull target position x") && data.contains("cull target position y")) 
        scene->cullTarget = { data["cull target name"], { data["cull target position x"], data["cull target position y"] }};

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

    for (auto& spawn : data["nodes"]["spawns"])
        Node::ReadData(spawn, false, scene);

    //loaded data

    if (data.contains("spritesheets")) 
        for (const auto& spritesheet : data["spritesheets"])
            scene->spritesheets.push_back({ spritesheet["key"], spritesheet["path"] });

    if (data.contains("animations")) 
        for (const auto& animation : data["animations"]) {
            std::vector<std::pair<std::string, std::pair<int, int>>> anims;

            for (const auto& collection : animation["anims"])
                anims.push_back({ collection["key"], { collection["start"], collection["end"] }});

            scene->animations.push_back({ animation["sprite name"], anims });
        }

    //copy preloaded assets / shaders into scene

    if (data.contains("assets"))
       for (const auto& asset : data["assets"])
           scene->assets.push_back(asset);

    if (data.contains("shaders")) {

        for (const auto& shader : data["shaders"])
            scene->shaders.push_back({ shader["key"], { shader["vertex"], shader["fragment"] } });

        scene->shaders_applied = true;
    }

    //global variables
    
    if (data.contains("globals")) {
        
        for (const auto& global : data["globals"])
            scene->globals.push_back({ global["key"], global["type"] });

        scene->globals_applied = true;
    }

    //scene ready for compilation 

    compileQueue.push_back({ sceneKey, scene });

}



