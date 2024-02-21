
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
		
		static inline bool useEditor = false;
		static inline EventListener events;
		
		static inline Camera* camera;

        static inline std::pair<std::string, GLuint> selectedAsset;
		static inline std::string platform = "Windows";
        static inline std::string projectPath = "";
		static inline std::string rootPath;

		static inline float gravityX = 0.0f,
                            gravityY = 500.0f;

		Editor();
		~Editor();

		static void Log(const std::string &message);
		static void Reset();


	private:

		static void Update();
};



