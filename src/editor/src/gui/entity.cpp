#include "./gui.h"
#include "../editor.h"
#include "../nodes/node.h"
#include "../assets/assets.h"
#include "../../../../build/include/app.h"

using namespace editor;

//----------------------- shaders


void GUI::RenderShaderOptions(const std::string &nodeId)
{
    
    auto node = Node::GetNode(nodeId);
    
    if (node)
    {

        auto component = node->GetComponent("Shader", node->m_ID);

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
                            system((Editor::projectPath + AssetManager::shader_dir + "/" + node->shader.first + "/" + node->shader.first + ".vert").c_str());

                        if (ImGui::MenuItem("fragment"))
                            system((Editor::projectPath + AssetManager::shader_dir + "/" + node->shader.first + "/" + node->shader.first + ".frag").c_str());
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

                if (std::filesystem::is_empty(Editor::projectPath + AssetManager::shader_dir))
                    ImGui::Text("no shaders in directory.");

                else
                    for (const auto &dir : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::shader_dir))
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

                                if (vertPath.length() && fragPath.length()) 
                                    Node::LoadShader(node, name, vertPath, fragPath);
                                    
                                else    
                                    Editor::Log("Loading custom shader failed. file not found."); 
                                
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
                    Shader::UnLoad(node->shader.first);
            }
            
            ImGui::EndMenu();
        }
    }
}


//----------------------- scripts


void GUI::RenderScriptOptions(const std::string &nodeId)
{
        
    auto node = Node::GetNode(nodeId);
    
    if (node)
    {

        auto component = node->GetComponent("Script", node->m_ID);

        if (!component)
            return; 

        if (node->behaviors.size())
        {
            ImGui::Text("Assigned Scripts:");

            for (const auto &behavior : node->behaviors)
                if (ImGui::BeginMenu(behavior.first.c_str()))
                {
                    if (ImGui::MenuItem("edit"))
                        #ifdef _WIN32
                            system((Editor::projectPath + AssetManager::script_dir + "/" + behavior.second).c_str());
                        #endif

                    if (ImGui::BeginMenu("remove script?")) 
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
            ImGui::Text("Assigned Scripts: (none)");

        ImGui::Separator();

        if (ImGui::BeginMenu("script library"))
        {
            if (ImGui::BeginMenu("new script")) 
            {
                
                ImGui::InputText("name", &component->filename);

                if (component->filename.size())
                    if (ImGui::MenuItem("submit")) 
                        component->Make();
                
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("select script"))
            {

                if (std::filesystem::is_empty(Editor::projectPath + AssetManager::script_dir))
                    ImGui::Text("no scripts in directory.");

                else
                    for (const auto &script : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::script_dir))
                    {

                        std::string filename = script.path().filename().string();

                        if (System::Utils::str_endsWith(filename, ".h"))
                            if (ImGui::MenuItem(System::Utils::ReplaceFrom(filename, ".", "").c_str())) 
                            {

                                std::replace(Editor::projectPath.begin(), Editor::projectPath.end(), '\\', '/');
                                std::replace(AssetManager::script_dir.begin(), AssetManager::script_dir.end(), '\\', '/');

                                std::string line,
                                            path = Editor::projectPath + AssetManager::script_dir + '/';
                                        
                                std::ifstream src(path + filename);

                                while (src >> line)
                                    if (line == "class")  
                                        if (src >> line)
                                            node->behaviors.insert({ line, filename });  

                            }

                    }

                ImGui::EndMenu();
            }
            
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("remove scripts?"))
        {
            if (ImGui::MenuItem("yes"))
                node->RemoveComponent(component); 
            
            ImGui::EndMenu();
        }
    }
}


//----------------------- render node types in gui


void GUI::RenderNodes()
{

    if (!m_running)
        return;

    if (ImGui::BeginMenu("New Game Object"))
    {

        if (ImGui::MenuItem("Empty"))
            Node::MakeNode<EmptyNode>(); 

        if (ImGui::MenuItem("Audio"))
            Node::MakeNode<AudioNode>(); 

        if (ImGui::MenuItem("Text"))
            Node::MakeNode<TextNode>(); 

        if (ImGui::MenuItem("Sprite"))
            Node::MakeNode<SpriteNode>(); 

        if (ImGui::MenuItem("Tilemap"))
            Node::MakeNode<TilemapNode>(); 

        ImGui::EndMenu();
    }

    //render active nodes
        
    if (Node::nodes.size())
        for (auto &node : Node::nodes)
            if (node && node->m_active)
                node->Render(node);
        
}

