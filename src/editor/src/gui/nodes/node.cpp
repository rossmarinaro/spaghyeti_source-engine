#include <algorithm>

#include "./node.h"
#include "../../editor.h"
#include "../../assets/assets.h"

#include "../../../../../vendors/UUID.hpp"


Node::Node(const std::string &id, std::string type, std::string name): 
    m_name(name),
    m_ID(id),
    m_type(type),
    created(false),
    m_active(true),
    show_options(false),
    positionX(0.0f),
    positionY(0.0f), 
    scaleX(1.0f),
    scaleY(1.0f),
    rotation(0.0f)
{

    for (auto &node : nodes)
        if (node->m_name == this->m_name)
            this->m_name = this->m_name + "_" +std::to_string(count); 
}



//---------------------------


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


//---------------------------


Node* Node::MakeNode(const char* type)
{

    if (count > MAX_NODES) {
        Editor::Log("Max nodes reached.");
        return nullptr; 
    }

    //generate UUID and replace hyphens with underscores

    std::string uuid = UUID::generate_uuid();

    for (int i = 0; i < uuid.length(); i++)
        if (uuid[i] == '-')
            uuid[i] = '_';

    if (strcmp(type, "Empty") == 0) 
        nodes[count] = Create<EmptyNode>(uuid);

    else if (strcmp(type, "Audio") == 0) 
        nodes[count] = Create<AudioNode>(uuid);

    else if (strcmp(type, "Text") == 0) 
        nodes[count] = Create<TextNode>(uuid);
    
    else if (strcmp(type, "Sprite") == 0) 
        nodes[count] = Create<SpriteNode>(uuid);
    
    else if (strcmp(type, "Tilemap") == 0) 
        nodes[count] = Create<TilemapNode>(uuid);
    
    else return nullptr;

    Node* node = nodes[count]; 

    count++;

    return node;
}


//-------------------------


void Node::DeleteNode (Node* node)
{

    node->m_active = false;

    //delete all attached components

    if (node->components.size())
        for (auto &component : node->components)
            node->RemoveComponent(component);

    //delete node instance

    std::vector<Node*>::iterator it = std::find(nodes.begin(), nodes.end(), node);

    if (it != nodes.end())
        nodes.erase(it);

    delete node;
    node = nullptr;

    count--;

}


//--------------------------


void Node::ClearAll ()
{

    for (auto &node : nodes)
        DeleteNode(node);

    count = 0;

    Editor::Log("nodes deleted.");
}

 

//--------------------------


void Node::AddComponent(const char* type, bool init)
{

    //return if component exists 
    
    for (auto &component : this->components)
        if ((std::string)component->m_type == type) 
            return;
        
    Component* component = new Component(this->m_ID, type);

    components.push_back(component); 

    if (init)
        component->Make();

}


//------------------------------


void Node::RemoveComponent(Component* component)
{

    std::vector<Component*>::iterator it = std::find(components.begin(), components.end(), component);

    if (it != components.end())
        components.erase(it);

    if (component) {
        delete component;
        component = nullptr;
    }
}


//------------------------------


const Component* Node::GetComponent(const char* type)
{
    for (const auto &component : components)
        if (strcmp(type, component->m_type) == 0) 
            return component;
        else
            return nullptr;
}


//------------------------------


const bool Node::HasComponent(const char* type)
{
    for (const auto &component : components)
        return strcmp(type, component->m_type) == 0; 
}


//-----------------------------


void Node::LoadShader(Node* node, const std::string &name, const std::string &vertPath, const std::string &fragPath)
{

    node->shader = { name, { vertPath, fragPath } };  

    Shader::Load(name, vertPath.c_str(), fragPath.c_str(), nullptr);

    if (node->m_type == "Sprite")
    {
        SpriteNode* sn = dynamic_cast<SpriteNode*>(node);

        if (sn->spriteHandle)
            sn->spriteHandle->m_shader = Shader::GetShader(name);
    }

    if (node->m_type == "Empty")
    {
        EmptyNode* en = dynamic_cast<EmptyNode*>(node);

        if (en->m_debugGraphic)
            en->m_debugGraphic->m_shader = Shader::GetShader(name);
    }
}