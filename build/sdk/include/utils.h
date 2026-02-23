#pragma once

#include <string>

//debug only logging 

#ifndef LOG

   // #if DEVELOPMENT == 1 && STANDALONE == 1
        #include <iostream>
        #include <fstream>
        #define LOG(msg) \
            std::cout << msg << std::endl; \
            std::ofstream ("log.txt", std::ofstream::app | std::ofstream::out) << msg << std::endl; 
  // #else
   //    #define LOG(msg) 0;
   //#endif

#endif

namespace System {

    //common utilities
 
	class Utils {
		
		public:

			static const int randInt(int n);
            static const int intBetween(int min, int max);

            //binary to decimal - mainly used to convert csv map data, extracting tile position and rotation flags
			static const int BinToDec (const int n); 
            static const int CountDigits (long long n);

			static const float randFloat(float n);
			static const float floatBetween(float min, float max);

			static const bool str_includes(const std::string& str, const std::string& sub);
			static const bool str_endsWith(const std::string_view& str, const std::string_view& suffix);
			static const bool CoinFlip();

			static const int GetFileType(const std::string& path);
			static std::string SanitizePath(std::string& path);
            static std::string ReplaceFrom(std::string& str, const char* position, const char* replace_str);

        private:

            struct removeDoubleSlash {
                bool operator() (char a, char b) const {
                return a == '/' && b == '/';
            }};
	};

}

