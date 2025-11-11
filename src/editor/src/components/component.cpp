#include "string.h"

#include "./component.h"
#include "../nodes/node.h"
#include "../assets/assets.h"
#include "../editor.h"
#include "../../../../build/sdk/include/app.h"


using namespace editor;


const std::string Component::Get(int type) {

    switch (type) {
        case SHADER: return "Shader";
        case SCRIPT: return "Script";
        case ANIMATOR: return "Animator";
        case PHYSICS: return "Physics";
    }
}


//-------------------------------


Component::Component(const std::string& id, int type, int node_type, bool init):
    m_init(init)
{
    s_count++;

    ID = id;
    name = "";
    
    this->type = type;
    this->m_nodeType = node_type;

    if (m_init)
        Editor::Log(Get(this->type) + " component added.");
}


//-------------------------------------


Component::~Component() {
    if (m_init) {
        s_count--;
        Editor::Log(Get(type) + " component" + " removed.");
    }
}



//-------------------------------


void Component::Make()
{

    //shader

    if (type == SHADER)
    {

        if (!filename.size())
            return;

        std::string path = Editor::projectPath + AssetManager::Get()->shader_dir + "/" + filename,
                    vert = path + "/" + filename + ".vert",
                    frag = path + "/" + filename + ".frag";


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
        vert_src << "uniform mat4 mvp;\n";
        vert_src << "uniform vec2 offset;\n\n";
        vert_src << "void main()\n";
        vert_src << "{\n";
        vert_src << "   uv = UV;\n";
        vert_src << "   gl_Position = mvp * vec4(vert.xy + offset.xy, 0.0, 1.0);\n";
        vert_src << "}";

        frag_src << "#version 330 core\n\n";
        frag_src << "in vec2 uv;\n";
        frag_src << "out vec4 color;\n";
        frag_src << "uniform sampler2D image;\n";
        frag_src << "void main()\n";
        frag_src << "{\n";
        frag_src << "   color = vec4(1., 0., 0., 1.) * texture(image, uv);\n";
        frag_src << "}";

        vert_src.close();
        frag_src.close();

        filename = "";

    }


    //script


    if (type == SCRIPT)
    {

        if (!filename.size())
            return;

        const std::string path = Editor::projectPath + AssetManager::Get()->script_dir + "/" + filename + ".h";

        std::string className(filename);
        className[0] = toupper(className[0]);

        if (std::filesystem::exists(Editor::projectPath + AssetManager::Get()->script_dir + "/" + className + ".h")) {
            Editor::Log("Script name already exists!");
            remove(path.c_str());
            return;
        }

        if (std::filesystem::exists(path)) {
            Editor::Log("Script path already exists!");
            return;
        }

        std::ofstream src(path);

        std::string root_path = Editor::rootPath;
        std::replace(root_path.begin(), root_path.end(), '\\', '/');
        
        for (int i = 0; i < className.length(); i++) 
        {
            if (className[i] == ' ')
                std::replace(className.begin(), className.end(), ' ', '_');

            if (className[i] == '_')
                className[i + 1] = toupper(className[i + 1]);
        }

        src << "#pragma once\n\n";
        src << "#include \"" << root_path << "/sdk/include/behaviors.h\"\n\n\n";
        src << "namespace entity_behaviors {\n\n";
        src <<  "   class " << className << " : public Behavior {\n\n";
        src <<  "       public:\n\n";
        src <<  "           //constructor, called on start\n\n";
        src <<  "           " << className << "(std::shared_ptr<Entity> entity):\n";
        src <<  "           Behavior(entity->ID, typeid(" << className << ").name(), \"" + className + "\")\n";
        src <<  "           {\n\n";
        src <<  "           }\n\n";
        src <<  "           ~" << className << "() = default;\n";
        src <<  "           //update every frame\n\n";
        src <<  "           void Update() override {\n\n";
        src <<  "           }\n";
        src <<  "           void Cleanup() override {\n\n";
        src <<  "           }\n\n";
        src <<  "   };\n";
        src <<  "};";

        src.close();

        filename = "";

    }

    //animator

    if (type == ANIMATOR)
    {
        for (const auto& node : Node::nodes)
            if (node->ID == ID) {
                if (m_nodeType == Node::SPRITE) {
                    auto sn = std::dynamic_pointer_cast<SpriteNode>(Node::Get(ID));
                    sn->anim_to_play_on_start = { "", 0, 0, 2, -1, false };
                }
            }
    }

    //physics


    if (type == PHYSICS)
    {
        for (const auto& node : Node::nodes)
            if (node->ID == ID) {

                if (m_nodeType == Node::SPRITE) {
                    auto sn = std::dynamic_pointer_cast<SpriteNode>(Node::Get(ID));
                    sn->CreateBody();
                }

                if (m_nodeType == Node::TILEMAP) {
                    auto tmn = std::dynamic_pointer_cast<TilemapNode>(Node::Get(ID));
                    tmn->CreateBody();
                }

            }
    }
}


//-------------------------------


void Component::ApplyAnimations(bool init) 
{
    const auto session = Editor::Get();

    for (const auto& animation : session->animations) {

        if ((!animation.first.length()) || 
            std::adjacent_find(session->animations.begin(), session->animations.end()) != session->animations.end()) {
                Editor::Log("error applying animations, remove duplicates.");
                break;
            }

        session->animations_applied = true;

        if (!init)
            Editor::Log("animation: " + animation.first + " added.");
    }
}