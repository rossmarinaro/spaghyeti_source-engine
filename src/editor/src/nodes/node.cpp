#include "./node.h"
#include "../editor.h"
#include "../assets/assets.h"
#include "../../../vendors/UUID.hpp"

using namespace editor;


Node::Node(bool init, int type, const std::string& name):
    m_init(init)
{
    ID = Entity::GenerateID();
    created = false;
    active = true;
    show_options = false;
    isStroked = false;
    isShadow = false;
    positionX = 0.0f;
    positionY = 0.0f; 
    actualPositionX = 0.0f;
    actualPositionY = 0.0f; 
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


void Node::Update(std::vector<std::shared_ptr<Node>>& arr) 
{
    static char name_buf[32] = ""; 

    NodeInfo data = { ID, arr };

    ImGui::InputText("name", name_buf, 32, ImGuiInputTextFlags_CallbackCompletion, ChangeName, &data);
    
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("press tab to confirm name.");

    if (ImGui::BeginMenu("Options")) { 
        ShowOptions(arr);
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


const bool Node::CheckCanAddNode(bool init, const std::vector<std::shared_ptr<Node>>& arr) 
{
    if (init && arr.size() > s_MAX_NODES) {
        Editor::Log("Max nodes reached.");
        return false; 
    }

    return true;
}


//--------------------------- get node recursively
    

std::shared_ptr<Node> Node::Get(const std::string& id, const std::vector<std::shared_ptr<Node>>& arr)    
{
    const auto it = std::find_if(arr.begin(), arr.end(), [&id](const auto& node){ return node->ID == id; });

    if (it != arr.end())
    {
        const auto node = *it;

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


void Node::DeleteNode(std::vector<std::shared_ptr<Node>>& arr)
{
    active = false;

    const auto n_it = std::find(s_names.begin(), s_names.end(), name);

    if (n_it != s_names.end()) 
        s_names.erase(std::move(n_it));

    //delete all attached components

    Reset();
    components.clear();

    //delete node instance 

    const auto node = Get(ID, arr);

    if (node) {
        auto it = std::find(arr.begin(), arr.end(), node);
        if (it != arr.end()) {
            it = arr.erase(std::move(it)); 
            --it;
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
    s_names.clear();
}


//---------------------------- options


void Node::ShowOptions(std::vector<std::shared_ptr<Node>>& arr)
{
    if (ImGui::MenuItem("Duplicate")) 
    {
        const auto node = Get(ID, arr);

        if (node)
        {
            json data = WriteData(node);
            
            if (ReadData(data, true, nullptr, arr)) 
                Editor::Log("node " + name + " duplicated.");  
            else
                Editor::Log("node could not be duplicated.");
        }
    }

    if (type != AUDIO && type != TILEMAP && ImGui::MenuItem("Save prefab")) 
        SavePrefab(arr); 

    if (ImGui::BeginMenu("Delete"))
    {
        if (ImGui::MenuItem("Are You Sure?")) 
            DeleteNode(arr);

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


void Node::RemoveComponent(const std::shared_ptr<Component>& component)
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
        const auto component = *it;
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


void Node::ApplyShader(std::shared_ptr<Node>& node, const std::string& name) 
{
    if (node->type == SPRITE) {
        const auto sn = std::dynamic_pointer_cast<SpriteNode>(node);
        if (sn->spriteHandle)
            sn->spriteHandle->SetShader(name);
    }

    if (node->type == EMPTY) {
        const auto en = std::dynamic_pointer_cast<EmptyNode>(node);
        if (en->debugGraphic)
            en->debugGraphic->SetShader(name);
    }
}


//---------------------------------- save prefab


void Node::SavePrefab(std::vector<std::shared_ptr<Node>>& arr) {

    if (AssetManager::SavePrefab(ID, arr))
        Editor::Log("Prefab " + name + " saved.");
    else    
        Editor::Log("There was a problem saving prefab.");
}


//----------------------- shaders


void Node::RenderShaderOptions(const std::string& nodeId, const std::vector<std::shared_ptr<Node>>& arr)
{
    const auto node = Node::Get(nodeId, arr);
    
    if (node)
    {
        const auto component = node->GetComponent(Component::SHADER, node->ID);

        if (!component)
            return;

        if (node->shader.first.length())
        {
            if (ImGui::BeginMenu(("Assigned Shader: " + node->shader.first).c_str()))
            {
                if (ImGui::BeginMenu("edit"))
                {
                    #ifdef _WIN32
                        if (ImGui::MenuItem("vertex"))
                            system((Editor::Get()->projectPath + AssetManager::Get()->shader_dir + "/" + node->shader.first + "/" + node->shader.first + ".vert").c_str());

                        if (ImGui::MenuItem("fragment"))
                            system((Editor::Get()->projectPath + AssetManager::Get()->shader_dir + "/" + node->shader.first + "/" + node->shader.first + ".frag").c_str());
                    #endif

                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }
        }

        else
            ImGui::Text("Assigned Shader: (none)");

        ImGui::Separator();

        if (ImGui::BeginMenu("shader"))
        {
            if (ImGui::BeginMenu("new shader")) 
            {
                
                ImGui::InputText("name", &component->filename);

                if (component->filename.size())
                    if (ImGui::MenuItem("submit")) 
                        component->Make();
                
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("select shader"))
            {

                if (std::filesystem::is_empty(Editor::projectPath + AssetManager::Get()->shader_dir))
                    ImGui::Text("no shaders in directory.");

                else
                    for (const auto &dir : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::Get()->shader_dir))
                    {

                        if (dir.is_directory())
                        {
                            std::string name = dir.path().filename().string();
                            
                            if (ImGui::MenuItem(name.c_str()))
                            {

                                std::string vertPath, fragPath;

                                for (const auto &shader : std::filesystem::recursive_directory_iterator(dir))
                                {
                                    std::string filename = shader.path().string();

                                    if (System::Utils::str_endsWith(filename, ".vert")) 
                                        vertPath = filename;
                        
                                    if (System::Utils::str_endsWith(filename, ".frag"))
                                        fragPath = filename;       
                                }

                                std::replace(vertPath.begin(), vertPath.end(), '\\', '/');
                                std::replace(fragPath.begin(), fragPath.end(), '\\', '/');

                                Node::LoadShader(node, name, vertPath, fragPath);
                                     
                            }

                        }

                    }

                ImGui::EndMenu();
            }
            
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("remove shader?"))
        {
            if (ImGui::MenuItem("yes"))
            {
                node->RemoveComponent(component); 
                
                if (node->shader.first.length())
                    Graphics::Shader::UnLoad(node->shader.first);
            }
            
            ImGui::EndMenu();
        }
    }
}


//----------------------- scripts


void Node::RenderScriptOptions(const std::string& nodeId, const std::vector<std::shared_ptr<Node>>& arr)
{
    const auto node = Node::Get(nodeId, arr);
    const auto am = AssetManager::Get();

    if (node)
    {
        const auto component = node->GetComponent(Component::SCRIPT, node->ID);

        if (!component)
            return; 

        if (node->behaviors.size())
        {
            ImGui::Text("Assigned Behaviors:");

            for (const auto &behavior : node->behaviors)
                if (ImGui::BeginMenu(behavior.first.c_str()))
                {
                    if (ImGui::MenuItem("edit"))
                        #ifdef _WIN32
                            system((Editor::projectPath + am->script_dir + "/" + behavior.second).c_str());
                        #endif

                    if (ImGui::BeginMenu("remove behavior?")) 
                    {
                        if (ImGui::MenuItem("yes")) {

                            std::map<std::string, std::string>::iterator it = node->behaviors.find(behavior.first);

                            if (it != node->behaviors.end())
                                node->behaviors.erase(it);
                        }
                        
                        ImGui::EndMenu();
                    }

                    ImGui::EndMenu();
                }
        }

        else
            ImGui::Text("Assigned Behaviors: (none)");

        ImGui::Separator();

        if (ImGui::BeginMenu("behavior library"))
        {
            if (ImGui::BeginMenu("new behavior")) 
            {
                
                ImGui::InputText("name", &component->filename);

                if (component->filename.size())
                    if (ImGui::MenuItem("submit")) 
                        component->Make();
                
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("select behavior"))
            {

                if (std::filesystem::is_empty(Editor::projectPath + am->script_dir))
                    ImGui::Text("no behaviors in directory.");

                else 
                {
                    std::string assetDir = am->script_dir;

                    //iterate over script dirs

                    for (const auto& script : std::filesystem::recursive_directory_iterator(Editor::projectPath + am->script_dir)) 
                        if (!script.is_directory())
                        {

                            std::string filename = script.path().filename().string();

                            if (System::Utils::str_endsWith(filename, ".h"))
                                if (ImGui::MenuItem(System::Utils::ReplaceFrom(filename, ".", "").c_str())) 
                                {
                                    std::replace(Editor::projectPath.begin(), Editor::projectPath.end(), '\\', '/');
                                    std::replace(assetDir.begin(), assetDir.end(), '\\', '/');

                                    std::string line; 
                                    std::ifstream src(script.path().string());

                                    while (src >> line)
                                        if (line == "class")  
                                            if (src >> line)
                                                node->behaviors.insert({ line, filename });  

                                    src.close();

                                }
                        }
                }

                ImGui::EndMenu();
            }
            
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("remove script?"))
        {
            if (ImGui::MenuItem("yes"))
                node->RemoveComponent(component); 
            
            ImGui::EndMenu();
        }
    }
}


