#ifdef _WIN32
    #include <windows.h>
    #include <tchar.h>
#endif

#include "./assets.h"
#include "../editor.h"
#include "../../../../build/sdk/include/app.h"

using namespace editor;


AssetManager::AssetManager() {
    s_self = this;
    s_self->folderSelected = false;
}

//--------------------------


void AssetManager::Reset() {

    s_self->selectedAsset.clear();
    s_self->images.clear();
    s_self->audio.clear();
    s_self->data.clear();
    s_self->loadedAssets.clear();
    s_self->assets.clear();
}


//-------------------------- 


void AssetManager::Register(const std::string& asset) {

    if (std::find(s_self->assets.begin(), s_self->assets.end(), asset) == s_self->assets.end())
        s_self->assets.push_back(asset);
}


 
//---------------------------


void AssetManager::LoadAsset(const std::string& asset) {
 
    const std::string folder = System::Utils::GetFileType(asset),
                      texture = GetThumbnail(asset),
                      key = "\"" + asset + "\"",
                      developmentPath = "resources/assets/" + folder + "/" + asset;

    s_self->loadedAssets.insert({ key, developmentPath });

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


bool AssetManager::SavePrefab(const std::string& nodeId)
{

    auto node = Node::Get(nodeId);
    
    if (!node)
        return false;

    try {

        auto saveFile = [&](const std::string& filename) -> bool
        {
            
            std::ofstream src(filename);

            json data;

            data = Node::WriteData(node);

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
        Editor::Log("error saving prefab: " + (std::string)err.what());
        return false;
    }
}


//---------------------------------


bool AssetManager::LoadPrefab(std::vector<std::shared_ptr<Node>>& nodes)
{

   #ifdef _WIN32

        OPENFILENAME ofn = { 0 };
        TCHAR szFile[260] = { 0 };

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

            Editor::Get()->events->DecodeFile(tmp, result); 

            std::ifstream JSON(tmp);

            if (JSON.good()) {
                json data = json::parse(JSON);
                Node::ReadData(data, true, nullptr, nodes, nodes != Node::nodes);
            }

            JSON.close();

            remove(tmp.c_str());

            return true;
        }

    #endif

    return false;
}