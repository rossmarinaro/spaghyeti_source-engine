/******* MAIN ******/

#include "../../build/include/app.h"
#include "./src/editor.h"
#include "./src/gameLayer.h"

#ifdef _WIN32
	#include <windows.h>
#endif

using namespace System;

//----------------


#ifdef _WIN32

	LONG UnhandledExceptionFilter(EXCEPTION_POINTERS *ExceptionInfo) {
		Editor::Log("Crash Report Exception Record: " + std::to_string(ExceptionInfo->ExceptionRecord->ExceptionCode));
	}

#endif


//-------------- main


#undef main

	int main(int argc, char* args[])
	{   

		#ifdef _WIN32

			#if DEVELOPMENT == 0
				ShowWindow(GetConsoleWindow(), SW_HIDE); 
			#endif

			SetUnhandledExceptionFilter(UnhandledExceptionFilter);
		#endif
		
		GameLayer layer;
		Application app { &layer };
        Editor editor;

		return 0; 
	}



