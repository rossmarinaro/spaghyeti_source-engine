#include "../../../../build/include/utils.h"
#include "../../../../build/include/app.h"


bool System::Utils::CoinFlip(void)
{

    srand(time(NULL)); //reset rand
    float randNum = rand() % 10;
    return randNum >= 6 ? true : false;
}

 
//---------------------------------- replace string from specific position


std::string System::Utils::ReplaceFrom(const std::string &str, const char* position, const char* replace_str)
{
    std::string::size_type position_at = str.rfind(position, str.length());

    std::string new_str = str;

    if (position_at != std::string::npos)
        return new_str.replace(position_at, str.length(), replace_str);

    return nullptr;
}

//---------------------------------------

std::string System::Utils::GetFileType(const std::string &path)
{

    if (
        str_endsWith(path, ".png") ||
        str_endsWith(path, ".jpg")
    ) 
        return "image"; 
    
    if (
        str_endsWith(path, ".flac") ||
        str_endsWith(path, ".ogg")
    ) 
        return "audio";
    
    if (
        str_endsWith(path, ".txt") ||
        str_endsWith(path, ".json") || 
        str_endsWith(path, ".csv")
    ) 
        return "data";

    return "";
}
