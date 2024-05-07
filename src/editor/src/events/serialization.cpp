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


void EventListener::Serialize(json& data)
{

    json nodes;

    json sprites;
    json tilemaps;
    json audio;
    json text;
    json empty;

    //camera

    data["camera"]["vignetteVisibility"] = Editor::vignetteVisibility;
    data["camera"]["x"] = Editor::game->camera->position.x;
    data["camera"]["y"] = Editor::game->camera->position.y;
    data["camera"]["width"] = Editor::worldWidth;
    data["camera"]["height"] = Editor::worldHeight;
    data["camera"]["zoom"] = Editor::game->camera->zoom; 
    data["camera"]["color"]["x"] = Editor::game->camera->backgroundColor.x;
    data["camera"]["color"]["y"] = Editor::game->camera->backgroundColor.y;
    data["camera"]["color"]["z"] = Editor::game->camera->backgroundColor.z;
    data["camera"]["color"]["w"] = Editor::game->camera->backgroundColor.w;
    data["camera"]["alpha"] = GUI::s_grid->alpha;
    data["camera"]["pitch"] = GUI::s_grid_quantity;
    data["camera"]["bounds"]["width"]["begin"] = Editor::game->camera->currentBoundsWidthBegin;
    data["camera"]["bounds"]["width"]["end"] = Editor::game->camera->currentBoundsWidthEnd;
    data["camera"]["bounds"]["height"]["begin"] = Editor::game->camera->currentBoundsHeightBegin;
    data["camera"]["bounds"]["height"]["end"] = Editor::game->camera->currentBoundsHeightEnd;

    //settings

    data["settings"]["physics"]["gravity"]["x"] = Editor::gravityX;
    data["settings"]["physics"]["gravity"]["y"] = Editor::gravityY;
    data["settings"]["physics"]["continuous"] = Editor::gravity_continuous;
    data["settings"]["physics"]["sleeping"] = Editor::gravity_sleeping;

    json globals = json::array();

    data["globals_applied"] = Editor::globals_applied;
 
    if (Editor::globals_applied)
        for (const auto& global : Editor::globals)
            globals.push_back({{ "key", global.first }, { "type", global.second } });

    data["globals"] = globals;

    //nodes

    for (const auto& node : Node::nodes)
    {

        if (node->type == "Sprite")
            sprites.push_back(Node::WriteData(node, "Sprite"));

        if (node->type == "Tilemap")
            tilemaps.push_back(Node::WriteData(node, "Tilemap"));

        if (node->type == "Audio")
            audio.push_back(Node::WriteData(node, "Audio"));

        if (node->type == "Empty")
            empty.push_back(Node::WriteData(node, "Empty"));

        if (node->type == "Text")
            text.push_back(Node::WriteData(node, "Text"));
    }

    //embed node data

    nodes = json::array();

    data["nodes"]["sprites"] = sprites;
    data["nodes"]["tilemaps"] = tilemaps;
    data["nodes"]["audio"] = audio;
    data["nodes"]["empty"] = empty;
    data["nodes"]["text"] = text;
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

    //camera 

    Editor::vignetteVisibility = data["camera"]["vignetteVisibility"];
    Editor::game->camera->position.x = data["camera"]["x"];
    Editor::game->camera->position.y = data["camera"]["y"];
    Editor::game->camera->zoom = data["camera"]["zoom"];
    Editor::game->camera->backgroundColor.x = data["camera"]["color"]["x"];
    Editor::game->camera->backgroundColor.y = data["camera"]["color"]["y"];
    Editor::game->camera->backgroundColor.z = data["camera"]["color"]["z"];
    Editor::game->camera->backgroundColor.w = data["camera"]["color"]["w"]; 

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

    //global variables
    
    if (data["globals"].size())
    {
        for (const auto& global : data["globals"])
            Editor::globals.push_back({ global["key"], global["type"] });

        if (data["globals_applied"])
            Editor::globals_applied = true;
    }

    //sprites

    for (auto& sprite : data["nodes"]["sprites"])
        Node::ReadData(sprite, "Sprite", true, nullptr);

    //tilemaps

    for (auto& tilemap : data["nodes"]["tilemaps"])
        Node::ReadData(tilemap, "Tilemap", true, nullptr);

    //audio

    for (auto& audio : data["nodes"]["audio"])
        Node::ReadData(audio, "Audio", true, nullptr);

    //empty

    for (auto& empty : data["nodes"]["empty"])
        Node::ReadData(empty, "Empty", true, nullptr);

    //text

    for (auto& text : data["nodes"]["text"])
        Node::ReadData(text, "Text", true, nullptr);

    Editor::Log("Project " + currentProject + " opened.\nProject root path set: " + Editor::projectPath);

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

    //global variables
    
    if (data["globals"].size()) {
        
        for (const auto& global : data["globals"])
            scene->globals.push_back({ global["key"], global["type"] });

        scene->globals_applied = true;
    }

    //sprites

    for (auto& sprite : data["nodes"]["sprites"])
        Node::ReadData(sprite, "Sprite", false, scene);

    //tilemaps

    for (auto& tilemap : data["nodes"]["tilemaps"])
        Node::ReadData(tilemap, "Tilemap", false, scene);

    //audio

    for (auto& audio : data["nodes"]["audio"])
        Node::ReadData(audio, "Audio", false, scene);

    //empty

    for (auto& empty : data["nodes"]["empty"])
        Node::ReadData(empty, "Empty", false, scene);

    //text

    for (auto& text : data["nodes"]["text"])
        Node::ReadData(text, "Text", false, scene);

    //scene ready for compilation

    compileQueue.insert({ sceneKey, scene });

 
}



