// platform.cpp
#include "platform.h"

#include "common.h"

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <Windows.h>
	#undef WIN32_LEAN_AND_MEAN
	#undef NOMINMAX
#else
	#include "slang-string.h"
	#include <dlfcn.h>
#endif

namespace Slang
{
	// SharedLibrary

/* static */SlangResult SharedLibrary::load(const char* filename, SharedLibrary::Handle& handleOut)
{
    StringBuilder builder;
    appendPlatformFileName(UnownedStringSlice(filename), builder);
    return loadWithPlatformFilename(builder.begin(), handleOut);
}

#ifdef _WIN32

/* static */SlangResult SharedLibrary::loadWithPlatformFilename(char const* platformFileName, SharedLibrary::Handle& handleOut)
{
    handleOut = nullptr;
    // https://docs.microsoft.com/en-us/windows/desktop/api/libloaderapi/nf-libloaderapi-loadlibrarya
    const HMODULE h = LoadLibraryA(platformFileName);
    if (!h)
    {
        const DWORD lastError = GetLastError();
        switch (lastError)
        {
            case ERROR_MOD_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
            case ERROR_FILE_NOT_FOUND:  
            {
                return SLANG_E_NOT_FOUND;
            }
            case ERROR_INVALID_ACCESS:
            case ERROR_ACCESS_DENIED: 
            case ERROR_INVALID_DATA:
            {
                return SLANG_E_CANNOT_OPEN;
            }
            default: break;
        }
        // Turn to Result, if not one of the well known errors
        return HRESULT_FROM_WIN32(lastError);
    }
    handleOut = (Handle)h;
    return SLANG_OK;
}

/* static */void SharedLibrary::unload(Handle handle)
{
    SLANG_ASSERT(handle);
    ::FreeLibrary((HMODULE)handle);
}

/* static */SharedLibrary::FuncPtr SharedLibrary::findFuncByName(Handle handle, char const* name)
{
    SLANG_ASSERT(handle);
    return (FuncPtr)GetProcAddress((HMODULE)handle, name);
}

/* static */void SharedLibrary::appendPlatformFileName(const UnownedStringSlice& name, StringBuilder& dst)
{
    // Windows doesn't need the extension or any prefix to work
    dst.Append(name);
}

#else // _WIN32

/* static */SlangResult SharedLibrary::loadWithPlatformFilename(char const* platformFileName, Handle& handleOut)
{
    handleOut = nullptr;

	void* h = dlopen(platformFileName, RTLD_NOW | RTLD_LOCAL);
	if(!h)
	{
		if(auto msg = dlerror())
		{
			fprintf(stderr, "error: %s\n", msg);
		}
        return SLANG_FAIL;
	}
    handleOut = (Handle)h;
    return SLANG_OK;
}

/* static */void SharedLibrary::unload(Handle handle)
{    
    SLANG_ASSERT(handle);
	dlclose(handle);
}

/* static */SharedLibrary::FuncPtr SharedLibrary::findFuncByName(Handle handle, char const* name)
{
    SLANG_ASSERT(handle);
	return (FuncPtr)dlsym((void*)handle, name);
}

/* static */void SharedLibrary::appendPlatformFileName(const UnownedStringSlice& name, StringBuilder& dst)
{
    dst.Append("lib");
    dst.Append(name);
    dst.Append(".so");
}

#endif // _WIN32

}