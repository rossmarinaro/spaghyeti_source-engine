#pragma once

#include <functional>
#include <string>

#if DEVELOPMENT == 1
    #include <iostream>
    #include <fstream>
#endif


namespace System {

    #define	RAND_LIMIT 32767

    //debug only logging

    #ifndef LOG

        #if DEVELOPMENT == 1 && STANDALONE == 1
            #define LOG(msg) \
                std::cout << msg << std::endl; \
                std::ofstream ("log.txt", std::ofstream::app | std::ofstream::out) << msg << std::endl; 
        #else
            #define LOG(msg)
        #endif
 
    #endif

    //common utilities
 
	class Utils {
		
		public:

			static const int randInt(int n);
            static const int intBetween(int min, int max);
            static const int CountDigits (long long n);
			static const int BinToDec (int n);

			static const float randFloat(float n);
			static const float floatBetween(float min, float max);

			static const bool str_includes(const std::string& str, const std::string& sub);
			static const bool str_endsWith(const std::string_view& str, const std::string_view& suffix);
			static const bool CoinFlip(void);

			static std::string ReplaceFrom(const std::string& str, const char* position, const char* replace_str);
			static const std::string GetFileType(const std::string& path);
			static const std::string SanitizePath(std::string path);

        private:

            struct removeDoubleSlash {
                bool operator() (char a, char b) const {
                return a == '/' && b == '/';
            }};
	};

}

