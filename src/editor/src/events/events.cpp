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



static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    #ifdef _WIN32

        if(uMsg == BFFM_INITIALIZED) {
            std::string tmp = (const char*)lpData;
            std::cout << "path: " << tmp << std::endl;
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
        }

    #endif

    return 0;
}



//-------------------------------- new project layer



bool EventListener::NewProject(const char* root_path)
{

    #ifdef _WIN32

        TCHAR path[MAX_PATH];

        BROWSEINFO bi = { 0 };

        bi.lpszTitle  = ("Select Project Root Path.");
        bi.ulFlags    = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
        bi.lpfn       = BrowseCallbackProc;
        bi.lParam     = (LPARAM) root_path;

        LPITEMIDLIST pidl = SHBrowseForFolder (&bi);

        if (pidl != 0)
        {
            SHGetPathFromIDList (pidl, path);

            IMalloc* imalloc = 0;

            if (SUCCEEDED(SHGetMalloc (&imalloc))) {
                imalloc->Free (pidl);
                imalloc->Release();
            }

            Editor::Reset();

            Editor::projectPath = (std::string)path;

            std::filesystem::current_path(Editor::projectPath);

            if (SaveScene()) {
                GenerateProject();
                return true;
            }
        }

    #endif

    return false;
}



//-------------------------------- new scene



bool EventListener::NewScene(const char* root_path)
{

    #ifdef _WIN32

        OPENFILENAME ofn;

        char szFileName[MAX_PATH] = "";

        ZeroMemory(&ofn, sizeof(ofn));

        ofn.lStructSize = sizeof(ofn); 
        ofn.hwndOwner = NULL;
        ofn.lpstrFilter = _T("SpaghYeti Files (*.spaghyeti)\0*.spaghyeti");
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

            const std::string tmp = Editor::projectPath + "spaghyeti_parse.json";

            DecodeFile(tmp, file);

            Editor::Reset();

            std::ifstream JSON(tmp);

            if (JSON.good()) {
                Editor::Log("scene " + s_currentScene + " opened.\n");
                Deserialize(JSON);
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


//----------------------------- open project layer


bool EventListener::Open() //makes temporary json file to parse data from .spaghyeti
{

   #ifdef _WIN32

        OPENFILENAME ofn = {0};
        TCHAR szFile[260] = {0};

        ofn.lStructSize = sizeof (ofn);
        ofn.hwndOwner = NULL;
        ofn.hInstance = NULL;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = _T("SpaghYeti Files (*.spaghyeti)\0*.spaghyeti");
        ofn.lpstrFile = szFile;
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileName(&ofn) == TRUE)
        {

            std::filesystem::path result((const char*)ofn.lpstrFile);

            Editor::projectPath = std::filesystem::path{ result.string() }.parent_path().string() + "\\"; //result path

            //project name and current scene

            s_currentProject = std::filesystem::path(Editor::projectPath).parent_path().filename().string();
            s_currentScene = System::Utils::ReplaceFrom(result.filename().string(), ".", "");

            //temporary file for decoding

            const std::string tmp = Editor::projectPath + "spaghyeti_parse.json";

            DecodeFile(tmp, result);

            Editor::Reset();

            const std::string asset_folders[] = { "images", "audio", "data" };

            for (const std::string& type : asset_folders)
                for (const auto& entry : std::filesystem::directory_iterator(Editor::projectPath + "resources\\assets\\" + type))
                {

                    const std::string asset = entry.path().filename().string(),
                                      dir = entry.path().string(), //includes filename
                                      texture = AssetManager::GetThumbnail(asset);

                    System::Resources::Manager::LoadFile(asset.c_str(), dir.c_str());
                    System::Resources::Manager::RegisterTextures();

                    AssetManager::LoadAsset(asset);

                    unsigned int id = Graphics::Texture2D::Get(texture).ID;

                    if (type == "images")
                        AssetManager::images.push_back({ asset, id });

                    if (type == "audio")
                        AssetManager::audio.push_back({ asset, id });

                    if (type == "data")
                        AssetManager::data.push_back({ asset, id });
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

            Editor::projectOpen = true;

            return true;

        }

    #endif

    return false;

}


//----------------------------- save current scene layer


bool EventListener::SaveScene(bool saveAs)
{

    try {

        std::string project_root = Editor::projectPath + s_currentScene + ".spaghyeti";

        std::ifstream file(project_root);

        if (!file.good()) //save as if file does not exist
            saveAs = true;

        file.close();

        auto saveFile = [&](std::string filepath) -> bool {

            EncodeFile(filepath);

            Editor::projectPath = std::filesystem::path{ filepath }.parent_path().string() + "\\";

            s_currentScene = System::Utils::ReplaceFrom(std::filesystem::path{ filepath }.filename().string(), ".", "");

            Editor::Log("Project saved: " + Editor::projectPath + std::filesystem::path{ filepath }.filename().string());

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
                ofn.lpstrFilter = _T("SpaghYeti Files (*.spaghyeti)\0*.spaghyeti");
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

        std::cout << "error saving project: " << err.what() << "\n";

        return false;
    }

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

            System::Resources::Manager::LoadFile(asset.c_str(), result.string().c_str());
            System::Resources::Manager::RegisterTextures();

            //copy asset to game template

            const auto options = std::filesystem::copy_options::update_existing | std::filesystem::copy_options::recursive;

            std::string folder = AssetManager::GetFolder(asset);
            const std::string texture = AssetManager::GetThumbnail(asset);

            std::ifstream file(Editor::projectPath + "resources\\assets" + folder + asset);

            folder.erase(remove(folder.begin(), folder.end(), '\\'), folder.end());

            //copy asset reference to respective project folder

            if (!file.good() && folder != "icon") 
                std::filesystem::copy_file(result.string(), Editor::projectPath + "resources\\assets" + asset, options);

            //register asset into temp cache

            std::map<std::string, std::string>::iterator iterator = AssetManager::loadedAssets.find(asset);

            if (iterator == AssetManager::loadedAssets.end() || AssetManager::selectedAsset.length() < 0) 
                AssetManager::LoadAsset(asset);

            //apply image to assets menu if not there already

            unsigned int id = Graphics::Texture2D::Get(texture).ID;

            auto insertAsset = [&](std::vector<std::pair<std::string, GLuint>>& vec) 
            {
                if (
                    std::find_if( 
                        vec.begin(), vec.end(),
                        [&](std::pair<std::string, GLuint> pair) { return pair.first == asset; }) == vec.end()
                )
                    vec.push_back({ asset, id });
            };

            if (folder == "images")
                insertAsset(AssetManager::images);

            if (folder == "audio")
                insertAsset(AssetManager::audio);

            if (folder == "data")
                insertAsset(AssetManager::data);

            if (folder == "icon") {
                std::string path = result.string();
                std::replace(path.begin(), path.end(), '\\', '/');
                AssetManager::projectIcon = path;
            }

        }

    #endif
}


//--------------------------------- embed calls to temp file for compilation


void EventListener::InsertTo(const std::string& code, const std::string& directory)
{

    std::ofstream src;

    src.open(directory, std::ofstream::app | std::ofstream::out);

    src << code;

    src.close();
}



//-------------------------------- build game from project source



void EventListener::BuildAndRun()
{
    //directory / builder files

    std::string makefile_path = Editor::projectPath + "\\Makefile";
    std::replace(makefile_path.begin(), makefile_path.end(), '\\', '/');

    const std::string web = Editor::projectPath + "\\web",
                      web_makeFile_path = web + "/Makefile",
                      web_preJS_path = web + "/pre-js.js",
                      web_HTML_path = web + "/template.html",
                      folder = Editor::platform == "WebGL" ? "web" : "build",
                      assetsFolder = Editor::platform == "WebGL" ? Editor::projectPath + "web\\assets\\" : Editor::projectPath + "build\\assets\\",
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

    //copy assets from development to build folder

    const auto options = std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive;

    const std::string asset_folders[] = { "images", "audio", "data" };

    for (const std::string& type : asset_folders)
        for (const auto& file : std::filesystem::directory_iterator(Editor::projectPath + "resources\\assets\\" + type)) 
        {
            const std::string name = file.path().filename().string(),
                              rsrcPath = file.path().string(),
                              copiedFile = assetsFolder + name;

            if (!std::filesystem::exists(copiedFile)) {
                Editor::Log("copying: " + rsrcPath + " to: " + copiedFile);
                std::filesystem::copy_file(rsrcPath, copiedFile, options);
            }
        }

    //copy runtime library to build folder

    const std::string lib = Editor::buildType == "static" ? 
                                Editor::platform == "WebGL" ? "spaghyeti-web.a" : "spaghyeti.a" 
                                : (Editor::buildType == "dynamic" && Editor::releaseType == "debug") ? 
                                        "spaghyeti-debug.dll" : "spaghyeti-release.dll",

                      copy_lib = Editor::projectPath + "build\\" + lib;

    if (Editor::buildType == "dynamic") 
    {
        remove((Editor::projectPath + "build\\spaghyeti-debug.dll").c_str());
        remove((Editor::projectPath + "build\\spaghyeti-release.dll").c_str());

        if (!std::filesystem::exists(copy_lib))
            std::filesystem::copy_file(Editor::rootPath + "\\sdk\\" + lib, copy_lib, options);
    }

    //Windows

    if (Editor::platform == "Windows")
    {

        bool has_icon = false;

        std::string icon_path;

        if (std::filesystem::exists(Editor::projectPath + "icon.rc")) 
            remove((Editor::projectPath + "icon.rc").c_str());

    
        if (std::filesystem::exists(AssetManager::projectIcon) && AssetManager::projectIcon.length()) {
            std::ofstream icon_rc(Editor::projectPath + "icon.rc");
            icon_rc << "1 ICON \"" + AssetManager::projectIcon + "\"";
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
        
        main_makeFile << "compile : $(OBJS)\n";
        main_makeFile << "\tg++ -g -std=c++17 $(OBJS) -w -lmingw32 -lopengl32 -lglfw3 -lgdi32 -luser32 -lkernel32 " << icon_path << " -o ./build/$(PROJECT).exe\n\n";

        main_makeFile.close();

    }

    //WebGL 

    else if (Editor::platform == "WebGL")
    {

        const std::string use_pthreads = Editor::use_pthreads ? "1" : "0",
                          shared_memory = Editor::shared_memory ? "1" : "0",
                          allow_memory_growth = Editor::allow_memory_growth ? "1" : "0",
                          allow_exception_catching = Editor::allow_memory_growth ? "1" : "0",
                          export_all = Editor::export_all ? "1" : "0",
                          wasm = Editor::wasm ? "1" : "0",
                          gl_assertions = Editor::gl_assertions ? "1" : "0",
                          use_webgl2 = Editor::use_webgl2 ? "1" : "0",
                          full_es3 = Editor::full_es3 ? "1" : "0";
                        

        std::string name_upper = s_currentProject;
        transform(name_upper.begin(), name_upper.end(), name_upper.begin(), ::toupper);

        //todo: tmp assets folder

        std::ofstream web_makeFile(web + "/Makefile"),
                      web_preJS(web + "/pre-js.js"),
                      web_HTML(web + "/template.html");

        web_makeFile << "OBJS = \\\n";
        web_makeFile << "   $(wildcard ../resources/scripts/*.cpp) \\\n";
        web_makeFile << "   $(wildcard ../resources/scripts/**/*.cpp) \\\n";
        web_makeFile << "   $(wildcard ../resources/scripts/**/**/*.cpp) \\\n";
        web_makeFile << "   ../game.cpp \\\n";
        web_makeFile << "   " << Editor::rootPath << "\\sdk\\spaghyeti-web.a \n\n";
        
        web_makeFile << "COMPILER_FLAGS = -O3 -o dist/index.html\n\n";

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
        web_makeFile << "   -sINITIAL_MEMORY=420mb\\\n";
        //web_makeFile << "   -sMAXIMUM_MEMORY=1000mb \\\n";
        //web_makeFile << "   -sPTHREAD_POOL_SIZE_STRICT=33 \\\n";
        web_makeFile << "   -sPTHREAD_POOL_SIZE=33 \\\n"; //-sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency \\\n";
        web_makeFile << "   -sUSE_GLFW=3 \\\n";
        web_makeFile << "   -sLEGACY_GL_EMULATION=0 \\\n";
        web_makeFile << "   -sASSERTIONS \\\n";
        web_makeFile << "   -sMAX_WEBGL_VERSION=2 \\\n";
        web_makeFile << "   -sMIN_WEBGL_VERSION=0 \\\n";
        web_makeFile << "   -sUSE_LIBPNG=1 \\\n";
        web_makeFile << "   -sUSE_ZLIB \\\n";
        web_makeFile << "   -sASYNCIFY \\\n";
        web_makeFile << "   -Wl,--whole-archive \\\n";
        
        web_makeFile << "   --pre-js pre-js.js \\\n";
        web_makeFile << "   --preload-file ./assets \\\n";
        web_makeFile << "   --use-preload-plugins \\\n";
        web_makeFile << "   --shell-file template.html\n\n";

        web_makeFile << "all: $(OBJS)\n";
        web_makeFile << "\tem++ -std=c++20 $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS)\n";

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
    }


    //set game source input file stream

    const std::string srcPath = Editor::projectPath + "\\game.cpp";

    std::ofstream game_src;
    
    game_src.open(srcPath, std::ofstream::trunc);

    //include core funtions

    std::string root_path = Editor::rootPath;
    std::replace(root_path.begin(), root_path.end(), '\\', '/');

    game_src << "#ifdef _WIN32\n";
    game_src <<	"#define WIN32_LEAN_AND_MEAN\n";
    game_src <<	"#include <windows.h>\n";
    game_src << "#endif\n";
    game_src << "\n#include \"" + root_path + "/sdk/include/app.h\"\n\n";

    //include scripts

    for (const auto& script : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::script_dir)) {

        std::string path = script.path().string();

        if (!script.is_directory() && System::Utils::str_endsWith(path, ".h")) {
            std::replace(path.begin(), path.end(), '\\', '/');
            game_src << "#include " << "\"" + path + "\"\n";
        }
    }

    //include shaders

    const auto iterate_shader = [] (const std::string& name, std::string path, const std::string& copiedFile) -> void
    {

        std::string line, type;

        if (System::Utils::str_endsWith(path, ".vert"))
            type = ".vert";

        else if (System::Utils::str_endsWith(path, ".frag"))
            type = ".frag";

        else 
            return;

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

        if (!std::filesystem::exists(copiedFile)) {
            std::filesystem::rename(tmp_file, copiedFile); 
            Editor::Log("copying shaders: " + path + " to: " + copiedFile);
        }

    };

    for (const auto& shader : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::shader_dir)) 
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

    //temp containers to track loaded configurations by key to avoid duplicate loads

    std::vector<std::string> loadedFrames;
    std::vector<std::string> loadedAnims;

    //iterate over scenes to include

    const auto parseScene = [this](const std::filesystem::directory_entry& filepath) -> void {

        for (const auto& scene : Editor::scenes)
        {
            if (scene == System::Utils::ReplaceFrom(filepath.path().filename().string(), ".", ""))
            {
                std::string tmp = Editor::projectPath + "spaghyeti_parse.json";

                DecodeFile(tmp, filepath);

                std::ifstream JSON(tmp);

                if (JSON.good()) {
                    ParseScene(scene, JSON);
                    Editor::Log("Scene parsed.");
                }

                else 
                    Editor::Log("There was a problem parsing scene.");

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
               if (file.exists() && System::Utils::str_endsWith(f.path().string(), ".spaghyeti"))
                   parseScene(f);
    }

    //now compile each scene

    for (const auto& target : compileQueue)
    {

        //temp files: asset and command lists

        std::ostringstream asset_queue, preload_queue, global_queue, command_queue, constructor_queue;

        //preload file assets

        for (const auto& asset : target.second->assets) {

            const std::string key = "\"" + asset + "\"";

            //assets are copied to user project's "build" directory, and then virtually referenced from this path, hence the prefix in webgl builds

            asset_queue << "  System::Resources::Manager::LoadFile(" + key + ", \"" + "assets/" + asset + "\");\n";
        }

        //load shaders

        for (const auto& shader : target.second->shaders) 
        { 
            std::filesystem::path vertPath { shader.second.first },
                                  fragPath { shader.second.second };

            const std::string vertex = "assets/" + vertPath.filename().string(),
                              fragment = "assets/" + fragPath.filename().string();

            asset_queue << "  Shader::Load(\"" + shader.first + "\", \"" + vertex + "\", \"" + fragment + "\");\n";
        }
            

        //load spritesheets (loaded data)

        if (target.second->spritesheets.size())
        {

            for (const auto& spritesheet : target.second->spritesheets)
            {
                
                const std::string filepath = Editor::projectPath + spritesheet.second;

                std::ifstream JSON(filepath);
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

                std::string type = global.second;

                if (global.second == "string")
                    type = "std::string";

                else if (global.second == "int[]")
                    type = "std::vector<int>";

                else if (global.second == "float[]")
                    type = "std::vector<float>";

                else if (global.second == "string[]")
                    type = "std::vector<std::string>";

                else break;

                global_queue << "   " + type + " " + "GLOBALVAR_" + global.first + ";\n";
            }

        command_queue << "   this->SetWorldDimensions(" + std::to_string(target.second->worldWidth) + ", " + std::to_string(target.second->worldHeight) + ");\n";
        
        command_queue << "   this->GetContext().camera->SetVignette(" + std::to_string(target.second->vignetteVisibility) + ");\n"; 
        command_queue << "   this->GetContext().camera->SetBounds(" + std::to_string(target.second->currentBoundsWidthBegin) + ", " + std::to_string(target.second->currentBoundsWidthEnd) + ", " + std::to_string(target.second->currentBoundsHeightBegin) + ", " + std::to_string(target.second->currentBoundsHeightEnd) + ");\n";
        command_queue << "   this->GetContext().camera->SetBackgroundColor({ " + std::to_string(target.second->cameraBackgroundColor.x) + ", " + std::to_string(target.second->cameraBackgroundColor.y) + ", " + std::to_string(target.second->cameraBackgroundColor.z) + ", " + std::to_string(target.second->cameraBackgroundColor.w) + " });\n";
        command_queue << "   this->GetContext().camera->SetZoom(" + std::to_string(target.second->cameraZoom) + ");\n";
        command_queue << "   this->GetContext().camera->SetPosition({ " + std::to_string(target.second->cameraPosition.x) + ", " + std::to_string(target.second->cameraPosition.y) + " });\n";
    
        std::string phys_isCont = target.second->gravity_continuous ? "true" : "false",
                    phys_isSleeping = target.second->gravity_sleeping ? "true" : "false";

        command_queue << "   this->GetContext().physics->continuous = " + phys_isCont + ";\n";
        command_queue << "   this->GetContext().physics->sleeping = " + phys_isSleeping + ";\n";
        command_queue << "   this->GetContext().physics->SetGravity(" + std::to_string(target.second->gravityX) + ", " + std::to_string(target.second->gravityY) + ");\n";
    
                
        //command data, iterate over nodes and create objects

        const std::function<void(std::vector<std::shared_ptr<Node>>&)> writeNodes = [&](std::vector<std::shared_ptr<Node>>& arr) -> void { 

            for (const auto& node : arr)
            {

                //load shaders

                if (node->HasComponent("Shader") && node->shader.first.length())
                    preload_queue << "  Shader::Load(\"" + node->shader.first + "\", \"" + node->shader.second.first + "\", \"" + node->shader.second.second + "\", nullptr);\n";


                //--------------- sprite

                if (node->type == "Sprite")
                {
                    std::ostringstream frame_oss;
                    std::vector<std::string> framesToLoad;

                    std::ostringstream anim_oss;
                    std::vector<std::string> animsToLoad;

                    auto sn = std::dynamic_pointer_cast<SpriteNode>(node);

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
                        animsToLoad.emplace_back("{\"" + std::string(anim.second.key) + "\"" + ", {" + std::to_string(anim.second.start) + ", " + std::to_string(anim.second.end) + "} }");

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
                        command_queue << "   auto sprite_" + node->ID + " = System::Game::CreateUI(\"" + sn->key + "\", " + std::to_string(sn->positionX) + ", " + std::to_string(sn->positionY) + ");\n";
                    
                    else  
                        command_queue << "   auto sprite_" + node->ID + " = System::Game::CreateSprite(\"" + sn->key + "\", " + std::to_string(sn->positionX) + ", " + std::to_string(sn->positionY) + ");\n";
            
                    //sprite configurations

                    command_queue << "   sprite_" + node->ID + "->SetScale(" + std::to_string(sn->scaleX) + ", " + std::to_string(sn->scaleY) + ");\n";
                    command_queue << "   sprite_" + node->ID + "->SetRotation(" + std::to_string(sn->rotation) + ");\n";
                    command_queue << "   sprite_" + node->ID + "->SetTint({ " + std::to_string(sn->tint.x) + ", " + std::to_string(sn->tint.y) + ", " + std::to_string(sn->tint.z) + " });\n";
                    command_queue << "   sprite_" + node->ID + "->SetDepth(" + std::to_string(sn->depth) + ");\n";
                    command_queue << "   sprite_" + node->ID + "->SetFlip(" + std::to_string(sn->flippedX) + ", " + std::to_string(sn->flippedY) + ");\n";
                    command_queue << "   sprite_" + node->ID + "->SetAlpha(" + std::to_string(sn->alpha) + ");\n";

                    const std::string filtering = sn->filter_nearest ? "GL_NEAREST" : "GL_LINEAR";

                    command_queue << "   sprite_" + node->ID + "->texture.Filter_Min = " + filtering + ";\n";
                    command_queue << "   sprite_" + node->ID + "->texture.Filter_Max = " + filtering + ";\n";

                    if (sn->lock_in_place) {
                        command_queue << "   sprite_" + node->ID + "->type = \"UI\";\n";
                        command_queue << "   sprite_" + node->ID + "->SetScrollFactor({ 0.0f, 1.0f });\n";
                    }

                    command_queue << "   sprite_" + node->ID + "->name = \"" + node->name + "\";\n"; 

                    //physics bodies

                    if (sn->HasComponent("Physics"))
                    {
                        for (int i = 0; i < sn->bodies.size(); i++) 
                            command_queue << "   sprite_" + node->ID + "->bodies.push_back({ Physics::CreateDynamicBody(\"box\", " + std::to_string(sn->positionX + sn->bodyX[i]) + ", " + std::to_string(sn->positionY + sn->bodyY[i]) + ", " + std::to_string(sn->body_width[i]) + ", " + std::to_string(sn->body_height[i]) + ", " + std::to_string(sn->is_sensor[i].b) + ", " + std::to_string(sn->body_pointer[i]) + ", " + std::to_string(sn->density) + ", " + std::to_string(sn->friction) + ", " + std::to_string(sn->restitution) + "), { " + std::to_string(sn->bodyX[i]) + ", " + std::to_string(sn->bodyY[i]) + ", " + std::to_string(sn->body_width[i]) + ", " + std::to_string(sn->body_height[i]) + " } });\n"; 

                        command_queue << "   for (const auto& body : sprite_" + node->ID + "->bodies)\n       body.first->SetFixedRotation(true);\n";

                    }
        
                    //shader

                    if (sn->HasComponent("Shader") && sn->shader.first.length()) 
                        command_queue << "   sprite_" + node->ID + "->shader = Shader::Get(\"" + sn->shader.first + "\");\n";

                }

                //--------------- text

                if (node->type == "Text")
                {          

                    auto tn = std::dynamic_pointer_cast<TextNode>(node);

                    //text render layer

                    tn->isUI = tn->UIFlag ? 2 : 1;

                    command_queue << "   auto text_" + node->ID + " = System::Game::CreateText(\"" + tn->textBuf + "\", " + std::to_string(tn->positionX) + ", " + std::to_string(tn->positionY) + ", " + std::to_string(tn->isUI) + ");\n"; 

                    command_queue << "   text_" + node->ID + "->SetScale(" + std::to_string(tn->scaleX) + ", " + std::to_string(tn->scaleY) + ");\n";
                    command_queue << "   text_" + node->ID + "->SetRotation(" + std::to_string(tn->rotation) + ");\n";
                    command_queue << "   text_" + node->ID + "->SetTint({ " + std::to_string(tn->tint.x) + ", " + std::to_string(tn->tint.y) + ", " + std::to_string(tn->tint.z) + " });\n";
                    command_queue << "   text_" + node->ID + "->SetAlpha(" + std::to_string(tn->alpha) + ");\n";
                    command_queue << "   text_" + node->ID + "->SetDepth(" + std::to_string(tn->depth) + ");\n";

                    command_queue << "   text_" + node->ID + "->name = \"" + node->name + "\";\n";
        
                }

                //--------------- empty

                if (node->type == "Empty")
                {

                    auto en = std::dynamic_pointer_cast<EmptyNode>(node);

                    if (en->m_debugGraphic)
                    {
                        //TODO: set shape

                        if (en->currentShape == "rectangle") {
                            command_queue << "   empty_" + node->ID + " = System::Game::CreateGeom(" + std::to_string(en->positionX) + ", " + std::to_string(en->positionY) + ", " + std::to_string(en->m_debugGraphic->width) + ", " + std::to_string(en->m_debugGraphic->height) + ");\n";
                            command_queue << "   empty_" + node->ID + "->SetDrawStyle(" + std::to_string(en->debug_fill) + ");\n";
                        }

                        if (en->currentShape.length()) {
                            command_queue << "   empty_" + node->ID + "->SetThickness" + std::to_string(en->line_weight) + ");\n";
                            command_queue << "   empty_" + node->ID + "->SetTint({" + std::to_string(en->m_debugGraphic->tint.r) + ", " + std::to_string(en->m_debugGraphic->tint.g) + ", " + std::to_string(en->m_debugGraphic->tint.b) + "});\n";
                            command_queue << "   empty_" + node->ID + "->SetAlpha(" + std::to_string(en->m_debugGraphic->alpha) + ");\n";

                            //shader

                            if (en->HasComponent("Shader") && en->shader.first.length()) 
                                command_queue << "   empty_" + node->ID + "->shader = Shader::Get(\"" + en->shader.first + "\");\n";
                        }
                    }

                    else 
                        command_queue << "   std::shared_ptr<Entity> empty_" + node->ID + " = System::Game::CreateEntity();\n\t";

                    command_queue << "   empty_" + node->ID + "->name = \"" + node->name + "\";\n";
                }

                //--------------- tilemap

                if (node->type == "Tilemap")
                {

                    auto tmn = std::dynamic_pointer_cast<TilemapNode>(node);

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
                                command_queue << "   MapManager::CreateLayer(\"" + tmn->layers[i][2] + "\", ""\"" + tmn->layers[i][0] + "\", " + std::to_string(tmn->map_width) + ", " + std::to_string(tmn->map_height) + ", " + std::to_string(tmn->tile_width) + ", " + std::to_string(tmn->tile_height) + ", " + std::to_string(tmn->depth[i]) + ");\n";
                        }

                        //create map if layers are defined

                        if (tmn->layers[0][0].length()) {
                            command_queue << "   auto map_" + node->ID + " = System::Game::CreateEntity(\"" + "tilemap" + "\");\n\t";
                            command_queue << "   map_" + node->ID + "->name = \"" + tmn->name + "\";\n";
                        }

                    }

                    command_queue << "   }\n";

                    //static physics bodies

                    if (tmn->HasComponent("Physics") && tmn->bodies.size())
                        for (int i = 0; i < tmn->bodies.size(); i++)    
                            command_queue << "   Physics::CreateStaticBody(" + std::to_string(tmn->bodyX[i] + tmn->body_width[i] / 2) + ", " + std::to_string(tmn->bodyY[i] + tmn->body_height[i] / 2) + ", " + std::to_string(tmn->body_width[i] / 2) + ", " + std::to_string(tmn->body_height[i] / 2) + ");\n";

                } 

                //--------------- audio

                if (node->type == "Audio")
                {

                    auto an = std::dynamic_pointer_cast<AudioNode>(node);

                    std::string loop = an->loop ? "true" : "false";

                    command_queue << "   System::Audio::play(\"" + an->audio_source_name + "\", " + loop + ", " + std::to_string(an->volume) + ");\n";

                    command_queue << "   std::shared_ptr<Entity> audio_" + node->ID + " = System::Game::CreateEntity(\"" + "audio" + "\");\n\t";
                    command_queue << "   audio_" + node->ID + "->name = \"" + an->name + "\";\n";
                    
                }

                //--------------- group

                if (node->type == "Group")
                {

                    auto gn = std::dynamic_pointer_cast<GroupNode>(node);

                    if (gn->_nodes.size())
                        writeNodes(gn->_nodes);
                }

                //define behaviors

                for (const auto& behavior : node->behaviors) 
                {

                    std::string entity = (node->type + "_" + node->ID);

                    transform(entity.begin(), entity.end(), entity.begin(), ::tolower);

                    command_queue << "   System::Game::CreateBehavior<entity_behaviors::" + behavior.first + ">(" + entity + ", this);\n";

                }

            }
        };

        loadedFrames.clear();
        loadedAnims.clear();

        //register scene textures

        preload_queue << "   System::Resources::Manager::RegisterTextures();\n";

        //write nodes

        writeNodes(target.second->nodes);

        //convert data string stream to string

        const std::string globalData = global_queue.str(),
                          assetData = asset_queue.str(),
                          preloadData = preload_queue.str(),
                          constructorData = constructor_queue.str(),
                          commandData = command_queue.str();

        std::string name_upper = target.first;

        std::replace(name_upper.begin(), name_upper.end(), '-', '_');
        std::replace(name_upper.begin(), name_upper.end(), ' ', '_');

        transform(name_upper.begin(), name_upper.end(), name_upper.begin(), ::toupper);

        //project source template

        game_src << "\n\nclass " + name_upper + " : public System::Scene {\n\n"; 
        game_src << "    public:\n";
        game_src << "       " + name_upper + "(const Process::Context& context):\n\t\tScene(context, \"" + name_upper + "\") {\n\t" + constructorData + "\n}\n";
        game_src << "           void Preload() override;\n";
        game_src << "           void Run(bool loadMap) override;\n";
        game_src << "    private:\n";
        game_src << "    " + globalData + "\n";
        game_src <<"};\n\n\n"; 

        game_src << "void " + name_upper + "::Preload() {\n" + assetData + "\n" + preloadData + "\n}\n\n";
        game_src << "void " + name_upper + "::Run(bool loadMap) {\n" + commandData + "}\n\n";
        

    }

    //clear scene queue

    for (auto& scene : compileQueue)
        delete scene.second;

    compileQueue.clear();

    //game template

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
    game_src <<	"       ShowWindow(GetConsoleWindow(), SW_HIDE);\n";
    game_src <<	"       SetUnhandledExceptionFilter(UnhandledExceptionFilter);\n";
    game_src <<	"   #endif\n";
    game_src <<	"   #ifdef __EMSCRIPTEN__\n";
    game_src <<	"       System::Application::isMobile = checkMobile();\n";
    game_src <<	"       fetchData();\n";
    game_src <<	"   #elif _ISMOBILE == 1\n";
    game_src <<	"       System::Application::isMobile = true;\n";
    game_src <<	"   #endif\n";
    game_src <<	"       System::Game game;\n";

    for (const auto& scene : Editor::scenes) 
    {
        std::string className = scene;

        std::replace(className.begin(), className.end(), '-', '_');
        std::replace(className.begin(), className.end(), ' ', '_');
        transform(className.begin(), className.end(), className.begin(), ::toupper); 

        game_src << "       game.LoadScene<" + className + ">();\n";
    }
    
    game_src << "       System::Application app { &game, \"" + s_currentProject + "\" };\n";
    game_src <<	"   #ifdef __EMSCRIPTEN__\n";
    game_src <<	"       emscripten_exit_with_live_runtime();\n";
    game_src <<	"   #endif\n";
    game_src << "   return 0;\n";
    game_src <<	"}";

    game_src.close();

    system("cls");

    ShowWindow(GetConsoleWindow(), SW_SHOW);

    if (Editor::platform == "WebGL")
        system(("chdir sdk && buildWebGL.bat " + Editor::projectPath).c_str());

    else if (Editor::platform == "Windows")
        system(("chdir sdk && buildGame.bat " + Editor::projectPath + " " + s_currentProject).c_str());

    if (!Editor::preserveSrc)
        remove(srcPath.c_str());
        
    remove(makefile_path.c_str());
    remove(web_makeFile_path.c_str());
    remove(web_preJS_path.c_str());
    remove(web_HTML_path.c_str());

    if (Editor::platform == "WebGL")
        std::filesystem::remove_all((web + "/assets").c_str());

    Editor::Log("Project " + s_currentProject + " built successfully.");

}



//----------------------------- generate new project layer


void EventListener::GenerateProject()
{

    std::string root_path = Editor::rootPath;
    std::replace(root_path.begin(), root_path.end(), '\\', '/');

    const std::string resources = Editor::projectPath + "\\resources";

    if (std::filesystem::exists(resources)) {
        Editor::Log("Project " + s_currentProject + " already exists.");
        return;
    }

    std::filesystem::create_directory(Editor::projectPath + "\\scenes");

    std::filesystem::create_directory(resources);
    std::filesystem::create_directory(resources + "\\scripts");
    std::filesystem::create_directory(resources + "\\shaders");
    std::filesystem::create_directory(resources + "\\assets");
    std::filesystem::create_directory(resources + "\\prefabs");
    std::filesystem::create_directory(resources + "\\assets\\images");
    std::filesystem::create_directory(resources + "\\assets\\audio");
    std::filesystem::create_directory(resources + "\\assets\\data");


    Editor::Log("New project " + s_currentProject + " generated.");
}

