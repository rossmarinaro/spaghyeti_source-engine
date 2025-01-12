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

			static int randInt(int n);
            static int intBetween(int min, int max);
            static int CountDigits (long long n);
			static int BinToDec (int n);

			static float randFloat(float n);
			static float floatBetween(float min, float max);

			static bool str_includes(const std::string& str, const std::string& sub);
			static bool str_endsWith(const std::string_view& str, const std::string_view& suffix);
			static bool CoinFlip(void);

			static std::string ReplaceFrom(const std::string& str, const char* position, const char* replace_str);
			static std::string GetFileType(const std::string& path);
				
	};

}

