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
    System::Application::resources->m_image_dimensions_and_channels.clear();
    System::Application::resources->m_audio_size.clear();

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

        static const char data[] = { 0xff, 0xff, 0xff, 0xff };

        LoadRawImage("base", data, 1, 1, 4);

    #endif

    //load textures

    for (const auto& asset : System::Application::resources->m_raw_assets)
        if (asset.second.first == "image" && System::Application::resources->textures.find(asset.first) == System::Application::resources->textures.end())
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


//--------------------------------- 


//load raw char array image / dimensions and bytes per pixel channel
void Manager::LoadRawImage(const char* key, const char* arr, int width, int height, int channel) {
    System::Application::resources->m_image_dimensions_and_channels.insert({ key, { width, height, channel } });
    System::Application::resources->m_raw_assets.insert({ key, { "image", arr } });
}

//------------------------------------- 


//load raw char array audio / size in bytes
void Manager::LoadRawAudio(const char* key, const char* arr, unsigned int bytes) {
    System::Application::resources->m_audio_size.insert({ key, bytes }); 
    System::Application::resources->m_raw_assets.insert({ key, { "audio", arr } }); 
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


//--------------------------------- 

//unload raw char array image
void Manager::UnLoadRawImage(const char* key) 
{

    if (System::Application::resources->m_image_dimensions_and_channels.find(key) != System::Application::resources->m_image_dimensions_and_channels.end())
    {
        System::Application::resources->m_image_dimensions_and_channels.erase(System::Application::resources->m_image_dimensions_and_channels.find(key));

        auto it = System::Application::resources->m_raw_assets.find(key); 

        if (it->second.first == "image")
            System::Application::resources->m_raw_assets.erase(System::Application::resources->m_raw_assets.find(key));
    } 

}

//------------------------------------- 

//unload raw char array audio
void Manager::UnLoadRawAudio(const char* key) 
{

    if (System::Application::resources->m_audio_size.find(key) != System::Application::resources->m_audio_size.end())
    {
        System::Application::resources->m_audio_size.erase(System::Application::resources->m_audio_size.find(key));
       
        auto it = System::Application::resources->m_raw_assets.find(key); 

        if (it->second.first == "audio")
            System::Application::resources->m_raw_assets.erase(System::Application::resources->m_raw_assets.find(key));
    } 

}


//----------------------------

//get raw data
const char* Manager::GetRawData(const std::string& key) 
{

    const auto resource = System::Application::resources->m_raw_assets.find(key.c_str());

    if (resource != System::Application::resources->m_raw_assets.end())
        return resource->second.second;

    return "not found";

}


//---------------------------- 

//get raw dimensions
const std::array<int, 3> Manager::GetRawDimensionsAndChannels(const std::string& key)
{

    const std::map<std::string, std::array<int, 3>>::iterator it = System::Application::resources->m_image_dimensions_and_channels.find(key);

    if (it != System::Application::resources->m_image_dimensions_and_channels.end())
    {
        int width = it->second[0],
            height = it->second[1],
            alpha = it->second[2];

        return { width, height, alpha };
    }

    return {};
}


//----------------------------------- 

//sizeof audio
const unsigned int Manager::GetSizeOfRawAudio(const std::string& key)
{

    const std::map<std::string, unsigned int>::iterator it = System::Application::resources->m_audio_size.find(key);

    return it != System::Application::resources->m_audio_size.end() ?
        it->second : 0;

};


//-------------------------------- 

//get raw atlas
std::vector<std::array<int, 6>> Manager::GetRawSpritesheetData(const std::string& key) 
{

    const std::map<std::string, std::vector<std::array<int, 6>>>::iterator it = System::Application::resources->m_atlases.find(key);
 
    if (it != System::Application::resources->m_atlases.end())
        return it->second; 
        
    return {};
}



//--------------------------------

//get atlas path
const char* Manager::GetSpritesheetPath(const std::string& key)
{

    const std::map<std::string, const char*>::iterator it = System::Application::resources->m_atlas_paths.find(key);

    if (it != System::Application::resources->m_atlas_paths.end())
        return it->second;

    else if (GetRawSpritesheetData(key).size())
        return "raw";

    return "not found";
}


//--------------------------------- 

//get anims by sprite key
const std::map<std::string, std::pair<int, int>> Manager::GetAnimations(const std::string& key)
{

    const std::map<std::string, std::map<std::string, std::pair<int, int>>>::iterator it = System::Application::resources->m_anims.find(key);

    if (it != System::Application::resources->m_anims.end())
        return it->second;

    return {};

}


//--------------------------- 

//get asset path
const char* Manager::GetFilePath(const std::string& key)
{

   const auto it = System::Application::resources->m_file_assets.find(key);

    return it != System::Application::resources->m_file_assets.end() ?
        it->second.second.c_str() : "not found";

}


//--------------------------- 

//parse CSV
std::vector<std::string> Manager::ParseCSV(const std::string& key, int index)
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


