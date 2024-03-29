#ifdef _WIN32

    #include <windows.h>
    #include <tchar.h>
    #include <bits/stdc++.h>
    #include <iostream>
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
#include "../../../../build/include/app.h"

using namespace editor;


//--------------------------------- file open callbacks (windows only)


static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData)
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


//---------------------------------------


static int CALLBACK BrowseFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    #ifdef _WIN32

        if (uMsg == BFFM_INITIALIZED) {
            LPCTSTR path = reinterpret_cast<LPCTSTR>(lpData);
            ::SendMessage(hwnd, BFFM_SETSELECTION, true, (LPARAM) path);
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

            if (SaveProject()) {
                GenerateProject();
                return true;
            }
        }

    #endif

    return false;
}


//----------------------------- open project layer


bool EventListener::OpenProject() //makes temporary json file to parse data from .spaghyeti
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

            //trim filename extension

            currentProject = std::filesystem::path(Editor::projectPath).parent_path().filename().string();
            currentScene = System::Utils::ReplaceFrom(result.filename().string(), ".", "");

            //temporary file for decoding

            std::string tmp = Editor::projectPath + "spaghyeti_parse.json";

            this->DecodeFile(tmp, result);

            Editor::Reset();

            //load all assets to menu from root folder

            const std::string asset_folders[] = { "images", "audio", "data" };

            for (const std::string &type : asset_folders)
                for (const auto &entry : std::filesystem::directory_iterator(Editor::projectPath + "resources\\assets\\" + type))
                {

                    const std::string dir = entry.path().string(); //includes filename

                    const std::string asset = entry.path().filename().string();

                    System::Resources::Manager::LoadFile(asset.c_str(), dir.c_str());
                    System::Resources::Manager::RegisterAssets();

                    const std::string folder = AssetManager::GetFolder(asset);
                    const std::string texture = AssetManager::GetThumbnail(asset);

                    const std::string key = "\"" + asset + "\"";

                    const std::string productionPath = "\"" +  Editor::projectPath + "resources\\assets" + folder + asset + "\"";

                    AssetManager::loadedAssets.insert({ key, productionPath });

                    unsigned int id = System::Resources::Manager::texture2D->GetTexture(texture).ID;

                    AssetManager::images.push_back({ asset, id });
                }

            //load entity nodes

            std::ifstream JSON(tmp);

            if (JSON.good())
                this->Deserialize(JSON);

            else
                Editor::Log("Error: project file not found.");

            JSON.close();

            //register opened scene

            Editor::scenes.push_back(currentScene);

            remove(tmp.c_str());

            return true;

        }

    #endif

    return false;

}


//----------------------------- save current project layer


bool EventListener::SaveProject(bool saveAs)
{

    try {

        std::string project_root = Editor::projectPath + currentScene + ".spaghyeti";

        std::ifstream file(project_root);

        if (!file.good()) //save as if file does not exist
            saveAs = true;

        file.close();

        auto saveFile = [&](std::string filename) -> bool {

            std::ofstream src(filename);

            json data;

            this->Serialize(data);

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

            Editor::projectPath = std::filesystem::path{ filename }.parent_path().string() + "\\";

            currentProject = System::Utils::ReplaceFrom(std::filesystem::path{ filename }.filename().string(), ".", "");

            Editor::Log("Project saved: " + Editor::projectPath + std::filesystem::path{ filename }.filename().string());

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

                    currentProject = result.filename().string();

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

            const char* dir = result.string().c_str();

            std::string asset = result.filename().string();

            //load asset in sandbox

            System::Resources::Manager::LoadFile(asset.c_str(), dir);
            System::Resources::Manager::RegisterAssets();

            //copy asset to game template

            const auto options = std::filesystem::copy_options::update_existing |
                                 std::filesystem::copy_options::recursive;

            const std::string folder = AssetManager::GetFolder(asset);
            const std::string texture = AssetManager::GetThumbnail(asset);

            std::ifstream file(Editor::projectPath + "resources\\assets" + folder + asset);

            //copy asset reference to respective project folder

            if (!file.good())
                std::filesystem::copy(result.string(), Editor::projectPath + "resources\\assets" + folder + asset, options);

            //register asset into temp cache

            std::map<std::string, std::string>::iterator iterator = AssetManager::loadedAssets.find(asset);

            if (iterator == AssetManager::loadedAssets.end() || Editor::selectedAsset.length() < 0) {

                const std::string key = "\"" + asset + "\"";

                const std::string productionPath = "\"" + Editor::projectPath + "resources\\assets" + folder + asset + "\"";

                AssetManager::loadedAssets.insert({ key, productionPath });
            }

            //apply image to assets menu if not there already

            unsigned int id = System::Resources::Manager::texture2D->GetTexture(texture).ID;

            if (
                std::find_if( 
                    AssetManager::images.begin(), AssetManager::images.end(),
                    [&](std::pair<std::string, GLuint> pair) { return pair.first == asset; }
                ) == AssetManager::images.end()
            )
                AssetManager::images.push_back({ asset, id });

        }

    #endif
}


//--------------------------------- embed calls to temp file for compilation


void EventListener::InsertTo(const std::string &code, const std::string &directory)
{

    std::ofstream src;

    src.open(directory, std::ofstream::app | std::ofstream::out);

    src << code;
    src.close();
}



//-------------------------------- build game from project source



void EventListener::BuildAndRun()
{


    //temp files: asset and command lists

    std::ostringstream asset_queue, global_queue, command_queue, update_queue;

    //set game source input file stream

    std::string srcPath = Editor::projectPath + "\\src\\game.cpp";

    remove(srcPath.c_str());

    std::ofstream game_src(srcPath, std::ofstream::trunc);

    std::string root_path = Editor::rootPath;
    std::replace(root_path.begin(), root_path.end(), '\\', '/');

    game_src << "#ifdef _WIN32\n";
    game_src <<	"#include <windows.h>\n";
    game_src << "#endif\n";
    game_src << "\n#include \"" + root_path + "/include/app.h\"\n\n";

    //include scripts

    for (const auto& script : std::filesystem::recursive_directory_iterator(Editor::projectPath + AssetManager::script_dir)) 
        game_src << "#include " << "\"../resources/scripts/" + script.path().filename().string() + "\"\n";


    //iterate over scenes to include

    for (const auto& scene : Editor::scenes)
    {
        //parse scene files

        for (const auto& file : std::filesystem::directory_iterator(Editor::projectPath)) 
        {

            if (!System::Utils::str_endsWith(file.path().string(), ".spaghyeti")) 
                continue;
            
            if (scene == System::Utils::ReplaceFrom(file.path().filename().string(), ".", ""))
            {
                std::string tmp = Editor::projectPath + "spaghyeti_parse.json";

                this->DecodeFile(tmp, file);

                std::ifstream JSON(tmp);

                if (JSON.good()) {
                    this->ParseScene(scene, JSON);
                    Editor::Log("Scene parsed.");
                }

                else 
                    Editor::Log("There was a problem parsing scene.");

                JSON.close();
                
                //remove temp json file

                remove(tmp.c_str()); 
            }
        }

        //compile scenes

        for (const auto& target : compileQueue)
        {
            //set global vars

            if (target.second.globals_applied)
                for (const auto& global : target.second.globals)
                {
                    if (std::adjacent_find(target.second.globals.begin(), target.second.globals.end()) != target.second.globals.end()) {
                        Editor::Log("Error: duplicate global variable found.");
                        break;
                    }

                    if (!global.first.length() || !global.second.length()) {
                        Editor::Log("Error: incomplete global variable.");
                        break;
                    }

                    std::string type = global.second, 
                                var = "GLOBALVAR_" + global.first;

                    if (global.second == "string")
                        type = "std::string";

                    if (global.second == "int[]")
                        type = "std::vector<int>";

                    if (global.second == "float[]")
                        type = "std::vector<float>";

                    if (global.second == "string[]")
                        type = "std::vector<std::string>";

                    global_queue << "   " + type + " " + var + ";\n";
                }


            command_queue << "   this->SetWorldDimensions(" + std::to_string(target.second.worldWidth) + ", " + std::to_string(target.second.worldHeight) + ");\n";

            command_queue << "   this->context.camera->SetBounds(" + std::to_string(target.second.currentBoundsWidthBegin) + ", " + std::to_string(target.second.currentBoundsWidthEnd) + ", " + std::to_string(target.second.currentBoundsHeightBegin) + ", " + std::to_string(target.second.currentBoundsHeightEnd) + ");\n";
            command_queue << "   this->context.camera->SetBackgroundColor({ " + std::to_string(target.second.cameraBackgroundColor.x) + ", " + std::to_string(target.second.cameraBackgroundColor.y) + ", " + std::to_string(target.second.cameraBackgroundColor.z) + ", " + std::to_string(target.second.cameraBackgroundColor.w) + " });\n";
            command_queue << "   this->context.camera->SetZoom(" + std::to_string(target.second.cameraZoom) + ");\n";
            command_queue << "   this->context.camera->SetPosition({ " + std::to_string(target.second.cameraPosition.x) + ", " + std::to_string(target.second.cameraPosition.y) + " });\n";
        
            std::string phys_isCont = target.second.gravity_continuous ? "true" : "false",
                        phys_isSleeping = target.second.gravity_sleeping ? "true" : "false";

            command_queue << "   this->context.physics->continuous = " + phys_isCont + ";\n";
            command_queue << "   this->context.physics->sleeping = " + phys_isSleeping + ";\n";
            command_queue << "   this->context.physics->SetGravity(" + std::to_string(target.second.gravityX) + ", " + std::to_string(target.second.gravityY) + ");\n";

            for (const auto& asset : AssetManager::loadedAssets)
            {
                std::string path = asset.second;

                std::replace(path.begin(), path.end(), '\\', '/');

                asset_queue << "  System::Resources::Manager::LoadFile(" + asset.first + ", " + path + ");\n";
            }

            //command data, iterate over nodes and create objects

            for (const auto& node : target.second.nodes)
            {

                //load shaders

                if (node->HasComponent("Shader") && node->shader.first.length())
                    asset_queue << "  System::Resources::Manager::shader->Load(\"" + node->shader.first + "\", \"" + node->shader.second.first + "\", \"" + node->shader.second.second + "\");\n";


                //--------------- sprite

                if (node->m_type == "Sprite")
                {

                    auto sn = std::dynamic_pointer_cast<SpriteNode>(node);

                    //load frames

                    std::ostringstream frame_oss;
                    std::vector<std::string> framesToLoad;

                    for (const auto& frame : sn->frames)
                        framesToLoad.push_back("{" + std::to_string(frame.x) + ", " + std::to_string(frame.y) + ", " + std::to_string(frame.width) + ", " + std::to_string(frame.height) + ", " + std::to_string(frame.factorX) + ", " + std::to_string(frame.factorY) + "}");

                    if (!framesToLoad.empty()) 
                    {
                        std::copy(framesToLoad.begin(), framesToLoad.end() - 1, std::ostream_iterator<std::string>(frame_oss, ", "));
                        frame_oss << framesToLoad.back();

                        if (sn->frames.size() > 1)
                            asset_queue << "  System::Resources::Manager::LoadFrames(\"" + sn->key + "\", {" + frame_oss.str() + "});\n";
                    }

                    //load animations

                    std::ostringstream anim_oss;
                    std::vector<std::string> animsToLoad;

                    for (const auto& anim : sn->animations)
                        animsToLoad.push_back("{\"" + std::string(anim.second.key) + "\"" + ", {" + std::to_string(anim.second.start) + ", " + std::to_string(anim.second.end) + "} }");

                    if (!animsToLoad.empty()) {
                        std::copy(animsToLoad.begin(), animsToLoad.end() - 1, std::ostream_iterator<std::string>(anim_oss, ", "));
                        anim_oss << animsToLoad.back();

                        asset_queue << "  System::Resources::Manager::LoadAnims(\"" + sn->key + "\", {" + anim_oss.str() + "});\n";
                    }

                    global_queue << "   std::shared_ptr<Sprite> sprite_" + node->m_ID + ";\n\t";

                    command_queue << "   this->sprite_" + node->m_ID + " = System::Game::CreateSprite(\"" + sn->key + "\", " + std::to_string(0.0f) + ", " + std::to_string(0.0f) + ");\n";
                                            
                    //sprite configurations

                    command_queue << "   this->sprite_" + node->m_ID + "->SetScale(" + std::to_string(sn->scaleX) + ", " + std::to_string(sn->scaleY) + ");\n";
                    command_queue << "   this->sprite_" + node->m_ID + "->SetPosition(" + std::to_string(sn->positionX) + ", " + std::to_string(sn->positionY) + ");\n";
                    command_queue << "   this->sprite_" + node->m_ID + "->SetRotation(" + std::to_string(sn->rotation) + ");\n";
                    command_queue << "   this->sprite_" + node->m_ID + "->SetTint({ " + std::to_string(sn->tint.x) + ", " + std::to_string(sn->tint.y) + ", " + std::to_string(sn->tint.z) + " });\n";
                    command_queue << "   this->sprite_" + node->m_ID + "->SetDepth(" + std::to_string(sn->depth) + ");\n";
                    command_queue << "   this->sprite_" + node->m_ID + "->SetFlip(" + std::to_string(sn->flippedX) + ", " + std::to_string(sn->flippedY) + ");\n";

                    std::string filtering = sn->filter_nearest ? "GL_NEAREST" : "GL_LINEAR";

                    command_queue << "   this->sprite_" + node->m_ID + "->m_texture.Filter_Min = " + filtering + ";\n";
                    command_queue << "   this->sprite_" + node->m_ID + "->m_texture.Filter_Max = " + filtering + ";\n";

                    if (sn->lock_in_place)
                        update_queue << "   this->sprite_" + node->m_ID + "->m_shader = System::Resources::Manager::shader->GetShader(\"sprite_billboard\");\n";

                    //physics bodies

                    if (sn->HasComponent("Physics"))
                    {
                        for (int i = 0; i < sn->bodies.size(); i++) 
                        {
    
                            if (sn->bodies[i].second == "static")
                                command_queue << "   this->sprite_" + node->m_ID + "->bodies.push_back({ Physics::CreateStaticBody(" + std::to_string(sn->positionX) + ", " + std::to_string(sn->positionY) + ", " + std::to_string(sn->body_width[i]) + ", " + std::to_string(sn->body_height[i]) + ", " + std::to_string(sn->bodyX[i]) + ", " + std::to_string(sn->bodyY[i]) + " });\n";
                            
                            if (sn->bodies[i].second == "dynamic")
                                command_queue << "   this->sprite_" + node->m_ID + "->bodies.push_back({ Physics::CreateDynamicBody(\"box\", " + std::to_string(sn->positionX) + ", " + std::to_string(sn->positionY) + ", " + std::to_string(sn->body_width[i]) + ", " + std::to_string(sn->body_height[i]) + ", " + std::to_string(sn->is_sensor[i].b) + ", " + std::to_string(sn->body_pointer[i]) + ", " + std::to_string(sn->density) + ", " + std::to_string(sn->friction) + ", " + std::to_string(sn->restitution) + "), { " + std::to_string(sn->bodyX[i]) + ", " + std::to_string(sn->bodyY[i]) + " } });\n";
                        }    

                        command_queue << "   for (const auto& body : sprite_" + node->m_ID + "->bodies)\n       body.first->SetFixedRotation(true);\n";

                    }
         
                    //animator

                    if (sn->HasComponent("Animator") && sn->animations.size()) {
                        command_queue << "   this->sprite_" + node->m_ID + "->m_anims = System::Resources::Manager::GetAnimations(\"" + sn->key + "\");\n";
                        command_queue << "   this->sprite_" + node->m_ID + "->ReadSpritesheetData();\n"; 
                    } 
     
                    //shader

                    if (sn->HasComponent("Shader") && sn->shader.first.length()) 
                        command_queue << "   this->sprite_" + node->m_ID + "->m_shader = System::Resources::Manager::shader->GetShader(\"" + sn->shader.first + "\");\n";
  
                }

                //--------------- text

                if (node->m_type == "Text")
                {          

                    auto tn = std::dynamic_pointer_cast<TextNode>(node);

                    global_queue << "   std::shared_ptr<Text> text_" + node->m_ID + ";\n\t";

                    command_queue << "   this->text_" + node->m_ID + " = System::Game::CreateText(\"" + tn->textBuf + "\", " + std::to_string(0.0f) + ", " + std::to_string(0.0f) + ");\n";

                    command_queue << "   this->text_" + node->m_ID + "->SetScale(" + std::to_string(tn->scaleX) + ", " + std::to_string(tn->scaleY) + ");\n";
                    command_queue << "   this->text_" + node->m_ID + "->SetPosition(" + std::to_string(tn->positionX) + ", " + std::to_string(tn->positionY) + ");\n";
                    command_queue << "   this->text_" + node->m_ID + "->SetRotation(" + std::to_string(tn->rotation) + ");\n";
                    command_queue << "   this->text_" + node->m_ID + "->SetTint({ " + std::to_string(tn->tint.x) + ", " + std::to_string(tn->tint.y) + ", " + std::to_string(tn->tint.z) + " });\n";
                    command_queue << "   this->text_" + node->m_ID + "->SetAlpha(" + std::to_string(tn->alpha) + ");\n";
                    command_queue << "   this->text_" + node->m_ID + "->SetDepth(" + std::to_string(tn->depth) + ");\n";
     
                }

                //--------------- empty

                if (node->m_type == "Empty")
                {
        
                    auto en = std::dynamic_pointer_cast<EmptyNode>(node);

                    global_queue << "   std::shared_ptr<Entity> empty_" + node->m_ID + ";\n\t";

                    if (en->currentShape.length()) {
                        command_queue << "   this->empty_" + node->m_ID + "->SetTint({" + std::to_string(en->m_debugGraphic->m_tint.r) + ", " + std::to_string(en->m_debugGraphic->m_tint.g) + ", " + std::to_string(en->m_debugGraphic->m_tint.b) + "});\n";
                        command_queue << "   this->empty_" + node->m_ID + "->SetAlpha(" + std::to_string(en->m_debugGraphic->m_alpha) + ");\n";
                    }

                    //TODO: set shape

                    if (en->currentShape == "rectangle") {
                        command_queue << "   this->empty_" + node->m_ID + " = System::Game::CreateGeom(" + std::to_string(en->positionX) + ", " + std::to_string(en->positionY) + ", " + std::to_string(en->m_debugGraphic->width) + ", " + std::to_string(en->m_debugGraphic->height) + ");\n";
                        command_queue << "   this->empty_" + node->m_ID + "->SetDrawStyle(" + std::to_string(en->debug_fill) + ");\n";
                    }

                    //shader

                    if (en->HasComponent("Shader") && en->shader.first.length()) 
                        command_queue << "   this->empty_" + node->m_ID + "->m_shader = System::Resources::Manager::shader->GetShader(\"" + en->shader.first + "\");\n";

                }

                //--------------- tilemap

                if (node->m_type == "Tilemap")
                {

                    auto tmn = std::dynamic_pointer_cast<TilemapNode>(node);

                    //load frames

                    std::ostringstream offset_oss;
                    std::vector<std::string> offsetsToLoad;

                    for (const auto& offset : tmn->offset)
                        offsetsToLoad.push_back("{" + std::to_string(offset[0])  + ", " + std::to_string(offset[1]) + ", " + std::to_string(offset[2]) + ", " + std::to_string(offset[3]) + ", " + std::to_string(offset[4]) + ", " + std::to_string(offset[5]) + "}");
                    
                    if (!offsetsToLoad.empty()) {
                        std::copy(offsetsToLoad.begin(), offsetsToLoad.end() - 1, std::ostream_iterator<std::string>(offset_oss, ", "));
                        offset_oss << offsetsToLoad.back();
                    } 

                    if (tmn->layers.size())
                        for (int i = 0; i < tmn->layer; i++)
                        {
                            asset_queue << "  System::Resources::Manager::LoadFrames(\"" + tmn->layers[i][2] + "\", { " + offset_oss.str() + " });\n";
                            asset_queue << "  System::Resources::Manager::LoadTilemap(\"" + tmn->layers[i][0] + "\", System::Resources::Manager::ParseCSV(\"" + tmn->layers[i][0] + "\"));\n";
                            
                            command_queue << "   MapManager::CreateLayer(\"" + tmn->layers[i][0] + "\", \"" + tmn->layers[i][2] + "\", " + std::to_string(tmn->map_width) + ", " + std::to_string(tmn->map_height) + ", " + std::to_string(tmn->tile_width) + ", " + std::to_string(tmn->tile_height) + ", " + std::to_string(tmn->depth[i]) + ");\n";
                        }

                    //static physics bodies

                    if (tmn->HasComponent("Physics"))
                        for (int i = 0; i < tmn->bodies.size(); i++)    
                            command_queue << "   Physics::CreateStaticBody(" + std::to_string(tmn->bodyX[i]) + ", " + std::to_string(tmn->bodyY[i]) + ", " + std::to_string(tmn->body_width[i]) + ", " + std::to_string(tmn->body_height[i]) + ");\n";

                } 

                //--------------- audio

                if (node->m_type == "Audio")
                {

                    auto an = std::dynamic_pointer_cast<AudioNode>(node);

                    std::string loop = an->loop ? "true" : "false";

                    command_queue << "   System::Audio::play(" + an->audio_source_name + ", " + loop + ", " + std::to_string(an->volume) + ");\n";

                }

                //define behaviors

                for (const auto& behavior : node->behaviors) {

                    std::string entity = (node->m_type + "_" + node->m_ID);

                    transform(entity.begin(), entity.end(), entity.begin(), ::tolower);

                    command_queue << "   System::Game::CreateBehavior<entity_behaviors::" + behavior.first + ">(" + entity + ", this);\n";

                }
            }

            //convert data string stream to string

            std::string globalData = global_queue.str(), 
                        commandData = command_queue.str(),  
                        preloadData = asset_queue.str(),
                        updateData = update_queue.str(),
                        name_upper = scene;

            transform(name_upper.begin(), name_upper.end(), name_upper.begin(), ::toupper);

            //project source template

            game_src << "\n\nclass " + name_upper + " : public System::Scene {\n\n"; 
            game_src << "    public:\n";
            game_src << "       " + name_upper + "(const Process::Context& context):\n\t\tScene(context, \"" + name_upper + "\") { }\n";
            game_src << "           void Preload() override;\n";
            game_src << "           void Run() override;\n";
            game_src << "           void Update() override;\n";
            game_src << "    private:\n";
            game_src << "    " + globalData + "\n";
            game_src <<"};\n\n\n"; 

            game_src << "void " + name_upper + "::Preload() {\n" + preloadData + "  System::Resources::Manager::RegisterAssets();\n}\n\n";
            game_src << "void " + name_upper + "::Run() {\n" + commandData + "}\n\n";
            game_src << "void " + name_upper + "::Update() {\n"      + updateData +            "\n}\n\n\n";

            //remove scene template after parsed

            auto it = compileQueue.find(scene);
            
            if (it != compileQueue.end())
                compileQueue.erase(it);
        }

    }  

    //game template

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
    game_src << "       System::Application::name = \"" + currentProject + "\";\n";

    for (const auto& scene : Editor::scenes) 
    {
        std::string className = scene;
        transform(className.begin(), className.end(), className.begin(), ::toupper); 

        game_src << "       System::Game::LoadScene<" + className + ">(&game);\n";
    }
    
    game_src << "       System::Application app { &game };\n";
    game_src <<	"   #ifdef __EMSCRIPTEN__\n";
    game_src <<	"       emscripten_exit_with_live_runtime();\n";
    game_src <<	"   #endif\n";
    game_src << "   return 0;\n";
    game_src <<	"}";

    game_src.close();

    system("cls");

    ShowWindow(GetConsoleWindow(), SW_SHOW);

        system(Editor::platform == "Windows" ?
            ("buildGame.bat " + Editor::projectPath + " " + currentProject).c_str() :
            ("buildWebGL.bat " + Editor::projectPath).c_str()
        );

    remove(srcPath.c_str());
 
    Editor::Log("Project " + currentProject + " built successfully.");


}



//----------------------------- generate new project layer


void EventListener::GenerateProject()
{

    std::string root_path = Editor::rootPath;
    std::replace(root_path.begin(), root_path.end(), '\\', '/');

    std::string name_upper = currentProject;

    transform(name_upper.begin(), name_upper.end(), name_upper.begin(), ::toupper);

    std::string src = Editor::projectPath + "\\src";
    std::string resources = Editor::projectPath + "\\resources";
    std::string web = Editor::projectPath + "\\web";

    std::filesystem::create_directory(src);
    std::filesystem::create_directory(resources);

    std::filesystem::create_directory(resources + "\\icon");
    std::filesystem::create_directory(resources + "\\scripts");
    std::filesystem::create_directory(resources + "\\shaders");
    std::filesystem::create_directory(resources + "\\assets");

    std::filesystem::create_directory(resources + "\\assets\\images");
    std::filesystem::create_directory(resources + "\\assets\\audio");
    std::filesystem::create_directory(resources + "\\assets\\data");

    std::filesystem::create_directory(web);
    std::filesystem::create_directory(web + "\\assets");
    std::filesystem::create_directory(web + "\\dist");

    std::ofstream main_makeFile(Editor::projectPath + "/Makefile");
    std::ofstream main_h(src + "/game.h");

    std::ofstream icon_rc(resources + "/icon/icon.rc");
    std::ofstream icon_makeFile(resources + "/icon/Makefile");

    std::ofstream web_makeFile(web + "/Makefile");
    std::ofstream web_preJS(web + "/pre-js.js");
    std::ofstream web_HTML(web + "/template.html");

    main_makeFile << "\n";
    main_makeFile << "OBJS = \\" << "\n";
    main_makeFile << "  $(wildcard ./src/*.cpp) \\" << "\n";
    main_makeFile << "  $(wildcard ./src/**/*.cpp) \\" << "\n";
    main_makeFile << "  $(wildcard ./resources/scripts/*.cpp) \\" << "\n";
    main_makeFile << "  spaghyeti_source_runtime-core.dll" << "\n\n";
    main_makeFile << "all : $(OBJS)" << "\n";
    main_makeFile << "      g++ -g -std=c++17 $(OBJS) -w -lmingw32 -lopengl32 -lglfw3 -lgdi32 -luser32 -lkernel32 ./resources/icon/icon.o -o $(PROJECT).exe";

    main_h << "#pragma once" << "\n";
    main_h << "\n#include \"" + root_path + "/include/game.h\"\n\n";
    main_h << "class " << name_upper << " : public Game {" << "\n";
    main_h << "     public:" << "\n";

    main_h << "         " << name_upper << "() { name = " << name_upper << "; }" << "\n";
    main_h << "         void Preload() override;" << "\n";
    main_h << "         void Run(Camera* camera) override;" << "\n";
    main_h << "         void Update(Inputs* inputs, Camera* camera) override;" << "\n";
    main_h << "};";

    icon_rc << "1 ICON \"./icon.ico\"";
    icon_makeFile << "icon.o: icon.rc" << "\n";
	icon_makeFile << "      windres icon.rc icon.o";

    web_makeFile << "OBJS = \\" << "\n";
    web_makeFile << "   $(wildcard ../src/*.cpp) \\" << "\n";
	web_makeFile << "   $(wildcard ../src/**/*.cpp) \\" << "\n";
	web_makeFile << "   $(wildcard ../resources/scripts/*.cpp)" << "\n";
    web_makeFile << "CC = em++ -std=c++20 --pre-js pre-js.js --preload-file assets --use-preload-plugins" << "\n";
    web_makeFile << "COMPILER_FLAGS = \\" << "\n";
    web_makeFile << "   -DDEVELOPMENT=0 \\" << "\n";
    web_makeFile << "   -DPRINT_LOGS=0 \\" << "\n";
    web_makeFile << "   -D_ISMOBILE=0 \\" << "\n";
    web_makeFile << "   -D_JSON=0" << "\n";
    web_makeFile << "LINKER_FLAGS = \\" << "\n";
    web_makeFile << "   -sEXPORT_ALL=1 \\" << "\n";
    web_makeFile << "   -sWASM=1 \\" << "\n";
    web_makeFile << "   -sLEGACY_GL_EMULATION=0 \\" << "\n";
    web_makeFile << "   -sASSERTIONS \\" << "\n";
    web_makeFile << "   -sGL_ASSERTIONS=1 \\" << "\n";
    web_makeFile << "   -sMAX_WEBGL_VERSION=3 \\" << "\n";
    web_makeFile << "   -sMIN_WEBGL_VERSION=0 \\" << "\n";
    web_makeFile << "   -sUSE_WEBGL2=1 \\" << "\n";
    web_makeFile << "   -sFULL_ES3=1 \\" << "\n";
    web_makeFile << "   -sUSE_GLFW=3 \\" << "\n";
    web_makeFile << "   -sUSE_LIBPNG=1 \\" << "\n";
    web_makeFile << "   -sUSE_ZLIB \\" << "\n";
    web_makeFile << "   -sASYNCIFY \\" << "\n";
    web_makeFile << "   -sUSE_PTHREADS=1 \\" << "\n";
    web_makeFile << "   -sPTHREAD_POOL_SIZE_STRICT=28 \\" << "\n";
    web_makeFile << "   -sSHARED_MEMORY=1" << "\n";
    web_makeFile << "all : $(OBJS)" << "\n";
    web_makeFile << "       $(CC) $(OBJS) -L../spaghyeti_source_runtime-core.dll $(COMPILER_FLAGS) $(LINKER_FLAGS) -O3 -o dist/index.html --shell-file template.html";

    web_preJS << "Module['window']" << "\n";
    web_preJS << "Module['document']" << "\n";
    web_preJS << "Module['logReadFiles'] = function(files) { console.log('using files: ', files)}" << "\n";

    web_HTML << "<!DOCTYPE html>" << "\n";
    web_HTML << "<html>" << "\n";
    web_HTML << "<head>" << "\n";
    web_HTML << "   <meta charset=\"utf-8\">" << "\n";
    web_HTML << "   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">" << "\n";
    web_HTML << "   <link rel=\"stylesheet\" type=\"text/css\" href=\"./style.css\" />" << "\n";
	web_HTML << "   <link rel=\"icon\" type=\"image/png\" sizes=\"192x192\" href=\"./icon.png\" />" << "\n";
    web_HTML << "   <title>" << name_upper << "</title>" << "\n";
    web_HTML << "</head>" << "\n";
    web_HTML << "<body>" << "\n";
    web_HTML << "   <!-- Create the canvas that the C++ code will draw into -->" << "\n";
    web_HTML << "   <canvas id=\"canvas\" oncontextmenu=\"event.preventDefault()\"></canvas>" << "\n";
    web_HTML << "   <!-- MANUAL RESIZE OPTIONS -->" << "\n";
    web_HTML << "   <!-- <span id=controls>" << "\n";
    web_HTML << "   <span>" << "\n";
    web_HTML << "       <input" << "\n";
    web_HTML << "           type=checkbox"  << "\n";
    web_HTML << "           id=resize>Resize canvas" << "\n";
    web_HTML << "   </span>" << "\n";
    web_HTML << "   <span>" << "\n";
    web_HTML << "       <input" << "\n";
    web_HTML << "           type=checkbox"  << "\n";
    web_HTML << "           id=pointerLock" << "\n";
    web_HTML << "           checked>Lock/hide mouse pointer    " << "\n";
    web_HTML << "   </span>" << "\n";
    web_HTML << "   <span>" << "\n";
    web_HTML << "       <input" << "\n";
    web_HTML << "           type=button"  << "\n";
    web_HTML << "           onclick='Module.requestFullscreen(document.getElementById(\"pointerLock\").checked,document.getElementById(\"resize\").checked)'" << "\n";
    web_HTML << "           value=Fullscreen>" << "\n";
    web_HTML << "   </span>" << "\n";
    web_HTML << "   </span> -->" << "\n";
    web_HTML << "   <!-- Allow the C++ to access the canvas element -->" << "\n";
    web_HTML << "   <script type='text/javascript'>" << "\n";
    web_HTML << "       var Module = {" << "\n";
    web_HTML << "           window: (function() { return window; })()," << "\n";
    web_HTML << "           document: (function() { return document; })()," << "\n";
    web_HTML << "           canvas: (function() { return document.getElementById('canvas'); })()" << "\n";
    web_HTML << "       };" << "\n";
    web_HTML << "   /* PREVENTS TOUCH EVENT FROM CLICKING ON INPUT AFTER 300MS */" << "\n";
    web_HTML << "       document.addEventListener('touchstart', () => setTimeout(() => document.querySelector('canvas').style.pointerEvents = 'none', 250));" << "\n";
    web_HTML << "       document.addEventListener('touchend', () => document.querySelector('canvas').style.pointerEvents = 'auto');" << "\n";
    web_HTML << "       document.addEventListener('touchmove', () => document.querySelector('canvas').style.pointerEvents = 'auto');" << "\n";
    web_HTML << "       document.addEventListener('touchcancel', () => document.querySelector('canvas').style.pointerEvents = 'auto');" << "\n";
    web_HTML << "   /* INIT FULLSCREEN RESIZE ON CLICK */" << "\n";
    web_HTML << "   //let isFullScreen = false;" << "\n";
    web_HTML << "   // document.addEventListener('click', () => {" << "\n";
    web_HTML << "   //     if (isFullScreen)" << "\n";
    web_HTML << "   //         return;" << "\n";
    web_HTML << "   //     isFullScreen = true;" << "\n";
    web_HTML << "   //     Module.requestFullscreen(false, true);" << "\n";
    web_HTML << "   // });" << "\n";
    web_HTML << "   </script>" << "\n";
    web_HTML << "   <!-- Add the javascript glue code (base.js) as generated by Emscripten -->" << "\n";
    web_HTML << "   <script src=\"base.js\"></script>" << "\n";
    web_HTML << "   { SCRIPT }" << "\n";
    web_HTML << "</body>" << "\n";
    web_HTML << "</html>";


    Editor::Log("New project " + currentProject + " generated.");
}



