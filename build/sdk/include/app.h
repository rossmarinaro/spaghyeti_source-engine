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

#if DEVELOPMENT == 1
    #include <iostream>
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
#include "./utils.h"
#include "./time.h"
#include "./game.h"
#include "./camera.h"
#include "./events.h"

/****** Application *****/

namespace /* SPAGHYETI_CORE */ System {


	class Application 
	{

		public:

    		static inline std::string name = "";
            
			static inline bool isMobile = false;

			static inline Game* game; 
            static inline EventPool* eventPool;
			static inline Resources::Manager* resources;   

			static void Init(Game* layer);

			template<typename T>
			static inline const T &GetData(std::string key) { return std::any_cast<T>(data.at(key)); }

			static inline void SetData(std::string key, std::any value) { data.insert({key, value}); }
			static inline void ClearData() { data.clear(); }
		
			Application(Game* layer = nullptr, const std::string& key = "");
		    ~Application();

		private:

			static std::map<std::string, std::any> data;

			static void Update(void* layer);

	};
}


