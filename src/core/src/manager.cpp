#include <sstream>
#include <fstream>

#if USE_JSON == 1
	#include "../../vendors/nlohmann/json.hpp"
	using json = nlohmann::json;
#endif

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
        LoadRaw(IMAGE, "base", data, 4);

    #endif

    //load textures

    for (const auto& asset : System::Application::resources->m_raw_assets)
        if (asset.second.type == IMAGE && System::Application::resources->textures.find(asset.first) == System::Application::resources->textures.end())
            Graphics::Texture2D::Load(asset.first);

    for (const auto& asset : System::Application::resources->m_file_assets)
        if (asset.second.first == IMAGE && System::Application::resources->textures.find(asset.first) == System::Application::resources->textures.end())
            Graphics::Texture2D::Load(asset.first);

    LOG("Resources: assets registered.");

}


//----------------------------------- 

//load individual files
void Manager::LoadFile(const std::string& key, const std::string& path) {

    if (System::Utils::GetFileType(path) == NOT_SUPPORTED) {
        LOG("Resources: filetype not available for loading.");
        return;
    }
    
    System::Application::resources->m_file_assets.insert({ key, { System::Utils::GetFileType(path), path } });
} 

 
//------------------------------------- 


//load raw char array / size in bytes
void Manager::LoadRaw(const int type, const std::string& key, const unsigned char* arr, const unsigned int bytes) {

    if (type != IMAGE && type != AUDIO && type != TEXT) {
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
void Manager::LoadAtlas(const std::string& key, const std::string& path) {
    UnLoadAtlas(key);
    System::Application::resources->m_atlas_paths.insert( { key, path } );
}


//------------------------------------ 


//load animations from map of start / end pairs defined by key
void Manager::LoadAnims(const std::string& key, const std::map<const std::string, std::pair<int, int>>& anims) {
    UnLoadAnims(key);
    System::Application::resources->m_anims.insert( { key, anims } );
}


//-------------------------------------  

//unload files
void Manager::UnLoadFile(const std::string& key) {
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
void Manager::UnLoadRaw(const int type, const std::string& key) {
    const auto it = System::Application::resources->m_raw_assets.find(key); 
    if (System::Application::resources->m_raw_assets.find(key) != System::Application::resources->m_raw_assets.end()) 
        if (it->second.type == type)
            System::Application::resources->m_raw_assets.erase(System::Application::resources->m_raw_assets.find(key));
}


//----------------------------

//get raw data
const BinaryResource* Manager::GetResource(const std::string& key) {
    const auto resource = System::Application::resources->m_raw_assets.find(key);
    return resource != System::Application::resources->m_raw_assets.end() ?
        &resource->second : nullptr;
}


//-------------------------------- 

//get raw atlas
const std::vector<std::array<int, 6>>* Manager::GetRawSpritesheetData(const std::string& key) {
    const auto it = System::Application::resources->m_atlases.find(key);
    return it != System::Application::resources->m_atlases.end() ?
        &it->second : nullptr;
}


//--------------------------------- 

//get anims by sprite key
const std::map<const std::string, std::pair<int, int>>* Manager::GetAnimations(const std::string& key) {
    const auto it = System::Application::resources->m_anims.find(key);
    return it != System::Application::resources->m_anims.end() ?
        &it->second : nullptr;
}


//--------------------------------

//get atlas path
const std::string* Manager::GetSpritesheetPath(const std::string& key) {
    const auto it = System::Application::resources->m_atlas_paths.find(key);
    return it != System::Application::resources->m_atlas_paths.end() ? 
        &it->second : nullptr;
}


//--------------------------- 

//get asset path
const std::string* Manager::GetFilePath(const std::string& key) {
   const auto it = System::Application::resources->m_file_assets.find(key);
    return it != System::Application::resources->m_file_assets.end() ?
        &it->second.second : nullptr;
}


//--------------------------- 

const std::vector<std::string> Manager::ParseMapData(const std::string& key, int index)
{
    std::vector<std::string> result;
    std::string line;

    result.reserve(10000);

    const auto it = System::Application::resources->m_file_assets.find(key);
 
    if (it->second.first == DATA && it != System::Application::resources->m_file_assets.end())  
    { 
        std::ifstream in;

        //json array

        if (System::Utils::str_endsWith(it->second.second, ".json")) 
        {
            #if USE_JSON == 1 

                in.open(it->second.second);

                if (in.is_open()) 
                {
                    json data = json::parse(in);
                    std::stringstream ss;

                    if (data.contains("layers")) 
                        if (data["layers"][index].contains("data")) 
                        {
                            for (auto& d : data["layers"][index]["data"]) 
                            { 
                                int gid = static_cast<int>(d);

                                //tiled uses 0 indexed gids

                                if (gid == 0)
                                   gid = -1;

                               else 
                                   gid = gid - 1; 

                                ss << gid << ","; 
                            }

                            while(getline(ss, line))
                                result.emplace_back(line);
                        }
                }

            #else 
                LOG("cannot parse map data. USE_JSON flag not enabled.");
            #endif       
        }

        //plain csv file 

        else if (System::Utils::str_endsWith(it->second.second, ".csv")) 
        {
            in.open(it->second.second);
            
            if (in.is_open()) 
                while(getline(in, line))
                    result.emplace_back(line + ",");
        }
     
        if (in.is_open())
            in.close();      
    }
        
    return result; 

}


