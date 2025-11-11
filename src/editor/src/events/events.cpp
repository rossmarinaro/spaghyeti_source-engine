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

            const std::string tmp = Editor::projectPath + "scenes/spaghyeti_parse.json";

            DecodeFile(tmp, file);

            Editor::Get()->Reset();

            std::ifstream JSON(tmp);

            if (JSON.good()) 
            {
                Editor::Log("scene " + s_currentScene + " opened.\n");
                Deserialize(JSON);

                JSON.close();
                remove(tmp.c_str());
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


//----------------------------- open project layer


const bool EventListener::OpenProject() //makes temporary json file to parse data from .spaghyeti
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

            //temporary file for decoding

            const std::string tmp = Editor::projectPath + "scenes/spaghyeti_parse.json";

            DecodeFile(tmp, result);

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

            std::ifstream JSON(tmp);

            if (JSON.good()) {
                Editor::Log("Project " + s_currentProject + " opened.\nProject root path set: " + Editor::projectPath);
                Deserialize(JSON);
            }

            else {
                Editor::Log("Error: project file not found.");
                return false;
            }

            JSON.close();

            remove(tmp.c_str());

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



//-------------------------------- build game from project source



void EventListener::BuildAndRun()
{
    system("cls");
    ShowWindow(GetConsoleWindow(), SW_SHOW);

    Editor::Log("building for " + Editor::platform + ", build type: " + Editor::buildType + ", release type: " + Editor::releaseType);

    //directory / builder files

    std::string makefile_path = Editor::projectPath + "Makefile";
    std::replace(makefile_path.begin(), makefile_path.end(), '\\', '/');
    std::replace(Editor::projectPath.begin(), Editor::projectPath.end(), '\\', '/');

    const std::string web = Editor::projectPath + "web",
                      web_makeFile_path = web + "/Makefile",
                      web_preJS_path = web + "/pre-js.js",
                      web_HTML_path = web + "/template.html",
                      folder = Editor::platform == "WebGL" ? "web" : "build",
                      assetsFolder = Editor::platform == "WebGL" ? Editor::projectPath + "web/assets/" : Editor::projectPath + "build/assets/",
                      buildPath = Editor::projectPath + folder;

    if (!std::filesystem::exists(buildPath)) 
        std::filesystem::create_directory(buildPath);

    if (Editor::platform == "WebGL" && !std::filesystem::exists(web)) 
        std::filesystem::create_directory(web);

    if (!std::filesystem::exists(assetsFolder)) 
        std::filesystem::create_directory(assetsFolder);

    //remove and make new assets folder

    else
        for (const auto& entry : std::filesystem::directory_iterator(assetsFolder))
            if (std::filesystem::exists(entry))
                remove(entry.path().string().c_str());

    //copy / embed assets from development to build folder

    const auto options = std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive;

    const std::string asset_types[] = { "images", "audio", "fonts" };

    Editor::Log(Editor::Get()->embed_files ? "embedding assets" : "copying assets to build folder"); 

    std::map<std::string, std::array<std::string, 2>> rawFiles;

    //iterate folders

    const auto iterateFolder = [&](const std::filesystem::directory_entry& file, const std::string& projResPath, const std::string& type) -> void 
    {
        std::string name = file.path().filename().string();
        const std::string destPath = assetsFolder + name;

        std::string rsrcPath = file.path().string();
        std::replace(rsrcPath.begin(), rsrcPath.end(), '\\', '/');

        if (System::Utils::GetFileType(rsrcPath) == System::Resources::Manager::NOT_SUPPORTED)
            return;

        //encode / embed assets

        if (Editor::Get()->embed_files) 
        {
            std::string rawFolder = Editor::projectPath + "resources/assets/raw/" + System::Utils::ReplaceFrom(name, ".", "") + ".hpp",
                        pathWithoutSpecialChars = rsrcPath;

            std::replace(rawFolder.begin(), rawFolder.end(), ' ', '_');
            std::replace_if(pathWithoutSpecialChars.begin(), pathWithoutSpecialChars.end(), [](auto ch) { return ch != '_' && ::isblank(ch) || ::ispunct(ch); }, '_');
            
            rawFiles.insert({ name, { type, pathWithoutSpecialChars } });

            if (!std::filesystem::exists(rawFolder)) 
            {       
                const std::string tmp_file = rawFolder + "_tmp";

                system(("chdir sdk && packResource.sh " + projResPath + " \"" + rsrcPath + "\" " + tmp_file).c_str());

                std::stringstream ss;
                std::ifstream is(tmp_file);
                std::ofstream os;

                ss << is.rdbuf();

                const std::string contents = "static constexpr " + ss.str();

                is.close();
                ss.clear();

                os.open(rawFolder);  
                os << contents;
                os.close();

                remove(tmp_file.c_str());

                Editor::Log("encoding asset: " + rawFolder); 
            }
        }

        //or copy them

        else if (!std::filesystem::exists(destPath)) 
            std::filesystem::copy_file(rsrcPath, destPath, options);
    };

    //loop over folders

    for (const std::string& folder : asset_types)
    {
        const std::string projResPath = Editor::projectPath + "resources/assets/" + folder;
        std::string type = folder;

        if (type == "images")
            type = "System::Resources::Manager::IMAGE";

        else if (type == "audio")
            type = "System::Resources::Manager::AUDIO";

        else if (type == "fonts")
            type = "System::Resources::Manager::TEXT";

        else continue;

        for (const auto& file : std::filesystem::directory_iterator(projResPath)) {
            if (file.is_directory()) {
                for (const auto& f : std::filesystem::directory_iterator(file))
                    if (!std::filesystem::is_empty(f.path()) && !f.is_directory())
                        iterateFolder(f, projResPath, type);
            }
            else
                iterateFolder(file, projResPath, type);
        }
    }

    //copy runtime library to build folder

    const std::string lib = Editor::buildType == "static" ? 
                                Editor::platform == "WebGL" ? "spaghyeti-web.a" : "spaghyeti.a" 
                                : (Editor::buildType == "dynamic" && Editor::releaseType == "debug") ? 
                                        "spaghyeti-debug.dll" : "spaghyeti.dll",

                      copy_lib = Editor::projectPath + "build/" + lib;

    //remove old libraries if they exist 

    if (std::filesystem::exists(Editor::projectPath + "build/spaghyeti-debug.dll"))
        remove((Editor::projectPath + "build/spaghyeti-debug.dll").c_str());

    if (std::filesystem::exists(Editor::projectPath + "build/spaghyeti.dll"))
        remove((Editor::projectPath + "build/spaghyeti.dll").c_str());

    if (Editor::buildType == "dynamic") 
    {
        if (!std::filesystem::exists(Editor::rootPath + "\\sdk\\" + lib)) {
            Editor::Log("Dynamic library not found! Aborting build.");
            return;
        }
        if (!std::filesystem::exists(copy_lib))
            std::filesystem::copy_file(Editor::rootPath + "\\sdk\\" + lib, copy_lib, options);
    }
    
    //cull target

    std::string cullTarget = "";

    //set game source input file stream

    const std::string srcPath = Editor::projectPath + "game.cpp";

    std::ofstream game_src;
    
    game_src.open(srcPath, std::ofstream::trunc);

    std::string root_path = Editor::rootPath;
    std::replace(root_path.begin(), root_path.end(), '\\', '/');

    game_src << "/* ---------- GENERATED CODE ----------- */\n";

    //Windows

    if (Editor::platform == "Windows")
    {

        bool has_icon = false;

        std::string icon_path;

        if (std::filesystem::exists(Editor::projectPath + "icon.rc")) 
            remove((Editor::projectPath + "icon.rc").c_str());

        if (std::filesystem::exists(Editor::projectPath + "icon.o")) 
            remove((Editor::projectPath + "icon.o").c_str());

        if (std::filesystem::exists(AssetManager::Get()->projectIcon) && AssetManager::Get()->projectIcon.length()) 
        {
            std::ofstream icon_rc(Editor::projectPath + "icon.rc");

            icon_rc << "1 ICON \"" + AssetManager::Get()->projectIcon + "\"\n";
            icon_rc << "1 VERSIONINFO\n";
            icon_rc << "FILEVERSION 1,0,0,0\n";
            icon_rc << "PRODUCTVERSION 1,0,0,0\n";
            icon_rc << "BEGIN\n";
            icon_rc << "    BLOCK \"""StringFileInfo""\"\n";
            icon_rc << "    BEGIN\n";
            icon_rc << "        BLOCK \"""040904E4\"""\n";
            icon_rc << "        BEGIN\n"; 
            icon_rc << "            VALUE \"""Comments""\"," "\"""Produced with SpaghYeti Engine\""" \n";
            icon_rc << "            VALUE \"""CompanyName\""", \"""Pastaboss Enterprise\""" \n";
            icon_rc << "            VALUE \"""FileDescription\""", \"""entertainment software\"""\n";
            icon_rc << "            VALUE \"""FileVersion\""", \"""" << std::to_string(Editor::Get()->maxVersion) << "." << std::to_string(Editor::Get()->midVersion) << "." << std::to_string(Editor::Get()->minVersion) << ".0\"""\n";
            icon_rc << "            VALUE \"""InternalName\""", \"""SpaghYeti Engine\"""\n";
            icon_rc << "            VALUE \"""LegalCopyright\""", \"""Copyright Pastaboss Enterprise\"""\n";
            icon_rc << "            VALUE \"""OriginalName\""", \"""SpaghYeti Engine\"""\n";
            icon_rc << "            VALUE \"""ProductName\""", \"""" << s_currentProject << "\"""\n";
            icon_rc << "            VALUE \"""ProductVersion\""", \"""" << std::to_string(Editor::Get()->maxVersion) << "." << std::to_string(Editor::Get()->midVersion) << "." << std::to_string(Editor::Get()->minVersion) << ".0\"""\n";
            icon_rc << "        END\n";
            icon_rc << "    END\n";
            icon_rc << "    BLOCK \"""VarFileInfo\"""\n";
            icon_rc << "    BEGIN\n";
            icon_rc << "        VALUE \"""Translation\""", 0x0409, 1200\n";
            icon_rc << "    END\n";
            icon_rc << "END\n";

            has_icon = true;
        }

        //generate MakeFile
    
        std::ofstream main_makeFile;
        
        main_makeFile.open(makefile_path, std::ofstream::trunc);

        main_makeFile << "OBJS = \\\n";
        main_makeFile << "    $(wildcard ./resources/scripts/*.cpp) \\\n";
        main_makeFile << "    $(wildcard ./resources/scripts/**/*.cpp) \\\n";
        main_makeFile << "    $(wildcard ./resources/scripts/**/**/*.cpp) \\\n";
        main_makeFile << "    $(wildcard ./resources/scripts/**/**/**/*.cpp) \\\n";
        main_makeFile << "    $(wildcard ./resources/scripts/**/**/**/**/*.cpp) \\\n";
        main_makeFile << "    ./game.cpp \\\n";

        //dynamic builds physically copies dll to local folder, static references sdk folder

        if (Editor::buildType == "dynamic")
            main_makeFile << "    ./build/" << lib << " \n\n";

        else 
            main_makeFile << "    " << Editor::rootPath << "\\sdk\\" << lib << "\n\n";

        //if project icon is defined, compile it, else use default

        if (has_icon) 
        {
            main_makeFile << "all: icon.o compile\n\n";
            main_makeFile << "icon.o: icon.rc\n";	

            #ifdef _WIN32
                main_makeFile << "\twindres icon.rc icon.o\n\n";
            #endif

            icon_path = "./icon.o";
        }

        else {
            main_makeFile << "all: compile\n\n";
            icon_path = Editor::rootPath + "\\sdk\\spaghyeti-icon.o";
        }
        
        const unsigned int devMode = Editor::releaseType == "debug" ? 1 : 0;
        
        main_makeFile << "compile : $(OBJS)\n"; 
        main_makeFile << "\tg++ -g -std=c++17 $(OBJS) -DDEVELOPMENT=" << devMode << " -DSTANDALONE=1 -w -lmingw32 -lopengl32 -lglfw3 -lfreetype -lpng -ljpeg -lz -lgdi32 -luser32 -lkernel32 " << icon_path << " -o ./build/$(PROJECT).exe\n\n";

        main_makeFile.close();

        //include core funtions

        game_src << "#ifdef _WIN32\n";
        game_src <<	"#define WIN32_LEAN_AND_MEAN\n";
        game_src <<	"#include <windows.h>\n";
        game_src << "#endif\n";

    }

    //WebGL 

    else if (Editor::platform == "WebGL")
    {

        const std::string use_pthreads = Editor::Get()->use_pthreads ? "1" : "0",
                          shared_memory = Editor::Get()->shared_memory ? "1" : "0",
                          allow_memory_growth = Editor::Get()->allow_memory_growth ? "1" : "0",
                          allow_exception_catching = Editor::Get()->allow_memory_growth ? "1" : "0",
                          export_all = Editor::Get()->export_all ? "1" : "0",
                          wasm = Editor::Get()->wasm ? "1" : "0",
                          gl_assertions = Editor::Get()->gl_assertions ? "1" : "0",
                          use_webgl2 = Editor::Get()->use_webgl2 ? "1" : "0",
                          full_es3 = Editor::Get()->full_es3 ? "1" : "0",
                          webgl_embed_files = Editor::Get()->webgl_embed_files ? "--embed-file" : "--preload-file";
   
        std::string name_upper = s_currentProject;
        transform(name_upper.begin(), name_upper.end(), name_upper.begin(), ::toupper);

        std::ofstream web_makeFile(web + "/Makefile"),
                      web_preJS(web + "/pre-js.js"),
                      web_HTML(web + "/template.html");

        web_makeFile << "OBJS = \\\n";
        web_makeFile << "   $(wildcard ../resources/scripts/*.cpp) \\\n";
        web_makeFile << "   $(wildcard ../resources/scripts/**/*.cpp) \\\n";
        web_makeFile << "   $(wildcard ../resources/scripts/**/**/*.cpp) \\\n";
        web_makeFile << "   ../game.cpp \\\n";
        web_makeFile << "   " << Editor::rootPath << "\\sdk\\spaghyeti-web.a \n\n";

        web_makeFile << "LINKER_FLAGS = \\\n";
        web_makeFile << "   -sEXPORT_ALL=" << export_all << " \\\n";
        web_makeFile << "   -sWASM=" << wasm << "\\\n";
        web_makeFile << "   -sGL_ASSERTIONS=" << gl_assertions << "\\\n";
        web_makeFile << "   -sUSE_WEBGL2=" << use_webgl2 << " \\\n";
        web_makeFile << "   -sFULL_ES3=" << full_es3 << " \\\n";
        web_makeFile << "   -sUSE_PTHREADS=" << use_pthreads << " \\\n";
        web_makeFile << "   -sSHARED_MEMORY=" << shared_memory << " \\\n";
        web_makeFile << "   -sALLOW_MEMORY_GROWTH=" << allow_memory_growth << " \\\n";
        web_makeFile << "   -sNO_DISABLE_EXCEPTION_CATCHING=" << allow_exception_catching << " \\\n";
        //web_makeFile << "   -sPTHREADS_DEBUG=1 \\\n";
        //web_makeFile << "   -sPROXY_TO_PTHREAD=1 \\\n";
        //web_makeFile << "   -sOFFSCREENCANVAS_SUPPORT=1 \\\n";
        //web_makeFile << "   -sOFFSCREEN_FRAMEBUFFER=1 \\\n";
        //web_makeFile << "   -sINITIAL_MEMORY=420mb \\\n";
        //web_makeFile << "   -sMAXIMUM_MEMORY=1000mb \\\n";
        //web_makeFile << "   -sPTHREAD_POOL_SIZE_STRICT=33 \\\n";
        //-sEXPORT_NAME="Example" //in html: window.Example(i).then(module => {})
        web_makeFile << "   -sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency \\\n";
        web_makeFile << "   -sUSE_GLFW=3 \\\n";
        web_makeFile << "   -sLEGACY_GL_EMULATION=0 \\\n";
        web_makeFile << "   -sASSERTIONS \\\n";
        web_makeFile << "   -sMAX_WEBGL_VERSION=2 \\\n";
        web_makeFile << "   -sMIN_WEBGL_VERSION=0 \\\n";
        web_makeFile << "   -sUSE_FREETYPE=1 \\\n";
        web_makeFile << "   -sUSE_LIBPNG=1 \\\n";
        web_makeFile << "   -sUSE_ZLIB \\\n";
        web_makeFile << "   -sASYNCIFY \\\n";
        web_makeFile << "   -Wl,--whole-archive \\\n";
        
        web_makeFile << "   --pre-js pre-js.js \\\n";
        web_makeFile << "   " << webgl_embed_files << " ./assets \\\n";
        web_makeFile << "   --use-preload-plugins \\\n";
        web_makeFile << "   --shell-file template.html\n\n";

        web_makeFile << "all: $(OBJS)\n";
        web_makeFile << "\tem++ -std=c++20 $(OBJS) -O3 -o dist/index.html $(LINKER_FLAGS)\n";

        web_preJS << "Module['window']\n";
        web_preJS << "Module['document']\n";
        web_preJS << "Module['logReadFiles'] = function(files) { console.log('using files: ', files)}\n";

        web_HTML << "<!DOCTYPE html>\n";
        web_HTML << "<html>\n";
        web_HTML << "<head>\n";
        web_HTML << "   <meta charset=\"utf-8\">\n";
        web_HTML << "   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n";
        web_HTML << "   <link rel=\"stylesheet\" type=\"text/css\" href=\"./style.css\" />\n";
        web_HTML << "   <link rel=\"icon\" type=\"image/png\" sizes=\"192x192\" href=\"./icon.png\" />\n";
        web_HTML << "   <title>" << name_upper << "</title>\n";
        web_HTML << "</head>\n";
        web_HTML << "<body>\n";
        web_HTML << "   <!-- Create the canvas that the C++ code will draw into -->\n";
        web_HTML << "   <canvas id=\"canvas\" oncontextmenu=\"event.preventDefault()\"></canvas>\n";
        web_HTML << "   <!-- MANUAL RESIZE OPTIONS -->\n";
        web_HTML << "   <!-- <span id=controls> -->\n";

        web_HTML << "   <!-- Allow the C++ to access the canvas element -->\n";
        web_HTML << "   <script type='text/javascript'>\n";
        web_HTML << "       var Module = {\n";
        web_HTML << "           window: (function() { return window; })(),\n";
        web_HTML << "           document: (function() { return document; })(),\n";
        web_HTML << "           canvas: (function() { return document.getElementById('canvas'); })()\n";
        web_HTML << "       };\n";

        web_HTML << "   /* PREVENTS TOUCH EVENT FROM CLICKING ON INPUT AFTER 300MS */\n\n";

        web_HTML << "       document.addEventListener('touchstart', () => setTimeout(() => document.querySelector('canvas').style.pointerEvents = 'none', 250));\n";
        web_HTML << "       document.addEventListener('touchend', () => document.querySelector('canvas').style.pointerEvents = 'auto');\n";
        web_HTML << "       document.addEventListener('touchmove', () => document.querySelector('canvas').style.pointerEvents = 'auto');\n";
        web_HTML << "       document.addEventListener('touchcancel', () => document.querySelector('canvas').style.pointerEvents = 'auto');\n\n";

        web_HTML << "   /* INIT FULLSCREEN RESIZE ON CLICK */\n";

        web_HTML << "   let isLocked = false, isResize = true;\n\n";

        web_HTML << "   document.addEventListener('click', () => {\n";
        web_HTML << "        Module.requestFullscreen(isLocked, isResize);\n";
        web_HTML << "        isLocked = !isLocked;\n";
        web_HTML << "        isResize = !isResize;\n";
        web_HTML << "    });\n";

        web_HTML << "   </script>\n";
        web_HTML << "   <!-- Add the javascript glue code (base.js) as generated by Emscripten -->\n\n";
        web_HTML << "   <script src=\"base.js\"></script>\n";
        web_HTML << "   {{{ SCRIPT }}}\n";

        web_HTML << "</body>\n";
        web_HTML << "</html>";

        //include core funtions
    
        game_src << "#ifndef ES\n";
        game_src << "#define ES\n";
        game_src << "   #include <unistd.h>\n";
        game_src << "   #include <emscripten.h>\n";
        game_src << "   #include <emscripten/html5.h>\n";
        game_src << "   #include <emscripten/eventloop.h>\n";
        game_src << "   #define GL_GLEXT_PROTOTYPES\n";
        game_src << "   #define EGL_EGLEXT_PROTOTYPES\n";
        game_src << "   #include <GLES3/gl3.h>\n"; 

        game_src << "#endif\n";
    }

    game_src << "\n#include \"" + root_path + "/sdk/include/app.h\"\n\n";

    //include embeddable assets if any

    if (Editor::Get()->embed_files) 
    {
        for (const auto& file : std::filesystem::recursive_directory_iterator(Editor::projectPath + "resources/assets/raw")) 
        {
            std::string path = file.path().string();

            if (!file.is_directory() && System::Utils::str_endsWith(path, ".hpp")) {
                std::replace(path.begin(), path.end(), '\\', '/');
                game_src << "#include " << "\"" + path + "\"\n";
            }
        }

        game_src << "\n";
    }

    //include scripts

    for (const auto& script : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::Get()->script_dir)) {

        std::string path = script.path().string();

        if (!script.is_directory() && System::Utils::str_endsWith(path, ".h")) {
            std::replace(path.begin(), path.end(), '\\', '/');
            game_src << "#include " << "\"" + path + "\"\n";
        }
    }

    //copy shaders (Windows)

    Editor::Log(Editor::Get()->embed_files ? "embedding shaders" : "copying shaders to build folder");

    if (!Editor::Get()->embed_files) 
    {
        const auto iterate_shader = [] (const std::string& name, std::string path, const std::string& copiedFile) -> void
        {
            std::string line, type;

            if (System::Utils::str_endsWith(path, ".vert"))
                type = ".vert";

            else if (System::Utils::str_endsWith(path, ".frag"))
                type = ".frag";

            else 
                return;

            std::replace(path.begin(), path.end(), '\\', '/');

            const std::string tmp_file = System::Utils::ReplaceFrom(path, ".", "") + "_tmp" + type;

            std::ifstream shaderFile(path);
            std::ofstream out_file(tmp_file);

            while (getline(shaderFile, line)) 
            {

                for (int i = 0; i < line.length(); i++) {
                    if (Editor::platform == "WebGL" && line == "#version 330 core")
                        line = "#version 300 es";

                    else if (line == "#version 300 es")
                        line = "#version 330 core";
                }

                out_file << line << "\n";
            }

            shaderFile.close();
            out_file.close();

            if (!std::filesystem::exists(copiedFile)) 
                std::filesystem::rename(tmp_file, copiedFile);
        };

       for (const auto& shader : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::Get()->shader_dir)) 
        {

            std::string path = shader.path().string();
            const std::string name = shader.path().filename().string(),
                            copiedFile = assetsFolder + name;

            if (shader.is_directory()) 
                for (const auto& folder : std::filesystem::recursive_directory_iterator(shader)) 
                    if (System::Utils::str_endsWith(path, ".vert") || System::Utils::str_endsWith(path, ".frag"))
                        iterate_shader(name, folder.path().string(), copiedFile);
                
            if (System::Utils::str_endsWith(path, ".vert") || System::Utils::str_endsWith(path, ".frag"))
                iterate_shader(name, path, copiedFile);
        }

    }
 
    //temp containers to track loaded configurations by key to avoid duplicate loads

    std::vector<std::string> loadedFrames;
    std::vector<std::string> loadedAnims;

    //iterate over scenes to include

    const auto parseScene = [this](const std::filesystem::directory_entry& filepath) -> void 
    {
        std::string path = filepath.path().filename().string();

        for (const auto& scene : Editor::Get()->scenes)
        {
            if (scene == System::Utils::ReplaceFrom(path, ".", ""))
            {
                std::string tmp = Editor::projectPath + "spaghyeti_parse.json";

                DecodeFile(tmp, filepath);

                std::ifstream JSON(tmp);

                if (JSON.good()) {
                    ParseScene(scene, JSON);
                    Editor::Log("Scene: " + scene + " parsed.");
                }

                else 
                    Editor::Log("There was a problem parsing scene: " + scene + ".");

                JSON.close();
                 
                //remove temp json file

                remove(tmp.c_str()); 
            }
        }
    };

    //parse scene files

    for (const auto& file : std::filesystem::directory_iterator(Editor::projectPath)) 
    {
        std::string sceneDir = Editor::projectPath + "scenes",
                    path = file.path().string();

        std::replace(sceneDir.begin(), sceneDir.end(), '\\', '/');
        std::replace(path.begin(), path.end(), '\\', '/');

        if (file.exists() && System::Utils::str_endsWith(path, ".spaghyeti")) 
            parseScene(file);

        else if (file.is_directory() && path == sceneDir)
           for (const auto& f : std::filesystem::directory_iterator(sceneDir)) 
               if (file.exists() && System::Utils::str_endsWith(f.path().string(), ".spaghyeti") && !f.is_directory())
                   parseScene(f);
    }

    //now compile each scene

    for (const auto& target : compileQueue)
    {

        //temp files: asset and command lists

        std::ostringstream asset_queue, preload_queue, global_queue, command_queue, update_queue;

        //preload file assets

        for (const auto& asset : target.second->assets) 
        {

            const std::string key = "\"" + asset + "\"";

            //embedded assets

            if (Editor::Get()->embed_files) {
                const auto rawFile = rawFiles.find(asset); 

                if (rawFile != rawFiles.end()) 
                    asset_queue << "  System::Resources::Manager::LoadRaw(" + rawFile->second[0] + ", " + key + ", " + rawFile->second[1] + ", " + rawFile->second[1] + "_len);\n";
            }

            //assets are copied to user project's "build" directory, and then virtually referenced from this path

            else 
                asset_queue << "  System::Resources::Manager::LoadFile(" + key + ", \"" + "assets/" + asset + "\");\n";
        }

        //load shaders

        for (const auto& shader : target.second->shaders) 
        { 
            std::string vertex, fragment;
            std::filesystem::path vertPath { shader.second.first },
                                  fragPath { shader.second.second };

            //embedded shaders

            if (Editor::Get()->embed_files) 
            {
                std::ifstream vertFile(vertPath),
                              fragFile(fragPath);
            
                std::stringstream vertStream, fragStream;

                vertStream << "R\"END(" << vertFile.rdbuf() << ")END\"";
                fragStream << "R\"END(" << fragFile.rdbuf() << ")END\"";

                vertex = vertStream.str(),
                fragment = fragStream.str();
                
                vertFile.close();
                fragFile.close();            
                
                asset_queue << "  Graphics::Shader::Load(\"" + shader.first + "\", " + vertex + ", " + fragment + ");\n";
            }

            //copied shaders

            else {
                vertex = "assets/" + vertPath.filename().string(),
                fragment = "assets/" + fragPath.filename().string();            
                
                asset_queue << "  Graphics::Shader::Load(\"" + shader.first + "\", \"" + vertex + "\", \"" + fragment + "\");\n";
            }
        }
            
        //load spritesheets (loaded data)

        if (target.second->spritesheets.size())
        {
            for (const auto& spritesheet : target.second->spritesheets)
            {
                const std::string filepath = Editor::projectPath + spritesheet.second;

                std::ifstream JSON(filepath);

                if (JSON.good())
                {
                    json data = json::parse(JSON);

                    std::ostringstream frame_oss;
                    std::vector<std::string> framesToLoad;

                    if (data["frames"].size())
                        for (const auto& frame : data["frames"])    
                            if (frame.contains("frame")) 
                            {
                                int x = frame["frame"]["x"],
                                    y = frame["frame"]["y"],
                                    width = frame["frame"]["w"],
                                    height = frame["frame"]["h"];

                                framesToLoad.emplace_back("{" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(width) + ", " + std::to_string(height) + ", 1, 1}");
                            }
                            
                    if (!framesToLoad.empty()) 
                    {
                        std::copy(framesToLoad.begin(), framesToLoad.end() - 1, std::ostream_iterator<std::string>(frame_oss, ", "));
                        frame_oss << framesToLoad.back();

                        if (std::find(loadedFrames.begin(), loadedFrames.end(), spritesheet.first) == loadedFrames.end()) {
                            preload_queue << "  System::Resources::Manager::LoadFrames(\"" + spritesheet.first + "\", {" + frame_oss.str() + "});\n";
                            loadedFrames.emplace_back(spritesheet.first);
                        }
                    }
                }
                
            }
        }

        //load animations (loaded data)

        if (target.second->animations.size())
        {

            for (const auto& animation : target.second->animations)
            {
                
                std::ostringstream anim_oss;
                std::vector<std::string> animsToLoad;

                for (const auto& anim : animation.second)
                    animsToLoad.emplace_back("{\"" + std::string(anim.first) + "\"" + ", {" + std::to_string(anim.second.first) + ", " + std::to_string(anim.second.second) + "} }");

                if (!animsToLoad.empty()) 
                {
                    std::copy(animsToLoad.begin(), animsToLoad.end() - 1, std::ostream_iterator<std::string>(anim_oss, ", "));
                    anim_oss << animsToLoad.back();

                    if (std::find(loadedAnims.begin(), loadedAnims.end(), animation.first) == loadedAnims.end()) {
                        loadedAnims.emplace_back(animation.first);
                        preload_queue << "  System::Resources::Manager::LoadAnims(\"" + animation.first + "\", {" + anim_oss.str() + "});\n";
                    }
                }
            }

        }

        //set global vars

        if (target.second->globals_applied)
            for (const auto& global : target.second->globals)
            {
                if (std::adjacent_find(target.second->globals.begin(), target.second->globals.end()) != target.second->globals.end()) {
                    Editor::Log("Error: duplicate global variable found.");
                    break;
                }

                if (!global.first.length() || !global.second.length()) {
                    Editor::Log("Error: incomplete global variable.");
                    break;
                }

                std::string type, value;

                if (global.second == "string") {
                    type = "std::string";
                    value = "";
                }

                else if (global.second == "int") {
                    type = "int";
                    value = "0";
                }

                else if (global.second == "float") {
                    type = "float";
                    value = "0.0f";
                }

                else if (global.second == "bool") {
                    type = "bool";
                    value = "false";
                }

                else if (global.second == "int[]") {
                    type = "std::vector<int>";
                    value = "{}";
                }

                else if (global.second == "float[]") {
                    type = "std::vector<float>";
                    value = "{}";
                }

                else if (global.second == "string[]") {
                    type = "std::vector<std::string>";
                    value = "{}";
                }

                else break;

                command_queue << "   this->SetGlobal(\"" + global.first + "\", " + value + ");\n";
            }

        command_queue << "   this->SetWorldDimensions(" + std::to_string(target.second->worldWidth) + ", " + std::to_string(target.second->worldHeight) + ");\n";
        
        command_queue << "   this->GetContext().camera->SetVignette(" + std::to_string(target.second->vignetteVisibility) + ");\n"; 
        command_queue << "   this->GetContext().camera->SetBounds(" + std::to_string(target.second->currentBoundsWidthBegin) + ", " + std::to_string(target.second->currentBoundsWidthEnd) + ", " + std::to_string(target.second->currentBoundsHeightBegin) + ", " + std::to_string(target.second->currentBoundsHeightEnd) + ");\n";
        command_queue << "   this->GetContext().camera->SetBackgroundColor({ " + std::to_string(target.second->cameraBackgroundColor.x) + ", " + std::to_string(target.second->cameraBackgroundColor.y) + ", " + std::to_string(target.second->cameraBackgroundColor.z) + ", " + std::to_string(target.second->cameraBackgroundColor.w) + " });\n";
        command_queue << "   this->GetContext().camera->SetZoom(" + std::to_string(target.second->cameraZoom) + ");\n";
        command_queue << "   this->GetContext().camera->SetPosition({ " + std::to_string(target.second->cameraPosition.x) + ", " + std::to_string(target.second->cameraPosition.y) + " });\n";
    
        const std::string phys_isCont = target.second->gravity_continuous ? "true" : "false",
                          phys_isSleeping = target.second->gravity_sleeping ? "true" : "false";

        command_queue << "   this->GetContext().physics->continuous = " + phys_isCont + ";\n";
        command_queue << "   this->GetContext().physics->sleeping = " + phys_isSleeping + ";\n";
        command_queue << "   this->GetContext().physics->SetGravity(" + std::to_string(target.second->gravityX) + ", " + std::to_string(target.second->gravityY) + ");\n";
    
        //command data, iterate over nodes and create objects

        const std::function<void(const std::vector<std::shared_ptr<Node>>&)> writeNodes = [&] (const std::vector<std::shared_ptr<Node>>& arr) -> void 
        {
            for (const auto& node : arr)
            {
                //load shaders

                if (node->HasComponent(Component::SHADER) && node->shader.first.length())
                    preload_queue << "  Graphics::Shader::Load(\"" + node->shader.first + "\", \"" + node->shader.second.first + "\", \"" + node->shader.second.second + "\", nullptr);\n";
                
                //--------------- group

                if (node->type == Node::GROUP) 
                    continue;
                    
                //--------------- sprite

                if (node->type == Node::SPRITE)
                {
                    std::ostringstream frame_oss;
                    std::vector<std::string> framesToLoad;

                    std::ostringstream anim_oss;
                    std::vector<std::string> animsToLoad;

                    const auto sn = std::dynamic_pointer_cast<SpriteNode>(node);

                    //load frames

                    for (const auto& frame : sn->frames)
                        framesToLoad.emplace_back("{" + std::to_string(frame.x) + ", " + std::to_string(frame.y) + ", " + std::to_string(frame.width) + ", " + std::to_string(frame.height) + ", " + std::to_string(frame.factorX) + ", " + std::to_string(frame.factorY) + "}");

                    if (!framesToLoad.empty()) 
                    {
                        std::copy(framesToLoad.begin(), framesToLoad.end() - 1, std::ostream_iterator<std::string>(frame_oss, ", "));
                        frame_oss << framesToLoad.back();

                        if (sn->frames.size() > 1 && std::find(loadedFrames.begin(), loadedFrames.end(), sn->key) == loadedFrames.end()) {
                            loadedFrames.emplace_back(sn->key);
                            preload_queue << "  System::Resources::Manager::LoadFrames(\"" + sn->key + "\", {" + frame_oss.str() + "});\n";
                        }
                    }

                    //load animations

                    for (const auto& anim : sn->animations)
                        animsToLoad.emplace_back("{\"" + std::string(anim.key) + "\"" + ", {" + std::to_string(anim.start) + ", " + std::to_string(anim.end) + "} }");

                    if (!animsToLoad.empty()) 
                    {
                        std::copy(animsToLoad.begin(), animsToLoad.end() - 1, std::ostream_iterator<std::string>(anim_oss, ", "));
                        anim_oss << animsToLoad.back();

                        if (std::find(loadedAnims.begin(), loadedAnims.end(), sn->key) == loadedAnims.end()) {
                            loadedAnims.emplace_back(sn->key);
                            preload_queue << "  System::Resources::Manager::LoadAnims(\"" + sn->key + "\", {" + anim_oss.str() + "});\n";
                        }
                    }

                    if (sn->make_UI) 
                        command_queue << "   const auto sprite_" + node->ID + " = System::Game::CreateUI(\"" + sn->key + "\", " + std::to_string(sn->positionX) + ", " + std::to_string(sn->positionY) + ");\n";
                    
                    else  
                        command_queue << "   const auto sprite_" + node->ID + " = System::Game::CreateSprite(\"" + sn->key + "\", " + std::to_string(sn->positionX) + ", " + std::to_string(sn->positionY) + ");\n";

                    if (sn->name == target.second->cullTarget.first)
                        cullTarget = "sprite_" + node->ID;

                    //sprite configurations

                    const std::string isStroke = sn->isStroked ? "true" : "false",
                                      filtering = sn->filter_nearest ? "true" : "false",
                                      cull = sn->cull ? "true" : "false";

                    //maybe add texture wrapping too?

                    command_queue << "   sprite_" + node->ID + "->texture.SetFiltering(" + filtering + ", " + filtering + ");\n";

                    command_queue << "   sprite_" + node->ID + "->SetFrame(" + std::to_string(sn->currentFrame) + ");\n";
                    command_queue << "   sprite_" + node->ID + "->SetScale(" + std::to_string(sn->scaleX) + ", " + std::to_string(sn->scaleY) + ");\n";
                    command_queue << "   sprite_" + node->ID + "->SetRotation(" + std::to_string(sn->rotation) + ");\n";
                    command_queue << "   sprite_" + node->ID + "->SetTint({ " + std::to_string(sn->tint.x) + ", " + std::to_string(sn->tint.y) + ", " + std::to_string(sn->tint.z) + " });\n";
                    command_queue << "   sprite_" + node->ID + "->SetDepth(" + std::to_string(sn->depth) + ");\n";
                    command_queue << "   sprite_" + node->ID + "->SetFlip(" + std::to_string(sn->flippedX) + ", " + std::to_string(sn->flippedY) + ");\n";
                    command_queue << "   sprite_" + node->ID + "->SetAlpha(" + std::to_string(sn->alpha) + ");\n";
                    command_queue << "   sprite_" + node->ID + "->SetScrollFactor({" + std::to_string(sn->scrollFactorX) + ", " + std::to_string(sn->scrollFactorY) + "});\n";
                    command_queue << "   sprite_" + node->ID + "->SetStroke(" + isStroke + ", { " + std::to_string(sn->strokeColor.x) + ", " + std::to_string(sn->strokeColor.y) +  ", " + std::to_string(sn->strokeColor.z) + " }, " + std::to_string(sn->strokeWidth) + ");\n";
                    command_queue << "   sprite_" + node->ID + "->SetName(\"" + node->name + "\");\n";   

                    //set default animation if applied

                    if (sn->anim_to_play_on_start.key.length()) {
                        const std::string is_yoyo = sn->anim_to_play_on_start.yoyo ? "true" : "false",
                                          repeat = sn->anim_to_play_on_start.repeat <= -1 ? std::to_string(sn->anim_to_play_on_start.repeat) : "-" + std::to_string(sn->anim_to_play_on_start.repeat);

                        command_queue << "   sprite_" + node->ID + "->SetAnimation(\"" + sn->anim_to_play_on_start.key + "\", " + is_yoyo + ", " + std::to_string(sn->anim_to_play_on_start.rate) + ", " + repeat + ");\n";
                    }

                    if (sn->lock_in_place) {
                        command_queue << "   sprite_" + node->ID + "->SetAsUI(true);\n";
                        command_queue << "   sprite_" + node->ID + "->SetScrollFactor({ 0.0f, 1.0f });\n";
                    }

                    command_queue << "   sprite_" + node->ID + "->SetCull(" + cull + ");\n";

                    //physics bodies

                    if (sn->HasComponent(Component::PHYSICS)) 
                    {
                        int i = 0; 

                        for (const auto& body : sn->bodies) {
                            const std::string is_sensor = sn->is_sensor[i].b ? "true" : "false";
                            command_queue << "   sprite_" + node->ID + "->AddBody(Physics::CreateBody(" + std::to_string(sn->body_type) + ", " + std::to_string(sn->body_shape) + ", " + std::to_string(sn->positionX + body.x) + ", " + std::to_string(sn->positionY + body.y) + ", " + std::to_string(body.width) + ", " + std::to_string(body.height) + ", " + is_sensor + ", " + std::to_string(sn->body_pointer[i]) + ", " + std::to_string(sn->density) + ", " + std::to_string(sn->friction) + ", " + std::to_string(sn->restitution) + "), { " + std::to_string(body.x) + ", " + std::to_string(body.y) + ", " + std::to_string(body.width) + ", " + std::to_string(body.height) + " });\n"; 
                            i++;
                        }

                        command_queue << "   for (const auto& body : sprite_" + node->ID + "->GetBodies())\n       body.first->SetFixedRotation(true);\n";
                    }
        
                    //shader

                    if (sn->HasComponent(Component::SHADER) && sn->shader.first.length()) 
                        command_queue << "   sprite_" + node->ID + "->SetShader(\"" + sn->shader.first + "\");\n";

                }

                //--------------- text

                if (node->type == Node::TEXT)
                {          

                    const auto tn = std::dynamic_pointer_cast<TextNode>(node);

                    //text render layer

                    const int isUI = tn->UIFlag ? 2 : 1;
                    
                    const std::string isStroke = tn->isStroked ? "true" : "false",
                                      isShadow = tn->isShadow ? "true" : "false";

                    command_queue << "   const auto text_" + node->ID + " = System::Game::CreateText(\"" + tn->textBuf + "\", " + std::to_string(tn->positionX) + ", " + std::to_string(tn->positionY) + ", " + "\"" +  tn->currentFont + "\", " + std::to_string(isUI) + ");\n"; 

                    command_queue << "   text_" + node->ID + "->SetStatic(" + (isUI == 2 ? "true" : "false") + ");\n";
                    command_queue << "   text_" + node->ID + "->SetScale(" + std::to_string(tn->scaleX) + ", " + std::to_string(tn->scaleY) + ");\n";
                    command_queue << "   text_" + node->ID + "->SetRotation(" + std::to_string(tn->rotation) + ");\n";
                    command_queue << "   text_" + node->ID + "->SetTint({ " + std::to_string(tn->tint.x) + ", " + std::to_string(tn->tint.y) + ", " + std::to_string(tn->tint.z) + " });\n";
                    command_queue << "   text_" + node->ID + "->SetAlpha(" + std::to_string(tn->alpha) + ");\n";
                    command_queue << "   text_" + node->ID + "->SetDepth(" + std::to_string(tn->depth) + ");\n";
                    command_queue << "   text_" + node->ID + "->SetShadow(" + isShadow + ", { " + std::to_string(tn->shadowColor.x) + ", " + std::to_string(tn->shadowColor.y) +  ", " + std::to_string(tn->shadowColor.z) + " }, " + std::to_string(tn->shadowDistanceX) + ", " + std::to_string(tn->shadowDistanceY) + ");\n";
                    command_queue << "   text_" + node->ID + "->SetStroke(" + isStroke + ", { " + std::to_string(tn->strokeColor.x) + ", " + std::to_string(tn->strokeColor.y) +  ", " + std::to_string(tn->strokeColor.z) + " }, " + std::to_string(tn->strokeWidth) + ");\n";
                    command_queue << "   text_" + node->ID + "->SetSlant(" + std::to_string(tn->charOffsetX) + ", " + std::to_string(tn->charOffsetY) + ");\n";
                    command_queue << "   text_" + node->ID + "->SetName(\"" + node->name + "\");\n";
        
                }

                //--------------- empty

                if (node->type == Node::EMPTY)
                {

                    const auto en = std::dynamic_pointer_cast<EmptyNode>(node);

                    if (en->debugGraphic)
                    {
                        //TODO: set shape

                        if (en->currentShape == "rectangle") {
                            command_queue << "   const auto empty_" + node->ID + " = System::Game::CreateGeom(" + std::to_string(en->positionX) + ", " + std::to_string(en->positionY) + ", " + std::to_string(en->rectWidth) + ", " + std::to_string(en->rectHeight) + ");\n";
                            command_queue << "   empty_" + node->ID + "->SetDrawStyle(" + std::to_string(en->debug_fill) + ");\n";
                        }

                        if (en->currentShape.length()) {
                            command_queue << "   empty_" + node->ID + "->SetThickness(" + std::to_string(en->line_weight) + ");\n";
                            command_queue << "   empty_" + node->ID + "->SetTint({" + std::to_string(en->debugGraphic->tint.x) + ", " + std::to_string(en->debugGraphic->tint.y) + ", " + std::to_string(en->debugGraphic->tint.z) + "});\n";
                            command_queue << "   empty_" + node->ID + "->SetAlpha(" + std::to_string(en->debugGraphic->alpha) + ");\n";

                            //shader

                            if (en->HasComponent(Component::SHADER) && en->shader.first.length()) 
                                command_queue << "   empty_" + node->ID + "->SetShader(\"" + en->shader.first + "\");\n";
                        }
                    }

                    else 
                        command_queue << "   const auto empty_" + node->ID + " = System::Game::CreateEntity();\n\t";

                    command_queue << "   empty_" + node->ID + "->SetName(\"" + node->name + "\");\n";
                }

                //--------------- tilemap

                if (node->type == Node::TILEMAP)
                {

                    const auto tmn = std::dynamic_pointer_cast<TilemapNode>(node);

                    //load frames

                    std::ostringstream offset_oss;
                    std::vector<std::string> offsetsToLoad;

                    if (tmn->offset.size()) 
                    {
                        for (const auto& offset : tmn->offset)
                            offsetsToLoad.emplace_back("{" + std::to_string(offset[0])  + ", " + std::to_string(offset[1]) + ", " + std::to_string(offset[2]) + ", " + std::to_string(offset[3]) + ", " + std::to_string(offset[4]) + ", " + std::to_string(offset[5]) + "}");
                    
                        if (!offsetsToLoad.empty()) {
                            std::copy(offsetsToLoad.begin(), offsetsToLoad.end() - 1, std::ostream_iterator<std::string>(offset_oss, ", "));
                            offset_oss << offsetsToLoad.back();
                        } 
                    }

                    //this flag loads maps on scene load but not restart

                    command_queue << "   if (loadMap) {\n";

                    if (tmn->layers.size())
                    {
                        for (int i = 0; i < tmn->layer; i++)
                        {
                            if (tmn->layers[i][2].length() && std::find(loadedFrames.begin(), loadedFrames.end(), tmn->layers[i][2]) == loadedFrames.end()) {
                                preload_queue << "  System::Resources::Manager::LoadFrames(\"" + tmn->layers[i][2] + "\", { " + offset_oss.str() + " });\n";
                                loadedFrames.emplace_back(tmn->layers[i][2]);
                            }
                            
                            if (tmn->layers[i][0].length()) 
                                command_queue << "   MapManager::CreateLayer(\"" + tmn->layers[i][2] + "\", ""\"" + tmn->layers[i][0] + "\", " + std::to_string(tmn->map_width) + ", " + std::to_string(tmn->map_height) + ", " + std::to_string(tmn->tile_width) + ", " + std::to_string(tmn->tile_height) + ", " + std::to_string(tmn->depth[i]) + ", " + std::to_string(tmn->positionX) + ", " + std::to_string(tmn->positionY) + ", " + std::to_string(tmn->rotation) + ", " + std::to_string(tmn->scaleX) + ", " + std::to_string(tmn->scaleY) + ");\n";
                        }

                        //create map if layers are defined

                        if (tmn->layers[0][0].length()) {
                            command_queue << "   const auto map_" + node->ID + " = System::Game::CreateEntity(Entity::TILE);\n\t";
                            command_queue << "   map_" + node->ID + "->SetName(\"" + tmn->name + "\");\n";
                        }

                    }

                    command_queue << "   }\n";

                    //static physics bodies

                    if (tmn->HasComponent(Component::PHYSICS) && tmn->bodies.size()) 
                        for (const auto& body : tmn->bodies) 
                            command_queue << "   Physics::CreateBody(Physics::Body::Type::STATIC, Physics::Body::Shape::BOX, " + std::to_string(body.x + body.width / 2) + ", " + std::to_string(body.y + body.height / 2) + ", " + std::to_string(body.width / 2) + ", " + std::to_string(body.height / 2) + ");\n";

                } 

                //--------------- audio

                if (node->type == Node::AUDIO) {

                    const auto an = std::dynamic_pointer_cast<AudioNode>(node);

                    std::string loop = an->loop ? "true" : "false";

                    command_queue << "   System::Audio::play(\"" + an->audio_source_name + "\", " + loop + ", " + std::to_string(an->volume) + ");\n";

                    command_queue << "   const auto audio_" + node->ID + " = System::Game::CreateEntity(\"" + "audio" + "\");\n\t";
                    command_queue << "   audio_" + node->ID + "->SetName(\"" + an->name + "\");\n";
                    
                }

                if (node->type == Node::SPAWNER) 
                {
                    const auto spawn_node = std::dynamic_pointer_cast<SpawnerNode>(node);

                    const std::string body_exists = spawn_node->body.exist ? "true" : "false",
                                      is_loop = spawn_node->loop ? "true" : "false",
                                      is_sensor = spawn_node->body.is_sensor ? "true" : "false";

                    command_queue << "   System::Game::CreateSpawn(" + std::to_string(spawn_node->typeOf) +  ", \"" + spawn_node->textureKey + "\", " + std::to_string(spawn_node->positionX) + ", " + std::to_string(spawn_node->positionY) + ", " + std::to_string(spawn_node->width) + ", " + std::to_string(spawn_node->height) + ", " + std::to_string(spawn_node->spawnWidth) + ", " + std::to_string(spawn_node->spawnHeight) + ", { " + std::to_string(spawn_node->tint.x) + ", " + std::to_string(spawn_node->tint.y) + ", " + std::to_string(spawn_node->tint.z) + " }, " + std::to_string(spawn_node->alpha) + ", " + is_loop + ", \"" + spawn_node->behaviorKey + "\", { " + std::to_string(spawn_node->body.type) + ", Physics::Body::Shape::BOX, " + body_exists + ", " + is_sensor + ", " + std::to_string(spawn_node->body.xOff) +  ", " + std::to_string(spawn_node->body.yOff) + ", " + std::to_string(spawn_node->body.w) +  ", " + std::to_string(spawn_node->body.h) + ", " + std::to_string(spawn_node->body.density) + ", " + std::to_string(spawn_node->body.friction) +  ", " + std::to_string(spawn_node->body.restitution) + " });\n";

                }

                //define behaviors

                std::string entity = Node::GetType(node->type) + "_" + node->ID;

                for (const auto& behavior : node->behaviors) {
                    transform(entity.begin(), entity.end(), entity.begin(), ::tolower);

                    command_queue << "   System::Game::CreateBehavior<entity_behaviors::" + behavior.first + ">(" + entity + ", this);\n";
                }

            }
        };

        loadedFrames.clear();
        loadedAnims.clear();

        //register scene textures

        preload_queue << "  System::Resources::Manager::RegisterTextures();\n"; 

        //scene update

        update_queue << "   for (auto& spawn : spawns) \n";
        update_queue << "   {\n\t\t std::shared_ptr<Entity> entity;\n\n";
        update_queue << "        if (spawn.type == Entity::SPRITE)\n";
        update_queue << "           entity = GetEntity<Sprite>(spawn.index);\n"; 
        update_queue << "        else if (spawn.type == Entity::GEOMETRY) \n";
        update_queue << "           entity = GetEntity<Geometry>(spawn.index);\n\n"; 
        update_queue << "        if (entity)\n";
        update_queue << "           for (const auto& behavior : spawn.behaviors_attached)\n";
        update_queue << "           {\n";

        if (!std::filesystem::is_empty(Editor::projectPath + AssetManager::Get()->script_dir))
            for (const auto& script : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::Get()->script_dir)) 
                if (!script.is_directory()) 
                {
                    std::string filename = script.path().filename().string();

                    if (System::Utils::str_endsWith(filename, ".h")) {

                        std::string file_name = System::Utils::ReplaceFrom(filename, ".", "");
                        std::string line, className, classKey(file_name);
                        std::ifstream src(script.path().string());

                        classKey[0] = toupper(classKey[0]);

                        while (src >> line)
                            if (line == "class")   
                                if (getline(src, line)) {
                                    className = System::Utils::ReplaceFrom(line, " :", "");
                                    className.erase(0, 1);
                                }
                                
                        src.close();

                        update_queue << "                if (behavior.first == \"" + classKey + "\" && spawn.hasBehavior(behavior.first))\n";  
                        update_queue << "                    System::Game::CreateBehavior<entity_behaviors::" + className + ">(entity, this);\n\t\t\n";
                    }
                }

        update_queue << "         }\n\t\t}\n\n";

        //write nodes

        writeNodes(target.second->nodes);

        if (cullTarget.length())
            command_queue << "   System::Game::SetCullPosition(&" + cullTarget + "->position);\n";

        cullTarget = "";

        //convert data string stream to string

        const std::string assetData = asset_queue.str(),
                          preloadData = preload_queue.str(),
                          commandData = command_queue.str(),
                          updateData = update_queue.str();

        std::string name_upper = target.first;

        std::replace(name_upper.begin(), name_upper.end(), '-', '_');
        std::replace(name_upper.begin(), name_upper.end(), ' ', '_');

        transform(name_upper.begin(), name_upper.end(), name_upper.begin(), ::toupper);

        //scene source class template

        game_src << "\n\nclass " + name_upper + " : public System::Scene {\n\n"; 
        game_src << "    public:\n";
        game_src << "        " + name_upper + "(const Process::Context& context):\n\t\tScene(context, \"" + name_upper + "\") {}\n";
        game_src << "        void Preload() override;\n";        
        game_src << "        void Update() override;\n";
        game_src << "        void Run(bool loadMap) override;\n";
        game_src <<"};\n\n\n"; 

        game_src << "void " + name_upper + "::Preload() {\n" + assetData + "\n" + preloadData + "\n}\n\n";
        game_src << "void " + name_upper + "::Run(bool loadMap) {\n" +  commandData + "}\n\n";
        game_src << "void " + name_upper + "::Update() {\n" +  updateData + "}\n\n";

    }

    //clear scene queue

    for (auto& scene : compileQueue)
        delete scene.second;

    compileQueue.clear();

    //game template

    const std::string isMultiThreaded = Editor::Get()->isMultiThreaded ? "true" : "false";

    game_src << "\n\n//-----------------------------------------------------------------------------\n\n";
    game_src << "\n#ifdef __EMSCRIPTEN__\n";
    game_src <<	"   EM_JS(float, getScreenWidth, (), { return window.screen.width; });\n";
    game_src <<	"   EM_JS(float, getScreenHeight, (), { return window.screen.height; });\n";
    game_src <<	"   EM_JS(bool, checkMobile, (), { \n";
    game_src <<	"       let check = false;\n";
    game_src <<	"       if (window.screen.width <= 800)\n";
    game_src <<	"           check = true;\n";
    game_src <<	"       return check;\n";
    game_src <<	"   });\n";
    game_src <<	"   EM_JS(void, fetchData, (), { \n";
    game_src <<	"       console.log('fetching data...');\n";
    game_src <<	"   });\n";
    game_src << "#endif\n\n\n";

    game_src <<	"int main(int argc, char* args[])\n";
    game_src <<	"{\n";
    game_src <<	"   #ifdef _WIN32\n";

    if (Editor::releaseType == "release")
        game_src <<	"       ShowWindow(GetConsoleWindow(), SW_HIDE);\n";

    game_src <<	"   #endif\n";
    game_src <<	"       bool isMobile = false;\n";
    game_src <<	"   #ifdef __EMSCRIPTEN__\n";
    game_src <<	"       isMobile = checkMobile();\n";
    game_src <<	"       fetchData();\n";
    game_src <<	"   #endif\n";
    game_src <<	"   #if _ISMOBILE == 1\n";
    game_src <<	"       isMobile = true;\n";
    game_src <<	"   #endif\n";

    game_src <<	"       System::Game game;\n";
 
    for (const auto& scene : Editor::Get()->scenes) 
    {
        std::string className = scene;

        std::replace(className.begin(), className.end(), '-', '_');
        std::replace(className.begin(), className.end(), ' ', '_');
        transform(className.begin(), className.end(), className.begin(), ::toupper); 

        game_src << "       game.LoadScene<" + className + ">();\n";
    }
    
    game_src << "       System::Application::Start(&game, \"" + s_currentProject + "\", " + isMultiThreaded + ", isMobile);\n";
    game_src <<	"   #ifdef __EMSCRIPTEN__\n";
    game_src <<	"       emscripten_exit_with_live_runtime();\n";
    game_src <<	"   #endif\n";
    game_src << "   return 0;\n";
    game_src <<	"}";

    game_src.close();

    if (Editor::platform == "WebGL")
        system(("chdir sdk && buildWebGL.bat " + Editor::projectPath).c_str());

    else if (Editor::platform == "Windows")
        system(("chdir sdk && buildGame.bat " + Editor::projectPath + " " + s_currentProject).c_str());

    if (!Editor::Get()->preserveSrc)
        remove(srcPath.c_str());
        
    remove(makefile_path.c_str());
    remove(web_makeFile_path.c_str());
    remove(web_preJS_path.c_str());
    remove(web_HTML_path.c_str());

    if (Editor::platform == "WebGL")
        std::filesystem::remove_all((web + "/assets").c_str());

    Editor::Log("Project " + s_currentProject + " built successfully.");
  //  ShowWindow(GetConsoleWindow(), SW_HIDE);

}



