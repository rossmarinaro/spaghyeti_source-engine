#include <algorithm>

#include "./node.h"
#include "../editor.h"
#include "../assets/assets.h"

#include "../../../vendors/UUID.hpp"

using namespace editor;


Node::Node(const std::string& type, const std::string& name, std::vector<std::shared_ptr<Node>>& arr)
{

    ID = s_Assign();
    created = false;
    active = true;
    show_options = false;
    positionX = 0.0f;
    positionY = 0.0f; 
    scaleX = 1.0f;
    scaleY = 1.0f;
    rotation = 0.0f;

    this->type = type;
    this->name = CheckName(name, arr, arr.size());
}



//--------------------------- get node


std::shared_ptr<Node> Node::Get(const std::string& id)    
{

    auto it = std::find_if(nodes.begin(), nodes.end(), [&](std::shared_ptr<Node> n) { return n->ID == id; });

    if (it != nodes.end()) 
        return *it;

    //else is child

    for (const auto& node : nodes)
    {
        if (node->type == "Group") 
        {
            auto group = std::dynamic_pointer_cast<GroupNode>(node);

            for (auto it = group->_nodes.begin(); it != group->_nodes.end(); ++it)
                if ((*it)->ID == id)
                    return *it;

            for (const auto& n : group->_nodes) 
            {
                if (n->type == "Group")
                {
                    auto _group = std::dynamic_pointer_cast<GroupNode>(n);

                    for (auto it = _group->_nodes.begin(); it != _group->_nodes.end(); ++it)
                        if ((*it)->ID == id)
                            return *it;
                }
            }
        }
    }

    return nullptr;
        
}


//--------------------------- change title name


int Node::ChangeName(ImGuiInputTextCallbackData* data)
{

    if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
    {

        std::string input_text = static_cast<std::string>(data->Buf); 
        const std::string* id = static_cast<std::string*>(data->UserData); 

        auto it = std::find_if(nodes.begin(), nodes.end(), [&](auto node) { return node->ID == *id; });

        if (it != nodes.end())
            (*it)->name = CheckName(input_text, nodes, nodes.size());

        else 
        {
            for (const auto& node : nodes)
            {
                if (node->type == "Group") {
                    auto group = std::dynamic_pointer_cast<GroupNode>(node);
                    for (auto it = group->_nodes.begin(); it != group->_nodes.end(); ++it)
                        if ((*it)->ID == *id)
                            (*it)->name = CheckName(input_text, group->_nodes, group->_nodes.size());
                }
            }

        }
        
        Editor::Log("Node name changed to " + input_text);

        data->DeleteChars(0, data->BufTextLen);
    }

    return 0;
}



//--------------------------- assign UUID to node and increment global counter


const char* Node::s_Assign()
{

    if (nodes.size() > s_MAX_NODES) {
        Editor::Log("Max nodes reached.");
        return nullptr; 
    }

    //generate UUID and replace hyphens with underscores

    std::string uuid = UUID::generate_uuid();

    for (int i = 0; i < uuid.length(); i++)
        if (uuid[i] == '-')
            uuid[i] = '_'; 
            
    return uuid.c_str();

}


//------------------------- delete


void Node::DeleteNode (std::shared_ptr<Node>& node)
{

    node->active = false;

    //delete all attached components

    node->Reset();
    node->components.clear();

    //delete node instance 
 
    auto it = std::find(nodes.begin(), nodes.end(), node);

    if (it != nodes.end())
        nodes.erase(it);

    else
    {
        for (const auto& n : nodes)
        {
            if (n->type == "Group") 
            {
                auto group = std::dynamic_pointer_cast<GroupNode>(n);

                auto _it = std::find(group->_nodes.begin(), group->_nodes.end(), node);

                if (_it != group->_nodes.end()) 
                    group->_nodes.erase(_it);
            }
        }
    }   

}


//-------------------------- remove all


void Node::ClearAll() 
{

    if (!nodes.size()) 
        return;

    for (const auto& node : nodes) 
        node->Reset();
    
    nodes.clear();
    
}


//---------------------------- options


void Node::ShowOptions(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr)
{

    if (ImGui::MenuItem("Duplicate")) {
        json data = WriteData(node);
        ReadData(data, true, nullptr, arr, arr != nodes);
        Editor::Log(node->type + " node " + name + " duplicated.");  
    }

    if (ImGui::BeginMenu("Delete"))
    {
        if (ImGui::MenuItem("Are You Sure?")) 
            DeleteNode(node);

        ImGui::EndMenu();
    }
}

 
//-------------------------- add component


void Node::AddComponent(const char* type, bool init)
{

    //return if component exists 
    
    if (std::find_if(this->components.begin(), this->components.end(), 
        [&](std::shared_ptr<Component> component){ return component->type == type; }) 
        != this->components.end()) {
            Editor::Log("Component " + (std::string)type + " already exists!");
            return;
        }
        
    const auto component = std::make_shared<Component>(this->ID, type, this->type, init);

    this->components.push_back(component); 

    if (init)
        component->Make();

}


//------------------------------ remove component


void Node::RemoveComponent(std::shared_ptr<Component>& component)
{

    std::vector<std::shared_ptr<Component>>::iterator it = std::find(this->components.begin(), this->components.end(), component);

    if (it != this->components.end()) {

        this->Reset((*it)->type.c_str());
        this->components.erase(it);
    }

}


//------------------------------ get component


const std::shared_ptr<Component> Node::GetComponent(const std::string& type, const std::string& id)
{

    for (auto it = this->components.begin(); it != this->components.end(); ++it) 
    {
        auto component = *it;

        if (id == component->ID && type == component->type) 
            return component;
    }

    return nullptr;
}


//------------------------------ node has component


const bool Node::HasComponent(const char* type) {
    
    return std::find_if(components.begin(), components.end(), [&](std::shared_ptr<Component> component) 
        { return (std::string)type == component->type; }) 
            != components.end(); 
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

    ApplyShader(node, name);
}


//------------------------------- apply shaders


void Node::ApplyShader(std::shared_ptr<Node> node, const std::string& name) 
{

    if (node->type == "Sprite") 
    {
        auto sn = std::dynamic_pointer_cast<SpriteNode>(node);

        if (sn->spriteHandle)
            sn->spriteHandle->shader = Shader::Get(name);
    }

    if (node->type == "Empty") 
    {
        auto en = std::dynamic_pointer_cast<EmptyNode>(node);

        if (en->m_debugGraphic)
            en->m_debugGraphic->shader = Shader::Get(name);
    }
}


//---------------------------------- save prefab


void Node::SavePrefab() {

    if (AssetManager::SavePrefab(this->ID))
        Editor::Log("Prefab " + this->name + " saved.");
    else    
        Editor::Log("There was a problem saving prefab.");
}


//--------------------------------- record current captured editor settings per node


json Node::WriteData(std::shared_ptr<Node>& node)
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

    if (node->type == "Sprite")
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

        data = {
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
            { "current frame", sn->currentFrame },
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

    if (node->type == "Tilemap")
    {

        auto tmn = std::dynamic_pointer_cast<TilemapNode>(node);

        //layers

        json layers = json::array();

        for (int i = 0; i < tmn->layers.size(); i++)
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

        for (int i = 0; i < tmn->bodies.size(); i++) 
            bodies.push_back({
                { "body_width", tmn->body_width.size() ? tmn->body_width[i] : 0 },
                { "body_height", tmn->body_height.size() ? tmn->body_height[i] : 0 },
                { "bodyX", tmn->bodyX.size() ? tmn->bodyX[i] : 0 },
                { "bodyY", tmn->bodyY.size() ? tmn->bodyY[i] : 0 }
            });

        data = {
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

    if (node->type == "Audio")
    {
        auto an = std::dynamic_pointer_cast<AudioNode>(node);

        data = {
            { "source name", an->audio_source_name },
            { "volume", an->volume },
            { "loop", an->loop }
        };
    }

    //------------- empty

    if (node->type == "Empty")
    {

        auto en = std::dynamic_pointer_cast<EmptyNode>(node);

        data = {
            { "debug graphics", en->show_debug },
            { "fill", en->debug_fill },
            { "width", en->rectWidth },
            { "height", en->rectHeight },
            { "radius", en->radius },
            { "shape", en->currentShape },
            { "position x", en->positionX },
            { "position y", en->positionY },
            { "line weight", en->line_weight },
            { "depth", en->depth },
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


    if (node->type == "Text")
    {

        auto tn = std::dynamic_pointer_cast<TextNode>(node);

        data = {
            { "content", tn->textBuf },
            { "tint", {
                    { "x", tn->tint.x },
                    { "y", tn->tint.y },
                    { "z", tn->tint.z }
                }
            },
            { "UIFlag", tn->UIFlag }, 
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


    //--------------- group


    if (node->type == "Group")
    {

        auto gn = std::dynamic_pointer_cast<GroupNode>(node);

        json nodeData = json::array();

        for (auto& _node : gn->_nodes) 
        {
            json _data = WriteData(_node);

            nodeData.push_back({ 
                { "data", _data },
                { "type", _node->type }
            });
        }
        
        data = { { "nodes", nodeData } };
    }


    data["type"] = node->type; 
    data["name"] = node->name;

    return data;
}



//--------------------------------- read data and create node of specific type



std::shared_ptr<Node> Node::ReadData(json& data, bool makeNode, void* scene, std::vector<std::shared_ptr<Node>>& arr, bool isChild)
{

    try { 

        if (!data.contains("type")) {
            Editor::Log("Error reading data. 'type' key not found.");
            return nullptr;
        }
        
        Scene* _scene = static_cast<Scene*>(scene);

        //sprite

        if (data["type"] == "Sprite")
        {

            std::shared_ptr<SpriteNode> sn;

            if (makeNode)
                sn = Make<SpriteNode>(arr); 
            
            else 
                sn = Scene::CreateObject<SpriteNode>(_scene); 

           if (data.contains("name"))
                sn->name = CheckName(data["name"], isChild ? arr : makeNode ? nodes : _scene->nodes, isChild ? arr.size() : nodes.size());

            if (data.contains("positionX"))
                sn->positionX = data["positionX"];

            if (data.contains("positionY"))
                sn->positionY = data["positionY"];

            if (data.contains("rotation"))
                sn->rotation = data["rotation"];

            if (data.contains("scaleX"))
                sn->scaleX = data["scaleX"];

            if (data.contains("scaleY"))
                sn->scaleY = data["scaleY"];

            if (data.contains("flipX"))
                sn->flippedX = data["flipX"];

            if (data.contains("flipY"))
                sn->flippedY = data["flipY"];

            if (data.contains("depth"))
                sn->depth = data["depth"];

            if (data.contains("lock"))
                sn->lock_in_place = data["lock"]; 

            if (data.contains("makeUI"))
                sn->make_UI = data["makeUI"];

            if (data.contains("U1"))
                sn->U1 = data["U1"]; 

            if (data.contains("V1"))
                sn->V1 = data["V1"];
            
            if (data.contains("U2"))
                sn->U2 = data["U2"]; 

            if (data.contains("V2"))
                sn->V2 = data["V2"];

            if (data.contains("currentTexture"))
                sn->key = data["currentTexture"];

            if (data.contains("alpha"))
                sn->alpha = data["alpha"];

            if (data.contains("tint"))
                sn->tint = { data["tint"]["x"], data["tint"]["y"], data["tint"]["z"] };

            if (data.contains("currentTexture"))
            {
                if (makeNode) {
                    sn->ApplyTexture(data["currentTexture"]);  
                    sn->spriteHandle->SetTexture(data["currentTexture"]);
                }

                else 
                    AssetManager::Register(data["currentTexture"]);
            }

            if (data.contains("frames") && data["frames"].size() > 1)
            {
                sn->frameBuf1.clear();
                sn->frameBuf2.clear();
                sn->frameBuf3.clear();
                sn->frameBuf4.clear();
                sn->frameBuf5.clear();
                sn->frameBuf6.clear();

                for (int i = 0; i < data["frames"].size(); i++) 
                {   
                    sn->frameBuf1.push_back(data["frames"][i]["offset x"]);
                    sn->frameBuf2.push_back(data["frames"][i]["offset y"]);
                    sn->frameBuf3.push_back(data["frames"][i]["width"]);
                    sn->frameBuf4.push_back(data["frames"][i]["height"]);
                    sn->frameBuf5.push_back(data["frames"][i]["factor x"]);
                    sn->frameBuf6.push_back(data["frames"][i]["factor y"]);
                }

                sn->frame = data["frames"].size();

                if (data.contains("current frame"))
                    sn->currentFrame = data["current frame"];
      
                for (int i = 0; i < sn->frame; i++) 
                    sn->frames.push_back({ sn->frameBuf1[i], sn->frameBuf2[i], sn->frameBuf3[i], sn->frameBuf4[i], sn->frameBuf5[i], sn->frameBuf6[i]}); 

                sn->RegisterFrames();

                if (makeNode) {
                    sn->framesApplied = true; 
                    sn->spriteHandle->ReadSpritesheetData();
                }
            }
            
            //animator

            if (data.contains("components"))
            {

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

                if (data["components"]["script"]["exists"]) 
                {
                    
                    sn->AddComponent("Script", false);

                    if (data["components"]["script"]["scripts"].size())
                        for (const auto& scripts : data["components"]["script"]["scripts"])
                            for (const auto& file : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::script_dir))
                                if (file.exists() && static_cast<std::string>(scripts["key"]) == Editor::events.GetScriptName(file.path().string()))
                                    sn->behaviors.insert({ static_cast<std::string>(scripts["key"]).c_str(), static_cast<std::string>(scripts["value"]).c_str() });   
                }

                //shader

                if (data["components"]["shader"]["exists"]) 
                {

                    sn->AddComponent("Shader", false); 

                    if (data["components"]["shader"]["shaders"].size())
                        LoadShader(sn, 
                            static_cast<std::string>(data["components"]["shader"]["shaders"]["key"]).c_str(),
                            static_cast<std::string>(data["components"]["shader"]["shaders"]["vertex"]).c_str(),
                            static_cast<std::string>(data["components"]["shader"]["shaders"]["fragment"]).c_str()
                        );
                }

            }

            return sn;
        }

        //tilemap

        if (data["type"] == "Tilemap")
        {

            std::shared_ptr<TilemapNode> tmn;

            if (makeNode)
                tmn = Make<TilemapNode>(arr); 
            
            else 
                tmn = Scene::CreateObject<TilemapNode>(_scene);  

            if (data.contains("name"))
                tmn->name = CheckName(data["name"], isChild ? arr : makeNode ? nodes : _scene->nodes, isChild ? arr.size() : nodes.size());

            if (data.contains("map_width"))
                tmn->map_width = data["map_width"];

            if (data.contains("map_height"))
                tmn->map_height = data["map_height"];

            if (data.contains("tile_width"))
                tmn->tile_width = data["tile_width"];

            if (data.contains("tile_height"))
                tmn->tile_height = data["tile_height"];
                
            if (data.contains("layer"))
                tmn->layer = data["layer"];

            if (data.contains("layers") && data["layers"].size()) 
                for (const auto& layer : data["layers"]) {
                    tmn->layers.push_back({ layer["csv"]["key"], layer["csv"]["path"], layer["csv"]["texture"] });
                    tmn->spr_sheet_width.push_back(layer["frames x"]);
                    tmn->spr_sheet_height.push_back(layer["frames y"]);
                    tmn->depth.push_back(layer["depth"]);
                }

            if (data.contains("components"))
            {
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
            }
            
            if (data.contains("layers") && data["layers"].size())
                tmn->ApplyTilemap(makeNode);

            return tmn;
        }

        //audio

        if (data["type"] == "Audio")
        {
            std::shared_ptr<AudioNode> an;

            if (makeNode)
                an = Make<AudioNode>(arr); 
            
            else 
            {
                an = Scene::CreateObject<AudioNode>(_scene); 

                if (data.contains("source name"))   
                    AssetManager::Register(data["source name"]);
            } 

            if (data.contains("name"))
                an->name = CheckName(data["name"], isChild ? arr : makeNode ? nodes : _scene->nodes, isChild ? arr.size() : nodes.size());

            if (data.contains("source name"))
                an->audio_source_name = data["source name"];

            if (data.contains("volume"))
                an->volume = data["volume"];

            if (data.contains("loop"))
                an->loop = data["loop"];
        }

        //empty

        if (data["type"] == "Empty")
        {

            std::shared_ptr<EmptyNode> en;

            if (makeNode)
                en = Make<EmptyNode>(arr); 
            
            else 
                en = Scene::CreateObject<EmptyNode>(_scene);   

            if (data.contains("name"))
                en->name = CheckName(data["name"], isChild ? arr : makeNode ? nodes : _scene->nodes, isChild ? arr.size() : nodes.size());

            if (data.contains("debug graphics"))
                en->show_debug = data["debug graphics"]; 

            if (data.contains("fill"))
                en->debug_fill = data["fill"];

            if (data.contains("width"))
                en->rectWidth = data["width"];  

            if (data.contains("height"))
                en->rectHeight = data["height"]; 

            if (data.contains("line weight"))
                en->line_weight = data["line weight"];

            if (data.contains("radius"))
                en->radius = data["radius"]; 

            if (data.contains("position x"))
                en->positionX = data["position x"]; 

            if (data.contains("position y"))
                en->positionY = data["position y"];

            if (data.contains("depth"))
                en->depth = data["depth"];

            if (data.contains("shape") && static_cast<std::string>(data["shape"]).length()) {

                en->CreateShape(data["shape"]);

                if (en->m_debugGraphic) {
                    en->m_debugGraphic->tint.x = data["tint"]["x"];
                    en->m_debugGraphic->tint.y = data["tint"]["y"];
                    en->m_debugGraphic->tint.z = data["tint"]["z"];
                    en->m_debugGraphic->alpha = data["alpha"];
                }
            }

            if (data.contains("components"))
            {
                //script

                if (data["components"]["script"]["exists"]) 
                {

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
                        LoadShader(en, 
                            static_cast<std::string>(data["components"]["shader"]["shaders"]["key"]).c_str(),
                            static_cast<std::string>(data["components"]["shader"]["shaders"]["vertex"]).c_str(),
                            static_cast<std::string>(data["components"]["shader"]["shaders"]["fragment"]).c_str()
                        );
                }
            }

            return en;
        }

        //text

        if (data["type"] == "Text")
        {
            std::shared_ptr<TextNode> tn;

            if (makeNode)
                tn = Make<TextNode>(arr); 
            
            else 
                tn = Scene::CreateObject<TextNode>(_scene);

            if (data.contains("name"))
                tn->name = CheckName(data["name"], isChild ? arr : makeNode ? nodes : _scene->nodes, isChild ? arr.size() : nodes.size());

            if (data.contains("content"))
                tn->textBuf = data["content"];

            if (data.contains("tint"))
                tn->tint = glm::vec3(data["tint"]["x"], data["tint"]["y"], data["tint"]["z"]);     

            if (data.contains("alpha"))
                tn->alpha = data["alpha"];   

            if (data.contains("position x"))
                tn->positionX = data["position x"];   

            if (data.contains("position y"))
                tn->positionY = data["position y"];   

            if (data.contains("rotation"))
                tn->rotation = data["rotation"];   

            if (data.contains("scale x"))
                tn->scaleX = data["scale x"];      

            if (data.contains("scale y"))
                tn->scaleY = data["scale y"];

            if (data.contains("depth"))
                tn->depth = data["depth"];    

            if (data.contains("UIFlag"))
                tn->UIFlag = data["UIFlag"];

            //script

            if (data.contains("components") && data["components"]["script"]["exists"]) 
            {
                tn->AddComponent("Script", false);

                if (data["components"]["script"]["scripts"].size())
                    for (const auto& scripts : data["components"]["script"]["scripts"])
                        for (const auto& file : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::script_dir))
                            if (file.exists() && static_cast<std::string>(scripts["key"]) == Editor::events.GetScriptName(file.path().string()))
                                tn->behaviors.insert({ static_cast<std::string>(scripts["key"]).c_str(), static_cast<std::string>(scripts["value"]).c_str() });

            }

            return tn;
        }

        //group

        if (data["type"] == "Group")
        {
            std::shared_ptr<GroupNode> gn;

            if (makeNode)
                gn = Make<GroupNode>(arr); 
            
            else 
                gn = Scene::CreateObject<GroupNode>(_scene);
            
            if (data.contains("name"))
                gn->name = CheckName(data["name"], isChild ? arr : makeNode ? nodes : _scene->nodes, isChild ? arr.size() : nodes.size());

            if (data.contains("nodes") && data["nodes"].size())
                for (auto& node : data["nodes"]) 
                    ReadData(node["data"], makeNode, makeNode ? nullptr : _scene, gn->_nodes, true);

            return gn;

        }
    }

    catch (std::runtime_error& err) {
        Editor::Log("error reading data: " + (std::string)err.what());
    }
}