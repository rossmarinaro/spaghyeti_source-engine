#include "string.h"

#include "./component.h"
#include "../gui/nodes/node.h"
#include "../editor.h"
#include "../../../../build/include/app.h"


//------------------------------------


Component::Component(const std::string &id, const char* type, bool init): 
    m_ID(id),
    m_type(type),
    m_initialized(false),
    m_name("")
{
    
    count++;  

    if (!init)
        return;

    this->Make(type);

}

//-------------------------------------


Component::~Component() 
{ 

    for (auto &node : Node::nodes) 
        if (node->m_ID == this->m_ID) 
        {  

            //script

            // if (strcmp(this->m_type, "Script") == 0)
            //     if(this->m_resourcePath.size() > 0)
            //         remove(this->m_resourcePath.c_str());

            //animator

            if (strcmp(this->m_type, "Animator") == 0)
            {
                if (node->m_type == "Sprite") 
                {
                    SpriteNode* sn = dynamic_cast<SpriteNode*>(node);

                    sn->animBuf1.clear();
                    sn->animBuf2.clear();
                    sn->animBuf3.clear();
                    sn->animBuf4.clear();
                }
            }

            //physics

            if (strcmp(this->m_type, "Physics") == 0)
            {
            
                if (node->m_type == "Sprite") {

                    SpriteNode* sn = dynamic_cast<SpriteNode*>(node);

                    for (const auto &body : sn->bodies)
                        Game::physics->DestroyBody(body.first);

                    sn->bodyX.clear();
                    sn->bodyY.clear();
                    sn->body_width.clear();
                    sn->body_height.clear();
                    sn->is_sensor.clear();
                    sn->body_pointer.clear();
                    sn->bodies.clear();

                    sn->restitution = 0.0f;
                    sn->density = 0.0f;
                    sn->friction = 0.0f;
                }

                if (node->m_type == "Tilemap") 
                {

                    TilemapNode* tmn = dynamic_cast<TilemapNode*>(node); 

                    for (auto &body : tmn->bodies)
                        Game::physics->DestroyBody(body);

                    tmn->bodyX.clear();
                    tmn->bodyY.clear();
                    tmn->body_width.clear();
                    tmn->body_height.clear();
                    
                    tmn->bodies.clear();

                }

            }
        }

    Editor::Log("Component " + this->m_ID + " removed."); 
}


//-------------------------------


void Component::Make(const std::string &name)
{

    //animator

    if (strcmp(this->m_type, "Animator") == 0)
    {
        
    }


    //shader


    if (strcmp(this->m_type, "Shader") == 0)
    {

        std::ofstream src;

        m_resourcePath = Editor::projectPath + "\\resources\\shaders\\" + m_ID + ".glsl";

        src.open(m_resourcePath, std::ofstream::app | std::ofstream::out);

        // #version 330 core

        // layout (location = 0) in vec2 vert;
        // layout (location = 1) in vec2 UV;

        // out vec2 uv;

        // uniform mat4 model;
        // uniform mat4 projection;
        // uniform float zoom;

        // void main()
        // {          
        //     uv = UV;
        //     gl_Position = projection * model * vec4(vert.xy * zoom, 0.0, 1.0);
        // }; 

        // #version 330 core

        // in vec2 uv;
        // out vec4 color;

        // uniform sampler2D image;
        // uniform vec3 spriteColor;
        // uniform float alphaVal;
        // uniform int repeat;

        // void main()
        // {

        //     color = vec4(spriteColor, alphaVal) * texture(image, uv * repeat);  
        // };
 
        src.close();
    }


    //script


    if (strcmp(this->m_type, "Script") == 0)
    {

        std::ofstream src;

        m_resourcePath = Editor::projectPath + "\\resources\\scripts\\" + name + ".h";

        src.open(m_resourcePath, std::ofstream::app | std::ofstream::out);

        if (!this->m_initialized)
        {

            this->m_initialized = true;

            std::string root_path = Editor::rootPath;
            std::replace(root_path.begin(), root_path.end(), '\\', '/');
            
            src << "#pragma once\n\n"; 
            src << "#include \"" + root_path + "/include/behaviors.h\"\n\n";    

        } 

        src <<  "class " + name + " : public Behavior {\n\n"; 
        src <<  "    public:\n\n";
        src <<  "        //constructor, called on start\n\n";
        src <<           name + " (Entity* entity):\n";
        src <<  "            Behavior(entity)\n";
        src <<  "        {\n\n";      
        src <<  "        }\n\n"; 
        src <<  "        //update every frame\n\n";
        src <<  "        void Update(Inputs* inputs, Camera* camera) override {\n\n";
        src <<  "        }\n\n";
        src <<  "};";
 
        src.close();

    }


    //physics


    if (strcmp(this->m_type, "Physics") == 0)
    {

        for (auto &node : Node::nodes) 
            if (node->m_ID == m_ID) 
            { 

                if (node->m_type == "Sprite") {

                    SpriteNode* sn = dynamic_cast<SpriteNode*>(node);

                    sn->CreateBody("dynamic");
                }

                if (node->m_type == "Tilemap") {

                    TilemapNode* tmn = dynamic_cast<TilemapNode*>(node);
                    
                    tmn->CreateBody();

                }

            }
    }
}