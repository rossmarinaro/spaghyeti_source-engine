#include "./node.h"
#include "../editor.h"
#include "../assets/assets.h"

using namespace editor;


GroupNode::GroupNode(bool init):  
    Node(init, GROUP) {
        if (m_init)
            Editor::Log("Group node " + name + " created.");   
    }

         
//---------------------------


GroupNode::~GroupNode() {
    if (m_init)
        Editor::Log("Group node " + name + " deleted.");
}


//---------------------------


void GroupNode::Reset(int component_type) {
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
                    
            Node::Update(node, arr);
            
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
                    Node::Make<GroupNode>(true, _nodes);

                if (ImGui::MenuItem("Empty"))
                    Node::Make<EmptyNode>(true, _nodes); 

                if (ImGui::MenuItem("Audio"))
                    Node::Make<AudioNode>(true, _nodes); 

                if (ImGui::MenuItem("Text"))
                    Node::Make<TextNode>(true, _nodes); 

                if (ImGui::MenuItem("Sprite"))
                    Node::Make<SpriteNode>(true, _nodes); 

                if (ImGui::MenuItem("Tilemap"))
                    Node::Make<TilemapNode>(true, _nodes); 

                if (ImGui::MenuItem("Spawner"))
                    Node::Make<SpawnerNode>(true, _nodes); 

                ImGui::EndMenu(); 
            }

            if (show_options) { 
                ImGui::SliderFloat("position x", &positionX, -System::Window::s_width, System::Window::s_width);
                ImGui::SliderFloat("position y", &positionY, -System::Window::s_height, System::Window::s_height);
                ImGui::SliderFloat("rotation", &rotation, 0.0f, 360.0f);
                ImGui::SliderFloat("scale x", &scaleX, -100.0f, 100.0f);
                ImGui::SliderFloat("scale y", &scaleY, -100.0f, 100.0f);
            }

            //update active nodes
        
            if (_nodes.size()) 
            {
                int i = 0;

                for (const auto& node : _nodes)
                    if (node && node->active) {
                        ImGui::PushID(i);
                        i++;
                        node->Update(node, _nodes);
                        ImGui::PopID();
                    }
            }          
        
            ImGui::TreePop();
        }

        ImGui::PopID();
    }
}


//------------------------------------ 


void GroupNode::Render(float _positionX, float _positionY, float _rotation, float _scaleX, float _scaleY)
{ 
    //render any active child nodes

    if (_nodes.size())
        for (const auto& node : _nodes)
            if (node && node->active) 
                node->Render(positionX + _positionX, positionY + _positionY, rotation + _rotation, scaleX * _scaleX, scaleY * _scaleY);
}


