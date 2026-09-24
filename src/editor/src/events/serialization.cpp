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

    //get string contents next to class keyword

    while (src >> line)
        if (line == "class")  
            if (src >> line)
                scriptName = line;
    
    return scriptName;  
}


//--------------------------------------- parse data


json EventListener::ParseJSONStream(std::stringstream& stream, int index)
{
    if (!stream.good()) 
    {
        Editor::Log("There was a problem reading the stream.");

        json empty_data;
        return empty_data;
    }

    return json::parse(stream);
}


//--------------------------------------- parse animation data


std::vector<Sprite::Anim> ParseAnimationData(json& JSON) 
{
    std::vector<Sprite::Anim> anims;

    for (const auto& animation : JSON) 
    {
        const std::string key = animation.contains("key") ? animation["key"] : "";
        
        int start = animation.contains("start") ? static_cast<int>(animation["start"]) : 0, 
            end = animation.contains("end") ? static_cast<int>(animation["end"]) : 0;
                            
        Sprite::Anim anim;

        anim.key = key; 
        anim.start = start; 
        anim.end = end;

        anims.emplace_back(anim);
    }

    return anims;
}
 

//----------------------------------- encode JSON to spaghyeti format


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


//----------------------------------- decode spaghyeti format to JSON


std::stringstream EventListener::DecodeFile(const std::filesystem::path& path)
{
    std::string line;

    std::ifstream in_file(path.string());
    std::stringstream stream;

    while (getline(in_file, line))
    {
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

        stream << line << "\n";
    }

    in_file.close();

    return stream;
}


//-----------------------------project serialization (saving scene from session data to JSON format)


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
    data["camera"]["color"]["x"] = newScene ? 0.5f : session->game->camera->GetBackgroundColor()->r;
    data["camera"]["color"]["y"] = newScene ? 0.5f : session->game->camera->GetBackgroundColor()->g;
    data["camera"]["color"]["z"] = newScene ? 0.5f : session->game->camera->GetBackgroundColor()->b;
    data["camera"]["color"]["w"] = newScene ? 1.0f : session->game->camera->GetBackgroundColor()->a;
    data["camera"]["alpha"] = newScene ? 1.0f : GUI::Get()->grid->alpha;
    data["camera"]["pitch"] = newScene ? 20.0f : GUI::Get()->grid_quantity;
    data["camera"]["grid tint"]["r"] = newScene ? 0.25f : GUI::Get()->grid_color.x;
    data["camera"]["grid tint"]["g"] = newScene ? 0.25f : GUI::Get()->grid_color.y;
    data["camera"]["grid tint"]["b"] = newScene ? 0.25f : GUI::Get()->grid_color.z;
    data["camera"]["bounds"]["width"]["begin"] = newScene ? 0.0f : session->game->camera->currentBoundsWidthBegin;
    data["camera"]["bounds"]["width"]["end"] = newScene ? 0.0f : session->game->camera->currentBoundsWidthEnd;
    data["camera"]["bounds"]["height"]["begin"] = newScene ? 0.0f : session->game->camera->currentBoundsHeightBegin;
    data["camera"]["bounds"]["height"]["end"] = newScene ? 0.0f : session->game->camera->currentBoundsHeightEnd;
    data["camera"]["depth sort"] = newScene ? false : session->depthSort;

    //settings

    data["settings"]["physics"]["gravity"]["x"] = newScene ? 0.0f : session->gravityX;
    data["settings"]["physics"]["gravity"]["y"] = newScene ? 500.0f : session->gravityY;
    data["settings"]["physics"]["continuous"] = newScene ? true : session->gravity_continuous;
    data["settings"]["physics"]["sleeping"] = newScene ? true : session->gravity_sleeping;

    data["globals_applied"] = newScene ? false : session->globals_applied;
    data["shaders_applied"] = newScene ? false : session->shaders_applied;

    //loaded data

    json spritesheets = json::array(),
         assets = json::array(),
         animators = json::array(),
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
                shaders.push_back({ { "key", shader.key }, { "vertex", shader.vertex }, { "fragment", shader.fragment }, { "depth", shader.depth } });

        if (session->spritesheets.size())
            for (const auto& spritesheet : session->spritesheets)
                spritesheets.push_back({ { "key", spritesheet.first }, { "path", spritesheet.second } });

        if (session->animators.size())
            for (const auto& animator : session->animators) 
            {
                json anims;

                for (const auto& animation : animator.animations) 
                    anims.push_back({{ "key", animation.key }, { "start", animation.start }, { "end", animation.end }});
                
                animators.push_back({ { "sprite name", animator.textureKey }, { "anims", anims } });
            }
            
        if (session->scenes.size() > 1) {
            for (const auto& scene : session->scenes)
                scenes.push_back({ { "key", scene } });
 
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
    data["animations"] = animators;
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


void EventListener::Deserialize(json& data, bool isSession)
{    
    auto session = Editor::Get();

    //version

    if (data.contains("minVersion")) 
        session->minVersion = data["minVersion"];

    if (data.contains("midVersion"))
        session->midVersion = data["midVersion"];

    if (data.contains("maxVersion"))
        session->maxVersion = data["maxVersion"];

    //scenes 

    session->scenes.clear();

    if (data.contains("scenes") && data["scenes"].size() > 1) //saved in queue
        for (const auto& scene : data["scenes"]) 
            if (scene.contains("key"))
                session->scenes.push_back(scene["key"]); 
        
    else //register opened scene
        session->scenes.push_back(session->events->s_currentScene);

    if (data.contains("icon"))
        AssetManager::Get()->projectIcon = data["icon"];

    //camera 

    if (data.contains("camera"))
    {
        session->vignetteVisibility = data["camera"].contains("vignetteVisibility") ? static_cast<float>(data["camera"]["vignetteVisibility"]) : 0.0f;

        float cameraX = data["camera"].contains("x") ? static_cast<float>(data["camera"]["x"]) : 0.0f,
              cameraY = data["camera"].contains("y") ? static_cast<float>(data["camera"]["y"]) : 0.0f,
              zoom = data["camera"].contains("zoom") ? static_cast<float>(data["camera"]["zoom"]) : 0.0f,
              red = data["camera"]["color"].contains("x") ? static_cast<float>(data["camera"]["color"]["x"]) : 0.0f,
              green = data["camera"]["color"].contains("y") ? static_cast<float>(data["camera"]["color"]["y"]) : 0.0f,
              blue = data["camera"]["color"].contains("z") ? static_cast<float>(data["camera"]["color"]["z"]) : 0.0f,
              alpha = data["camera"]["color"].contains("w") ? static_cast<float>(data["camera"]["color"]["w"]) : 0.0f;
              
        session->game->camera->SetPosition({ cameraX, cameraY });
        session->game->camera->SetZoom(zoom);
        session->game->camera->SetBackgroundColor({ red, green, blue, alpha }); 

        if (data["camera"].contains("bounds") && data["camera"]["bounds"].contains("width") && data["camera"]["bounds"].contains("height")) 
        {
            float wBegin = data["camera"]["bounds"]["width"].contains("begin") ? static_cast<float>(data["camera"]["bounds"]["width"]["begin"]) : 0.0f, 
                  wEnd = data["camera"]["bounds"]["width"].contains("begin") ? static_cast<float>(data["camera"]["bounds"]["width"]["begin"]) : 0.0f,
                  hBegin = data["camera"]["bounds"]["height"].contains("begin") ? static_cast<float>(data["camera"]["bounds"]["width"]["begin"]) : 0.0f,
                  hEnd = data["camera"]["bounds"]["height"].contains("begin") ? static_cast<float>(data["camera"]["bounds"]["width"]["begin"]) : 0.0f;
            
            session->game->camera->SetBounds(wBegin, wEnd, hBegin, hEnd);
        }

        GUI::Get()->grid->alpha = data["camera"].contains("alpha") ? static_cast<float>(data["camera"]["alpha"]) : 0.0f;
        GUI::Get()->grid_quantity = data["camera"].contains("pitch") ? static_cast<float>(data["camera"]["pitch"]) : 0.0f;

        if (data["camera"].contains("grid tint"))
        {
            float r = data["camera"]["grid tint"].contains("r") ? static_cast<float>(data["camera"]["grid tint"]["r"]) : 0.0f, 
                  g = data["camera"]["grid tint"].contains("g") ? static_cast<float>(data["camera"]["grid tint"]["g"]) : 0.0f,
                  b = data["camera"]["grid tint"].contains("b") ? static_cast<float>(data["camera"]["grid tint"]["b"]) : 0.0f;

            GUI::Get()->grid_color = { r, g, b }; 
        }
        
        session->worldWidth = data["camera"].contains("width") ? static_cast<float>(data["camera"]["width"]) : 0.0f;
        session->worldHeight = data["camera"].contains("height") ? static_cast<float>(data["camera"]["height"]) : 0.0f;

        if (data["camera"].contains("depth sort"))
            session->depthSort = data["camera"]["depth sort"];
    }

    //physics

    if (data.contains("settings") && data["settings"].contains("physics")) 
    {
        if (data["settings"]["physics"].contains("gravity")) {
            session->gravityX = data["settings"]["physics"]["gravity"].contains("x") ? static_cast<float>(data["settings"]["physics"]["gravity"]["x"]) : 0.0f;
            session->gravityY = data["settings"]["physics"]["gravity"].contains("y") ? static_cast<float>(data["settings"]["physics"]["gravity"]["y"]) : 0.0f;
        }

        session->gravity_continuous = data["settings"]["physics"].contains("continuous") ? static_cast<bool>(data["settings"]["physics"]["continuous"]) : false;
        session->gravity_sleeping = data["settings"]["physics"].contains("sleeping") ? static_cast<bool>(data["settings"]["physics"]["sleeping"]) : false;
    }

    //spritesheets

    session->spritesheets.clear();

    if (data.contains("spritesheets"))
        for (const auto& spritesheet : data["spritesheets"]) {
            const std::string key = spritesheet.contains("key") ? spritesheet["key"] : "", 
                        path = spritesheet.contains("path") ? spritesheet["path"] : "";
            session->spritesheets.push_back({ key, path });
        }

    if (data.contains("assets"))
        for (const auto& asset : data["assets"]) 
            AssetManager::Get()->Register(asset);

    //shaders

    session->shaders.clear();
    session->shaders.push_back({ "sprite", "", "", 0 });

    if (data.contains("shaders"))
        for (const auto& shader : data["shaders"]) 
        {
            const std::string key = shader.contains("key") ? shader["key"] : "", 
                              vertex = shader.contains("vertex") ? shader["vertex"] : "none selected",
                              fragment = shader.contains("fragment") ? shader["fragment"] : "none selected";

            int depth = shader.contains("depth") ? static_cast<int>(shader["depth"]) : 0;

            session->shaders.push_back({ key, vertex, fragment, depth });
            session->shaders_applied = true;
        }

    //animations

    session->animators.clear();

    if (data.contains("animations"))
        for (auto& animator : data["animations"]) 
        { 
            if (!animator.contains("sprite name"))
                continue;

            const auto key_it = std::find_if(session->animators.begin(), session->animators.end(), [&](const auto& a){ return a.textureKey == animator["sprite name"]; });  
           
            if (animator.contains("anims")) 
            {
                const auto anims = ParseAnimationData(animator["anims"]);

                if (key_it == session->animators.end()) {
                    session->animators.push_back({ animator["sprite name"], anims });
                    Component::ApplyAnimations(true);
                } 
            }
        }

    //global variables

    session->globals.clear();
    
    if (data.contains("globals")) {
        for (const auto& global : data["globals"]) {
            const std::string key = global.contains("key") ? global["key"] : "", 
                              type = global.contains("type") ? global["type"] : "";
            session->globals.push_back({ key, type });
        }

        if (data["globals_applied"]) 
            session->globals_applied = true;
    }

    //nodes

    if (data.contains("nodes")) 
    {
        if (data["nodes"].contains("sprites"))
            for (auto& sprite : data["nodes"]["sprites"])
                Node::ReadData(sprite, true, nullptr);

        if (data["nodes"].contains("tilemaps"))
            for (auto& tilemap : data["nodes"]["tilemaps"])
                Node::ReadData(tilemap, true, nullptr);

        if (data["nodes"].contains("audio"))
            for (auto& audio : data["nodes"]["audio"])
                Node::ReadData(audio, true, nullptr);

        if (data["nodes"].contains("empty"))
            for (auto& empty : data["nodes"]["empty"])
                Node::ReadData(empty, true, nullptr);

        if (data["nodes"].contains("text"))
            for (auto& text : data["nodes"]["text"])
                Node::ReadData(text, true, nullptr);

        if (data["nodes"].contains("groups"))
            for (auto& group : data["nodes"]["groups"])
                Node::ReadData(group, true, nullptr);

        if (data["nodes"].contains("spawns"))
            for (auto& spawn : data["nodes"]["spawns"])
                Node::ReadData(spawn, true, nullptr);
    }

}



//-------------------------------------------------

    
//extracts saved data from scene to compile
void EventListener::ParseScene(const std::string& sceneKey, std::stringstream& stream)
{
    auto scene = new editor::Scene;

    json data = json::parse(stream);

    stream.seekg(0);

    //camera

    if (data.contains("camera"))
    {
        scene->vignetteVisibility = data["camera"].contains("vignetteVisibility") ? static_cast<float>(data["camera"]["vignetteVisibility"]) : 0.0f;
        scene->cameraPosition.x = data["camera"].contains("x") ? static_cast<float>(data["camera"]["x"]) : 0.0f;
        scene->cameraPosition.y = data["camera"].contains("y") ? static_cast<float>(data["camera"]["y"]) : 0.0f;
        scene->cameraZoom = data["camera"].contains("zoom") ? static_cast<float>(data["camera"]["zoom"]) : 0.0f;

        if (data["camera"].contains("color")) {
            scene->cameraBackgroundColor.r = data["camera"]["color"].contains("x") ? static_cast<float>(data["camera"]["color"]["x"]) : 0.0f;
            scene->cameraBackgroundColor.g = data["camera"]["color"].contains("y") ? static_cast<float>(data["camera"]["color"]["y"]) : 0.0f;
            scene->cameraBackgroundColor.b = data["camera"]["color"].contains("z") ? static_cast<float>(data["camera"]["color"]["z"]) : 0.0f;
            scene->cameraBackgroundColor.a = data["camera"]["color"].contains("w") ? static_cast<float>(data["camera"]["color"]["w"]) : 0.0f;
        }

        if (data["camera"].contains("bounds")) 
        {
            if (data["camera"]["bounds"].contains("width")) {
                scene->currentBoundsWidthBegin = data["camera"]["bounds"]["width"].contains("begin") ? static_cast<float>(data["camera"]["bounds"]["width"]["begin"]) : 0.0f;
                scene->currentBoundsWidthEnd = data["camera"]["bounds"]["width"].contains("end") ? static_cast<float>(data["camera"]["bounds"]["width"]["end"]) : 0.0f;
            }

            if (data["camera"]["bounds"].contains("height")) {
                scene->currentBoundsHeightBegin = data["camera"]["bounds"]["height"].contains("begin") ? static_cast<float>(data["camera"]["bounds"]["height"]["begin"]) : 0.0f;
                scene->currentBoundsHeightEnd = data["camera"]["bounds"]["height"].contains("end") ? static_cast<float>(data["camera"]["bounds"]["height"]["end"]) : 0.0f;
            }
        }
        
        scene->worldWidth = data["camera"].contains("width") ? static_cast<int>(data["camera"]["width"]) : 0.0f;
        scene->worldHeight = data["camera"].contains("height") ? static_cast<int>(data["camera"]["height"]) : 0.0f;

        if (data["camera"].contains("depth sort"))
            scene->depthSort = data["camera"]["depth sort"];
    }

    //settings

    if (data.contains("settings") && data["settings"].contains("physics")); 
    {
        if (data["settings"]["physics"].contains("gravity")) {
            scene->gravityX = data["settings"]["physics"]["gravity"].contains("x") ? static_cast<float>(data["settings"]["physics"]["gravity"]["x"]) : 0.0f;
            scene->gravityY = data["settings"]["physics"]["gravity"].contains("y") ? static_cast<float>(data["settings"]["physics"]["gravity"]["y"]) : 0.0f;
        }
        
        scene->gravity_continuous = data["settings"]["physics"].contains("continuous") ? static_cast<bool>(data["settings"]["physics"]["continuous"]) : false;
        scene->gravity_sleeping = data["settings"]["physics"].contains("sleeping") ? static_cast<bool>(data["settings"]["physics"]["sleeping"]) : false;
    }

    //loaded data

    if (data.contains("spritesheets")) 
        for (const auto& spritesheet : data["spritesheets"]) 
        {
            const std::string key = spritesheet.contains("key") ? spritesheet["key"] : "", 
                              path = spritesheet.contains("path") ? spritesheet["path"] : "";

            scene->spritesheets.push_back({ key, path });
        }

    if (data.contains("animations")) 
        for (auto& animator : data["animations"]) {
            if (animator.contains("anims")) {
                const auto anims = ParseAnimationData(animator["anims"]); 
                scene->animators.push_back({ animator["sprite name"], anims });
            }
        }

    //copy preloaded assets / shaders into scene

    if (data.contains("assets"))
       for (const auto& asset : data["assets"])
           scene->assets.push_back(asset);

    if (data.contains("shaders")) {
        for (const auto& shader : data["shaders"]) {
            const std::string key = shader.contains("key") ? shader["key"] : "", 
                              vertex = shader.contains("vertex") ? shader["vertex"] : "",
                              fragment = shader.contains("fragment") ? shader["fragment"] : "";

            scene->shaders.push_back({ key, vertex, fragment, scene->shaders.size() });
        }

        scene->shaders_applied = true;
    }

    //global variables
    
    if (data.contains("globals")) {
        for (const auto& global : data["globals"]) {
            const std::string key = global.contains("key") ? global["key"] : "", 
                              type = global.contains("type") ? global["type"] : "";

            scene->globals.push_back({ key, type });
        }
            
        scene->globals_applied = true;
    }

    if (data.contains("nodes"))
    {
        if (data["nodes"].contains("sprites"))
            for (auto& sprite : data["nodes"]["sprites"])
                Node::ReadData(sprite, false, scene);

        if (data["nodes"].contains("tilemaps"))
            for (auto& tilemap : data["nodes"]["tilemaps"])
                Node::ReadData(tilemap, false, scene);

        if (data["nodes"].contains("audio"))
            for (auto& audio : data["nodes"]["audio"])
                Node::ReadData(audio, false, scene);

        if (data["nodes"].contains("empty"))
            for (auto& empty : data["nodes"]["empty"])
                Node::ReadData(empty, false, scene);

        if (data["nodes"].contains("text"))        
            for (auto& text : data["nodes"]["text"])
                Node::ReadData(text, false, scene);

        if (data["nodes"].contains("groups"))
            for (auto& group : data["nodes"]["groups"])
                Node::ReadData(group, false, scene);

        if (data["nodes"].contains("spawns"))
            for (auto& spawn : data["nodes"]["spawns"])
                Node::ReadData(spawn, false, scene);
    }

    //scene ready for compilation 

    Editor::Get()->events->compileQueue.push_back({ sceneKey, scene });

}



