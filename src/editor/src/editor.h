
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
#include <chrono>
#include <thread>
#include <algorithm>
#include <memory>

#include "../../../build/sdk/include/audio.h"
#include "../../../build/sdk/include/window.h"      
#include "../../../build/sdk/include/inputs.h"
#include "../../../build/sdk/include/time.h"
#include "../../../build/sdk/include/game.h"
#include "../../../build/sdk/include/camera.h"

#include "./events/events.h"


/****** Base Editor *****/

namespace editor {

	class Editor 
	{

		public:

			static inline int worldWidth, worldHeight;

			static inline float gravityX, gravityY, vignetteVisibility;

			static inline bool projectOpen,
                               shaders_applied,
                               globals_applied,
                               animations_applied,
							   gravity_continuous, 
							   gravity_sleeping,
                               use_pthreads,
                               shared_memory,
                               allow_memory_growth,
                               allow_exception_catching,
                               export_all,
                               wasm,
                               gl_assertions,
                               use_webgl2,
                               full_es3,
                               preserveSrc;

			static inline EventListener events;
			
			static inline System::Game* game;

            static inline std::shared_ptr<Entity> selectedEntity;
			
			static inline std::vector<std::pair<std::string, std::string>> globals;
            static inline std::vector<std::pair<std::string, std::string>> spritesheets;
            static inline std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::pair<int, int>>>>> animations;
            static inline std::vector<std::pair<std::string, std::pair<std::string, std::string>>> shaders;

			static inline std::vector<std::string> scenes;

			static inline std::string platform = "Windows",
                                      releaseType = "debug",
                                      buildType = "dynamic",
									  projectPath = "",
									  rootPath;

			Editor();
			~Editor();

			static void Log(const std::string& message);
			static void Reset();


		private:

            static inline std::shared_ptr<Geometry> s_selector;
			static void Update();
	};

}


