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


bool AssetManager::LoadAsset(const std::string& asset) {
 
    const std::string folder = GetFolder(asset), 
                      key = "\"" + asset + "\"";

    if (!folder.length()) {
        Editor::Log("cannot load asset. invalid folder, folders must be named one of the following: [images, audio, data, fonts].");
        return false;
    }           
    
    const std::string developmentPath = "resources/assets" + folder + asset;

    s_self->loadedAssets.insert({ key, developmentPath });

    return true;
}

//-----------------------------


const std::string AssetManager::GetFolder(const std::string& asset) {
    switch(System::Utils::GetFileType(asset)) {
        default: return "";
        case System::Resources::Manager::IMAGE: return "/images/";       
        case System::Resources::Manager::AUDIO: return "/audio/"; 
        case System::Resources::Manager::DATA: return "/data/"; 
        case System::Resources::Manager::TEXT: return "/fonts/"; 
        case System::Resources::Manager::ICON: return "/icon/"; 
    }
}


//-----------------------------


const std::string AssetManager::GetThumbnail(const std::string& asset) {
    switch(System::Utils::GetFileType(asset)) {
        default: return "";
        case System::Resources::Manager::DATA: return "data src";       
        case System::Resources::Manager::TEXT: return "text src";       
        case System::Resources::Manager::AUDIO: return "audio src"; 
        case System::Resources::Manager::IMAGE: return asset; 
    }
}


//---------------------------------


const bool AssetManager::SavePrefab(const std::string& nodeId, std::vector<std::shared_ptr<Node>>& arr)
{
    const auto node = Node::Get(nodeId, arr);
    
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

        if (GetSaveFileName(&ofn) == TRUE) {
            std::filesystem::path result((const char*)ofn.lpstrFile);
            if (saveFile(result.string() + ".prefab"))
                return true;
        }
 
    #endif
    }

    catch (std::runtime_error& err) {
        Editor::Log("Runtime error saving prefab: " + (std::string)err.what());
        return false;
    }
}


//---------------------------------


const bool AssetManager::LoadPrefab(std::vector<std::shared_ptr<Node>>& nodes)
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
                Node::ReadData(data, true, nullptr, nodes);
            }

            JSON.close();

            remove(tmp.c_str());

            return true;
        }

    #endif

    return false;
}