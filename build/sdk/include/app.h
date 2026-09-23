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

    struct Display {
        static inline bool isFullscreen; 
        static inline int screenWidth, 
                          screenHeight, 
                          resolutionWidth = 1480,
                          resolutionHeight = 860;
    };

    class Application 
	{
		public:

            static inline std::string name;

            static void Init(Game* layer);
            
            static void Start(
                int screenWidth, 
                int screenHeight, 
                int resolutionWidth,
                int resolutionHeight,
                Game* layer = nullptr, 
                const std::string& key = "", 
                bool isMultithreaded = false, 
                bool isMobileSupported = false,
                bool isFullScreen = false,
                int vsync = 0
            );

            static void ShutDown();

			static inline Game* game; 
            static inline Events* events;  
			static inline Resources::Manager* resources;   
            
		private:

			static void Update(void* layer);
	};
}


