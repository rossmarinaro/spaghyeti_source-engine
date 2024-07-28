#include "./gui.h"
#include "../editor.h"
#include "../nodes/node.h"
#include "../assets/assets.h"
#include "../../../../build/sdk/include/app.h"

using namespace editor;

//----------------------- shaders


void GUI::RenderShaderOptions(const std::string& nodeId)
{
    
    auto node = Node::Get(nodeId);
    
    if (node)
    {

        auto component = node->GetComponent("Shader", node->ID);

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


void GUI::RenderScriptOptions(const std::string& nodeId)
{
        
    auto node = Node::Get(nodeId);
     
    if (node)
    {

        auto component = node->GetComponent("Script", node->ID);

        if (!component)
            return; 

        if (node->behaviors.size())
        {
            ImGui::Text("Assigned Behaviors:");

            for (const auto &behavior : node->behaviors)
                if (ImGui::BeginMenu(behavior.first.c_str()))
                {
                    if (ImGui::MenuItem("edit"))
                        #ifdef _WIN32
                            system((Editor::projectPath + AssetManager::script_dir + "/" + behavior.second).c_str());
                        #endif

                    if (ImGui::BeginMenu("remove behavior?")) 
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
            ImGui::Text("Assigned Behaviors: (none)");

        ImGui::Separator();

        if (ImGui::BeginMenu("behavior library"))
        {
            if (ImGui::BeginMenu("new behavior")) 
            {
                
                ImGui::InputText("name", &component->filename);

                if (component->filename.size())
                    if (ImGui::MenuItem("submit")) 
                        component->Make();
                
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("select behavior"))
            {

                if (std::filesystem::is_empty(Editor::projectPath + AssetManager::script_dir))
                    ImGui::Text("no behaviors in directory.");

                else
                {

                    //load the script from dir

                    auto loadScript = [&](auto& script) -> void {

                        std::string filename = script.path().filename().string();

                        if (System::Utils::str_endsWith(filename, ".h"))
                            if (ImGui::MenuItem(System::Utils::ReplaceFrom(filename, ".", "").c_str())) 
                            {
                                std::string assetDir = AssetManager::script_dir;
                                
                                std::replace(Editor::projectPath.begin(), Editor::projectPath.end(), '\\', '/');
                                std::replace(assetDir.begin(), assetDir.end(), '\\', '/');

                                std::string line,
                                            path = Editor::projectPath + assetDir + '/';
                                        
                                std::ifstream src(script.path().string());

                                while (src >> line)
                                    if (line == "class")  
                                        if (src >> line)
                                            node->behaviors.insert({ line, filename });  

                            }
                    }; 

                    //iterate over script dirs

                    for (const auto& script : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::script_dir)) 
                        if (!script.is_directory())
                            loadScript(script);

                }                    

                ImGui::EndMenu();
            }
            
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("remove script?"))
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

    if (!s_running)
        return;

    if (ImGui::BeginMenu("New Node"))
    {

        if (ImGui::MenuItem("Load Prefab")) 
        {
            if (AssetManager::LoadPrefab())
                Editor::Log("Prefab loaded.");
            else 
                Editor::Log("There was a problem loading prefab.");
        } 

        ImGui::Separator();

        if (ImGui::MenuItem("Group"))
            Node::Make<GroupNode>();

        if (ImGui::MenuItem("Empty"))
            Node::Make<EmptyNode>(); 

        if (ImGui::MenuItem("Audio"))
            Node::Make<AudioNode>(); 

        if (ImGui::MenuItem("Text"))
            Node::Make<TextNode>(); 

        if (ImGui::MenuItem("Sprite"))
            Node::Make<SpriteNode>(); 

        if (ImGui::MenuItem("Tilemap"))
            Node::Make<TilemapNode>(); 

        ImGui::EndMenu();
    }

    //render active nodes
        
    if (Node::nodes.size())
        for (const auto &node : Node::nodes)
            if (node && node->active) {
                node->Update(node, Node::nodes); 
                node->Render();
            }
}

