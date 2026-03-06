#ifdef _WIN32

    #include <windows.h>
    #include <tchar.h>
    #include <bits/stdc++.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <shlobj.h>

#endif

#include <filesystem>
#include <iterator>

#include "./events.h"
#include "../editor.h"
#include "../assets/assets.h" 
#include "../nodes/node.h"
#include "../../../../build/sdk/include/app.h"

using namespace editor;


//--------------------------------- file open callbacks (windows only)


static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {

    #ifdef _WIN32

        if(uMsg == BFFM_INITIALIZED) {
            std::string tmp = (const char*)lpData;
            std::cout << "path: " << tmp << std::endl;
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
        }

    #endif

    return 0;
}


//--------------------------------


EventListener::EventListener() {

    exitFlag = false;
    saveFlag = false;
    buildFlag = false;
    canSave = true;
    
    Editor::Log("Events initialized.");
}



//-------------------------------- new project layer



const bool EventListener::NewProject(const char* root_path)
{
    #ifdef _WIN32

        TCHAR p[MAX_PATH];

        BROWSEINFO bi = { 0 };

        bi.lpszTitle  = ("Select Project Root Path.");
        bi.ulFlags    = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
        bi.lpfn       = BrowseCallbackProc;
        bi.lParam     = (LPARAM) root_path;

        LPITEMIDLIST pidl = SHBrowseForFolder (&bi);

        if (pidl != 0)
        {
            SHGetPathFromIDList (pidl, p);

            IMalloc* imalloc = 0;

            if (SUCCEEDED(SHGetMalloc (&imalloc))) {
                imalloc->Free (pidl);
                imalloc->Release();
            }

            Editor::Get()->Reset();

            std::string path = (std::string)p;

            Editor::projectPath = System::Utils::SanitizePath(path) + "/";
       
            s_currentProject = std::filesystem::path{ path }.filename().string();

            std::string root_path = Editor::rootPath;
            std::replace(root_path.begin(), root_path.end(), '\\', '/');

            const std::string resources = Editor::projectPath + "/resources";

            if (std::filesystem::exists(resources)) 
                Editor::Log("Project " + s_currentProject + " already exists.");

            else {

                std::filesystem::create_directory(Editor::projectPath + "/scenes");

                std::filesystem::create_directory(resources);
                std::filesystem::create_directory(resources + "/scripts");
                std::filesystem::create_directory(resources + "/shaders");
                std::filesystem::create_directory(resources + "/assets");
                std::filesystem::create_directory(resources + "/prefabs");
                std::filesystem::create_directory(resources + "/assets/images");
                std::filesystem::create_directory(resources + "/assets/audio");
                std::filesystem::create_directory(resources + "/assets/data");


                Editor::Log("New project " + s_currentProject + " generated.");
            }

            if (SaveScene()) 
            { 
                const std::string tmp = Editor::projectPath + "scenes/spaghyeti_parse.json"; 
 
                DecodeFile(tmp, Editor::projectPath + "scenes/" + s_currentScene + ".spaghyeti");

                std::ifstream JSON(tmp);

                if (JSON.good()) 
                    Deserialize(JSON);

                JSON.close();

                remove(tmp.c_str());

                Editor::Get()->projectOpen = true;

                return true;
            }

            return false;

        }

    #endif

    return false;
}



//-------------------------------- new scene



const bool EventListener::NewScene(const char* root_path)
{
    std::filesystem::current_path(Editor::projectPath + "scenes");

    #ifdef _WIN32

        OPENFILENAME ofn;

        char szFileName[MAX_PATH] = "";

        ZeroMemory(&ofn, sizeof(ofn));

        ofn.lStructSize = sizeof(ofn); 
        ofn.hwndOwner = NULL;
        ofn.lpstrFilter = _T("SpaghYeti Scene Files (*.spaghyeti)\0*.spaghyeti");
        ofn.lpstrFile = szFileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
        ofn.lpstrDefExt = NULL;

        if (GetSaveFileName(&ofn) == TRUE) 
        {
            std::filesystem::path result((const char*)ofn.lpstrFile);

            const std::string file = result.string() + ".spaghyeti";

            EncodeFile(file, true);

            s_currentScene = result.filename().string();

            const std::string tmp_json_path = Editor::projectPath + "scenes/spaghyeti_parse.json";

            DecodeFile(tmp_json_path, file);

            Editor::Get()->Reset();

            std::ifstream JSON(tmp_json_path);

            if (JSON.good()) 
            {
                StartSession(JSON);

                JSON.close();
                JSON.open(tmp_json_path);

                Deserialize(JSON);
                remove(tmp_json_path.c_str());

                JSON.close();

                Editor::Log("scene " + s_currentScene + " opened.\n");

            }

            else {
                Editor::Log("there was a problem deserializing scene.");
                return false;
            }

            return true;
        }

    #endif

    return false;
}


//----------------------------- save current scene layer


const bool EventListener::SaveScene(bool saveAs)
{
    std::filesystem::current_path(Editor::projectPath + "/scenes");

    try {
        std::string project_root = Editor::projectPath + "scenes/" + s_currentScene + ".spaghyeti";

        std::ifstream file(project_root);

        if (!file.good()) //save as if file does not exist
            saveAs = true;

        file.close();

        auto saveFile = [&](std::string filepath) -> bool {

            EncodeFile(filepath);

            std::string path = System::Utils::ReplaceFrom(filepath, ".", ""),
                        rep_path = System::Utils::ReplaceFrom(path, "scenes", "");

            Editor::projectPath = System::Utils::SanitizePath(rep_path);

            s_currentScene = std::filesystem::path{ path }.filename().string();

            Editor::Log("Scene saved: " + s_currentScene);

            return true;
        };

        if (saveAs)
        {
            #ifdef _WIN32

                OPENFILENAME ofn;

                char szFileName[MAX_PATH] = "";

                ZeroMemory(&ofn, sizeof(ofn));

                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = NULL;
                ofn.lpstrFilter = _T("SpaghYeti Scene Files (*.spaghyeti)\0*.spaghyeti");
                ofn.lpstrFile = szFileName;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
                ofn.lpstrDefExt = NULL;

                if (GetSaveFileName(&ofn) == TRUE)
                {
                    std::filesystem::path result((const char*)ofn.lpstrFile);

                    s_currentScene = result.filename().string();

                    if (saveFile(result.string() + ".spaghyeti"))
                        return true;
                }

            #endif
        }

        else if (saveFile(project_root))
            return true;

        return false;

    }

    catch (std::runtime_error& err) {

        Editor::Log("error saving project: " + (std::string)err.what());

        return false;
    }

}


//----------------------------- open scene layer


const bool EventListener::OpenScene() //makes temporary json file to parse data from .spaghyeti
{
   #ifdef _WIN32

        OPENFILENAME ofn = {0};
        TCHAR szFile[260] = {0};

        ofn.lStructSize = sizeof (ofn);
        ofn.hwndOwner = NULL;
        ofn.hInstance = NULL;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = _T("SpaghYeti Scene Files (*.spaghyeti)\0*.spaghyeti");
        ofn.lpstrFile = szFile;
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileName(&ofn) == TRUE)
        {
            std::filesystem::path result((const char*)ofn.lpstrFile);

            //result path

            std::string resultPath = std::filesystem::path{ result.string() }.parent_path().string(),
                        path = System::Utils::ReplaceFrom(resultPath, "scenes", ""),
                        filename = result.filename().string();

            Editor::projectPath = System::Utils::SanitizePath(path); 

            //project name and current scene

            s_currentProject = std::filesystem::path(Editor::projectPath).parent_path().filename().string();
            s_currentScene = System::Utils::ReplaceFrom(filename, ".", "");

            //temporary helper file for decoding

            const std::string tmp_json_path = Editor::projectPath + "scenes/spaghyeti_parse.json";

            DecodeFile(tmp_json_path, result);

            Editor::Get()->Reset();

            std::vector<std::array<const std::string, 3>> loaded_assets;

            const std::string asset_types[] = { "images", "audio", "data", "fonts" };

            //load the asset into memory

            const auto loadAsset = [&](const std::filesystem::directory_entry& entry, const std::string& type) -> void 
            {
                const std::string asset = entry.path().filename().string(),
                                  folder = AssetManager::GetFolder(asset);

                if (!folder.length()) {
                    Editor::Log("Skipping file: \"" + asset + "\" - invalid file type");
                    return;
                }
    
                const std::string texture = AssetManager::GetThumbnail(asset);

                if (!texture.length()) {
                    Editor::Log("thumbnail error, cannot load asset");
                    return;
                }

                std::string dir = entry.path().string(); //includes filename
                                
                std::replace(dir.begin(), dir.end(), '\\', '/');
                System::Resources::Manager::LoadFile(asset, dir);

                std::array<const std::string, 3> asset_info {{ type, asset, texture }};
                
                loaded_assets.push_back(asset_info);

            };

            //loop over asset dirs

            for (const std::string& type : asset_types) 
            {
                const std::string asset_path = Editor::projectPath + "resources/assets/" + type;

                if (std::filesystem::exists(asset_path))
                    for (const auto& entry : std::filesystem::directory_iterator(asset_path))
                    {
                        if (std::filesystem::is_empty(entry.path()))
                            continue;

                        if (entry.is_directory()) {
                            for (const auto& e : std::filesystem::directory_iterator(entry))
                                if (!std::filesystem::is_empty(e.path()) && !e.is_directory())
                                    loadAsset(e, type);
                        }
                        else
                            loadAsset(entry, type);
                    }
            }

            //register the textures

            System::Resources::Manager::RegisterTextures();

            //load thumbnails

            for (const auto& loaded : loaded_assets) 
            {
                const std::string type = loaded[0],
                                  asset = loaded[1],
                                  texture = loaded[2];
                                  
                if (!AssetManager::LoadAsset(asset))
                    continue;

                const unsigned int id = Graphics::Texture2D::Get(texture).ID;

                if (type == "images")
                    AssetManager::Get()->images.push_back({ asset, id });

                else if (type == "audio")
                    AssetManager::Get()->audio.push_back({ asset, id });

                else if (type == "data")
                    AssetManager::Get()->data.push_back({ asset, id });

                else if (type == "fonts")
                    AssetManager::Get()->text.push_back({ asset, id });
            } 

            //load entity nodes

            std::ifstream JSON(tmp_json_path);            

            if (JSON.good()) 
            {
                StartSession(JSON);
                JSON.close();
                JSON.open(tmp_json_path);
                Deserialize(JSON);
                JSON.close();
                remove(tmp_json_path.c_str());
                
                Editor::Log("Project " + s_currentProject + " opened.\nProject root path set: " + Editor::projectPath);
            }

            else {
                Editor::Log("Error: project file not found.");
                return false;
            }

            Editor::Get()->projectOpen = true;

            return true;
        }

    #endif

    return false;

}



//--------------------------------- open asset


void EventListener::OpenFile()
{

    #ifdef _WIN32

        OPENFILENAME ofn = {0};
        TCHAR szFile[260] = {0};

        ofn.lStructSize = sizeof (ofn);
        ofn.hwndOwner = NULL;
        ofn.hInstance = NULL;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = _T("Supported Files(*.png, *.flac, *.csv)\0*.png;*\0*.flac;*\0*.csv");
        ofn.lpstrFile = szFile;
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        // Show windows file dialog

        if (GetOpenFileName(&ofn) == TRUE)
        {

            std::filesystem::path result((const char*)ofn.lpstrFile);

            std::string asset = result.filename().string();

            //load asset in sandbox

            System::Resources::Manager::LoadFile(asset, result.string());
            System::Resources::Manager::RegisterTextures();

            //copy asset to game template

            const auto options = std::filesystem::copy_options::update_existing | std::filesystem::copy_options::recursive;

            const std::string folder = AssetManager::GetFolder(asset);

            if (!folder.length()) {
                Editor::Log("cannot open file: invalid file type");
                return;
            }

            const std::string texture = AssetManager::GetThumbnail(asset),
                              path = Editor::projectPath + "resources/assets" + folder + asset;

            std::ifstream file(path);

            //copy asset reference to respective project folder

            if (!file.good() && !std::filesystem::exists(path) && folder != "/icon/") 
                std::filesystem::copy_file(result.string(), path, options);

            //register asset into temp cache

            std::map<std::string, std::string>::iterator iterator = AssetManager::Get()->loadedAssets.find(asset);

            if (iterator == AssetManager::Get()->loadedAssets.end() || AssetManager::Get()->selectedAsset.length() < 0) {
                if (!AssetManager::LoadAsset(asset)) {
                    Editor::Log("cannot open file: there was a problem with the provided folder.");
                    return;
                }
            }

            //apply image to assets menu if not there already

            const unsigned int id = Graphics::Texture2D::Get(texture).ID;

            auto insertAsset = [&](std::vector<std::pair<std::string, GLuint>>& vec) -> void {
                if (std::find_if(vec.begin(), vec.end(), [&](std::pair<std::string, GLuint> pair) { return pair.first == asset; }) == vec.end())
                    vec.push_back({ asset, id });
            };

            if (folder == "/images/")
                insertAsset(AssetManager::Get()->images);

            if (folder == "/audio/")
                insertAsset(AssetManager::Get()->audio);     

            if (folder == "/data/")
                insertAsset(AssetManager::Get()->data);

            if (folder == "/fonts/")
                insertAsset(AssetManager::Get()->text);

            if (folder == "/icon/") {
                std::string path = result.string();
                std::replace(path.begin(), path.end(), '\\', '/');
                AssetManager::Get()->projectIcon = path;
            }
        }

    #endif
}



