#include <filesystem>
#include <iterator>

#include "./events.h"
#include "../scene.h"
#include "../../../../build/include/app.h"

using namespace editor;


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


//---------------------------------------

//extracts saved data from scene to compile
void EventListener::ParseScene(const std::string& sceneKey, std::ifstream& JSON)
{

    editor::Scene scene;

    json data = json::parse(JSON);

    //camera

    scene.cameraPosition.x = data["camera"]["x"];
    scene.cameraPosition.y = data["camera"]["y"];
    scene.cameraZoom = data["camera"]["zoom"];
    scene.cameraBackgroundColor.x = data["camera"]["color"]["x"];
    scene.cameraBackgroundColor.y = data["camera"]["color"]["y"];
    scene.cameraBackgroundColor.z = data["camera"]["color"]["z"];
    scene.cameraBackgroundColor.w = data["camera"]["color"]["w"]; 

    scene.currentBoundsWidthBegin = data["camera"]["bounds"]["width"]["begin"];
    scene.currentBoundsWidthEnd = data["camera"]["bounds"]["width"]["end"];
    scene.currentBoundsHeightBegin = data["camera"]["bounds"]["height"]["begin"];
    scene.currentBoundsHeightEnd = data["camera"]["bounds"]["height"]["end"];

    scene.GUIGridAlpha = data["camera"]["alpha"];
    scene.GUIGridQuantity = data["camera"]["pitch"];
    
    scene.worldWidth = data["camera"]["width"];
    scene.worldHeight = data["camera"]["height"];

    scene.gravityX = data["settings"]["physics"]["gravity"]["x"];
    scene.gravityY = data["settings"]["physics"]["gravity"]["y"];
    scene.gravity_continuous = data["settings"]["physics"]["continuous"];
    scene.gravity_sleeping = data["settings"]["physics"]["sleeping"];

    //global variables
    
    if (data["globals"].size())
        for (const auto& global : data["globals"])
            scene.globals.push_back({ global["key"], global["type"] });

    //sprites

    for (const auto& sprite : data["nodes"]["sprites"])
    {

        auto sn = Scene::MakeNode<SpriteNode>(&scene); 

        sn->m_ID = sprite["ID"];
        sn->m_name = sprite["name"];
        sn->positionX = sprite["positionX"];
        sn->positionY = sprite["positionY"];
        sn->rotation = sprite["rotation"];
        sn->scaleX = sprite["scaleX"];
        sn->scaleY = sprite["scaleY"];
        sn->flippedX = sprite["flipX"];
        sn->flippedY = sprite["flipY"];
        sn->depth = sprite["depth"];
        sn->lock_in_place = sprite["lock"];

        sn->ApplyTexture(sprite["currentTexture"]);  

        sn->spriteHandle->m_texture.U1 = sprite["U1"];
        sn->spriteHandle->m_texture.V1 = sprite["V1"];
        sn->spriteHandle->m_texture.U2 = sprite["U2"]; 
        sn->spriteHandle->m_texture.V2 = sprite["V2"];
        sn->spriteHandle->m_key = sprite["currentTexture"];
        sn->spriteHandle->m_tint.x = sprite["m_tint"]["x"];
        sn->spriteHandle->m_tint.y = sprite["m_tint"]["y"];
        sn->spriteHandle->m_tint.z = sprite["m_tint"]["z"];
        sn->spriteHandle->m_alpha = sprite["m_alpha"];

        sn->spriteHandle->SetTexture(sprite["currentTexture"]);

        for (const auto& frame : sprite["frames"]) {
            sn->frameBuf1.push_back(frame["offset x"]);
            sn->frameBuf2.push_back(frame["offset y"]);
            sn->frameBuf3.push_back(frame["width"]);
            sn->frameBuf4.push_back(frame["height"]);
            sn->frameBuf5.push_back(frame["factor x"]);
            sn->frameBuf6.push_back(frame["factor y"]);
            sn->frames.push_back({ frame["offset x"], frame["offset y"], frame["width"], frame["height"], frame["factor x"], frame["factor y"] });
        }

        sn->frame = sn->frames.size();
    
        std::vector<std::array<int, 6>> framesToPush;

        for (const auto& frame : sn->frames)
            framesToPush.push_back({ frame.x, frame.y, frame.width, frame.height, frame.factorX, frame.factorY });

        System::Resources::Manager::UnLoadFrames(sn->spriteHandle->m_key);
        System::Resources::Manager::LoadFrames(sn->spriteHandle->m_key, framesToPush);

        if (sn->frames.size() > 1) {
            sn->framesApplied = true; 
            sn->spriteHandle->ReadSpritesheetData();
            sn->spriteHandle->SetFrame(0);
        }

        //animator

        if (sprite["components"]["animator"]["exists"])
            sn->AddComponent("Animator");

        for (const auto& anim : sprite["components"]["animator"]["animations"]) 
        { 

            SpriteNode::StringContainer sc = { anim["key"] };

            sn->animBuf1.push_back(sc);
            sn->animBuf2.push_back(anim["start"]); 
            sn->animBuf3.push_back(anim["end"]);
        
            sn->ApplyAnimation(anim["key"], anim["start"], anim["end"]);
        }

        sn->anim = sn->animations.size();   

        //physics 

        if (sprite["components"]["physics"]["exists"]) 
        {
            sn->AddComponent("Physics", false);
            sn->friction = sprite["components"]["physics"]["friction"];
            sn->restitution = sprite["components"]["physics"]["restitution"];
            sn->density = sprite["components"]["physics"]["density"]; 

            if (sprite["components"]["physics"]["bodies"].size())
                for (const auto &body : sprite["components"]["physics"]["bodies"]) 
                    sn->CreateBody(
                        static_cast<std::string>(body["type"]).c_str(), 
                        body["bodyX"], 
                        body["bodyY"], 
                        body["body_width"], 
                        body["body_height"],
                        body["sensor"], 
                        body["pointer"]
                    );
        }

        //script

        if (sprite["components"]["script"]["exists"]) {
            
            sn->AddComponent("Script", false);

            if (sprite["components"]["script"]["scripts"].size())
                for (const auto &scripts : sprite["components"]["script"]["scripts"])
                    sn->behaviors.insert({ static_cast<std::string>(scripts["key"]).c_str(), static_cast<std::string>(scripts["value"]).c_str() });
        }

        //shader

        if (sprite["components"]["shader"]["exists"]) {

            sn->AddComponent("Shader", false); 

            if (sprite["components"]["shader"]["shaders"].size())
                Node::LoadShader(sn, 
                    static_cast<std::string>(sprite["components"]["shader"]["shaders"]["key"]).c_str(),
                    static_cast<std::string>(sprite["components"]["shader"]["shaders"]["vertex"]).c_str(),
                    static_cast<std::string>(sprite["components"]["shader"]["shaders"]["fragment"]).c_str()
                );
        }

    }

    //tilemaps

    for (const auto& tilemap : data["nodes"]["tilemaps"])
    {

        auto tmn = Scene::MakeNode<TilemapNode>(&scene); 

        tmn->m_ID = tilemap["ID"];
        tmn->m_name = tilemap["name"];

        tmn->map_width = tilemap["map_width"];
        tmn->map_height = tilemap["map_height"];
        tmn->tile_width = tilemap["tile_width"];
        tmn->tile_height = tilemap["tile_height"];
        tmn->layer = tilemap["layer"];

        if (tilemap["layers"].size())
            for (const auto &layer : tilemap["layers"]) {
                tmn->layers.push_back({ layer["csv"]["key"], layer["csv"]["path"], layer["csv"]["texture"] });
                tmn->spr_sheet_width.push_back(layer["frames x"]);
                tmn->spr_sheet_height.push_back(layer["frames y"]);
                tmn->depth.push_back(layer["depth"]);
            }

        //physics 
        
        if (tilemap["components"]["physics"]["exists"]) 
            tmn->AddComponent("Physics", false);

        if (tilemap["components"]["physics"]["bodies"].size())
            for (const auto& body : tilemap["components"]["physics"]["bodies"]) 
                tmn->CreateBody(body["bodyX"], body["bodyY"], body["body_width"], body["body_height"]);
        
        if (tilemap["layers"].size())
            tmn->ApplyTilemap();
    }

    //audio

    for (const auto& audio : data["nodes"]["audio"])
    {

        auto an = Scene::MakeNode<AudioNode>(&scene); 

        an->m_ID = audio["ID"];
        an->m_name = audio["name"];
        an->audio_source_name = audio["source name"];
        an->volume = audio["volume"];
        an->loop = audio["loop"];

    }

    //empty

    for (const auto& empty : data["nodes"]["empty"])
    {

        auto en = Scene::MakeNode<EmptyNode>(&scene); 

        en->show_debug = empty["debug graphics"]; 
        en->debug_fill = empty["fill"];
        en->rectWidth = empty["width"];  
        en->rectHeight = empty["height"]; 
        en->radius = empty["radius"]; 
        en->positionX = empty["position x"]; 
        en->positionY = empty["position y"];

        if (static_cast<std::string>(empty["shape"]).length()) {

            en->CreateShape(empty["shape"]);

            if (en->m_debugGraphic) {
                en->m_debugGraphic->m_tint.x = empty["m_tint"]["x"];
                en->m_debugGraphic->m_tint.y = empty["m_tint"]["y"];
                en->m_debugGraphic->m_tint.z = empty["m_tint"]["z"];
                en->m_debugGraphic->m_alpha = empty["m_alpha"];
            }
        }

        //script

        if (empty["components"]["script"]["exists"]) {

            en->AddComponent("Script", false);

            if (empty["components"]["script"]["scripts"].size())
                for (const auto &scripts : empty["components"]["script"]["scripts"])
                    en->behaviors.insert({ static_cast<std::string>(scripts["key"]), static_cast<std::string>(scripts["value"]) });
        }

        //shader

        if (empty["components"]["shader"]["exists"]) {
            
            en->AddComponent("Shader", false);

            if (empty["components"]["shader"]["shaders"].size())
                Node::LoadShader(en, 
                    static_cast<std::string>(empty["components"]["shader"]["shaders"]["key"]).c_str(),
                    static_cast<std::string>(empty["components"]["shader"]["shaders"]["vertex"]).c_str(),
                    static_cast<std::string>(empty["components"]["shader"]["shaders"]["fragment"]).c_str()
                );
        }
        

    }

    //text

    for (const auto& text : data["nodes"]["text"])
    {

        auto tn = Scene::MakeNode<TextNode>(&scene); 

        tn->m_ID = text["ID"];
        tn->m_name = text["name"];

        tn->textBuf = text["content"];
        tn->tint = glm::vec3(text["tint"]["x"], text["tint"]["y"], text["tint"]["z"]);     
        tn->alpha = text["alpha"];       
        tn->positionX = text["position x"];        
        tn->positionY = text["position y"];   
        tn->rotation = text["rotation"];      
        tn->scaleX = text["scale x"];      
        tn->scaleY = text["scale y"];
        tn->depth = text["depth"];    

        //script

        if (text["components"]["script"]["exists"]) 
        {
            tn->AddComponent("Script", false);

            if (text["components"]["script"]["scripts"].size())
                for (const auto &scripts : text["components"]["script"]["scripts"])
                    tn->behaviors.insert({ static_cast<std::string>(scripts["key"]).c_str(), static_cast<std::string>(scripts["value"]).c_str() });

        }

    }

    compileQueue.insert({ sceneKey, scene });

 
}



