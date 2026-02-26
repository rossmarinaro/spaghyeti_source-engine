
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

			int worldWidth, worldHeight, minVersion, midVersion, maxVersion;

			float gravityX, gravityY, vignetteVisibility;

			bool projectOpen,
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
                 embed_files,
                 webgl_embed_files,
                 preserveSrc,
                 isMultiThreaded,
                 depthSort,
                 vsync;
			 
			System::Game* game;
            EventListener* events;
            std::pair<std::string, Math::Vector2> cullTarget;
			
			std::vector<std::pair<std::string, std::string>> globals;
            std::vector<std::pair<std::string, std::string>> spritesheets;
            std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::pair<int, int>>>>> animations;
            std::vector<std::pair<std::string, std::pair<std::string, std::string>>> shaders;

			std::vector<std::string> scenes;

            static void Start();
            static void Reset();
            static void ShutDown();

            static inline std::shared_ptr<Entity> selectedEntity;
    
			static inline std::string platform,
                                      releaseType,
                                      buildType,
									  projectPath,
									  rootPath;

			static void Log(const std::string& message);

            static inline Editor* Get() {
                return s_self;
            }


		private:

            std::shared_ptr<Geometry> s_selector;

            static inline Editor* s_self;
            static void Update();

	};

}


