#include "string.h"

#include "./component.h"
#include "../gui/nodes/node.h"
#include "../assets/assets.h"
#include "../editor.h"
#include "../../../../build/include/app.h"


//------------------------------------


Component::Component(
    const std::string &id, 
    const std::string &type, 
    const std::string &node_type
):
    m_ID(id),
    m_type(type),
    m_nodeType(node_type),
    m_name("")
{
    count++;

    Editor::Log((std::string)type + " component added.");
}

//-------------------------------------


Component::~Component() {
    Editor::Log((std::string)this->m_type + " component" + " removed.");
}


//-------------------------------


void Component::Make()
{

    //shader

    if (this->m_type == "Shader")
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

        //default red tint image shader

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
        frag_src << "   color = vec4(1., 0., 0., 1.) * texture(image, uv);\n";
        frag_src << "};";

        vert_src.close();
        frag_src.close();

        this->filename = "";

    }


    //script


    if (this->m_type == "Script")
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
        src << "#include \"" + root_path + "/include/behaviors.h\"\n\n\n";
        src << "namespace entity_behaviors {\n\n";
        src <<  "   class " + this->filename + " : public Behavior {\n\n";
        src <<  "       public:\n\n";
        src <<  "           //constructor, called on start\n\n";
        src <<  "           " + this->filename + "(std::shared_ptr<Entity> entity):\n";
        src <<  "               Behavior(entity, \""+ this->filename + "\")\n";
        src <<  "           {\n\n";
        src <<  "           }\n\n";
        src <<  "           //update every frame\n\n";
        src <<  "           void Update(Process::Context context) override {\n\n";
        src <<  "           }\n\n";
        src <<  "   };\n";
        src <<  "};";

        src.close();

        this->filename = "";

    }

    //animator

    if (this->m_type == "Animator")
    {

        for (const auto &node : Node::nodes)
            if (node->m_ID == m_ID)
            {

                if (this->m_nodeType == "Sprite") {

                    auto sn = std::dynamic_pointer_cast<SpriteNode>(Node::GetNode(this->m_ID));

                    sn->anim++;
                }

            }
    }

    //physics


    if (this->m_type == "Physics")
    {

        for (const auto &node : Node::nodes)
            if (node->m_ID == m_ID)
            {

                if (this->m_nodeType == "Sprite") {

                    auto sn = std::dynamic_pointer_cast<SpriteNode>(Node::GetNode(this->m_ID));

                    sn->CreateBody("dynamic");
                }

                if (this->m_nodeType == "Tilemap") {

                    auto tmn = std::dynamic_pointer_cast<TilemapNode>(Node::GetNode(this->m_ID));

                    tmn->CreateBody();

                }

            }
    }
}


