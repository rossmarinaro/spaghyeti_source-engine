#pragma once

#include <functional>
#include <thread>
#include <stdlib.h>
#include <string>

#define	RAND_LIMIT 32767

// template <typename T>
// 	concept output_streamable = requires(std::ostream &stm, const T &v)
// 	{
// 		{ stm <<v }->std::same_as<std::ostream&>;
// 	}

namespace System {

	class Utils {
		
		public:

			template<typename T>
			using Scope = std::unique_ptr<T>;
			template<typename T, typename ... Args>
			constexpr Scope<T> CreateScope(Args&& ... args)
			{
				return std::make_unique<T>(std::forward<Args>(args)...);
			}

			// template<typename T>
			// using Ref = std::shared_ptr<T>;
			// template<typename T, typename ... Args>
			// constexpr Ref<T> CreateRef(Args&& ... args)
			// {
			// 	return std::make_shared<T>(std::forward<Args>(args)...);
			// }

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
			
			static bool CoinFlip(void);

			static std::string ReplaceFrom(const std::string &str, const char* position, const char* replace_str);

			static std::string GetFileType(const std::string &path);
			
			static inline bool str_includes(const std::string &str, const std::string &sub) { return str.find(sub) != std::string::npos; }

			static inline bool str_endsWith(std::string_view str, std::string_view suff) {
				return str.size() >= suff.size() && 0 == str.compare(str.size() - suff.size(), suff.size(), suff);
			}
	
	};
}
