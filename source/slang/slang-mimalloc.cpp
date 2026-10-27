#if defined(SLANG_ENABLE_MIMALLOC)

// Override global C++ allocation in slang-compiler with mimalloc.
//
// This file includes only mimalloc headers. It is compiled with /Zc:__cplusplus on MSVC, so a Slang
// header such as slang-common.h would expand differently here than in the rest of slang-compiler.
//
// In the shared MSVC build this file also holds slang-compiler.dll's `DllMain`, which releases
// mimalloc's memory on unload.

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4559)
#endif
#include "mimalloc-new-delete.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

// We repeat the condition that mimalloc-new-delete.h places around its aligned overloads, so that a
// build in which over-aligned allocations would silently bypass mimalloc fails instead. A change to
// that condition in a later mimalloc has to be mirrored here.
#if !(__cplusplus > 201402L && defined(__cpp_aligned_new))
#error "aligned new/delete need __cplusplus > 201402L and __cpp_aligned_new; MSVC: /Zc:__cplusplus"
#endif

// `SLANG_DYNAMIC_EXPORT` is defined only while Slang is compiled into a shared library. In a
// static build this object is linked into the host's own module, where a `DllMain` would collide
// with or replace the host's.
#if defined(_MSC_VER) && defined(SLANG_DYNAMIC_EXPORT)
#include <windows.h>

// This hook, and its copy in source/slang-glslang/slang-glslang.cpp, release the OS memory of the
// mimalloc that is statically linked into the DLL when the DLL is unloaded by `FreeLibrary`.
// Without it, mimalloc keeps its arena reservations, and the pages of threads that are still
// running, after the DLL is gone, so a host that repeatedly loads and unloads Slang grows on every
// cycle (#13828).
//
// The hook only sets `mi_option_destroy_on_exit`; mimalloc v2.1.7's `mi_process_done` reads it
// and then frees the OS memory of every arena it owns (external/mimalloc/src/init.c,
// `mi_process_done`). That happens late
// enough because of two facts we rely on:
// - On `DLL_PROCESS_DETACH`, the MSVC CRT's `_DllMainCRTStartup` calls this `DllMain` before it
//   runs the DLL's static destructors and `atexit` functions.
// - mimalloc's static MSVC build registers `atexit(&mi_process_done)` from a `.CRT$XIU` C
//   initializer, which runs before any C++ static is constructed, so `mi_process_done` is the
//   last `atexit` function to run, after every static of this DLL has freed its allocations.
// Both are specific to the MSVC CRT and to this mimalloc version, which is why the hooks are
// limited to `_MSC_VER` and must be re-checked when mimalloc is upgraded. A MinGW build is left as
// before: there mimalloc registers `mi_process_done` from an `__attribute__((constructor))`, which
// is not ordered before the C++ static constructors, so a static destructor could run after the
// arenas were freed.
//
// Destroying the arenas also destroys every block still allocated in them, including pages of
// other live threads and buffers a host got from this DLL and has not freed. Those are already
// unusable once the DLL's code is unmapped, so a host must release everything it got from the DLL
// before unloading it.
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
