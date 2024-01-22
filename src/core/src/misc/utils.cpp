#include "./utils.h"
#include "../app/app.h"


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
