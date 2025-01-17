#pragma once

#ifdef _WIN32

	#ifdef SPAGHYETI_CORE
	    #define SPAGHYETI_CORE __declspec(dllexport)
	#else
	    #define SPAGHYETI_CORE __declspec(dllimport)
	#endif
	
#endif

#ifndef THREAD_COUNT
    #define THREAD_COUNT 10
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <chrono>
#include <thread>
#include <algorithm>
#include <memory>

#include "./audio.h"
#include "./window.h"       
#include "./time.h"
#include "./game.h"
#include "./camera.h"
#include "./events.h"


/****** Application *****/

namespace /* SPAGHYETI_CORE */ System {

    class Application 
	{

		public:

            Application(Game* layer = nullptr, const std::string& key = "");
		    ~Application();

    		static inline std::string name;
            static inline bool isMobile;

			static inline Game* game; 
            static inline EventPool* eventPool;
			static inline Resources::Manager* resources;   

            static void Init(Game* layer);

		private:

			static void Update(void* layer);

	};
}


