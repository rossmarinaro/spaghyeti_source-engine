#include <algorithm>

#include "./node.h"
#include "../editor.h"
#include "../assets/assets.h"

#include "../../../../vendors/UUID.hpp"

using namespace editor;


Node::Node(const std::string& type): 
    m_type(type) {}



//-------------------------------


Node::Node(const std::string &id, const std::string& type, const std::string& name): 
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

    node->m_active = false;

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
        
    auto component = std::make_shared<Component>(this->m_ID, type, this->m_type);

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
            sn->spriteHandle->m_shader = Shader::GetShader(name);
    }

    if (node->m_type == "Empty") {
        auto en = std::dynamic_pointer_cast<EmptyNode>(node);

        if (en->m_debugGraphic)
            en->m_debugGraphic->m_shader = Shader::GetShader(name);
    }
}