#include <filesystem>
#include <iterator>

#include "./events.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../gui/nodes/node.h"
#include "../../../../build/include/app.h"


//--------------------------------------- Save / Load Projects

//project de-serialization
void EventListener::Deserialize(std::ifstream &JSON, std::filesystem::path &result)
{

    if (!JSON.good()) {
        Editor::Log("There was a problem reading the file.");
        return;
    }

    json data = json::parse(JSON);

    //camera

    Editor::camera->m_position.x = data["camera"]["x"];
    Editor::camera->m_position.y = data["camera"]["y"];
    Editor::camera->m_zoom = data["camera"]["zoom"];
    Editor::camera->m_backgroundColor.x = data["camera"]["color"]["x"];
    Editor::camera->m_backgroundColor.y = data["camera"]["color"]["y"];
    Editor::camera->m_backgroundColor.z = data["camera"]["color"]["z"];
    Editor::camera->m_backgroundColor.w = data["camera"]["color"]["w"];

    Editor::camera->SetBounds(
        data["camera"]["bounds"]["width"]["begin"], data["camera"]["bounds"]["width"]["end"],
        data["camera"]["bounds"]["height"]["begin"], data["camera"]["bounds"]["height"]["end"]
    );

    GUI::grid->m_alpha = data["camera"]["alpha"];
    GUI::grid_quantity = data["camera"]["pitch"];
    
    Editor::worldWidth = data["camera"]["width"];
    Editor::worldHeight = data["camera"]["height"];

    Editor::gravityX = data["settings"]["physics"]["gravity"]["x"];
    Editor::gravityY = data["settings"]["physics"]["gravity"]["y"];
    Editor::gravity_continuous = data["settings"]["physics"]["continuous"];
    Editor::gravity_sleeping = data["settings"]["physics"]["sleeping"];

    //global variables
    
    if (data["globals"].size())
        for (const auto &global : data["globals"])
            Editor::globals.push_back({ global["key"], global["type"] });

    //sprites

    for (const auto &sprite : data["nodes"]["sprites"])
    {

        auto sn = Node::MakeNode<SpriteNode>(); 

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

        for (const auto &frame : sprite["frames"]) {
            sn->frames.push_back({ frame["frame x"], frame["frame y"], frame["frame width"], frame["frame height"] });
            sn->frameBuf1.push_back(frame["frame x"]);
            sn->frameBuf2.push_back(frame["frame y"]);
            sn->frameBuf3.push_back(frame["frame width"]);
            sn->frameBuf4.push_back(frame["frame height"]);
            sn->framesApplied = true;
        }

        sn->frame = sn->frames.size();

        std::vector<std::array<int, 4>> framesToPush;

        for (const auto& frame : sn->frames)
            framesToPush.push_back({ frame.x, frame.y, frame.width, frame.height });

        System::Resources::Manager::LoadFrames(sn->spriteHandle->m_key, framesToPush);

        //animator

        if (sprite["components"]["animator"]["exists"])
            sn->AddComponent("Animator");

        for (const auto &anim : sprite["components"]["animator"]["animations"]) 
        { 

            sn->animations.insert({ anim["key"], { anim["key"], anim["start"], anim["end"] } });

            SpriteNode::StringContainer sc = { anim["key"]};

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

    for (const auto &tilemap : data["nodes"]["tilemaps"])
    {

        auto tmn = Node::MakeNode<TilemapNode>(); 

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
            for (const auto &body : tilemap["components"]["physics"]["bodies"]) 
                tmn->CreateBody(body["bodyX"], body["bodyY"], body["body_width"], body["body_height"]);
        
        if (tilemap["layers"].size())
            tmn->ApplyTilemap();
    }

    //audio

    for (const auto &audio : data["nodes"]["audio"])
    {

        auto an = Node::MakeNode<AudioNode>(); 

        an->m_ID = audio["ID"];
        an->m_name = audio["name"];
        an->audio_source_name = audio["source name"];
        an->volume = audio["volume"];
        an->loop = audio["loop"];

    }

    //empty

    for (const auto &empty : data["nodes"]["empty"])
    {

        auto en = Node::MakeNode<EmptyNode>(); 

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

    for (const auto &text : data["nodes"]["text"])
    {

        auto tn = Node::MakeNode<TextNode>(); 

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

        //script

        if (text["components"]["script"]["exists"]) 
        {
            tn->AddComponent("Script", false);

            if (text["components"]["script"]["scripts"].size())
                for (const auto &scripts : text["components"]["script"]["scripts"])
                    tn->behaviors.insert({ static_cast<std::string>(scripts["key"]).c_str(), static_cast<std::string>(scripts["value"]).c_str() });

        }

    }

    Editor::Log("Project " + currentProject + " opened.\nProject root path set: " + Editor::projectPath);

}



//--------------------------------------


//project serialization
void EventListener::Serialize(json &data)
{

    json nodes;

    json sprites;
    json tilemaps;
    json audio;
    json text;
    json empty;

    //camera

    data["camera"]["x"] = Editor::camera->m_position.x;
    data["camera"]["y"] = Editor::camera->m_position.y;
    data["camera"]["width"] = Editor::worldWidth;
    data["camera"]["height"] = Editor::worldHeight;
    data["camera"]["zoom"] = Editor::camera->m_zoom; 
    data["camera"]["color"]["x"] = Editor::camera->m_backgroundColor.x;
    data["camera"]["color"]["y"] = Editor::camera->m_backgroundColor.y;
    data["camera"]["color"]["z"] = Editor::camera->m_backgroundColor.z;
    data["camera"]["color"]["w"] = Editor::camera->m_backgroundColor.w;
    data["camera"]["alpha"] = GUI::grid->m_alpha;
    data["camera"]["pitch"] = GUI::grid_quantity;
    data["camera"]["bounds"]["width"]["begin"] = Editor::camera->currentBoundsWidthBegin;
    data["camera"]["bounds"]["width"]["end"] = Editor::camera->currentBoundsWidthEnd;
    data["camera"]["bounds"]["height"]["begin"] = Editor::camera->currentBoundsHeightBegin;
    data["camera"]["bounds"]["height"]["end"] = Editor::camera->currentBoundsHeightEnd;

    //settings

    data["settings"]["physics"]["gravity"]["x"] = Editor::gravityX;
    data["settings"]["physics"]["gravity"]["y"] = Editor::gravityY;
    data["settings"]["physics"]["continuous"] = Editor::gravity_continuous;
    data["settings"]["physics"]["sleeping"] = Editor::gravity_sleeping;

    json globals = json::array();

    if (Editor::globals_applied)
        for (const auto &global : Editor::globals)
            globals.push_back({
                { "key", global.first },
                { "type", global.second }
            });

    data["globals"] = globals;

    for (const auto &node : Node::nodes)
    {

        //scripts

        json scripts = json::array();

        for (const auto &behavior : node->behaviors)
            scripts.push_back({
                { "key", behavior.first }, 
                { "value", behavior.second }
            });

        //shader

        json shader = {};

        if (node->shader.first.length())
            shader = {
                    { "key", node->shader.first }, 
                    { "vertex", node->shader.second.first },
                    { "fragment", node->shader.second.second }
                };

        //sprites

        if (node->m_type == "Sprite")
        {

            auto sn = std::dynamic_pointer_cast<SpriteNode>(node);

            //frames

            json frames = json::array();

            for (int i = 0; i < sn->frame; ++i)
                frames.push_back({
                    { "frame x", sn->frameBuf1.size() ? sn->frameBuf1[i] : 0 },
                    { "frame y", sn->frameBuf2.size() ? sn->frameBuf2[i] : 0 },
                    { "frame width", sn->frameBuf3.size() ? sn->frameBuf3[i] : 0 },
                    { "frame height", sn->frameBuf4.size() ? sn->frameBuf4[i] : 0 }
                });

            //animations

            json animations = json::array();

            for (int i = 0; i < sn->anim; ++i)
                if (sn->animBuf1.size())
                    animations.push_back({
                        { "key", sn->animBuf1[i].s },
                        { "start", sn->animBuf2[i] },
                        { "end", sn->animBuf3[i] }
                    });

            //physics bodies

            json bodies = json::array();

            for (int i = 0; i < sn->bodies.size(); ++i)
                bodies.push_back({
                    { "type", sn->bodies[i].second },
                    { "body_width", sn->body_width.size() ? sn->body_width[i] : 0 },
                    { "body_height", sn->body_height.size() ? sn->body_height[i] : 0 },
                    { "bodyX", sn->bodyX.size() ? sn->bodyX[i] : 0 },
                    { "bodyY", sn->bodyY.size() ? sn->bodyY[i] : 0 },
                    { "pointer", sn->body_pointer.size() ? sn->body_pointer[i] : 0 },
                    { "sensor", sn->is_sensor.size() ? sn->is_sensor[i].b : false }
                });


            //settings

            sprites.push_back({
                { "ID", node->m_ID }, 
                { "name", node->m_name },
                { "U1", sn->spriteHandle->m_texture.U1 },
                { "V1", sn->spriteHandle->m_texture.V1 },
                { "U2", sn->spriteHandle->m_texture.U2 },
                { "V2", sn->spriteHandle->m_texture.V2 },
                { "currentTexture", sn->spriteHandle->m_key },
                { "m_tint", {
                        { "x", sn->spriteHandle->m_tint.x },
                        { "y", sn->spriteHandle->m_tint.y },
                        { "z", sn->spriteHandle->m_tint.z }
                    }
                },
                { "m_alpha", sn->spriteHandle->m_alpha },
                { "positionX", sn->positionX },
                { "positionY", sn->positionY },
                { "rotation", sn->rotation },
                { "scaleX", sn->scaleX },
                { "scaleY", sn->scaleY },
                { "flipX", sn->flippedX },
                { "flipY", sn->flippedY },
                { "depth", sn->depth },
                { "lock", sn->lock_in_place },
                { "frames", frames },
                { "components", {
                        { "physics", {
                                { "exists", sn->HasComponent("Physics") },
                                { "bodies", bodies },
                                { "friction", sn->friction },
                                { "restitution", sn->restitution },
                                { "density", sn->density }
                            }
                        },
                        { "animator", {
                                { "exists", sn->HasComponent("Animator") },
                                { "animations", animations }
                            }
                        },
                        { "script", {
                                { "exists", sn->HasComponent("Script") },
                                { "scripts", scripts }
                            }
                        },
                        { "shader", {
                                { "exists", sn->HasComponent("Shader") },
                                { "shaders", shader }
                            }
                        }
                    }
                }
            });
        }

        //tilemaps

        if (node->m_type == "Tilemap")
        {

            auto tmn = std::dynamic_pointer_cast<TilemapNode>(node);

            //layers

            json layers = json::array();

            for (int i = 0; i < tmn->layers.size(); ++i)
                layers.push_back({
                    { "frames x", tmn->spr_sheet_width.size() ? tmn->spr_sheet_width[i] : 0 },
                    { "frames y", tmn->spr_sheet_width.size() ? tmn->spr_sheet_width[i] : 0 },
                    { "depth", tmn->depth.size() ? tmn->depth[i] : 0 },
                    { "csv", {
                            { "key", tmn->layers[i][0] },
                            { "path", tmn->layers[i][1] },
                            { "texture", tmn->layers[i][2] }
                        } 
                    }
                });

            //physics bodies

            json bodies = json::array();

            for (int i = 0; i < tmn->bodies.size(); ++i) 
                bodies.push_back({
                    { "body_width", tmn->body_width.size() ? tmn->body_width[i] : 0 },
                    { "body_height", tmn->body_height.size() ? tmn->body_height[i] : 0 },
                    { "bodyX", tmn->bodyX.size() ? tmn->bodyX[i] : 0 },
                    { "bodyY", tmn->bodyY.size() ? tmn->bodyY[i] : 0 }
                });

            tilemaps.push_back({
                { "ID", node->m_ID },
                { "name", node->m_name },
                { "layer", tmn->layer },
                { "layers", layers },
                { "map_width", tmn->map_width },
                { "map_height", tmn->map_height },
                { "tile_width", tmn->tile_width },
                { "tile_height", tmn->tile_height },
                { "components", {
                        { "physics", {
                                { "exists", tmn->HasComponent("Physics") },
                                { "bodies", bodies }
                            }
                        }
                    }
                }
            });
        }

        //audio

        if (node->m_type == "Audio")
        {
            auto an = std::dynamic_pointer_cast<AudioNode>(node);

            audio.push_back({
                { "ID", node->m_ID },
                { "name", node->m_name },
                { "source name", an->audio_source_name },
                { "volume", an->volume },
                { "loop", an->loop }
            });

        }

        //empty

        if (node->m_type == "Empty")
        {
            auto en = std::dynamic_pointer_cast<EmptyNode>(node);

            empty.push_back({

                { "debug graphics", en->show_debug },
                { "fill", en->debug_fill },
                { "width", en->rectWidth },
                { "height", en->rectHeight },
                { "radius", en->radius },
                { "shape", en->currentShape },
                { "position x", en->positionX },
                { "position y", en->positionY },
                { "m_tint", {
                        { "x", en->m_debugGraphic->m_tint.x },
                        { "y", en->m_debugGraphic->m_tint.y },
                        { "z", en->m_debugGraphic->m_tint.z }
                    }
                },
                { "m_alpha", en->m_debugGraphic->m_alpha },
                { "components", {
                        { "script", {
                                { "exists", en->HasComponent("Script") },
                                { "scripts", scripts }
                            }
                        },
                        { "shader", {
                                { "exists", en->HasComponent("Shader") },
                                { "shaders", shader }
                            }
                        }
                    }
                }
            });

        }

        //text

        if (node->m_type == "Text")
        {
            auto tn = std::dynamic_pointer_cast<TextNode>(node);

            text.push_back({
                { "ID", node->m_ID },
                { "name", node->m_name },
                { "content", tn->textBuf },
                { "tint", {
                        { "x", tn->tint.x },
                        { "y", tn->tint.y },
                        { "z", tn->tint.z }
                    }
                },
                { "alpha", tn->alpha },    
                { "position x", tn->positionX },      
                { "position y", tn->positionY }, 
                { "rotation", tn->rotation },
                { "scale x", tn->scaleX },     
                { "scale y", tn->scaleY },
                { "components", {
                        { "script", {
                                { "exists", tn->HasComponent("Script") },
                                { "scripts", scripts }
                            }
                        }
                    }
                }
            });
        }

        //embed node data

        nodes = json::array();

        data["nodes"]["sprites"] = sprites;
        data["nodes"]["tilemaps"] = tilemaps;
        data["nodes"]["audio"] = audio;
        data["nodes"]["empty"] = empty;
        data["nodes"]["text"] = text;
         
    }
}