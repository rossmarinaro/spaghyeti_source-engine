#include <filesystem>
#include <iterator>

#include "./events.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../gui/nodes/node.h"
#include "../../../../build/include/app.h"


//---------------------------------------

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

    //sprites

    for (const auto &sprite : data["nodes"]["sprites"])
    {

        SpriteNode* sn = dynamic_cast<SpriteNode*>(Node::MakeNode("Sprite"));

        sn->m_ID = sprite["ID"];
        sn->m_name = sprite["name"];
        sn->positionX = sprite["positionX"];
        sn->positionY = sprite["positionY"];
        sn->rotation = sprite["rotation"];
        sn->scaleX = sprite["scaleX"];
        sn->scaleY = sprite["scaleY"];

        sn->ApplyTexture({ sprite["currentTexture"], System::Resources::Manager::texture2D->GetTexture(sprite["currentTexture"]).ID });  

        sn->spriteHandle->m_texture.U1 = sprite["U1"];
        sn->spriteHandle->m_texture.V1 = sprite["V1"];
        sn->spriteHandle->m_texture.U2 = sprite["U2"];
        sn->spriteHandle->m_texture.V2 = sprite["V2"];
        sn->spriteHandle->m_key = sprite["currentTexture"];
        sn->spriteHandle->m_tint.x = sprite["m_tint"]["x"];
        sn->spriteHandle->m_tint.y = sprite["m_tint"]["y"];
        sn->spriteHandle->m_tint.z = sprite["m_tint"]["z"];
        sn->spriteHandle->m_alpha = sprite["m_alpha"];

        sn->spriteHandle->SetTexture(System::Resources::Manager::texture2D->GetTexture(sprite["currentTexture"]).ID);

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
            sn->AddComponent("Physics Body", false);
            sn->friction = sprite["components"]["physics"]["friction"];
            sn->restitution = sprite["components"]["physics"]["restitution"];
            sn->density = sprite["components"]["physics"]["density"];
        }

        for (const auto &body : sprite["components"]["physics"]["bodies"]) 
            sn->CreateBody(static_cast<std::string>(body["type"]).c_str(), body["bodyX"], body["bodyY"], body["body_width"], body["body_height"]);

        //script

        for (const auto &script : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::script_dir))
            if (script.exists()) {
                sn->AddComponent("Script", false);
                break;
            }
        
    }

    //tilemaps

    for (const auto &tilemap : data["nodes"]["tilemaps"])
    {

        TilemapNode* tmn = dynamic_cast<TilemapNode*>(Node::MakeNode("Tilemap"));

        tmn->m_ID = tilemap["ID"];
        tmn->m_name = tilemap["name"];

        tmn->map_width = tilemap["map_width"];
        tmn->map_height = tilemap["map_height"];
        tmn->tile_width = tilemap["tile_width"];
        tmn->tile_height = tilemap["tile_height"];

        for (const auto &layer : tilemap["layers"]) {
            tmn->layers.push_back({ layer["csv"]["key"], layer["csv"]["path"], layer["csv"]["texture"] });
            tmn->spr_sheet_width.push_back(layer["frames x"]);
            tmn->spr_sheet_height.push_back(layer["frames y"]);
            tmn->depth.push_back(layer["depth"]);
        }

        //physics 
        
        if (tilemap["components"]["physics"]["exists"]) 
            tmn->AddComponent("Physics Body", false);

        for (const auto &body : tilemap["components"]["physics"]["bodies"]) 
            tmn->CreateBody(body["bodyY"], body["bodyX"], body["body_width"], body["body_height"]);
        
        tmn->ApplyTilemap();
    }

    //audio

    for (const auto &audio : data["nodes"]["audio"])
    {

        AudioNode* an = dynamic_cast<AudioNode*>(Node::MakeNode("Audio"));

        an->m_ID = audio["ID"];
        an->m_name = audio["name"];
        an->audio_source_name = audio["source name"];
        an->volume = audio["volume"];
        an->loop = audio["loop"];

    }

    //empty

    for (const auto &empty : data["nodes"]["empty"])
    {

        EmptyNode* en = dynamic_cast<EmptyNode*>(Node::MakeNode("Empty"));

        en->show_debug = empty["debug graphics"]; 
        en->debug_fill = empty["fill"];
        en->m_debugGraphic->width = empty["width"];  
        en->m_debugGraphic->height = empty["height"]; 
        en->positionX = empty["position x"]; 
        en->positionY = empty["position y"];

    }

    //text

    for (const auto &text : data["nodes"]["text"])
    {

        TextNode* tn = dynamic_cast<TextNode*>(Node::MakeNode("Text"));

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
    data["camera"]["zoom"] = Editor::camera->m_zoom;
    data["camera"]["color"]["x"] = Editor::camera->m_backgroundColor.x;
    data["camera"]["color"]["y"] = Editor::camera->m_backgroundColor.y;
    data["camera"]["color"]["z"] = Editor::camera->m_backgroundColor.z;
    data["camera"]["color"]["w"] = Editor::camera->m_backgroundColor.w;

    for (const auto &node : Node::nodes)
    {

        //sprites

        if (node->m_type == "Sprite")
        {

            SpriteNode* sn = dynamic_cast<SpriteNode*>(node);

            bool physics, animator, script, shader = false;
 
            if (sn->HasComponent("Physics Body"))
                physics = true;

            if (sn->HasComponent("Animator"))
                animator = true;

            if (sn->HasComponent("Script"))
                script = true;

            if (sn->HasComponent("Shader"))
                shader = true;
            
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
                    { "body_width", sn->body_width.size() ? sn->body_width[i] : 0 },
                    { "body_height", sn->body_height.size() ? sn->body_height[i] : 0 },
                    { "bodyX", sn->bodyX.size() ? sn->bodyX[i] : 0 },
                    { "bodyY", sn->bodyY.size() ? sn->bodyY[i] : 0 },
                    { "type", sn->bodies[i].second }
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
                { "frames", frames },
                { "components", {
                        { "physics", {
                                { "exists", physics },
                                { "bodies", bodies },
                                { "friction", sn->friction },
                                { "restitution", sn->restitution },
                                { "density", sn->density }
                            }
                        },
                        { "animator", {
                                { "exists", animator },
                                { "animations", animations }
                            }
                        },
                        { "script", {
                                { "exists", script }
                            }
                        },
                        { "shader", {
                                { "exists", shader }
                            }
                        }
                    }
                }
            });
        }

        //tilemaps

        if (node->m_type == "Tilemap")
        {

            TilemapNode* tmn = dynamic_cast<TilemapNode*>(node);

            bool physics = tmn->HasComponent("Physics Body") ? true : false;

            //layers

            json layers = json::array();

            for (int i = 0; i < tmn->layer; ++i)
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

            if (physics)
                for (int i = 0; i < tmn->layer; ++i)
                    bodies.push_back({
                        { "body_width", tmn->body_width.size() ? tmn->body_width[i] : 0 },
                        { "body_height", tmn->body_height.size() ? tmn->body_height[i] : 0 },
                        { "bodyX", tmn->bodyX.size() ? tmn->bodyX[i] : 0 },
                        { "bodyY", tmn->bodyY.size() ? tmn->bodyY[i] : 0 }
                    });

            tilemaps.push_back({
                { "ID", node->m_ID },
                { "name", node->m_name },
                { "layers", layers },
                { "map_width", tmn->map_width },
                { "map_height", tmn->map_height },
                { "tile_width", tmn->tile_width },
                { "tile_height", tmn->tile_height },
                { "components", {
                        { "physics", {
                                { "exists", physics },
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
            AudioNode* an = dynamic_cast<AudioNode*>(node);

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
            EmptyNode* en = dynamic_cast<EmptyNode*>(node);

            bool physics, script, shader = false;

            empty.push_back({

                { "debug graphics", en->show_debug },
                { "fill", en->debug_fill },
                { "width", en->m_debugGraphic->width },
                { "height", en->m_debugGraphic->height },
                { "position x", en->positionX },
                { "position y", en->positionY },
                    { "components", {
                        { "script", {
                                { "exists", script }
                            }
                        },
                        { "shader", {
                                { "exists", shader }
                            }
                        }
                    }
                }
            });

        }

        //text

        if (node->m_type == "Text")
        {
            TextNode* tn = dynamic_cast<TextNode*>(node);

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
                { "scale y", tn->scaleY }
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