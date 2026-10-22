#if defined(SLANG_ENABLE_MIMALLOC)

// Override global C++ allocation in slang-compiler with mimalloc.

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4559)
#endif
#include "mimalloc-new-delete.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#if defined(_WIN32) && defined(SLANG_DYNAMIC_EXPORT)
#include <windows.h>

// The mimalloc statically linked into slang-compiler.dll keeps its OS memory reserved after
// `FreeLibrary` unless `mi_option_destroy_on_exit` is set when it shuts down, so a host that
// repeatedly loads and unloads Slang would keep another arena reservation on every cycle (#13828).
// The MSVC CRT calls this `DllMain` before it runs the DLL's static destructors and `atexit`
// functions, and mimalloc registers its shutdown as the first of those `atexit` functions, so it
// releases the memory only after every static has freed its allocations.
//
// `reserved` is null when the DLL is unloaded by `FreeLibrary` (or its load failed) and non-null
// when the whole process is exiting. We leave process exit alone: the OS reclaims the memory
// anyway, and other DLLs' teardown may still free through ours at that point.
extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_DETACH && reserved == nullptr)
        mi_option_enable(mi_option_destroy_on_exit);
    return TRUE;
}
#endif

#endif
