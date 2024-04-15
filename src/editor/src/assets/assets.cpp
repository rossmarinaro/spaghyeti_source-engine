#ifdef _WIN32
    #include <windows.h>
    #include <tchar.h>
#endif

#include "./assets.h"
#include "../editor.h"
#include "../../../../build/sdk/include/app.h"

using namespace editor;

//---------------------------


void AssetManager::LoadAsset(const std::string& asset, const std::string& path)
{

    const std::string folder = GetFolder(asset),
                      texture = GetThumbnail(asset),
                      key = "\"" + asset + "\"",
                      developmentPath = "\"" + path + "resources\\assets" + folder + asset + "\"",
                      productionPath = "\"" + path + "build\\assets\\" + asset + "\"";

    loadedAssets.insert({ key, developmentPath });
    productionAssets.insert({ key, productionPath });

}

//----------------------------


void AssetManager::SetIcon(const std::string& key)
{
    //splash image

    #ifndef __EMSCRIPTEN__

        GLFWimage image; 
        
        image.width = 70;
        image.height = 70;
        image.pixels = reinterpret_cast<unsigned char*>(const_cast<char*>(System::Resources::Manager::GetRawData(key)));
    
        glfwSetWindowIcon(System::Window::s_instance, 1, &image);

    #endif
}


//----------------------------


std::string AssetManager::GetFolder(const std::string& asset)
{
    std::string folder;

    if (System::Utils::GetFileType(asset) == "image") 
        folder = "\\images\\";

    if (System::Utils::GetFileType(asset) == "audio") 
        folder = "\\audio\\";

    if (System::Utils::GetFileType(asset) == "data") 
        folder = "\\data\\";

    return folder;
}


//-----------------------------


std::string AssetManager::GetThumbnail(const std::string& asset)
{
    std::string texture;

    if (System::Utils::GetFileType(asset) == "image") 
        texture = asset; 
    
    if (System::Utils::GetFileType(asset) == "audio") 
        texture = "audio src";
    
    if (System::Utils::GetFileType(asset) == "data") 
        texture = "data src";
    

    return texture;
}


//---------------------------------


bool AssetManager::SavePrefab(const std::string& type, const std::string& nodeId)
{

    auto node = Node::GetNode(nodeId);
    
    if (!node)
        return false;

    try {

        auto saveFile = [&](const std::string& filename) -> bool
        {
            
            std::ofstream src(filename);

            json data;

            //scripts

            json scripts = json::array();

            for (const auto& behavior : node->behaviors)
                scripts.push_back({
                    { "key", behavior.first }, 
                    { "value", behavior.second }
                });

            //shader

            json shader = {};

            if (node->shader.first.length())
                shader = {
                        { "key", node->shader.first }, 
                        { "vertex", node->shader.second.first },
                        { "fragment", node->shader.second.second }
                    };

            //sprite

            if (type == "Sprite")
            {

                auto sn = std::dynamic_pointer_cast<SpriteNode>(node); 

                //frames

                json frames = json::array();

                for (int i = 0; i < sn->frame; ++i)
                    frames.push_back({
                        { "offset x", sn->frameBuf1.size() ? sn->frameBuf1[i] : 0 },
                        { "offset y", sn->frameBuf2.size() ? sn->frameBuf2[i] : 0 },
                        { "width", sn->frameBuf3.size() ? sn->frameBuf3[i] : 0 },
                        { "height", sn->frameBuf4.size() ? sn->frameBuf4[i] : 0 },
                        { "factor x", sn->frameBuf5.size() ? sn->frameBuf5[i] : 1 },
                        { "factor y", sn->frameBuf6.size() ? sn->frameBuf6[i] : 1 }
                    });

                //animations

                json animations = json::array();

                for (int i = 0; i < sn->anim; ++i)
                    if (sn->animBuf1.size())
                        animations.push_back({
                            { "key", sn->animBuf1[i].s },
                            { "start", sn->animBuf2[i] },
                            { "end", sn->animBuf3[i] }
                        });

                //physics bodies

                json bodies = json::array();

                for (int i = 0; i < sn->bodies.size(); ++i)
                    bodies.push_back({
                        { "body_width", sn->body_width.size() ? sn->body_width[i] : 0 },
                        { "body_height", sn->body_height.size() ? sn->body_height[i] : 0 },
                        { "bodyX", sn->bodyX.size() ? sn->bodyX[i] : 0 },
                        { "bodyY", sn->bodyY.size() ? sn->bodyY[i] : 0 },
                        { "pointer", sn->body_pointer.size() ? sn->body_pointer[i] : 0 },
                        { "sensor", sn->is_sensor.size() ? sn->is_sensor[i].b : false }
                    });


                //settings

                data.push_back({
                    { "type", type },
                    { "ID", node->m_ID }, 
                    { "name", node->m_name },
                    { "U1", sn->U1 },
                    { "V1", sn->V1 },
                    { "U2", sn->U2 },
                    { "V2", sn->V2 },
                    { "currentTexture", sn->key },
                    { "m_tint", {
                            { "x", sn->tint.x },
                            { "y", sn->tint.y },
                            { "z", sn->tint.z }
                        }
                    },
                    { "m_alpha", sn->alpha },
                    { "positionX", sn->positionX },
                    { "positionY", sn->positionY },
                    { "rotation", sn->rotation },
                    { "scaleX", sn->scaleX },
                    { "scaleY", sn->scaleY },
                    { "flipX", sn->flippedX },
                    { "flipY", sn->flippedY },
                    { "depth", sn->depth },
                    { "lock", sn->lock_in_place },
                    { "frames", frames },
                    { "components", {
                            { "physics", {
                                    { "exists", sn->HasComponent("Physics") },
                                    { "bodies", bodies },
                                    { "friction", sn->friction },
                                    { "restitution", sn->restitution },
                                    { "density", sn->density }
                                }
                            },
                            { "animator", {
                                    { "exists", sn->HasComponent("Animator") },
                                    { "animations", animations }
                                }
                            },
                            { "script", {
                                    { "exists", sn->HasComponent("Script") },
                                    { "scripts", scripts }
                                }
                            },
                            { "shader", {
                                    { "exists", sn->HasComponent("Shader") },
                                    { "shaders", shader }
                                }
                            }
                        }
                    }
                });

            }

            //audio

            if (type == "Audio")
            {

                auto an = std::dynamic_pointer_cast<AudioNode>(node);


            }

            //empty

            if (type == "Empty")
            {

                auto en = std::dynamic_pointer_cast<EmptyNode>(node);

            }

            //text

            if (type == "Text")
            {

                auto tn = std::dynamic_pointer_cast<TextNode>(node);

            }

            std::string JSON = data.dump();

            for (int i = 0; i < JSON.length(); i++)
                if (JSON[i] == '{')
                    JSON[i] = '%';
                else if (JSON[i] == '}')
                    JSON[i] = '|';
                else if (JSON[i] == '[')
                    JSON[i] = '?';
                else if (JSON[i] == ']')
                    JSON[i] = '!';
                else if (JSON[i] == ',')
                    JSON[i] = '&';
                else if (JSON[i] == ',')
                    JSON[i] = '&';
                else if (JSON[i] == ':')
                    JSON[i] = '$';

            src << JSON;

            src.close();
    };

    //windows save prompt

    #ifdef _WIN32

        OPENFILENAME ofn;

        char szFileName[MAX_PATH] = "";

        ZeroMemory(&ofn, sizeof(ofn));

        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFilter = _T("SpaghYeti Prefabs (*.prefab)\0*.prefab");
        ofn.lpstrFile = szFileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
        ofn.lpstrDefExt = NULL;

        if (GetSaveFileName(&ofn) == TRUE) 
        {
            std::filesystem::path result((const char*)ofn.lpstrFile);
            
            if (saveFile(result.string() + ".prefab"))
                return true;
        }

    #endif
    }

    catch (std::runtime_error& err) {
        std::cout << "error saving prefab: " << err.what() << "\n";
        return false;
    }
}


//---------------------------------


bool AssetManager::LoadPrefab()
{

   #ifdef _WIN32

        OPENFILENAME ofn = {0};
        TCHAR szFile[260] = {0};

        ofn.lStructSize = sizeof (ofn);
        ofn.hwndOwner = NULL;
        ofn.hInstance = NULL;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = _T("SpaghYeti Prefabs (*.prefab)\0*.prefab");
        ofn.lpstrFile = szFile;
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileName(&ofn) == TRUE)
        {

            std::filesystem::path result((const char*)ofn.lpstrFile);

            //temporary file for decoding

            const std::string tmp = Editor::projectPath + "spaghyeti_parse.json";

            Editor::events.DecodeFile(tmp, result);

            std::ifstream JSON(tmp);

            if (JSON.good())
            {
                json data = json::parse(JSON);
                data = data[0];

                if (static_cast<std::string>(data["type"]) == "Sprite")
                {
                    auto sn = Node::MakeNode<SpriteNode>(); 

                    sn->m_ID = data["ID"];
                    sn->m_name = data["name"];
                    sn->positionX = data["positionX"];
                    sn->positionY = data["positionY"];
                    sn->rotation = data["rotation"];
                    sn->scaleX = data["scaleX"];
                    sn->scaleY = data["scaleY"];
                    sn->flippedX = data["flipX"];
                    sn->flippedY = data["flipY"];
                    sn->depth = data["depth"];
                    sn->lock_in_place = data["lock"];
                    sn->U1 = data["U1"];
                    sn->V1 = data["V1"];
                    sn->U2 = data["U2"]; 
                    sn->V2 = data["V2"];
                    sn->key = data["currentTexture"];
                    sn->tint = { data["m_tint"]["x"], data["m_tint"]["y"], data["m_tint"]["z"] };
                    sn->alpha = data["m_alpha"];

                    sn->ApplyTexture(data["currentTexture"]);  

                    sn->spriteHandle->SetTexture(data["currentTexture"]);

                    for (const auto& frame : data["frames"]) {
                        sn->frameBuf1.push_back(frame["offset x"]);
                        sn->frameBuf2.push_back(frame["offset y"]);
                        sn->frameBuf3.push_back(frame["width"]);
                        sn->frameBuf4.push_back(frame["height"]);
                        sn->frameBuf5.push_back(frame["factor x"]);
                        sn->frameBuf6.push_back(frame["factor y"]);
                        sn->frames.push_back({ frame["offset x"], frame["offset y"], frame["width"], frame["height"], frame["factor x"], frame["factor y"] });
                    }

                    sn->frame = sn->frames.size();

                    sn->RegisterFrames();

                    if (sn->frames.size() > 1) {
                        sn->framesApplied = true; 
                        sn->spriteHandle->ReadSpritesheetData();
                        sn->spriteHandle->SetFrame(0);
                    }

                    //animator

                    if (data["components"]["animator"]["exists"]) {
                        sn->AddComponent("Animator", false);
                        sn->anim++;
                    }

                    for (const auto& anim : data["components"]["animator"]["animations"]) 
                    { 

                        SpriteNode::StringContainer sc = { anim["key"] };

                        sn->animBuf1.push_back(sc);
                        sn->animBuf2.push_back(anim["start"]); 
                        sn->animBuf3.push_back(anim["end"]);
                    
                        sn->ApplyAnimation(anim["key"], anim["start"], anim["end"]);
                    }

                    sn->anim = sn->animations.size();   

                    //physics 

                    if (data["components"]["physics"]["exists"]) 
                    {
                        sn->AddComponent("Physics", false);
                        sn->friction = data["components"]["physics"]["friction"];
                        sn->restitution = data["components"]["physics"]["restitution"];
                        sn->density = data["components"]["physics"]["density"]; 

                        if (data["components"]["physics"]["bodies"].size())
                            for (const auto& body : data["components"]["physics"]["bodies"]) 
                                sn->CreateBody(body["bodyX"], body["bodyY"], body["body_width"], body["body_height"], body["sensor"], body["pointer"]);
                    }

                    //script

                    if (data["components"]["script"]["exists"]) {
                        
                        sn->AddComponent("Script", false);

                        if (data["components"]["script"]["scripts"].size())
                            for (const auto& scripts : data["components"]["script"]["scripts"])
                                for (const auto& file : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::script_dir))
                                    if (file.exists() && static_cast<std::string>(scripts["key"]) == Editor::events.GetScriptName(file.path().string()))
                                        sn->behaviors.insert({ static_cast<std::string>(scripts["key"]).c_str(), static_cast<std::string>(scripts["value"]).c_str() });   
                    }

                    //shader

                    if (data["components"]["shader"]["exists"]) {

                        sn->AddComponent("Shader", false); 

                        if (data["components"]["shader"]["shaders"].size())
                            Node::LoadShader(sn, 
                                static_cast<std::string>(data["components"]["shader"]["shaders"]["key"]).c_str(),
                                static_cast<std::string>(data["components"]["shader"]["shaders"]["vertex"]).c_str(),
                                static_cast<std::string>(data["components"]["shader"]["shaders"]["fragment"]).c_str()
                            );
                    }
                }
            }

            JSON.close();

            remove(tmp.c_str());

            return true;

        }

    #endif

    return false;
}