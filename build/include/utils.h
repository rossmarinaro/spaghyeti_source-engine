#pragma once

#include <functional>
#include <thread>
#include <stdlib.h>
#include <string>
#include <sstream>

#define	RAND_LIMIT 32767


namespace System {

	class Utils {
		
		public:


			static inline int randInt(int n) { return rand() % n; }

			static inline float randFloat(float n) { return ((float)(rand()) / (float)(RAND_MAX)) * n; }

			static inline float floatBetween(float lo, float hi)
			{
				float r = (float)(rand() & (RAND_LIMIT));

				r /= RAND_LIMIT;
				r = (hi - lo) * r + lo;

				return r;
			}

			static inline int intBetween(int min, int max) { return rand() % (min - max + 1) + min; } 
			
			static inline bool str_includes(const std::string &str, const std::string &sub) { 
				return str.find(sub) != std::string::npos; 
			}

			static inline bool str_endsWith(std::string_view str, std::string_view suff) {
				return str.size() >= suff.size() && 0 == str.compare(str.size() - suff.size(), suff.size(), suff);
			}

			static inline bool CoinFlip(void) {
				return rand() % 10 >= 6;
			}

			//returns number of digits in tiletype
			static inline int CountDigits (long long n) 
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

			//binary to dec
			static inline int BinToDec (int n) 
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
			
			//replace string from specific position
			static inline std::string ReplaceFrom(const std::string &str, const char* position, const char* replace_str)
			{
				std::string::size_type position_at = str.rfind(position, str.length());

				std::string new_str = str;

				if (position_at != std::string::npos)
					return new_str.replace(position_at, str.length(), replace_str);

				return nullptr;
			}

			static inline std::string GetFileType(const std::string &path)
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

				
		};
}
