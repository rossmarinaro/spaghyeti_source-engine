
#pragma once

#define SPAGHYETI_EDITOR

#ifdef SPAGHYETI_CORE
#  define SPAGHYETI_CORE __declspec(dllexport)
#else
#  define SPAGHYETI_CORE __declspec(dllimport)
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <memory>

#include "../../../build/include/audio.h"
#include "../../../build/include/window.h"      
#include "../../../build/include/inputs.h"
#include "../../../build/include/utils.h" 
#include "../../../build/include/time.h"
#include "../../../build/include/game.h"
#include "../../../build/include/camera.h"

#include "./events/events.h"


/****** Base Editor *****/

class Editor 
{

	public:

		static inline int worldWidth = 2000, 
						  worldHeight = 2000;

		static inline float gravityX = 0.0f,
						    gravityY = 500.0f;

		static inline bool globals_applied = false,
						   gravity_continuous = true,
						   gravity_sleeping = true;

		static inline EventListener events;
		
		static inline Game* game;
		
		static inline std::vector<std::pair<std::string, std::string>> globals;
        static inline std::string selectedAsset;
		static inline std::string platform = "Windows";
        static inline std::string projectPath = "";
		static inline std::string rootPath;

		Editor();
		~Editor();

		static void Log(const std::string &message);
		static void Reset();


	private:

		static void Update();
};



