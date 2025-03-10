#include <sstream>
#include <fstream>

#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/manager.h"

using namespace System::Resources;


//clear resources and assets, "all" flag true removes textures and shaders
void Manager::Clear(bool all)
{
    
    System::Application::resources->m_atlases.clear();
    System::Application::resources->m_atlas_paths.clear();
    System::Application::resources->m_anims.clear();

    if (all)
    {
        System::Application::resources->m_file_assets.clear();
        System::Application::resources->m_raw_assets.clear();

        for (auto& texture : System::Application::resources->textures) 
            texture.second.Delete();

        System::Application::resources->textures.clear();

        for (auto& shader : System::Application::resources->shaders)
            shader.second.Delete();

        System::Application::resources->shaders.clear();
        
    }

    LOG("Resources: assets cleared.");

}


//-----------------------------------

//register the textures
void Manager::RegisterTextures()
{

    #ifndef __EMSCRIPTEN__

        //load base texture (white 1 x 1)

        static unsigned char data[] = { 0xff, 0xff, 0xff, 0xff };
        LoadRaw("image", "base", data, 4);

    #endif

    //load textures

    for (const auto& asset : System::Application::resources->m_raw_assets)
        if (asset.second.type == "image" && System::Application::resources->textures.find(asset.first) == System::Application::resources->textures.end())
            Graphics::Texture2D::Load(asset.first);

    for (const auto& asset : System::Application::resources->m_file_assets)
        if (asset.second.first == "image" && System::Application::resources->textures.find(asset.first) == System::Application::resources->textures.end())
            Graphics::Texture2D::Load(asset.first);

    LOG("Resources: assets registered.");

}


//----------------------------------- 

//load individual files
void Manager::LoadFile(const char* key, const char* path)
{
    std::string type;

    if (System::Utils::GetFileType(path) == "image")
        type = "image";

    else if (System::Utils::GetFileType(path) == "audio")
        type = "audio";

    else if (System::Utils::GetFileType(path) == "data")
        type = "data";

    else {
        LOG("Resources: filetype not available for loading.");
        return;
    }
    
    System::Application::resources->m_file_assets.insert({ key, { type, path } });

} 


//------------------------------------- 


//load raw char array / size in bytes
void Manager::LoadRaw(const std::string& type, const char* key, unsigned char* arr, unsigned int bytes) {

    if (type != "image" && type != "audio") {
        LOG("Resources: file not available for loading.");
        return;
    }

    System::Application::resources->m_raw_assets.insert({ key, { type, arr, bytes } }); 
}


//------------------------------------

//load frames from vector of int arrays
void Manager::LoadFrames(const std::string& key, const std::vector<std::array<int, 6>>& frames) {
    UnLoadFrames(key);
    System::Application::resources->m_atlases.insert( { key, frames } );
}


//------------------------------------ 


//load frames from file
void Manager::LoadAtlas(const std::string& key, const char* path) {
    UnLoadAtlas(key);
    System::Application::resources->m_atlas_paths.insert( { key, path } );
}


//------------------------------------ 


//load animations from map of start / end pairs defined by key
void Manager::LoadAnims(const std::string& key, const std::map<std::string, std::pair<int, int>>& anims) {
    UnLoadAnims(key);
    System::Application::resources->m_anims.insert( { key, anims } );
}


//-------------------------------------  

//unload files
void Manager::UnLoadFile(const char* key) {
    if (System::Application::resources->m_file_assets.find(key) != System::Application::resources->m_file_assets.end())
        System::Application::resources->m_file_assets.erase(System::Application::resources->m_file_assets.find(key));
} 


//------------------------------------

// unload frames from vector of int arrays
void Manager::UnLoadFrames(const std::string& key) {
    if (System::Application::resources->m_atlases.find(key) != System::Application::resources->m_atlases.end())
        System::Application::resources->m_atlases.erase(System::Application::resources->m_atlases.find(key));
}


//------------------------------------ 

//unload frames from file
void Manager::UnLoadAtlas(const std::string& key) {
    if (System::Application::resources->m_atlas_paths.find(key) != System::Application::resources->m_atlas_paths.end())
        System::Application::resources->m_atlas_paths.erase(System::Application::resources->m_atlas_paths.find(key));
}


//------------------------------------ 

//unload animations 
void Manager::UnLoadAnims(const std::string& key) {
    if (System::Application::resources->m_anims.find(key) != System::Application::resources->m_anims.end())
        System::Application::resources->m_anims.erase(System::Application::resources->m_anims.find(key));
}


//------------------------------------- 

//unload raw char array audio
void Manager::UnLoadRaw(const char* type, const char* key) {
    auto it = System::Application::resources->m_raw_assets.find(key); 
    if (System::Application::resources->m_raw_assets.find(key) != System::Application::resources->m_raw_assets.end()) 
        if (it->second.type == type)
            System::Application::resources->m_raw_assets.erase(System::Application::resources->m_raw_assets.find(key));
}


//----------------------------

//get raw data
const BinaryResource Manager::GetResource(const std::string& key) {
    const auto resource = System::Application::resources->m_raw_assets.find(key);
    if (resource != System::Application::resources->m_raw_assets.end())
        return resource->second;

    return {};
}


//-------------------------------- 

//get raw atlas
const std::vector<std::array<int, 6>> Manager::GetRawSpritesheetData(const std::string& key) {
    const std::map<std::string, std::vector<std::array<int, 6>>>::iterator it = System::Application::resources->m_atlases.find(key);
    if (it != System::Application::resources->m_atlases.end())
        return it->second; 
        
    return {};
}


//--------------------------------- 

//get anims by sprite key
const std::map<std::string, std::pair<int, int>> Manager::GetAnimations(const std::string& key) {
    const std::map<std::string, std::map<std::string, std::pair<int, int>>>::iterator it = System::Application::resources->m_anims.find(key);
    if (it != System::Application::resources->m_anims.end())
        return it->second;

    return {};
}


//--------------------------------

//get atlas path
const char* Manager::GetSpritesheetPath(const std::string& key) {
    const std::map<std::string, const char*>::iterator it = System::Application::resources->m_atlas_paths.find(key);
    return it != System::Application::resources->m_atlas_paths.end() ? 
        it->second : "not found";
}


//--------------------------- 

//get asset path
const char* Manager::GetFilePath(const std::string& key) {
   const auto it = System::Application::resources->m_file_assets.find(key);
    return it != System::Application::resources->m_file_assets.end() ?
        it->second.second.c_str() : "not found";

}


//--------------------------- 

//parse CSV
const std::vector<std::string> Manager::ParseCSV(const std::string& key, int index)
{

    std::vector<std::string> result;
    std::string line;

    result.reserve(10000);

    const auto it = System::Application::resources->m_file_assets.find(key);
 
    if (it->second.first == "data" && it != System::Application::resources->m_file_assets.end()) 
    { 

        std::ifstream in(it->second.second);

        if (System::Utils::str_endsWith(it->second.second, ".json")) //json array
        {
            #if USE_JSON == 1 

                json data = json::parse(in);
                std::stringstream ss;

                if (data.contains("layers")) 
                    for (int i = 0; i < data["layers"].size(); i++) 
                        if (index == i && data["layers"][i].contains("data")) 
                        {
                            for (auto& d : data["layers"][i]["data"]) 
                            { 
                                if (d == 0)
                                    d = -1;

                                ss << d << ",";
                            }

                            while(getline(ss, line))
                                result.emplace_back(line);
                            
                        }
            
            #endif
                        
        }

        else if (System::Utils::str_endsWith(it->second.second, ".csv")) //plain csv file
            while(getline(in, line))
                result.emplace_back(line + ",");
     
        in.close();      
    }
        
    return result; 

}


