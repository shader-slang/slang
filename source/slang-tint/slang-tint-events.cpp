#include "tint/tint.h"

#ifdef _MSC_VER
#include <windows.h>
#endif

#ifdef _MSC_VER

BOOL WINAPI
DllMain(
        HINSTANCE library,
        DWORD reason,
        LPVOID reserved
        )
{

    switch(reason)
    {

    case DLL_PROCESS_ATTACH:
    {
        tint::Initialize();
    }
    break;

    case DLL_PROCESS_DETACH:
    {
        tint::Shutdown();
    }
    break;

    }

    return TRUE;
}

#else

void __attribute__((constructor)) Load()
{
    tint::Initialize();
}

void __attribute__((destructor)) Unload()
{
    tint::Shutdown();
}

#endif
