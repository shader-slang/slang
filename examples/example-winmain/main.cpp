#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "../stacktrace-windows/common.h"

extern int exampleMain(int argc, char** argv);
extern const char *const g_logFileName;

int WinMain(
            HINSTANCE /* instance */,
            HINSTANCE /* prevInstance */,
            LPSTR /* commandLine */,
            int /*showCommand*/)

{
    __try
    {
        // Redirect stdout to a file
        freopen(g_logFileName, "w", stdout);

        int argc = 0;
        char** argv = nullptr;
        return exampleMain(argc, argv);
    }
    __except (exceptionFilter(GetExceptionInformation()))
    {
        ::exit(1);
    }
}
