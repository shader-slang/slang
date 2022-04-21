// slang-rt.h
#ifndef SLANG_RT_H
#define SLANG_RT_H

#include "../core/slang-string.h"
#include "../core/slang-smart-pointer.h"

#ifdef SLANG_RT_DYNAMIC_EXPORT
#    define SLANG_RT_API SLANG_DLL_EXPORT
#else
#    define SLANG_RT_API
#endif

extern "C"
{
    SLANG_RT_API void SLANG_MCALL _slang_rt_abort(Slang::String errorMessage);
    SLANG_RT_API void* SLANG_MCALL _slang_rt_load_dll(Slang::String modulePath);
    SLANG_RT_API void* SLANG_MCALL
        _slang_rt_load_dll_func(void* moduleHandle, Slang::String modulePath);
}

#endif
