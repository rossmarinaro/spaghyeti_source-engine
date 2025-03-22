#pragma once

#ifdef _WIN32

	#ifdef SPAGHYETI_CORE
	    #define SPAGHYETI_CORE __declspec(dllexport)
	#else
	    #define SPAGHYETI_CORE __declspec(dllimport)
	#endif
	
#endif

#ifndef THREAD_COUNT
    #define THREAD_COUNT 69
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <chrono>
#include <thread>
#include <algorithm>
#include <memory>

#include "./game.h"
#include "./events.h"


/****** Application *****/

namespace /* SPAGHYETI_CORE */ System {

    class Application 
	{

		public:

            static void Init(Game* layer);
            static void Start(Game* layer = nullptr, const std::string& key = "");
            static void ShutDown();

            static inline std::string name;
            static inline bool isMobile = false, isMultiThreaded = false;

			static inline Game* game; 
            static inline EventPool* eventPool;
			static inline Resources::Manager* resources;   

		private:

			static void Update(void* layer);

	};
}


