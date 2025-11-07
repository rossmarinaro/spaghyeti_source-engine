#include <algorithm>

#include "./node.h"
#include "../editor.h"
#include "../assets/assets.h"

#include "../../../vendors/UUID.hpp"

using namespace editor;


Node::Node(bool init, int type, const std::string& name):
    m_init(init)
{
    std::string uuid = UUID::generate_uuid();

    for (int i = 0; i < uuid.length(); i++)
        if (uuid[i] == '-')
            uuid[i] = '_'; 

    ID = uuid;
    created = false;
    active = true;
    show_options = false;
    isStroked = false;
    isShadow = false;
    positionX = 0.0f;
    positionY = 0.0f; 
    scaleX = 1.0f;
    scaleY = 1.0f;
    rotation = 0.0f;
    shadowDistanceX = 0.0f;
    shadowDistanceY = 0.0f;
    strokeWidth = 1.0f;
    strokeColor = { 1.0f, 1.0f, 1.0f };
    shadowColor = { 0.0f, 0.0f, 0.0f };

    this->type = type;

    if (init)
        this->name = CheckName(name);
}


//---------------------------- check if name exists in node array / existing entities


std::string Node::CheckName(const std::string& name, std::vector<std::string>& arr) 
{
    std::string n = name;

    const auto it = std::find_if(arr.begin(), arr.end(), [&](const std::string& n) { return n == name; });

    if (it != arr.end()) 
        n = System::Utils::ReplaceFrom(n, "-", "") + "_" + UUID::generate_uuid() + std::to_string(arr.size()); 

    arr.emplace_back(n);

    return n;
}


//--------------------------- virtual update method for all subclasses


void Node::Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr) 
{
    static char name_buf[32] = ""; 

    NodeInfo data = { ID, arr };

    ImGui::InputText("name", name_buf, 32, ImGuiInputTextFlags_CallbackCompletion, ChangeName, &data);
    
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("press tab to confirm name.");

    if (ImGui::BeginMenu("Options")) { 
        ShowOptions(node, arr);
        ImGui::EndMenu();
    }

    ImGui::Checkbox("Edit", &show_options);
}


//--------------------------- get node type 


const std::string Node::GetType(int type) {

    switch (type) {
        case SPRITE: return "Sprite";
        case EMPTY: return "Empty";
        case TILEMAP: return "Tilemap";
        case AUDIO: return "Audio";
        case GROUP: return "Group";
        case TEXT: return "Text";
        case SPAWNER: return "Spawner";
    }
}


//---------------------------


bool Node::CheckCanAddNode(bool init, const std::vector<std::shared_ptr<Node>>& arr) {
    if (init && arr.size() > s_MAX_NODES) {
        Editor::Log("Max nodes reached.");
        return false; 
    }
    return true;
}


//--------------------------- get node
    

std::shared_ptr<Node> Node::Get(const std::string& id, std::vector<std::shared_ptr<Node>>& arr)    
{
    const auto it = std::find_if(arr.begin(), arr.end(), [&id](const auto& node){ return node->ID == id; });

    if (it != arr.end())
    {
        auto node = *it;

        //get at root
    
        if (node->ID == id)
            return node;

        //get subgroup

        if (node->type == GROUP) {
            const auto group = std::dynamic_pointer_cast<GroupNode>(node);
            return Get(id, group->_nodes);
        }
    }

    //else return null

    return nullptr;
}


//--------------------------- change title name


int Node::ChangeName(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
    {
        std::string input_text = static_cast<std::string>(data->Buf); 
        NodeInfo* info = static_cast<NodeInfo*>(data->UserData); 

        const auto node = Get(info->ID, info->arr);

        if (node) {
            node->name = CheckName(input_text);
            Editor::Log("Node name changed to " + input_text);
        }
        else
            Editor::Log("Cannot change node name, node not found.");

        data->DeleteChars(0, data->BufTextLen);
    }

    return 0;
}

//------------------------- delete


void Node::DeleteNode (const std::string& id, std::vector<std::shared_ptr<Node>>& arr)
{
    auto node = Get(id, arr);

    if (node)
    {
        node->active = false;

        const auto n_it = std::find(s_names.begin(), s_names.end(), node->name);

        if (n_it != s_names.end()) 
            s_names.erase(std::move(n_it));

        //delete all attached components

        node->Reset();
        node->components.clear();

        //delete node instance 

        auto it = std::find(arr.begin(), arr.end(), node);

        if (it != arr.end()) {
            it = arr.erase(std::move(it)); 
            --it;
        }
    }
}


//-------------------------- remove all


void Node::ClearAll() {

    if (!nodes.size()) 
        return;

    for (const auto& node : nodes) 
        node->Reset();
    
    nodes.clear();
    s_names.clear();
}


//---------------------------- options


void Node::ShowOptions(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr)
{
    if (ImGui::MenuItem("Duplicate")) 
    {
        json data = WriteData(node);
        
        if (ReadData(data, true, nullptr, arr)) 
            Editor::Log("node " + node->name + " duplicated.");  
        else
            Editor::Log("node could not be duplicated.");
    }

    if (node->type != AUDIO && node->type != TILEMAP && ImGui::MenuItem("Save prefab")) 
        SavePrefab(); 

    if (ImGui::BeginMenu("Delete"))
    {
        if (ImGui::MenuItem("Are You Sure?")) 
            DeleteNode(node->ID, arr);

        ImGui::EndMenu();
    }
}

 
//-------------------------- add component


void Node::AddComponent(int type, bool init)
{
    //return if component exists 
    
    if (std::find_if(components.begin(), components.end(), 
        [&](std::shared_ptr<Component> component){ return component->type == type; }) 
        != components.end()) {
            Editor::Log("Component " + Component::Get(type) + " already exists!");
            return;
        }
        
    const auto component = std::make_shared<Component>(ID, type, this->type, init);

    components.push_back(component); 

    if (init)
        component->Make();

}


//------------------------------ remove component


void Node::RemoveComponent(std::shared_ptr<Component>& component)
{
    std::vector<std::shared_ptr<Component>>::iterator it = std::find(components.begin(), components.end(), component);

    if (it != components.end()) {
        Reset((*it)->type);
        components.erase(it);
    }
}


//------------------------------ get component


const std::shared_ptr<Component> Node::GetComponent(int type, const std::string& id) {

    for (auto it = components.begin(); it != components.end(); ++it) {
        auto component = *it;
        if (id == component->ID && type == component->type) 
            return component;
    }

    return nullptr;
}


//------------------------------ node has component


const bool Node::HasComponent(int type) {
    return std::find_if(components.begin(), components.end(), [&](std::shared_ptr<Component> component) 
        { return (int)type == component->type; }) 
            != components.end(); 
}


//----------------------------- load custom shader


void Node::LoadShader(std::shared_ptr<Node> node, const std::string& name, const std::string& vertPath, const std::string& fragPath)
{
    node->shader = { name, { vertPath, fragPath } };  

    Graphics::Shader::Load(name, vertPath.c_str(), fragPath.c_str());

    ApplyShader(node, name);
}


//------------------------------- apply shaders


void Node::ApplyShader(std::shared_ptr<Node> node, const std::string& name) 
{
    if (node->type == SPRITE) {
        auto sn = std::dynamic_pointer_cast<SpriteNode>(node);

        if (sn->spriteHandle)
            sn->spriteHandle->SetShader(name);
    }

    if (node->type == EMPTY) {
        auto en = std::dynamic_pointer_cast<EmptyNode>(node);

        if (en->debugGraphic)
            en->debugGraphic->SetShader(name);
    }
}


//---------------------------------- save prefab


void Node::SavePrefab() {

    if (AssetManager::SavePrefab(ID))
        Editor::Log("Prefab " + name + " saved.");
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

    if (node->type == SPRITE)
    {
        auto sn = std::dynamic_pointer_cast<SpriteNode>(node); 

        //frames

        json frames = json::array();

        for (int i = 0; i < sn->frame; ++i)
            frames.push_back({
                { "offset x", sn->frame_x.size() ? sn->frame_x[i] : 0 },
                { "offset y", sn->frame_y.size() ? sn->frame_y[i] : 0 },
                { "width", sn->frame_width.size() ? sn->frame_width[i] : 0 },
                { "height", sn->frame_height.size() ? sn->frame_height[i] : 0 },
                { "factor x", sn->frame_fX.size() ? sn->frame_fX[i] : 1 },
                { "factor y", sn->frame_fY.size() ? sn->frame_fY[i] : 1 }
            });

        //animations

        json animations = json::array();

        for (const auto& anim : sn->animations)
            if (anim.key.size()) 
                 animations.push_back({
                    { "key", anim.key },
                    { "start", anim.start },
                    { "end", anim.end },
                    { "rate", anim.rate },
                    { "repeat", anim.repeat },
                    { "yoyo", anim.yoyo }
                });  
        
        //physics bodies

        json bodies = json::array();

        int bodIndex = 0;

        for (const auto& body : sn->bodies)
        {
            bodies.push_back({
                { "body_width", body.width },
                { "body_height", body.height },
                { "bodyX", body.x },
                { "bodyY", body.y },
                { "pointer", sn->body_pointer.size() ? sn->body_pointer[bodIndex] : 0 },
                { "sensor", sn->is_sensor.size() ? sn->is_sensor[bodIndex].b : false }
            });

            bodIndex++;
        }

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
            { "stroke", sn->isStroked }, 
            { "stroke color", {
                    { "x", sn->strokeColor.x },
                    { "y", sn->strokeColor.y },
                    { "z", sn->strokeColor.z }
                }
            },   
            { "stroke width", sn->strokeWidth },
            { "positionX", sn->spriteHandle ? sn->spriteHandle->position.x : sn->positionX },
            { "positionY", sn->spriteHandle ? sn->spriteHandle->position.y : sn->positionY },
            { "rotation", sn->spriteHandle ? sn->spriteHandle->rotation : sn->rotation },
            { "scaleX", sn->spriteHandle ? sn->spriteHandle->scale.x : sn->scaleX },
            { "scaleY", sn->spriteHandle ? sn->spriteHandle->scale.y : sn->scaleY },
            { "flipX", sn->flippedX },
            { "flipY", sn->flippedY },
            { "depth", sn->depth },
            { "scroll factor x", sn->scrollFactorX },
            { "scroll factor y", sn->scrollFactorY },
            { "lock", sn->lock_in_place },
            { "cull", sn->cull },
            { "makeUI", sn->make_UI },
            { "frames", frames },
            { "current frame", sn->currentFrame },
            { "components", {
                    { "physics", {
                            { "exists", sn->HasComponent(Component::PHYSICS) },
                            { "bodies", bodies },
                            { "friction", sn->friction },
                            { "restitution", sn->restitution },
                            { "density", sn->density },
                            { "type", sn->body_type },
                            { "shape", sn->body_shape }
                        }
                    },
                    { "animator", {
                            { "exists", sn->HasComponent(Component::ANIMATOR) },
                            { "animations", animations },
                            { "on start", {
                                    { "key", sn->anim_to_play_on_start.key }, 
                                    { "rate", sn->anim_to_play_on_start.rate }, 
                                    { "repeat", sn->anim_to_play_on_start.repeat }, 
                                    { "yoyo", sn->anim_to_play_on_start.yoyo } 
                                } 
                            }
                        }
                    },
                    { "script", {
                            { "exists", sn->HasComponent(Component::SCRIPT) },
                            { "scripts", scripts }
                        }
                    },
                    { "shader", {
                            { "exists", sn->HasComponent(Component::SHADER) },
                            { "shaders", shader }
                        }
                    }
                }
            }
        }; 
    } 

    //------------tilemap

    if (node->type == TILEMAP)
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

        for (const auto& body : tmn->bodies) 
            bodies.push_back({ { "body_width", body.width }, { "body_height", body.height }, { "bodyX", body.x }, { "bodyY", body.y } });

        data = {
            { "position x", tmn->positionX },
            { "position y", tmn->positionY },
            { "rotation", tmn->rotation },
            { "scale x", tmn->scaleX },
            { "scale y", tmn->scaleY },
            { "layer", tmn->layer },
            { "layers", layers },
            { "map_width", tmn->map_width },
            { "map_height", tmn->map_height },
            { "tile_width", tmn->tile_width },
            { "tile_height", tmn->tile_height },
            { "components", {
                    { "physics", {
                            { "exists", tmn->HasComponent(Component::PHYSICS) },
                            { "bodies", bodies }
                        }
                    }
                }
            }
        };
    }

    //-------------audio

    if (node->type == AUDIO)
    {
        auto an = std::dynamic_pointer_cast<AudioNode>(node);

        data = {
            { "source name", an->audio_source_name },
            { "volume", an->volume },
            { "loop", an->loop }
        };
    }

    //------------- empty

    if (node->type == EMPTY)
    {

        auto en = std::dynamic_pointer_cast<EmptyNode>(node);

        data = {
            { "debug graphics", en->show_debug },
            { "fill", en->debug_fill },
            { "width", en->debugGraphic ? en->debugGraphic->width : en->rectWidth },
            { "height", en->debugGraphic ? en->debugGraphic->height : en->rectHeight },
            { "radius", en->radius },
            { "shape", en->currentShape },
            { "position x", en->debugGraphic ? en->debugGraphic->position.x : en->positionX },
            { "position y", en->debugGraphic ? en->debugGraphic->position.y : en->positionY },
            { "scale x", en->debugGraphic ? en->debugGraphic->scale.x : en->scaleX },
            { "scale y", en->debugGraphic ? en->debugGraphic->scale.y : en->scaleY },
            { "line weight", en->line_weight },
            { "depth", en->depth },
            { "tint", {
                    { "x", en->debugGraphic ? en->debugGraphic->tint.x : 0 },
                    { "y", en->debugGraphic ? en->debugGraphic->tint.y : 0 },
                    { "z", en->debugGraphic ? en->debugGraphic->tint.z : 0 }
                }
            },
            { "alpha", en->debugGraphic ? en->debugGraphic->alpha : 0 },
            { "components", {
                    { "script", {
                            { "exists", en->HasComponent(Component::SCRIPT) },
                            { "scripts", scripts }
                        }
                    },
                    { "shader", {
                            { "exists", en->HasComponent(Component::SHADER) },
                            { "shaders", shader }
                        }
                    }
                }
            }
        };
    }


    //--------------- text


    if (node->type == TEXT)
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
            { "shadow", tn->isShadow }, 
            { "stroke", tn->isStroked }, 
            { "stroke color", {
                    { "x", tn->strokeColor.x },
                    { "y", tn->strokeColor.y },
                    { "z", tn->strokeColor.z }
                }
            },   
            { "shadow color", {
                    { "x", tn->shadowColor.x },
                    { "y", tn->shadowColor.y },
                    { "z", tn->shadowColor.z }
                }
            }, 
            { "shadow distance x", tn->shadowDistanceX },
            { "shadow distance y", tn->shadowDistanceY },
            { "stroke width", tn->strokeWidth },
            { "UIFlag", tn->UIFlag }, 
            { "alpha", tn->alpha },    
            { "position x", tn->textHandle ? tn->textHandle->position.x : tn->positionX },      
            { "position y", tn->textHandle ? tn->textHandle->position.y : tn->positionY }, 
            { "rotation", tn->textHandle ? tn->textHandle->rotation : tn->rotation },
            { "scale x", tn->textHandle ? tn->textHandle->scale.x : tn->scaleX },     
            { "scale y", tn->textHandle ? tn->textHandle->scale.y : tn->scaleY },
            { "depth", tn->depth },
            { "character offset x", tn->charOffsetX },
            { "character offset y", tn->charOffsetY },
            { "current font", tn->currentFont },
            { "components", {
                    { "script", {
                            { "exists", tn->HasComponent(Component::SCRIPT) },
                            { "scripts", scripts }
                        }
                    }
                }
            }
        };
    }


    //--------------- group


    if (node->type == GROUP)
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
        
        data = { 
            { "position x", gn->positionX },      
            { "position y", gn->positionY }, 
            { "rotation", gn->rotation },
            { "scale x", gn->scaleX },     
            { "scale y", gn->scaleY },
            { "nodes", nodeData } 
        };
    }

    //--------------- spawner


    if (node->type == SPAWNER)
    {
        auto sn = std::dynamic_pointer_cast<SpawnerNode>(node);

        data = {
   
            { "position x", sn->rectHandle ? sn->rectHandle->position.x : sn->positionX },      
            { "position y", sn->rectHandle ? sn->rectHandle->position.y : sn->positionY }, 
            { "width", sn->width },      
            { "height", sn->height },
            { "alpha", sn->alpha },      
            { "loop", sn->loop },
            { "type of", sn->typeOf }, 
            { "category", sn->category }, 
            { "animation key", sn->animationKey }, 
            { "texture key", sn->textureKey }, 
            { "behavior key", sn->behaviorKey },
            { "sprite sheet key", {
                    { "first", sn->spriteSheetKey.first },
                    { "second", sn->spriteSheetKey.second}
                }
            },
            { "tint", {
                    { "x", sn->tint.x },
                    { "y", sn->tint.y },
                    { "z", sn->tint.z }
                }
            },
            { "body", {
                    { "exist", sn->body.exist },
                    { "is sensor", sn->body.is_sensor },
                    { "type", sn->body.type },
                    { "xOff", sn->body.xOff },
                    { "yOff", sn->body.yOff },
                    { "w", sn->body.w },
                    { "h", sn->body.h },
                    { "density", sn->body.density },
                    { "friction", sn->body.friction },
                    { "restitution", sn->body.restitution }
                } 
            }
        };

    }

    data["type"] = GetType(node->type); 
    data["name"] = node->name;

    return data;
}



//--------------------------------- read data and create node of specific type



std::shared_ptr<Node> Node::ReadData(json& data, bool makeNode, void* scene, std::vector<std::shared_ptr<Node>>& arr)
{
    try { 

        if (!data.contains("type")) {
            Editor::Log("Error reading data. 'type' key not found.");
            return nullptr;
        }
        
        Scene* _scene;
        std::vector<std::string> names; 
        
        if (!makeNode) 
           _scene = static_cast<Scene*>(scene); 

        //sprite

        if (data["type"] == "Sprite") 
        {

            std::shared_ptr<SpriteNode> sn;

            if (makeNode) 
                sn = Make<SpriteNode>(false, arr); 
            
            else 
                sn = Scene::CreateObject<SpriteNode>(_scene); 

            if (!sn)
                return nullptr;

            if (data.contains("name"))
                sn->name = CheckName(data["name"], makeNode ? s_names : names);

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

            if (data.contains("scroll factor x"))
                sn->scrollFactorX = data["scroll factor x"];

            if (data.contains("scroll factor y"))
                sn->scrollFactorY = data["scroll factor y"];

            if (data.contains("lock"))
                sn->lock_in_place = data["lock"]; 

            if (data.contains("cull"))
                sn->cull = data["cull"]; 

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

            if (data.contains("stroke"))
                sn->isStroked = data["stroke"];

            if (data.contains("stroke color"))
                sn->strokeColor = { data["stroke color"]["x"], data["stroke color"]["y"], data["stroke color"]["z"] };

            if (data.contains("stroke width"))
                sn->strokeWidth = data["stroke width"];

            if (data.contains("currentTexture"))
            {
                if (makeNode) {
                    sn->ApplyTexture(data["currentTexture"]);  
                    sn->spriteHandle->SetTexture(data["currentTexture"]);
                }
            }

            if (data.contains("frames") && data["frames"].size() > 1)
            {
                sn->frame_x.clear();
                sn->frame_y.clear();
                sn->frame_width.clear();
                sn->frame_height.clear();
                sn->frame_fX.clear();
                sn->frame_fY.clear();

                for (int i = 0; i < data["frames"].size(); i++) 
                {   
                    sn->frame_x.push_back(data["frames"][i]["offset x"]);
                    sn->frame_y.push_back(data["frames"][i]["offset y"]);
                    sn->frame_width.push_back(data["frames"][i]["width"]);
                    sn->frame_height.push_back(data["frames"][i]["height"]);
                    sn->frame_fX.push_back(data["frames"][i]["factor x"]);
                    sn->frame_fY.push_back(data["frames"][i]["factor y"]);
                }

                sn->frame = data["frames"].size();

                if (data.contains("current frame"))
                    sn->currentFrame = data["current frame"];
      
                for (int i = 0; i < sn->frame; i++) 
                    sn->frames.push_back({ sn->frame_x[i], sn->frame_y[i], sn->frame_width[i], sn->frame_height[i], sn->frame_fX[i], sn->frame_fY[i]}); 

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
                {
                    sn->AddComponent(Component::ANIMATOR, false);

                    for (const auto& anim : data["components"]["animator"]["animations"]) {  
                        sn->animations.push_back({ anim["key"], anim["start"], anim["end"], anim["rate"], anim["repeat"], anim["yoyo"] });
                        sn->ApplyAnimation(anim["key"]);
                    }
                 
                    sn->anim_to_play_on_start.key = data["components"]["animator"]["on start"]["key"];
                    sn->anim_to_play_on_start.rate = data["components"]["animator"]["on start"]["rate"];
                    sn->anim_to_play_on_start.repeat = data["components"]["animator"]["on start"]["repeat"];
                    sn->anim_to_play_on_start.yoyo = data["components"]["animator"]["on start"]["yoyo"];

                }

                //physics 

                if (data["components"]["physics"]["exists"]) 
                {
                    sn->AddComponent(Component::PHYSICS, false);

                    sn->friction = data["components"]["physics"]["friction"];
                    sn->restitution = data["components"]["physics"]["restitution"];
                    sn->density = data["components"]["physics"]["density"]; 

                    if (data["components"]["physics"]["bodies"].size()) 
                        for (const auto& body : data["components"]["physics"]["bodies"]) 
                            if (makeNode)
                                sn->CreateBody(body["bodyX"], body["bodyY"], body["body_width"], body["body_height"], body["sensor"], body["pointer"]);

                            else 
                            {
                                SpriteNode::BoolContainer bc;

                                if (body.contains("type"))
                                    sn->body_type = body["type"];

                                if (body.contains("shape"))
                                    sn->body_shape = body["shape"];

                                bc.b = body["sensor"];
                                sn->is_sensor.push_back(bc);
                                sn->body_pointer.push_back(body["pointer"]);
                                sn->bodies.push_back({ nullptr, body["bodyX"], body["bodyY"], body["body_width"], body["body_height"]});
                            }
                }

                //script

                if (data["components"]["script"]["exists"]) 
                {
                    
                    sn->AddComponent(Component::SCRIPT, false);

                    if (data["components"]["script"]["scripts"].size())
                        for (const auto& scripts : data["components"]["script"]["scripts"])
                            for (const auto& file : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::Get()->script_dir))
                                if (file.exists() && static_cast<std::string>(scripts["key"]) == Editor::Get()->events->GetScriptName(file.path().string()))
                                    sn->behaviors.insert({ static_cast<std::string>(scripts["key"]).c_str(), static_cast<std::string>(scripts["value"]).c_str() });   
                }

                //shader

                if (data["components"]["shader"]["exists"]) 
                {

                    sn->AddComponent(Component::SHADER, false); 

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
                tmn = Make<TilemapNode>(false, arr); 
            
            else 
                tmn = Scene::CreateObject<TilemapNode>(_scene);  

            if (!tmn)
                return nullptr;

            if (data.contains("name"))
                tmn->name = CheckName(data["name"], makeNode ? s_names : names);

            if (data.contains("positionX"))
                tmn->positionX = data["positionX"];

            if (data.contains("positionY"))
                tmn->positionY = data["positionY"];

            if (data.contains("rotation"))
                tmn->rotation = data["rotation"];

            if (data.contains("scaleX"))
                tmn->scaleX = data["scaleX"];

            if (data.contains("scaleY"))
                tmn->scaleY = data["scaleY"];    

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
                    tmn->AddComponent(Component::PHYSICS, false);

                if (data["components"]["physics"]["bodies"].size())
                    for (const auto& body : data["components"]["physics"]["bodies"]) 
                    {
                        if (makeNode) {
                            tmn->CreateBody(body["bodyX"], body["bodyY"], body["body_width"], body["body_height"]);

                            for (int i = 0; i < tmn->bodies.size(); i++)
                                tmn->UpdateBody(i);
                        }

                        else 
                            tmn->bodies.push_back({ nullptr, body["bodyX"], body["bodyY"], body["body_width"], body["body_height"] });
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
                an = Make<AudioNode>(false, arr); 
                
            if (!an)
                return nullptr;

            if (data.contains("name"))
                an->name = CheckName(data["name"], makeNode ? s_names : names);

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
                en = Make<EmptyNode>(false, arr); 
            
            else 
                en = Scene::CreateObject<EmptyNode>(_scene);   

            if (!en)
                return nullptr;

            if (data.contains("name"))
                en->name = CheckName(data["name"], makeNode ? s_names : names);

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

                if (en->debugGraphic) {
                    en->debugGraphic->tint.x = data["tint"]["x"];
                    en->debugGraphic->tint.y = data["tint"]["y"];
                    en->debugGraphic->tint.z = data["tint"]["z"];
                    en->debugGraphic->alpha = data["alpha"];
                }
            }

            if (data.contains("components"))
            {
                //script

                if (data["components"]["script"]["exists"]) 
                {
                    en->AddComponent(Component::SCRIPT, false);

                    if (data["components"]["script"]["scripts"].size())
                        for (const auto& scripts : data["components"]["script"]["scripts"])
                            for (const auto& file : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::Get()->script_dir))
                                if (file.exists() && static_cast<std::string>(scripts["key"]) == Editor::Get()->events->GetScriptName(file.path().string()))
                                    en->behaviors.insert({ static_cast<std::string>(scripts["key"]), static_cast<std::string>(scripts["value"]) });
                }

                //shader

                if (data["components"]["shader"]["exists"]) 
                {
                    en->AddComponent(Component::SHADER, false);

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
                tn = Make<TextNode>(false, arr); 
            
            else 
                tn = Scene::CreateObject<TextNode>(_scene);

            if (!tn)
                return nullptr;

            if (data.contains("name"))
                tn->name = CheckName(data["name"], makeNode ? s_names : names);

            if (data.contains("content"))
                tn->textBuf = data["content"];

            if (data.contains("tint"))
                tn->tint = { data["tint"]["x"], data["tint"]["y"], data["tint"]["z"] };     

            if (data.contains("alpha"))
                tn->alpha = data["alpha"];  

            if (data.contains("shadow"))
                tn->isShadow = data["shadow"];
                
            if (data.contains("stroke"))
                tn->isStroked = data["stroke"];

            if (data.contains("character offset x"))
                tn->charOffsetX = data["character offset x"];

            if (data.contains("character offset y"))
                tn->charOffsetY = data["character offset y"];

            if (data.contains("stroke color"))
                tn->strokeColor = { data["stroke color"]["x"], data["stroke color"]["y"], data["stroke color"]["z"] };

            if (data.contains("shadow color"))
                tn->shadowColor = { data["shadow color"]["x"], data["shadow color"]["y"], data["shadow color"]["z"] };

            if (data.contains("stroke width"))
                tn->strokeWidth = data["stroke width"]; 

            if (data.contains("shadow distance x"))
                tn->shadowDistanceX = data["shadow distance x"]; 

            if (data.contains("shadow distance y"))
                tn->shadowDistanceY = data["shadow distance y"]; 

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

            if (data.contains("current font")) {
                tn->currentFont = data["current font"];
                if (makeNode)
                    tn->ChangeFont(tn->currentFont);
            }

            //script

            if (data.contains("components") && data["components"]["script"]["exists"]) 
            {
                tn->AddComponent(Component::SCRIPT, false);

                if (data["components"]["script"]["scripts"].size())
                    for (const auto& scripts : data["components"]["script"]["scripts"])
                        for (const auto& file : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::Get()->script_dir))
                            if (file.exists() && static_cast<std::string>(scripts["key"]) == Editor::Get()->events->GetScriptName(file.path().string()))
                                tn->behaviors.insert({ static_cast<std::string>(scripts["key"]).c_str(), static_cast<std::string>(scripts["value"]).c_str() });

            }

            return tn;
        }

        //group

        if (data["type"] == "Group")
        {
            std::shared_ptr<GroupNode> gn;

            if (makeNode)
                gn = Make<GroupNode>(false, arr); 
            
            else 
                gn = Scene::CreateObject<GroupNode>(_scene);

            if (!gn)
                return nullptr;
            
            if (data.contains("name"))
                gn->name = CheckName(data["name"], makeNode ? s_names : names);

            if (data.contains("position x"))
                gn->positionX = data["position x"];
            if (data.contains("position y"))
                gn->positionY = data["position y"];
            if (data.contains("rotation"))
                gn->rotation = data["rotation"];
            if (data.contains("scale x"))
                gn->scaleX = data["scale x"];
            if (data.contains("scale y"))
                gn->scaleY = data["scale y"];

            if (data.contains("nodes") && data["nodes"].size()) 
                for (auto& node : data["nodes"]) 
                    ReadData(node["data"], makeNode, makeNode ? nullptr : _scene, gn->_nodes);
            
            return gn;
        }

        //spawner

        if (data["type"] == "Spawner")
        {
            std::shared_ptr<SpawnerNode> sn;

            if (makeNode) {
                sn = Make<SpawnerNode>(false, arr); 
                sn->CreateMarker();
            }
            
            else 
                sn = Scene::CreateObject<SpawnerNode>(_scene);

            if (!sn)
                return nullptr;

            if (data.contains("name"))
                sn->name = CheckName(data["name"], makeNode ? s_names : names);

            if (data.contains("type of"))
                sn->typeOf = data["type of"]; 

            if (data.contains("category"))
                sn->category = data["category"];

            if (data.contains("loop"))
                sn->loop = data["loop"];

            if (data.contains("alpha"))
                sn->alpha = data["alpha"];

            if (data.contains("tint"))
                sn->tint = { data["tint"]["x"], data["tint"]["y"], data["tint"]["z"] };

            if (data.contains("behavior key"))
                sn->behaviorKey = data["behavior key"]; 

            if (data.contains("animation key"))
                sn->animationKey = data["animation key"]; 

            if (data.contains("texture key")) {
                sn->textureKey = data["texture key"];
                
                if (makeNode)
                    sn->ApplyTexture(data["texture key"]);
            }

            if (data.contains("sprite sheet key"))
                sn->spriteSheetKey = { data["sprite sheet key"]["first"], data["sprite sheet key"]["second"] };
        
            if (data.contains("position x"))
                sn->positionX = data["position x"];   

            if (data.contains("position y"))
                sn->positionY = data["position y"];  

            if (data.contains("width"))
                sn->width = data["width"];   

            if (data.contains("height"))
                sn->height = data["height"];  

            if (data.contains("body")) {
                sn->body.exist = data["body"]["exist"]; 
                sn->body.w = data["body"]["w"]; 
                sn->body.h = data["body"]["h"]; 
                sn->body.xOff = data["body"]["xOff"]; 
                sn->body.yOff = data["body"]["yOff"];
                sn->body.is_sensor = data["body"]["is sensor"]; 
                sn->body.type = data["body"]["type"];
                sn->body.density = data["body"]["density"];
                sn->body.friction = data["body"]["friction"]; 
                sn->body.restitution = data["body"]["restitution"];
            }

            return sn;
        }
    } 

    catch (std::runtime_error& err) {
        Editor::Log("error reading data: " + (std::string)err.what());
    }
}