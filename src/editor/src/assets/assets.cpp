#include "./assets.h"
#include "../../../../build/include/app.h"



std::string AssetManager::GetFolder(const std::string &asset)
{
    std::string folder;

    if (System::Utils::GetFileType(asset) == "image") 
        folder = "\\images\\";

    if (System::Utils::GetFileType(asset) == "audio") 
        folder = "\\audio\\";

    if (System::Utils::GetFileType(asset) == "data") 
        folder = "\\data\\";

    return folder;
}


//-----------------------------


std::string AssetManager::GetThumbnail(const std::string &asset)
{
    std::string texture;

    if (System::Utils::GetFileType(asset) == "image") 
        texture = asset; 
    
    if (System::Utils::GetFileType(asset) == "audio") 
        texture = "audio src";
    
    if (System::Utils::GetFileType(asset) == "data") 
        texture = "data src";
    

    return texture;
}


