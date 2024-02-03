#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include "../../app/app.h"


using namespace System::Resources;



//--------------------------- clear resources


void Manager::Clear()
{

    for (const auto &shader : shaders)
        glDeleteProgram(shader.second.ID);

    for (const auto &tex : textures)
    {
       glDeleteTextures(1, &tex.second.ID);
       glBindTexture(GL_TEXTURE_2D, 0);
       glDeleteVertexArrays(1, &tex.second.quadVAO);
       glDeleteBuffers(1, &tex.second.VBO);
       glDeleteBuffers(1, &tex.second.UVBO);
    }

}


//----------------------------------- register the assets


void Manager::RegisterAssets()
{
    //load textures

    for (const auto &texture : System::Application::resources->file_image_assets)
       System::Application::resources->texture2D->Load(texture.first);

    for (const auto &texture : System::Application::resources->raw_image_assets)
       System::Application::resources->texture2D->Load(texture.first);
}


//----------------------------------- load individual files


void Manager::LoadFile(const char* key, const char* path)
{

    if (
       System::Utils::str_endsWith(path, ".png") ||
       System::Utils::str_endsWith(path, ".jpg") &&
       System::Application::resources->file_image_assets.find(key) == System::Application::resources->file_image_assets.end()
    )
        System::Application::resources->file_image_assets.insert({key, path});

    else if (
        System::Utils::str_endsWith(path, ".flac") ||
        System::Utils::str_endsWith(path, ".ogg") && 
        System::Application::resources->file_audio_assets.find(key) == System::Application::resources->file_audio_assets.end()

    )
        System::Application::resources->file_audio_assets.insert({key, path});

    else if (
        System::Utils::str_endsWith(path, ".txt") ||
        System::Utils::str_endsWith(path, ".json") ||
        System::Utils::str_endsWith(path, ".csv") &&
        System::Application::resources->file_text_assets.find(key) == System::Application::resources->file_text_assets.end()
    )
        System::Application::resources->file_text_assets.insert({key, path});

    else
        return;


    if (System::Application::resources->file_assets.find(key) == System::Application::resources->file_assets.end())
        System::Application::resources->file_assets.insert({key, path});

} 


//------------------------------------ load frames from vector of int arrays


void Manager::LoadFrames(const std::string &key, const std::vector<std::array<int, 4>>& frames) {

    if (System::Application::resources->ATLASES.find(key) == System::Application::resources->ATLASES.end())
        System::Application::resources->ATLASES.insert( { key, frames } );
 
}


//------------------------------------ load frames from file


void Manager::LoadAtlas(const std::string &key, const char* path) {

    if (System::Application::resources->ATLAS_PATH.find(key) == System::Application::resources->ATLAS_PATH.end())
        System::Application::resources->ATLAS_PATH.insert( { key, path } );
}


//------------------------------------ load animations from map of start / end pairs defined by key


void Manager::LoadAnims(const std::string &key, const std::map<std::string, std::pair<int, int>> &anims) {

    if (System::Application::resources->ANIMS.find(key) == System::Application::resources->ANIMS.end())
        System::Application::resources->ANIMS.insert( { key, anims } );
}

//------------------------------------ load tilemaps


void Manager::LoadTilemap(const std::string &key, const std::vector<std::string> &data) {
    
    if (System::Application::resources->TILEMAP.find(key) == System::Application::resources->TILEMAP.end())
        System::Application::resources->TILEMAP.insert({ key, data });
}


//--------------------------------- load raw char array image / dimensions and bytes per pixel channel


void Manager::LoadRawImage(const char* key, const char* arr, int width, int height, int channel) {

    if (System::Application::resources->IMAGE_DIMENSIONS_AND_CHANNELS.find(key) == System::Application::resources->IMAGE_DIMENSIONS_AND_CHANNELS.end())
    {
        System::Application::resources->IMAGE_DIMENSIONS_AND_CHANNELS.insert( { key, { width, height, channel } } );
        System::Application::resources->raw_image_assets.insert({ key, arr });
        System::Application::resources->raw_assets.insert({key, arr});
    }

}

//------------------------------------- load raw char array audio / size in bytes


void Manager::LoadRawAudio(const char* key, const char* arr, unsigned int bytes) 
{
    if (System::Application::resources->AUDIO_SIZE.find(key) == System::Application::resources->AUDIO_SIZE.end())
    {
        System::Application::resources->AUDIO_SIZE.insert( { key, bytes } );
        System::Application::resources->raw_audio_assets.insert({ key, arr });
        System::Application::resources->raw_assets.insert({ key, arr });
    } 

}


//------------------------------------- unload files

void Manager::UnLoadFile(const char* key)
{

    if (System::Application::resources->file_image_assets.find(key) != System::Application::resources->file_image_assets.end())
        System::Application::resources->file_image_assets.erase(System::Application::resources->file_image_assets.find(key));

    else if (System::Application::resources->file_audio_assets.find(key) != System::Application::resources->file_audio_assets.end() )
        System::Application::resources->file_audio_assets.erase(System::Application::resources->file_audio_assets.find(key));

    else if (System::Application::resources->file_text_assets.find(key) != System::Application::resources->file_text_assets.end())
        System::Application::resources->file_text_assets.erase(System::Application::resources->file_text_assets.find(key));

    else
        return;

    if (System::Application::resources->file_assets.find(key) != System::Application::resources->file_assets.end())
        System::Application::resources->file_assets.erase(System::Application::resources->file_assets.find(key));

} 


//------------------------------------ unload frames from vector of int arrays


void Manager::UnLoadFrames(const std::string &key) {

    if (System::Application::resources->ATLASES.find(key) != System::Application::resources->ATLASES.end())
        System::Application::resources->ATLASES.erase(System::Application::resources->ATLASES.find(key));
 
}


//------------------------------------ unload frames from file


void Manager::UnLoadAtlas(const std::string &key) {

    if (System::Application::resources->ATLAS_PATH.find(key) != System::Application::resources->ATLAS_PATH.end())
        System::Application::resources->ATLAS_PATH.erase(System::Application::resources->ATLAS_PATH.find(key));
}


//------------------------------------ unload animations 


void Manager::UnLoadAnims(const std::string &key) {

    if (System::Application::resources->ANIMS.find(key) != System::Application::resources->ANIMS.end())
        System::Application::resources->ANIMS.erase(System::Application::resources->ANIMS.find(key));
}

//------------------------------------ unload tilemaps


void Manager::UnLoadTilemap(const std::string &key) {
    
    if (System::Application::resources->TILEMAP.find(key) != System::Application::resources->TILEMAP.end())
        System::Application::resources->TILEMAP.erase(System::Application::resources->TILEMAP.find(key));
}


//--------------------------------- unload raw char array image


void Manager::UnLoadRawImage(const char* key) {

    if (System::Application::resources->IMAGE_DIMENSIONS_AND_CHANNELS.find(key) != System::Application::resources->IMAGE_DIMENSIONS_AND_CHANNELS.end())
    {
        System::Application::resources->IMAGE_DIMENSIONS_AND_CHANNELS.erase(System::Application::resources->IMAGE_DIMENSIONS_AND_CHANNELS.find(key));
        System::Application::resources->raw_image_assets.erase(System::Application::resources->raw_image_assets.find(key));
        System::Application::resources->raw_assets.erase(System::Application::resources->raw_assets.find(key));
    }

}

//------------------------------------- unload raw char array audio


void Manager::UnLoadRawAudio(const char* key) {

    if (System::Application::resources->AUDIO_SIZE.find(key) != System::Application::resources->AUDIO_SIZE.end())
    {
        System::Application::resources->AUDIO_SIZE.erase(System::Application::resources->AUDIO_SIZE.find(key));
        System::Application::resources->raw_audio_assets.erase(System::Application::resources->raw_audio_assets.find(key));
        System::Application::resources->raw_assets.erase(System::Application::resources->raw_assets.find(key));
    } 

}


//---------------------------- get raw data


const char* Manager::GetRawData(const std::string &key)
{

    const auto resource = System::Application::resources->raw_assets.find(key.c_str());

    return resource->second;

}


//---------------------------- get raw dimensions


const std::array<int, 3> Manager::GetRawDimensionsAndChannels(const std::string &key)
{

    std::map<std::string, std::array<int, 3>>::iterator it = System::Application::resources->IMAGE_DIMENSIONS_AND_CHANNELS.find(key);

    if (it != System::Application::resources->IMAGE_DIMENSIONS_AND_CHANNELS.end())
    {
        int width = it->second[0],
            height = it->second[1],
            alpha = it->second[2];

        return { width, height, alpha };
    }

    return {};
}


//----------------------------------- sizeof audio


const unsigned int Manager::GetSizeOfRawAudio(const std::string &key)
{

    std::map<std::string, unsigned int>::iterator it = System::Application::resources->AUDIO_SIZE.find(key);

    return it != System::Application::resources->AUDIO_SIZE.end() ?
        it->second : 0;

};


//-------------------------------- get raw atlas


std::vector<std::array<int, 4>> Manager::GetRawSpritesheetData(const std::string &key)
{

    std::map<std::string, std::vector<std::array<int, 4>>>::iterator it = System::Application::resources->ATLASES.find(key);

    if (it != System::Application::resources->ATLASES.end())
        return it->second; 
        
    return {};
}


//-------------------------------- get raw tilemap


const std::vector<std::string> Manager::GetRawTilemapData(const std::string &key)
{

    std::map<std::string, std::vector<std::string>>::iterator it = System::Application::resources->TILEMAP.find(key);

    if (it != System::Application::resources->TILEMAP.end())
        return it->second;

	return {};
}


//-------------------------------- get atlas path


const char* Manager::GetSpritesheetPath(const std::string &key)
{

    std::map<std::string, const char*>::iterator it = System::Application::resources->ATLAS_PATH.find(key);

    if (it != System::Application::resources->ATLAS_PATH.end())
        return it->second;

    else if (GetRawSpritesheetData(key).size())
        return "raw";

    return "";
}


//--------------------------------- get anims


const std::map<std::string, std::pair<int, int>> Manager::GetAnimations(const std::string &key)
{

    std::map<std::string, std::map<std::string, std::pair<int, int>>>::iterator it = System::Application::resources->ANIMS.find(key);

    if (it != System::Application::resources->ANIMS.end())
        return it->second;

    return {};

}


//--------------------------- get asset path


const char* Manager::GetFilePath(const std::string &key)
{

   std::map<std::string, std::string>::iterator it = System::Application::resources->file_assets.find(key);

    return it != System::Application::resources->file_assets.end() ?
      it->second.c_str() : "";

}


//--------------------------- parse CSV


std::vector<std::string> Manager::ParseCSV(const std::string &key)
{

    std::vector<std::string> result;
    std::string line;

    std::map<std::string, std::string>::iterator it = System::Application::resources->file_text_assets.find(key);

    if (it != System::Application::resources->file_text_assets.end()) {

        std::ifstream in(it->second);

        while(getline(in, line))
            result.push_back(line + ",");

        in.close();

    }

    return result;

}





