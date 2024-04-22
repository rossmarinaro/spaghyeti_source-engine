#include <algorithm>

#include "./node.h"
#include "../editor.h"
#include "../assets/assets.h"

#include "../../vendors/UUID.hpp"

using namespace editor;


Node::Node(const std::string& type): 
    m_type(type),
    virtual_node(true) {}


//-------------------------------


Node::Node(const std::string &id, const std::string& type, const std::string& name): 
    m_name(name),
    m_ID(id),
    m_type(type),
    created(false),
    active(true),
    show_options(false),
    positionX(0.0f),
    positionY(0.0f), 
    scaleX(1.0f),
    scaleY(1.0f),
    rotation(0.0f)
{

    for (const auto& node : nodes)
        if (node->m_name == this->m_name)
            this->m_name = this->m_name + "_" + std::to_string(count); 
}



//--------------------------- change title name


int Node::ChangeName(ImGuiInputTextCallbackData* data)
{

    if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
    {

        std::string input_text = static_cast<std::string>(data->Buf); 
        std::string* id = static_cast<std::string*>(data->UserData); 

        for (auto &node : nodes)
            if (node->m_ID == *id) 
                node->m_name = input_text;
        
        Editor::Log("Node name changed to " + input_text);

        data->DeleteChars(0, data->BufTextLen);
    }

    return 0;
}


//--------------------------- assign UUID to node and increment global counter


const char* Node::Assign()
{

    if (count > MAX_NODES) {
        Editor::Log("Max nodes reached.");
        return nullptr; 
    }

    count++;

    //generate UUID and replace hyphens with underscores

    std::string uuid = UUID::generate_uuid();

    for (int i = 0; i < uuid.length(); i++)
        if (uuid[i] == '-')
            uuid[i] = '_'; 
            
    return uuid.c_str();

}


//------------------------- delete


void Node::DeleteNode (std::shared_ptr<Node> node)
{

    node->active = false;

    //delete all attached components

    node->Reset();
    node->components.clear();

    //delete node instance
 
    auto it = std::find(nodes.begin(), nodes.end(), node);

    if (it != nodes.end())
        nodes.erase(it);

    if (count > 0) 
        count--;

}


//-------------------------- remove all


void Node::ClearAll() 
{

    if (!nodes.size()) 
        return;

    for (const auto& node : nodes) 
        node->Reset();
    
    nodes.clear();
    count = 0;
    
}

 
//-------------------------- add component


void Node::AddComponent(const char* type, bool init)
{

    //return if component exists 
    
    if (std::find_if(this->components.begin(), this->components.end(), 
        [&](std::shared_ptr<Component> component){ return component->m_type == type; }) 
        != this->components.end()) {
            Editor::Log("Component " + (std::string)type + " already exists!");
            return;
        }
        
    const auto component = std::make_shared<Component>(this->m_ID, type, this->m_type, init);

    this->components.push_back(component); 

    if (init)
        component->Make();

}


//------------------------------ remove component


void Node::RemoveComponent(std::shared_ptr<Component> component)
{

    std::vector<std::shared_ptr<Component>>::iterator it = std::find(this->components.begin(), this->components.end(), component);

    if (it != this->components.end()) {

        this->Reset((*it)->m_type.c_str());
        this->components.erase(it);
    }

}


//------------------------------ get component


const std::shared_ptr<Component> Node::GetComponent(const std::string &type, const std::string &id)
{

    for (auto it = this->components.begin(); it != this->components.end(); ++it) {

        auto component = *it;

        if (id == component->m_ID && type == component->m_type) 
            return component;
    }

    return nullptr;
}


//------------------------------ node has component


const bool Node::HasComponent(const char* type) {
    
    return std::find_if(components.begin(), components.end(), [&](std::shared_ptr<Component> component) { 
        return (std::string)type == component->m_type; }) != components.end(); 
}


//----------------------------- load custom shader


void Node::LoadShader(
    std::shared_ptr<Node> node, 
    const std::string& name, 
    const std::string& vertPath, 
    const std::string& fragPath
)
{

    node->shader = { name, { vertPath, fragPath } };  

    Shader::Load(name, vertPath.c_str(), fragPath.c_str(), nullptr);

    Node::ApplyShader(node, name);
}


//------------------------------- apply shaders


void Node::ApplyShader(std::shared_ptr<Node> node, const std::string& name) 
{

    if (node->m_type == "Sprite") {
        auto sn = std::dynamic_pointer_cast<SpriteNode>(node);

        if (sn->spriteHandle)
            sn->spriteHandle->shader = Shader::GetShader(name);
    }

    if (node->m_type == "Empty") {
        auto en = std::dynamic_pointer_cast<EmptyNode>(node);

        if (en->m_debugGraphic)
            en->m_debugGraphic->shader = Shader::GetShader(name);
    }
}


//---------------------------------- save prefab


void Node::SavePrefab() {

    if (AssetManager::SavePrefab("Sprite", this->m_ID))
        Editor::Log("Prefab " + this->m_name + " saved.");
    else    
        Editor::Log("There was a problem saving prefab.");
}


//--------------------------------- record current captured editor settings per node


json Node::writeData(std::shared_ptr<Node> node, const std::string& type)
{

    json data;

    //scripts

    json scripts = json::array();

    for (const auto& behavior : node->behaviors)
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

    //-----------sprite

    if (type == "Sprite")
    {
        auto sn = std::dynamic_pointer_cast<SpriteNode>(node); 

        //frames

        json frames = json::array();

        for (int i = 0; i < sn->frame; ++i)
            frames.push_back({
                { "offset x", sn->frameBuf1.size() ? sn->frameBuf1[i] : 0 },
                { "offset y", sn->frameBuf2.size() ? sn->frameBuf2[i] : 0 },
                { "width", sn->frameBuf3.size() ? sn->frameBuf3[i] : 0 },
                { "height", sn->frameBuf4.size() ? sn->frameBuf4[i] : 0 },
                { "factor x", sn->frameBuf5.size() ? sn->frameBuf5[i] : 1 },
                { "factor y", sn->frameBuf6.size() ? sn->frameBuf6[i] : 1 }
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
                { "pointer", sn->body_pointer.size() ? sn->body_pointer[i] : 0 },
                { "sensor", sn->is_sensor.size() ? sn->is_sensor[i].b : false }
            });


        //settings

        data={
            { "type", type },
            { "ID", node->m_ID }, 
            { "name", node->m_name },
            { "U1", sn->U1 },
            { "V1", sn->V1 },
            { "U2", sn->U2 },
            { "V2", sn->V2 },
            { "currentTexture", sn->key },
            { "tint", {
                    { "x", sn->tint.x },
                    { "y", sn->tint.y },
                    { "z", sn->tint.z }
                }
            },
            { "alpha", sn->alpha },
            { "positionX", sn->positionX },
            { "positionY", sn->positionY },
            { "rotation", sn->rotation },
            { "scaleX", sn->scaleX },
            { "scaleY", sn->scaleY },
            { "flipX", sn->flippedX },
            { "flipY", sn->flippedY },
            { "depth", sn->depth },
            { "lock", sn->lock_in_place },
            { "makeUI", sn->make_UI },
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
        };
    }

    //------------tilemap

    if (type == "Tilemap")
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

        data={
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
        };
    }

    //-------------audio

    if (type == "Audio")
    {
        auto an = std::dynamic_pointer_cast<AudioNode>(node);

        data.push_back({
            { "ID", node->m_ID },
            { "name", node->m_name },
            { "source name", an->audio_source_name },
            { "volume", an->volume },
            { "loop", an->loop }
        });
    }

    //------------- empty

    if (type == "Empty")
    {

        auto en = std::dynamic_pointer_cast<EmptyNode>(node);

        data={
            { "ID", node->m_ID },
            { "name", node->m_name },
            { "debug graphics", en->show_debug },
            { "fill", en->debug_fill },
            { "width", en->rectWidth },
            { "height", en->rectHeight },
            { "radius", en->radius },
            { "shape", en->currentShape },
            { "position x", en->positionX },
            { "position y", en->positionY },
            { "tint", {
                    { "x", en->m_debugGraphic ? en->m_debugGraphic->tint.x : 0 },
                    { "y", en->m_debugGraphic ? en->m_debugGraphic->tint.y : 0 },
                    { "z", en->m_debugGraphic ? en->m_debugGraphic->tint.z : 0 }
                }
            },
            { "alpha", en->m_debugGraphic ? en->m_debugGraphic->alpha : 0 },
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
        };
    }


    //--------------- text


    if (type == "Text")
    {

        auto tn = std::dynamic_pointer_cast<TextNode>(node);

        data={
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
            { "depth", tn->depth },
            { "components", {
                    { "script", {
                            { "exists", tn->HasComponent("Script") },
                            { "scripts", scripts }
                        }
                    }
                }
            }
        };
    }

    return data;
}



//--------------------------------- read data and create node of specific type



void Node::readData(json& data, const std::string& type, bool makeNode, void* scene)
{

    try {
        
        Scene* _scene = static_cast<Scene*>(scene);

        //sprite

        if (type == "Sprite")
        {

            std::shared_ptr<SpriteNode> sn;

            if (makeNode)
                sn = Node::MakeNode<SpriteNode>(); 
            
            else 
                sn = Scene::CreateObject<SpriteNode>(_scene); 

            sn->m_ID = data["ID"];
            sn->m_name = data["name"];
            sn->positionX = data["positionX"];
            sn->positionY = data["positionY"];
            sn->rotation = data["rotation"];
            sn->scaleX = data["scaleX"];
            sn->scaleY = data["scaleY"];
            sn->flippedX = data["flipX"];
            sn->flippedY = data["flipY"];
            sn->depth = data["depth"];
            sn->lock_in_place = data["lock"]; 
            sn->make_UI = data["makeUI"];
            sn->U1 = data["U1"]; 
            sn->V1 = data["V1"];
            sn->U2 = data["U2"]; 
            sn->V2 = data["V2"];
            sn->key = data["currentTexture"];
            sn->tint = { data["tint"]["x"], data["tint"]["y"], data["tint"]["z"] };
            sn->alpha = data["alpha"];

            if (makeNode) {
                sn->ApplyTexture(data["currentTexture"]);  
                sn->spriteHandle->SetTexture(data["currentTexture"]);
            }
                
            for (const auto& frame : data["frames"]) {
                sn->frameBuf1.push_back(frame["offset x"]);
                sn->frameBuf2.push_back(frame["offset y"]);
                sn->frameBuf3.push_back(frame["width"]);
                sn->frameBuf4.push_back(frame["height"]);
                sn->frameBuf5.push_back(frame["factor x"]);
                sn->frameBuf6.push_back(frame["factor y"]);
                sn->frames.push_back({ frame["offset x"], frame["offset y"], frame["width"], frame["height"], frame["factor x"], frame["factor y"] });
            }

            sn->frame = sn->frames.size();

            sn->RegisterFrames();

            if (makeNode)
                if (sn->frames.size() > 1) {
                    sn->framesApplied = true; 
                    sn->spriteHandle->ReadSpritesheetData();
                    sn->spriteHandle->SetFrame(0);
                }

            //animator

            if (data["components"]["animator"]["exists"]) 
                sn->AddComponent("Animator", false);

            for (const auto& anim : data["components"]["animator"]["animations"]) 
            { 

                SpriteNode::StringContainer sc = { anim["key"] };

                sn->animBuf1.push_back(sc);
                sn->animBuf2.push_back(anim["start"]); 
                sn->animBuf3.push_back(anim["end"]);
            
                sn->ApplyAnimation(anim["key"], anim["start"], anim["end"]);
            }

            sn->anim = sn->animations.size();   

            //physics 

            if (data["components"]["physics"]["exists"]) 
            {
                sn->AddComponent("Physics", false);

                sn->friction = data["components"]["physics"]["friction"];
                sn->restitution = data["components"]["physics"]["restitution"];
                sn->density = data["components"]["physics"]["density"]; 

                if (data["components"]["physics"]["bodies"].size()) 
                    for (const auto& body : data["components"]["physics"]["bodies"]) 
                        if (makeNode)
                            sn->CreateBody(body["bodyX"], body["bodyY"], body["body_width"], body["body_height"], body["sensor"], body["pointer"]);
                        else 
                        {
                            sn->bodyX.push_back(body["bodyX"]);
                            sn->bodyY.push_back(body["bodyY"]);
                            sn->body_width.push_back(body["body_width"]);
                            sn->body_height.push_back(body["body_height"]);

                            SpriteNode::BoolContainer bc;
                            bc.b = body["sensor"];

                            sn->is_sensor.push_back(bc);
                            sn->body_pointer.push_back(body["pointer"]);

                            sn->bodies.push_back(nullptr);
                        }
            }

            //script

            if (data["components"]["script"]["exists"]) {
                
                sn->AddComponent("Script", false);

                if (data["components"]["script"]["scripts"].size())
                    for (const auto& scripts : data["components"]["script"]["scripts"])
                        for (const auto& file : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::script_dir))
                            if (file.exists() && static_cast<std::string>(scripts["key"]) == Editor::events.GetScriptName(file.path().string()))
                                sn->behaviors.insert({ static_cast<std::string>(scripts["key"]).c_str(), static_cast<std::string>(scripts["value"]).c_str() });   
            }

            //shader

            if (data["components"]["shader"]["exists"]) {

                sn->AddComponent("Shader", false); 

                if (data["components"]["shader"]["shaders"].size())
                    Node::LoadShader(sn, 
                        static_cast<std::string>(data["components"]["shader"]["shaders"]["key"]).c_str(),
                        static_cast<std::string>(data["components"]["shader"]["shaders"]["vertex"]).c_str(),
                        static_cast<std::string>(data["components"]["shader"]["shaders"]["fragment"]).c_str()
                    );
            }
        }

        //tilemap

        if (type == "Tilemap")
        {

            std::shared_ptr<TilemapNode> tmn;

            if (makeNode)
                tmn = Node::MakeNode<TilemapNode>(); 
            
            else 
                tmn = Scene::CreateObject<TilemapNode>(_scene); 

            tmn->m_ID = data["ID"];
            tmn->m_name = data["name"];

            tmn->map_width = data["map_width"];
            tmn->map_height = data["map_height"];
            tmn->tile_width = data["tile_width"];
            tmn->tile_height = data["tile_height"];
            tmn->layer = data["layer"];

            if (data["layers"].size())
                for (const auto& layer : data["layers"]) {
                    tmn->layers.push_back({ layer["csv"]["key"], layer["csv"]["path"], layer["csv"]["texture"] });
                    tmn->spr_sheet_width.push_back(layer["frames x"]);
                    tmn->spr_sheet_height.push_back(layer["frames y"]);
                    tmn->depth.push_back(layer["depth"]);
                }

            //physics 
            
            if (data["components"]["physics"]["exists"]) 
                tmn->AddComponent("Physics", false);

            if (data["components"]["physics"]["bodies"].size())
                for (const auto& body : data["components"]["physics"]["bodies"]) 
                {
                    if (makeNode) 
                    {
                        tmn->CreateBody(body["bodyX"], body["bodyY"], body["body_width"], body["body_height"]);

                        for (int i = 0; i < tmn->bodies.size(); i++)
                            tmn->UpdateBody(i);
                    }
                    else {
                        tmn->bodyX.push_back(body["bodyX"]);
                        tmn->bodyY.push_back(body["bodyY"]);
                        tmn->body_width.push_back(body["body_width"]);
                        tmn->body_height.push_back(body["body_height"]);
                        tmn->bodies.push_back(nullptr);
                    }

                }
            
            if (data["layers"].size())
                tmn->ApplyTilemap(makeNode);
        }

        //audio

        if (type == "Audio")
        {
            std::shared_ptr<AudioNode> an;

            if (makeNode)
                an = Node::MakeNode<AudioNode>(); 
            
            else 
                an = Scene::CreateObject<AudioNode>(_scene);  

            an->m_ID = data["ID"];
            an->m_name = data["name"];
            an->audio_source_name = data["source name"];
            an->volume = data["volume"];
            an->loop = data["loop"];
        }

        //empty

        if (type == "Empty")
        {

            std::shared_ptr<EmptyNode> en;

            if (makeNode)
                en = Node::MakeNode<EmptyNode>(); 
            
            else 
                en = Scene::CreateObject<EmptyNode>(_scene);   

            en->m_ID = data["ID"];
            en->m_name = data["name"];
            en->show_debug = data["debug graphics"]; 
            en->debug_fill = data["fill"];
            en->rectWidth = data["width"];  
            en->rectHeight = data["height"]; 
            en->radius = data["radius"]; 
            en->positionX = data["position x"]; 
            en->positionY = data["position y"];

            if (static_cast<std::string>(data["shape"]).length()) {

                en->CreateShape(data["shape"]);

                if (en->m_debugGraphic) {
                    en->m_debugGraphic->tint.x = data["tint"]["x"];
                    en->m_debugGraphic->tint.y = data["tint"]["y"];
                    en->m_debugGraphic->tint.z = data["tint"]["z"];
                    en->m_debugGraphic->alpha = data["alpha"];
                }
            }

            //script

            if (data["components"]["script"]["exists"]) {

                en->AddComponent("Script", false);

                if (data["components"]["script"]["scripts"].size())
                    for (const auto& scripts : data["components"]["script"]["scripts"])
                        for (const auto& file : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::script_dir))
                            if (file.exists() && static_cast<std::string>(scripts["key"]) == Editor::events.GetScriptName(file.path().string()))
                                en->behaviors.insert({ static_cast<std::string>(scripts["key"]), static_cast<std::string>(scripts["value"]) });
            }

            //shader

            if (data["components"]["shader"]["exists"]) 
            {
                
                en->AddComponent("Shader", false);

                if (data["components"]["shader"]["shaders"].size())
                    Node::LoadShader(en, 
                        static_cast<std::string>(data["components"]["shader"]["shaders"]["key"]).c_str(),
                        static_cast<std::string>(data["components"]["shader"]["shaders"]["vertex"]).c_str(),
                        static_cast<std::string>(data["components"]["shader"]["shaders"]["fragment"]).c_str()
                    );
            }
        }

        //text

        if (type == "Text")
        {
            std::shared_ptr<TextNode> tn;

            if (makeNode)
                tn = Node::MakeNode<TextNode>(); 
            
            else 
                tn = Scene::CreateObject<TextNode>(_scene);

            tn->m_ID = data["ID"];
            tn->m_name = data["name"];

            tn->textBuf = data["content"];
            tn->tint = glm::vec3(data["tint"]["x"], data["tint"]["y"], data["tint"]["z"]);     
            tn->alpha = data["alpha"];       
            tn->positionX = data["position x"];        
            tn->positionY = data["position y"];   
            tn->rotation = data["rotation"];      
            tn->scaleX = data["scale x"];      
            tn->scaleY = data["scale y"];
            tn->depth = data["depth"];    

            //script

            if (data["components"]["script"]["exists"]) 
            {
                tn->AddComponent("Script", false);

                if (data["components"]["script"]["scripts"].size())
                    for (const auto& scripts : data["components"]["script"]["scripts"])
                        for (const auto& file : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::script_dir))
                            if (file.exists() && static_cast<std::string>(scripts["key"]) == Editor::events.GetScriptName(file.path().string()))
                                tn->behaviors.insert({ static_cast<std::string>(scripts["key"]).c_str(), static_cast<std::string>(scripts["value"]).c_str() });

            }
        }
    }

    catch (std::runtime_error& err) {
        
        std::cout << "error reading data: " << err.what() << "\n";
    }
}