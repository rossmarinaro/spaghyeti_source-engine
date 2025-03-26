/******* MAIN ******/

#include "../../build/sdk/include/app.h"

#include "./src/editor.h"

#ifdef _WIN32
	#include <windows.h>
#endif

#ifdef _WIN32

	LONG UnhandledExceptionFilter(EXCEPTION_POINTERS *ExceptionInfo) {
		editor::Editor::Log("Crash Report Exception Record: " + std::to_string(ExceptionInfo->ExceptionRecord->ExceptionCode));
	}

#endif


//-------------- main


int main(int argc, char* args[])
{   

	#ifdef _WIN32

		#if DEVELOPMENT == 0
			ShowWindow(GetConsoleWindow(), SW_HIDE); 
		#endif

		SetUnhandledExceptionFilter(UnhandledExceptionFilter);
	#endif
	
	System::Application::Start();
	editor::Editor::Start();

	return 0; 
}




