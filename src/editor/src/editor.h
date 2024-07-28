
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

#include "../../../build/sdk/include/audio.h"
#include "../../../build/sdk/include/window.h"      
#include "../../../build/sdk/include/inputs.h"
#include "../../../build/sdk/include/utils.h" 
#include "../../../build/sdk/include/time.h"
#include "../../../build/sdk/include/game.h"
#include "../../../build/sdk/include/camera.h"

#include "./events/events.h"


/****** Base Editor *****/

namespace editor {

	class Editor 
	{

		public:

			static inline int worldWidth = 2000, 
							  worldHeight = 2000;

			static inline float gravityX = 0.0f,
                                vignetteVisibility = 0.0f,
								gravityY = 500.0f;

			static inline bool projectOpen = false,
                               globals_applied = false,
							   gravity_continuous = true, 
							   gravity_sleeping = true;

			static inline EventListener events;
			
			static inline System::Game* game;
			
			static inline std::vector<std::pair<std::string, std::string>> globals;
            static inline std::vector<std::string> spritesheets;
			static inline std::vector<std::string> scenes;

			static inline std::string selectedAsset,
									  platform = "Windows",
                                      releaseType = "debug",
                                      buildType = "dynamic",
									  projectPath = "",
									  rootPath;

			Editor();
			~Editor();

			static void Log(const std::string& message);
			static void Reset();


		private:

			static void Update();
	};

}


