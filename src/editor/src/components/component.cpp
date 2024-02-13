#include "string.h"

#include "./component.h"
#include "../gui/nodes/node.h"
#include "../editor.h"
#include "../../../../build/include/app.h"

Component::Component(const std::string &id, const char* type): 
    m_ID(id),
    m_type(type),
    m_initialized(false),
    m_name("")
{
    
    count++;  

    //----------------animator


    if (strcmp(this->m_type, "Animator") == 0)
    {
        
    }


    //---------------- shader


    if (strcmp(this->m_type, "Shader") == 0)
    {
        
    }


    //-----------------script


    if (strcmp(this->m_type, "Script") == 0)
    {

        std::ofstream src;

        m_resourcePath = Editor::projectPath + "\\resources\\scripts\\" + id + ".h";

        src.open(m_resourcePath, std::ofstream::app | std::ofstream::out);

        if (!this->m_initialized)
        {

            this->m_initialized = true;
            
            src << "#pragma once\n\n"; 
            src << "#include \"" + Editor::rootPath + "/build/include/entity.h\"\n\n";   
            src << "using app = System::Application;\n";

        } 

        src <<  "class Sprite_" + id + " : public Sprite {\n\n";
        src <<  "    public:\n\n";
        src <<  "        //constructor, called on start\n\n";
        src <<  "        Sprite_" + id + " (const std::string &key, float x, float y):\n";
        src <<  "            Sprite(key, glm::vec2(x, y))\n";
        src <<  "        {\n\n";      
        src <<  "        }\n\n"; 
        src <<  "        //update every frame\n\n";
        src <<  "        void Update(Inputs* inputs) override {\n\n";
        src <<  "        }\n\n";
        src <<  "};";

        src.close();

    };

    //------------------physics


    if (strcmp(this->m_type, "Physics Body") == 0)
    {

        for (auto &node : Node::nodes) 
            if (node->m_ID == id) 
            { 

                if (node->m_type == "Sprite") {

                    SpriteNode* sn = dynamic_cast<SpriteNode*>(node);

                    sn->spriteHandle->m_body.self = Game::physics->CreateDynamicBody(glm::vec2(sn->spriteHandle->m_position.x, sn->spriteHandle->m_position.y), glm::vec2(sn->body_width, sn->body_height)); 
                }

                if (node->m_type == "Tilemap") {

                    TilemapNode* tn = dynamic_cast<TilemapNode*>(node);
                    
                    tn->CreateBody();

                }

            }
    }

}


//-------------------------------------


Component::~Component() 
{ 

    if (strcmp(this->m_type, "Script") == 0)
        if(this->m_resourcePath.size() > 0)
            remove(this->m_resourcePath.c_str());

    Editor::Log("Component " + this->m_ID + " removed."); 
}


