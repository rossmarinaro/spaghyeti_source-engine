#include "./assets.h"
#include "../../../../build/include/app.h"



std::string AssetManager::GetFolder(const std::string &asset)
{
    std::string folder;

    if (GetType(asset) == "image") 
        folder = "\\images\\";

    if (GetType(asset) == "audio") 
        folder = "\\audio\\";

    if (GetType(asset) == "data") 
        folder = "\\data\\";

    return folder;
}


//-----------------------------


std::string AssetManager::GetThumbnail(const std::string &asset)
{
    std::string texture;

    if (GetType(asset) == "image") 
        texture = asset; 
    
    if (GetType(asset) == "audio") 
        texture = "audio src";
    
    if (GetType(asset) == "data") 
        texture = "data src";
    

    return texture;
}


//-----------------------------


std::string AssetManager::GetType(const std::string &asset)
{

    if (System::Utils::str_endsWith(asset, ".png")) 
        return "image"; 
    
    if (System::Utils::str_endsWith(asset, ".flac")) 
        return "audio";
    
    if (System::Utils::str_endsWith(asset, ".csv")) 
        return "data";

    return "";
}