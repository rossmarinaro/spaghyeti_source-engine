/******* MAIN ******/

#include "../../build/sdk/include/app.h"
#include "./src/editor.h"

#ifdef _WIN32

	#include <windows.h>

    void PrintNtStatusMessage(DWORD statusCode) 
    {
        HMODULE hNtDll = GetModuleHandleA("ntdll.dll");
        if (!hNtDll) 
            return;

        LPSTR messageBuffer = nullptr;

        DWORD size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
            hNtDll,
            statusCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&messageBuffer,
            0,
            nullptr
        );

        if (size > 0 && messageBuffer) 
        {
            std::string errorMessage = messageBuffer;
            editor::Editor::Log("Crash Report Exception: " + errorMessage);
            LocalFree(messageBuffer);
        }
        else 
            editor::Editor::Log("Crash Report Exception: Unknown Error.");
    }

	LONG UnhandledExceptionFilter(EXCEPTION_POINTERS *ExceptionInfo) {
		PrintNtStatusMessage(ExceptionInfo->ExceptionRecord->ExceptionCode);
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




