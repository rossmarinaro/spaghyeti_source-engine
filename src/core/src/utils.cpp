#include <functional>

#include "../../../build/sdk/include/utils.h"
#include "../../../build/sdk/include/manager.h"

#define	RAND_LIMIT 32767


const int System::Utils::randInt(int n) { 
    return (rand() % n);
}

//---------------------------------------

const float System::Utils::randFloat(float n) { 
    return (((float)(rand()) / (float)(RAND_MAX)) * n);
}

//---------------------------------------

const float System::Utils::floatBetween(float min, float max) {
    float n = (float)(rand() & (RAND_LIMIT));

    n /= RAND_LIMIT;
    n = (max - min) * n + min;

    return n;
}

//---------------------------------------
 
const int System::Utils::intBetween(int min, int max) { 
    return rand() % (min - max + 1) + min;
} 

//---------------------------------------

const bool System::Utils::str_includes(const std::string& str, const std::string& sub) { 
    std::string sub_upper = sub;
    transform(sub_upper.begin(), sub_upper.end(), sub_upper.begin(), ::toupper); 
    
    return str.find(sub) != std::string::npos || str.find(sub_upper) != std::string::npos; 
}


//---------------------------------------


const bool System::Utils::str_endsWith(const std::string_view& str, const std::string_view& suffix) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}


//---------------------------------------


const bool System::Utils::CoinFlip() {
    return rand() % 10 >= 6;
}

//--------------------------------------- returns number of digits in tiletype

const int System::Utils::CountDigits (long long n) 
{
    if (n == 0)
        return 1;

    int count = 0;

    while (n != 0) {
        n = n / 10;
        ++count;
    }

    return count;

}

//--------------------------------------- binary to decimal 

const int System::Utils::BinToDec (const int n) 
{
    int num = n, 
        dec_val = 0,
        base = 1,
        temp = num;

    while(temp) 
    {
        int last_dig = temp % 10;
        temp = temp / 10;	
        dec_val += last_dig * base;
        base = base * 2;
    }

    return dec_val;
} 

//---------------------------------------replace string from specific position

std::string System::Utils::ReplaceFrom(std::string& str, const char* position, const char* replace_str) 
{
    std::string::size_type position_at = str.rfind(position, str.length());
    std::string new_str = str;

    if (position_at != std::string::npos)
        return new_str.replace(position_at, str.length(), replace_str);

    return new_str;
}

//---------------------------------------

std::string System::Utils::SanitizePath(std::string& path) {
    
    std::replace(path.begin(), path.end(), '\\', '/');
    path.erase(std::unique(path.begin(), path.end(), removeDoubleSlash()), path.end());

    return path;
}


//---------------------------------------


const int System::Utils::GetFileType(const std::string& path) 
{
    if (
        str_endsWith(path, ".ttf")
    ) 
        return System::Resources::Manager::TEXT; 
    
    if (
        str_endsWith(path, ".png") ||
        str_endsWith(path, ".jpg")
    ) 
        return System::Resources::Manager::IMAGE; 
    
    if (
        str_endsWith(path, ".flac") ||
        str_endsWith(path, ".ogg")
    ) 
        return System::Resources::Manager::AUDIO;
    
    if (
        str_endsWith(path, ".json") || 
        str_endsWith(path, ".csv") ||
        str_endsWith(path, ".vert") || 
        str_endsWith(path, ".frag")
    ) 
        return System::Resources::Manager::DATA;

    if (str_endsWith(path, ".ico")) 
        return System::Resources::Manager::ICON;

    return System::Resources::Manager::NOT_SUPPORTED;
} 


