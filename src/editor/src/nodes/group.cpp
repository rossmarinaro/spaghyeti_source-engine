#include "./node.h"
#include "../editor.h"
#include "../assets/assets.h"

using namespace editor;


GroupNode::GroupNode():  
    Node("Group")
{

    Editor::Log("Group node " + name + " created.");   
}

         
//---------------------------


GroupNode::~GroupNode() {

    if (!virtual_node)
        Editor::Log("Group node " + name + " deleted.");
}


//---------------------------


void GroupNode::Reset(const char* component_type)
{
    for (const auto& node : _nodes) 
        node->Reset();
    
    _nodes.clear();
}


//---------------------------


void GroupNode::Update(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& arr)
{
	
    ImGui::Separator(); 

    {

        assert(active);

        ImGui::PushID(("(Group) " + name).c_str());

        if (ImGui::TreeNode(("(Group) " + name).c_str()))
        {
                    
            static char name_buf[32] = ""; ImGui::InputText("name", name_buf, 32, ImGuiInputTextFlags_CallbackCompletion, ChangeName, &ID);

            if (ImGui::Button("Save prefab")) 
                SavePrefab(); 
            
            if (ImGui::BeginMenu("Add Node"))
            {

                if (ImGui::MenuItem("Load Prefab")) 
                {
                    if (AssetManager::LoadPrefab(_nodes))
                        Editor::Log("Prefab loaded.");
                    else 
                        Editor::Log("There was a problem loading prefab.");
                } 

                ImGui::Separator();
               
                if (ImGui::MenuItem("Group"))
                    Node::Make<GroupNode>(_nodes);

                if (ImGui::MenuItem("Empty"))
                    Node::Make<EmptyNode>(_nodes); 

                if (ImGui::MenuItem("Audio"))
                    Node::Make<AudioNode>(_nodes); 

                if (ImGui::MenuItem("Text"))
                    Node::Make<TextNode>(_nodes); 

                if (ImGui::MenuItem("Sprite"))
                    Node::Make<SpriteNode>(_nodes); 

                if (ImGui::MenuItem("Tilemap"))
                    Node::Make<TilemapNode>(_nodes); 

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Options")) {
                ShowOptions(node, arr);
                ImGui::EndMenu();
            }

            //update active nodes
        
            if (_nodes.size())
                for (const auto &node : _nodes)
                    if (node && node->active)
                        node->Update(node, _nodes);
        
            ImGui::TreePop();
        }

        ImGui::PopID();

    }

}


//------------------------------------


void GroupNode::Render()
{
    //render active nodes

    if (_nodes.size())
        for (const auto &node : _nodes)
            if (node && node->active)
                node->Render();
}


