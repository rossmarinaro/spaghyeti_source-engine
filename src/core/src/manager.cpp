#include <sstream>
#include <fstream>
#include <type_traits>

#if USE_JSON == 1
	#include "../../vendors/nlohmann/json.hpp"
	using json = nlohmann::json;
#endif

#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/audio.h"
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

//load registered assets
void Manager::Register()
{
    for (const auto& asset : System::Application::resources->m_raw_assets) 
    {
        if (asset.second.type == IMAGE && System::Application::resources->textures.find(asset.first) == System::Application::resources->textures.end())
            Graphics::Texture2D::Load(asset.first);
            
        if (asset.second.type == AUDIO)
                Audio::Load(asset.first);
    }

    for (const auto& asset : System::Application::resources->m_file_assets) 
    {
        if (asset.second.first == IMAGE && System::Application::resources->textures.find(asset.first) == System::Application::resources->textures.end()) 
            Graphics::Texture2D::Load(asset.first);
        
        if (asset.second.first == AUDIO)
            Audio::Load(asset.first);
    }
}


//----------------------------------- 

//load individual files
void Manager::LoadFile(const std::string& key, const std::string& path) {

    if (System::Utils::GetFileType(path) == NOT_SUPPORTED) {
        LOG("Resources: failed to load " + path + " - filetype not available for loading.");
        return;
    }
    
    System::Application::resources->m_file_assets.insert({ key, { System::Utils::GetFileType(path), path } });
} 

 
//------------------------------------- 


//load raw char array / size in bytes
void Manager::LoadRaw(const int type, const std::string& key, const unsigned char* arr, const unsigned int bytes) {

    if (type != DATA && type != IMAGE && type != AUDIO && type != TEXT) {
        LOG("Resources: file " + key + " not available for loading.");
        return;
    }

    System::Application::resources->m_raw_assets.insert({ key, { type, arr, bytes } }); 
}


//------------------------------------

//load frames from vector of int arrays
void Manager::LoadFrames(const std::string& key, const std::vector<std::array<unsigned int, 6>>& frames) {
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


//------------------------------------ 


std::string Manager::LoadTilemapFromJSON(const std::string& key, const std::string& path)
{
    std::string errorMessage = "";
 
    #if USE_JSON == 1

    std::ifstream JSON(path);

    if (!JSON.good()) {
        errorMessage = "Tilemap: unable to parse, invalid JSON.";
        LOG(errorMessage);
        return errorMessage;
    }

    const json data = json::parse(JSON);

    if (!data.contains("width") || 
        !data.contains("height") || 
        !data.contains("tilewidth") || 
        !data.contains("tileheight") || 
        !data.contains("layers") ||
        !data.contains("tilesets")) {
            errorMessage = "Tilemap: cannot construct from JSON - missing params: width | height | tilewidth | tileheight | layers | tilesets.";
            LOG(errorMessage);
            return errorMessage;
        }
          
    unsigned int map_width = static_cast<unsigned int>(data["width"]),
                 map_height = static_cast<unsigned int>(data["height"]),
                 tile_width = static_cast<unsigned int>(data["tilewidth"]),
                 tile_height = static_cast<unsigned int>(data["tileheight"]);

    LoadFile(key, path); //json data relative to current project

    for (const auto& tileset : data["tilesets"])
    {
        if (!tileset.contains("columns")) {
            errorMessage = "Tilemap: Cannot load frames - JSON does not contain columns value.";
            LOG(errorMessage);
            return errorMessage;
        }

        std::string textureRelPath = static_cast<std::string>(tileset["image"]),
                    ext = Utils::GetFileExtension(textureRelPath);

        const std::string textureWithExt = static_cast<std::string>(tileset["name"]) + ext;

        if (System::Application::resources->m_atlases.find(textureWithExt) != System::Application::resources->m_atlases.end()) {
            errorMessage = "Tilemap: warning - multiple maps share the same texture " + textureWithExt + ". This may result in conflicting atlas dimensions.";
            LOG(errorMessage);
        }

        LoadTilemapFrames(textureWithExt, static_cast<int>(tileset["columns"]), map_width, map_height, tile_width, tile_height);
    } 

    LOG("Tilemap: loaded map " + key);

    #else
        LOG("Tilemap: Cannot load tilemap from JSON file. USE_JSON=0");
    #endif

    return errorMessage;
}


//--------------------------------------------------------------


void Manager::LoadTilemapFrames(
    const std::string& textureKey,
    unsigned int columns,
    unsigned int map_width,
    unsigned int map_height,
    unsigned int tile_width,
    unsigned int tile_height
) 
{
    std::vector<std::array<unsigned int, 6>> offset;

    unsigned int w = 0,  
                 h = 0; 

    for (int row = 0; row < map_height; ++row)
        for (int column = 0; column < map_width; ++column)
        {
            if (w == columns) { //columns are amouunt of frames per sprite sheet
                w = 0;
                h++; 
            }   

            if (w < map_width) {
                std::array<unsigned int, 6> off = { w, h, tile_width, tile_height, 1, 1 };
                offset.emplace_back(off); 
                w++;
            }
        }

    LoadFrames(textureKey, offset); //image texture with frame offsets
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
const std::vector<std::array<unsigned int, 6>>* Manager::GetRawSpritesheetData(const std::string& key) {
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

    //parse inputs

    const auto parseJSON = [&result, &line, index](const json& data) -> void 
    {
        std::stringstream ss;

        if (data.contains("layers")) 
            if (data["layers"][index].contains("data")) 
            {
                for (auto& d : data["layers"][index]["data"]) 
                { 
                    //check if data is base64 (not supported)

                    if (std::is_same<std::string, decltype(d)>::value) {
                        LOG("cannot parse map data. Only CSV tile format supported.");
                        break;
                    }

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
    };

    //files

    const auto f_it = System::Application::resources->m_file_assets.find(key);

    if (f_it->second.first == DATA && f_it != System::Application::resources->m_file_assets.end())  
    {
        std::ifstream in;

        if (System::Utils::str_endsWith(f_it->second.second, ".json")) 
        {
            #if USE_JSON == 1 

                in.open(f_it->second.second);

                if (in.is_open()) {
                    json data = json::parse(in);
                    parseJSON(data);
                }

            #else 
                LOG("cannot parse map data. USE_JSON flag not enabled.");
            #endif       
        }

        else if (System::Utils::str_endsWith(f_it->second.second, ".csv")) 
        {
            in.open(f_it->second.second);
            
            if (in.is_open()) 
                while(getline(in, line))
                    result.emplace_back(line + ",");
        }

        if (in.is_open())
            in.close(); 
    }

    //raw assets

    else 
    {
        const auto r_it = System::Application::resources->m_raw_assets.find(key);

        if (r_it->second.type == DATA && r_it != System::Application::resources->m_raw_assets.end()) 
        {
            #if USE_JSON == 1 
            
                if (json::accept(r_it->second.array_buffer, r_it->second.array_buffer + r_it->second.byte_length)) {
                    std::string jsonStr(reinterpret_cast<const char*>(r_it->second.array_buffer), r_it->second.byte_length);
                    json data = json::parse(jsonStr);
                    parseJSON(data);
                }
            #else 
                LOG("cannot parse map data. USE_JSON flag not enabled.");
            #endif       
        }

        else if (System::Utils::str_endsWith(f_it->second.second, ".csv")) 
        {
            LOG("cannot parse map data. raw csv is not compatible.");
        }
    }
  
    return result; 
}


