#include <sstream>
#include <fstream>

#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/manager.h"

using namespace System::Resources;


//clear resources and assets, "all" flag true removes textures and shaders
void Manager::Clear(bool all)
{
    
    System::Application::resources->atlases.clear();
    System::Application::resources->atlas_paths.clear();
    System::Application::resources->anims.clear();
    System::Application::resources->image_dimensions_and_channels.clear();
    System::Application::resources->audio_size.clear();

    if (all)
    {
        System::Application::resources->m_file_audio_assets.clear();
        System::Application::resources->m_file_text_assets.clear();
        System::Application::resources->m_file_assets.clear();
        System::Application::resources->m_file_image_assets.clear();
        
        System::Application::resources->m_raw_image_assets.clear();
        System::Application::resources->m_raw_audio_assets.clear();
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

    for (const auto& texture : System::Application::resources->m_file_image_assets)
        if (System::Application::resources->textures.find(texture.first) == System::Application::resources->textures.end())
            Graphics::Texture2D::Load(texture.first);

    for (const auto& texture : System::Application::resources->m_raw_image_assets)
        if (System::Application::resources->textures.find(texture.first) == System::Application::resources->textures.end())
            Graphics::Texture2D::Load(texture.first);

    LOG("Resources: assets registered.");

}


//----------------------------------- 

//load individual files
void Manager::LoadFile(const char* key, const char* path)
{

    if (System::Utils::GetFileType(path) == "image")
        System::Application::resources->m_file_image_assets.insert({ key, path });

    else if (System::Utils::GetFileType(path) == "audio")
        System::Application::resources->m_file_audio_assets.insert({ key, path });

    else if (System::Utils::GetFileType(path) == "data")
        System::Application::resources->m_file_text_assets.insert({ key, path });

    else {
        LOG("Resources: filetype not available for loading.");
        return;
    }
    
    System::Application::resources->m_file_assets.insert({ key, path });

} 


//------------------------------------

//load frames from vector of int arrays
void Manager::LoadFrames(const std::string& key, const std::vector<std::array<int, 6>>& frames) {
    System::Application::resources->atlases.insert( { key, frames } );
}


//------------------------------------ 


//load frames from file
void Manager::LoadAtlas(const std::string& key, const char* path) {
    System::Application::resources->atlas_paths.insert( { key, path } );
}


//------------------------------------ 


//load animations from map of start / end pairs defined by key
void Manager::LoadAnims(const std::string& key, const std::map<std::string, std::pair<int, int>>& anims) {
    System::Application::resources->anims.insert( { key, anims } );
}


//--------------------------------- 


//load raw char array image / dimensions and bytes per pixel channel
void Manager::LoadRawImage(const char* key, const char* arr, int width, int height, int channel) 
{
    System::Application::resources->image_dimensions_and_channels.insert({ key, { width, height, channel } });

    System::Application::resources->m_raw_image_assets.insert({ key, arr });

    System::Application::resources->m_raw_assets.insert({ key, arr });
}

//------------------------------------- 


//load raw char array audio / size in bytes
void Manager::LoadRawAudio(const char* key, const char* arr, unsigned int bytes) 
{

    System::Application::resources->audio_size.insert({ key, bytes }); 

    System::Application::resources->m_raw_audio_assets.insert({ key, arr });

    System::Application::resources->m_raw_assets.insert({ key, arr }); 

}


//------------------------------------- 

//unload files
void Manager::UnLoadFile(const char* key)
{

    if (System::Application::resources->m_file_image_assets.find(key) != System::Application::resources->m_file_image_assets.end())
        System::Application::resources->m_file_image_assets.erase(System::Application::resources->m_file_image_assets.find(key));

    else if (System::Application::resources->m_file_audio_assets.find(key) != System::Application::resources->m_file_audio_assets.end() )
        System::Application::resources->m_file_audio_assets.erase(System::Application::resources->m_file_audio_assets.find(key));

    else if (System::Application::resources->m_file_text_assets.find(key) != System::Application::resources->m_file_text_assets.end())
        System::Application::resources->m_file_text_assets.erase(System::Application::resources->m_file_text_assets.find(key));

    else
        return;

    if (System::Application::resources->m_file_assets.find(key) != System::Application::resources->m_file_assets.end())
        System::Application::resources->m_file_assets.erase(System::Application::resources->m_file_assets.find(key));

} 


//------------------------------------

// unload frames from vector of int arrays
void Manager::UnLoadFrames(const std::string& key) {

    if (System::Application::resources->atlases.find(key) != System::Application::resources->atlases.end())
        System::Application::resources->atlases.erase(System::Application::resources->atlases.find(key));
 
}


//------------------------------------ 

//unload frames from file
void Manager::UnLoadAtlas(const std::string& key) {

    if (System::Application::resources->atlas_paths.find(key) != System::Application::resources->atlas_paths.end())
        System::Application::resources->atlas_paths.erase(System::Application::resources->atlas_paths.find(key));
}


//------------------------------------ 

//unload animations 
void Manager::UnLoadAnims(const std::string& key) {

    if (System::Application::resources->anims.find(key) != System::Application::resources->anims.end())
        System::Application::resources->anims.erase(System::Application::resources->anims.find(key));
}


//--------------------------------- 

//unload raw char array image
void Manager::UnLoadRawImage(const char* key) {

    if (System::Application::resources->image_dimensions_and_channels.find(key) != System::Application::resources->image_dimensions_and_channels.end())
    {
        System::Application::resources->image_dimensions_and_channels.erase(System::Application::resources->image_dimensions_and_channels.find(key));
        System::Application::resources->m_raw_image_assets.erase(System::Application::resources->m_raw_image_assets.find(key));
        System::Application::resources->m_raw_assets.erase(System::Application::resources->m_raw_assets.find(key));
    }

}

//------------------------------------- 

//unload raw char array audio
void Manager::UnLoadRawAudio(const char* key) 
{

    if (System::Application::resources->audio_size.find(key) != System::Application::resources->audio_size.end())
    {
        System::Application::resources->audio_size.erase(System::Application::resources->audio_size.find(key));
        System::Application::resources->m_raw_audio_assets.erase(System::Application::resources->m_raw_audio_assets.find(key));
        System::Application::resources->m_raw_assets.erase(System::Application::resources->m_raw_assets.find(key));
    } 

}


//----------------------------

//get raw data
const char* Manager::GetRawData(const std::string& key) 
{

    const auto resource = System::Application::resources->m_raw_assets.find(key.c_str());

    if (resource != System::Application::resources->m_raw_assets.end())
        return resource->second;

    return "not found";

}


//---------------------------- 

//get raw dimensions
const std::array<int, 3> Manager::GetRawDimensionsAndChannels(const std::string& key)
{

    const std::map<std::string, std::array<int, 3>>::iterator it = System::Application::resources->image_dimensions_and_channels.find(key);

    if (it != System::Application::resources->image_dimensions_and_channels.end())
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

    const std::map<std::string, unsigned int>::iterator it = System::Application::resources->audio_size.find(key);

    return it != System::Application::resources->audio_size.end() ?
        it->second : 0;

};


//-------------------------------- 

//get raw atlas
std::vector<std::array<int, 6>> Manager::GetRawSpritesheetData(const std::string& key) 
{

    const std::map<std::string, std::vector<std::array<int, 6>>>::iterator it = System::Application::resources->atlases.find(key);
 
    if (it != System::Application::resources->atlases.end())
        return it->second; 
        
    return {};
}



//--------------------------------

//get atlas path
const char* Manager::GetSpritesheetPath(const std::string& key)
{

    const std::map<std::string, const char*>::iterator it = System::Application::resources->atlas_paths.find(key);

    if (it != System::Application::resources->atlas_paths.end())
        return it->second;

    else if (GetRawSpritesheetData(key).size())
        return "raw";

    return "not found";
}


//--------------------------------- 

//get anims by sprite key
const std::map<std::string, std::pair<int, int>> Manager::GetAnimations(const std::string& key)
{

    const std::map<std::string, std::map<std::string, std::pair<int, int>>>::iterator it = System::Application::resources->anims.find(key);

    if (it != System::Application::resources->anims.end())
        return it->second;

    return {};

}


//--------------------------- 

//get asset path
const char* Manager::GetFilePath(const std::string& key)
{

   const std::map<std::string, std::string>::iterator it = System::Application::resources->m_file_assets.find(key);

    return it != System::Application::resources->m_file_assets.end() ?
        it->second.c_str() : "not found";

}


//--------------------------- 

//parse CSV
std::vector<std::string> Manager::ParseCSV(const std::string& key, int index)
{

    std::vector<std::string> result;
    std::string line;

    const std::map<std::string, std::string>::iterator it = System::Application::resources->m_file_text_assets.find(key);
 
    if (it != System::Application::resources->m_file_text_assets.end()) 
    { 

        std::ifstream in(it->second);

        if (System::Utils::str_endsWith(it->second, ".json")) //json array
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
                                result.push_back(line);
                            
                        }
            
            #endif
                        
        }

        else if (System::Utils::str_endsWith(it->second, ".csv")) //plain csv file
            while(getline(in, line))
                result.push_back(line + ",");
     
        in.close();      
    }
        

    return result; 

}


