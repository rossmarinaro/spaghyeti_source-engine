#ifdef _WIN32

	#ifdef SPAGHYETI_CORE
	#  define SPAGHYETI_CORE __declspec(dllexport)
	#else
	#  define SPAGHYETI_CORE __declspec(dllimport)
	#endif
	
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <memory>

#include "./audio.h"
#include "./window.h"       
#include "./inputs.h"
#include "./utils.h"
#include "./time.h"
#include "./game.h"
#include "./camera.h"


/****** Application *****/

namespace SPAGHYETI_CORE System {


	class Application 
	{

		public:

			static inline bool isMobile = false;

			static inline Game* game;
			static inline Inputs* inputs; 

			static inline Resources::Manager* resources;

			static inline std::vector<Game*> layers;   

			static void Init();
			static void Update(Camera* camera);

			template<typename T>
			static inline const T &GetData(std::string key) { return std::any_cast<T>(data.at(key)); }

			static inline void SetData(std::string key, std::any value) { data.insert({key, value}); }
			static inline void ClearData() { data.clear(); }
		
			Application(Game* layer);
		    ~Application();

		private:

			static std::map<std::string, std::any> data;

	};
}


