#include "string.h"

#include "./component.h"
#include "../gui/nodes/node.h"
#include "../assets/assets.h"
#include "../editor.h"
#include "../../../../build/include/app.h"


//------------------------------------


Component::Component(const std::string &id, const char* type):
    m_ID(id),
    m_type(type),
    m_name("")
{
    count++;

    Editor::Log((std::string)type + " component added.");
}

//-------------------------------------


Component::~Component()
{

    for (const auto &node : Node::nodes)

        if (node->m_ID == this->m_ID)
        {

            //shader

            if (strcmp(this->m_type, "Shader") == 0)
            {

                if (node->m_type == "Sprite") {

                    auto sn = dynamic_cast<SpriteNode*>(node);

                    if (sn->spriteHandle.get())
                        sn->spriteHandle->m_shader = Shader::GetShader("sprite");
                }

                if (node->m_type == "Empty") {
                    
                    auto en = dynamic_cast<EmptyNode*>(node);
                    
                    if (en->m_debugGraphic.get())
                        en->m_debugGraphic->m_shader = Shader::GetShader("graphics");
                }
            }

            //script

            if (strcmp(this->m_type, "Script") == 0)
                node->behaviors.clear();

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
                    sn->anim = 0;
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

    Editor::Log((std::string)this->m_type + " component" + " removed.");
}


//-------------------------------


void Component::Make()
{

    //shader

    if (strcmp(this->m_type, "Shader") == 0)
    {

        if (!this->filename.size())
            return;

        std::string path = Editor::projectPath + AssetManager::shader_dir + "/" + this->filename,
                    vert = path + "/" + this->filename + ".vert",
                    frag = path + "/" + this->filename + ".frag";


        if (std::filesystem::exists(vert) && std::filesystem::exists(frag)) {
            Editor::Log("Shader name already exists!");
            return;
        }

        std::filesystem::create_directory(path);
        std::ofstream vert_src(vert);
        std::ofstream frag_src(frag);

        vert_src << "#version 330 core\n\n";
        vert_src << "layout (location = 0) in vec2 vert;\n";
        vert_src << "layout (location = 1) in vec2 UV;\n\n";
        vert_src << "out vec2 uv;\n\n";
        vert_src << "uniform mat4 model;\n";
        vert_src << "uniform mat4 view;\n";
        vert_src << "uniform mat4 projection;\n";
        vert_src << "uniform vec2 offset;\n\n";
        vert_src << "void main()\n";
        vert_src << "{\n";
        vert_src << "   uv = UV;\n";
        vert_src << "   gl_Position = projection * model * view * vec4(vert.xy + offset.xy, 0.0, 1.0);\n";
        vert_src << "};";

        frag_src << "#version 330 core\n\n";
        frag_src << "in vec2 uv;\n";
        frag_src << "out vec4 color;\n";
        frag_src << "uniform sampler2D image;\n";
        frag_src << "void main()\n";
        frag_src << "{\n";
        frag_src << "   color = vec4(1., 1., 1., 1.) * texture(image, uv);\n";
        frag_src << "};";

        vert_src.close();
        frag_src.close();

        this->filename = "";

    }


    //script


    if (strcmp(this->m_type, "Script") == 0)
    {

        if (!this->filename.size())
            return;

        std::string path = Editor::projectPath + AssetManager::script_dir + "/" + this->filename + ".h";

        if (std::filesystem::exists(path)) {
            Editor::Log("Script name already exists!");
            return;
        }

        std::ofstream src(path);

        std::string root_path = Editor::rootPath;
        std::replace(root_path.begin(), root_path.end(), '\\', '/');

        transform(this->filename.begin(), this->filename.end(), this->filename.begin(), ::toupper);

        src << "#pragma once\n\n";
        src << "#include \"" + root_path + "/include/behaviors.h\"\n\n";

        src <<  "class " + this->filename + "_Behavior : public Behavior {\n\n";
        src <<  "    public:\n\n";
        src <<  "        //constructor, called on start\n\n";
        src <<  "        " + this->filename + "_Behavior (std::shared_ptr<Entity> entity):\n";
        src <<  "            Behavior(entity)\n";
        src <<  "        {\n\n";
        src <<  "        }\n\n";
        src <<  "        //update every frame\n\n";
        src <<  "        void Update(Inputs* inputs, Camera* camera) override {\n\n";
        src <<  "        }\n\n";
        src <<  "};";

        src.close();

        this->filename = "";

    }

    //animator

    if (strcmp(this->m_type, "Animator") == 0)
    {

        for (const auto &node : Node::nodes)
            if (node->m_ID == m_ID)
            {

                if (node->m_type == "Sprite") {

                    SpriteNode* sn = dynamic_cast<SpriteNode*>(node);

                    sn->anim++;
                }

            }
    }

    //physics


    if (strcmp(this->m_type, "Physics") == 0)
    {

        for (const auto &node : Node::nodes)
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


