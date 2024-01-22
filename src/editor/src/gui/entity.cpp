#include "./gui.h"
#include "../editor.h"
#include "../gui/nodes/node.h"
#include "../../../../build/include/app.h"

void GUI::RenderNodes()
{

    if (ImGui::BeginMenu("New Game Object"))
    {

        if (ImGui::MenuItem("Empty"))
            Node::MakeNode("Empty");

        if (ImGui::MenuItem("Audio"))
            Node::MakeNode("Audio");

        if (ImGui::MenuItem("Text"))
            Node::MakeNode("Text");

        if (ImGui::MenuItem("Sprite"))
            Node::MakeNode("Sprite");

        if (ImGui::MenuItem("Tilemap"))
            Node::MakeNode("Tilemap");

        ImGui::EndMenu();
    }
        
    for (auto &node : Node::nodes)
        if (node && node->m_active)
            node->Render();
        
}

