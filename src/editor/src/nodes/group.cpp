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


void GroupNode::Reset(int component_type) 
{
    Editor::Get()->events->selected_nodes = &nodes;

    for (const auto& node : _nodes) 
        node->Reset();
    
    _nodes.clear();
}


//---------------------------


void GroupNode::Update(std::vector<std::shared_ptr<Node>>& arr)
{
    ImGui::Separator(); 

    {
        assert(active);

        ImGui::PushID(("(Group) " + name).c_str());

        if (GUI::Get()->collapseFolders)
            ImGui::SetNextItemOpen(false, ImGuiCond_Always);

        if (ImGui::TreeNode(("(Group) " + name).c_str()))
        {
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                Editor::Get()->events->selected_nodes = &nodes;

            Node::Update(arr);
            
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
               
                if (ImGui::MenuItem("Group")) {
                    Node::Make<GroupNode>(true, _nodes);
                    EventListener::UpdateSession();
                }

                if (ImGui::MenuItem("Empty")) {
                    Node::Make<EmptyNode>(true, _nodes); 
                    EventListener::UpdateSession();
                }

                if (ImGui::MenuItem("Audio")) {
                    Node::Make<AudioNode>(true, _nodes); 
                    EventListener::UpdateSession();
                }

                if (ImGui::MenuItem("Text")) {
                    Node::Make<TextNode>(true, _nodes); 
                    EventListener::UpdateSession();
                }

                if (ImGui::MenuItem("Sprite")) {
                    Node::Make<SpriteNode>(true, _nodes); 
                    EventListener::UpdateSession();
                }

                if (ImGui::MenuItem("Tilemap")) {
                    Node::Make<TilemapNode>(true, _nodes); 
                    EventListener::UpdateSession();
                }

                if (ImGui::MenuItem("Spawner")) {
                    Node::Make<SpawnerNode>(true, _nodes); 
                    EventListener::UpdateSession();
                }

                ImGui::EndMenu(); 
            }

            if (show_options) 
            { 
                ImGui::SliderFloat("position x", &positionX, -Editor::Get()->worldWidth, Editor::Get()->worldWidth);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();

                ImGui::SliderFloat("position y", &positionY, -Editor::Get()->worldHeight, Editor::Get()->worldHeight);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();

                ImGui::SliderFloat("scale x", &scaleX, -100.0f, 100.0f);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();

                ImGui::SliderFloat("scale y", &scaleY, -100.0f, 100.0f);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    EventListener::UpdateSession();
            }

            //update active nodes
        
            if (_nodes.size()) 
            {
                int i = 0;

                for (const auto& node : _nodes)
                    if (node && node->active) {
                        ImGui::PushID(i);
                        i++;
                        node->Update(_nodes); 
                        ImGui::PopID();
                    }
            }          
        
            ImGui::TreePop();
        }

        //set nodes for sorting 

        if (ImGui::IsItemClicked() && _nodes.size())
            Editor::Get()->events->selected_nodes = &_nodes;

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


