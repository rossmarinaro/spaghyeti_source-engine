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

#include "../../../window/window.h"
 
#include "../audio/audio.h"
#include "../inputs/inputs.h"
#include "../misc/utils.h"
#include "../misc/time.h"
#include "../game/game.h"
#include "../camera/camera.h"


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
			static void Update();

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

