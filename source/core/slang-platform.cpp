// slang-platform.cpp

#define _CRT_SECURE_NO_WARNINGS

#include "slang-platform.h"

#include "slang-common.h"
#include "slang-io.h"

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

/* static */SlangResult SharedLibrary::load(const char* path, SharedLibrary::Handle& handleOut)
{
    StringBuilder builder;
    calcPlatformPath(UnownedStringSlice(path), builder);
    return loadWithPlatformPath(builder.begin(), handleOut);
}

/* static */void SharedLibrary::calcPlatformPath(const UnownedStringSlice& path, StringBuilder& outPath)
{
    // Work out the shared library name
    String parent = Path::getParentDirectory(path);
    String filename = Path::getFileName(path);

    if (parent.getLength() > 0)
    {
        // Work out the filename platform name (as in add .dll say on windows)
        StringBuilder platformFileNameBuilder;
        SharedLibrary::appendPlatformFileName(filename.getUnownedSlice(), platformFileNameBuilder);

        Path::combineIntoBuilder(parent.getUnownedSlice(), platformFileNameBuilder.getUnownedSlice(), outPath);
    }
    else
    {     
        appendPlatformFileName(filename.getUnownedSlice(), outPath);
    }
}

/* static */String SharedLibrary::calcPlatformPath(const UnownedStringSlice& path)
{
    StringBuilder builder;
    calcPlatformPath(path, builder);
    return builder.ToString();
}

#ifdef _WIN32

// Make sure SlangResult match for common standard window HRESULT
SLANG_COMPILE_TIME_ASSERT(E_FAIL == SLANG_FAIL);
SLANG_COMPILE_TIME_ASSERT(E_NOINTERFACE == SLANG_E_NO_INTERFACE);
SLANG_COMPILE_TIME_ASSERT(E_HANDLE == SLANG_E_INVALID_HANDLE);
SLANG_COMPILE_TIME_ASSERT(E_NOTIMPL == SLANG_E_NOT_IMPLEMENTED);
SLANG_COMPILE_TIME_ASSERT(E_INVALIDARG == SLANG_E_INVALID_ARG);
SLANG_COMPILE_TIME_ASSERT(E_OUTOFMEMORY == SLANG_E_OUT_OF_MEMORY);

/* static */SlangResult PlatformUtil::appendResult(SlangResult res, StringBuilder& builderOut)
{
    if (SLANG_FAILED(res) && res != SLANG_FAIL)
    {
        LPWSTR buffer = nullptr;
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            nullptr,
            res,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            (LPWSTR)&buffer,
            0,
            nullptr);

        if (buffer)
        {
            builderOut << " ";
            // Convert to string
            builderOut.Append(String::fromWString(buffer));
            LocalFree(buffer);
            return SLANG_OK;
        }
    }
    return SLANG_FAIL;
}

/* static */SlangResult SharedLibrary::loadWithPlatformPath(char const* platformFileName, SharedLibrary::Handle& handleOut)
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
    dst.Append(name);
    dst.Append(".dll");
}

#else // _WIN32

/* static */SlangResult PlatformUtil::appendResult(SlangResult res, StringBuilder& builderOut)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

/* static */SlangResult SharedLibrary::loadWithPlatformPath(char const* platformFileName, Handle& handleOut)
{
    handleOut = nullptr;

	void* h = dlopen(platformFileName, RTLD_NOW | RTLD_LOCAL);
	if(!h)
	{
#if 0
        // We can't output the error message here, because it will cause output when testing what code gen is available
		if(auto msg = dlerror())
		{
			fprintf(stderr, "error: %s\n", msg);
		}
#endif
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
#if __CYGWIN__
    dst.Append(name);
    dst.Append(".dll");
#elif SLANG_APPLE_FAMILY
    dst.Append("lib");
    dst.Append(name);
    dst.Append(".dylib");
#elif SLANG_LINUX_FAMILY
    dst.Append("lib");
    dst.Append(name);
    dst.Append(".so");
#else
    // Just guess we can do with the name on it's own
    dst.Append(name);
#endif
}

#endif // _WIN32


/* static */SlangResult PlatformUtil::getEnvironmentVariable(const UnownedStringSlice& name, StringBuilder& out)
{
    const char* value = getenv(String(name).getBuffer());
    if (value)
    {
        out.append(value);
        return SLANG_OK;
    }
    return SLANG_E_NOT_FOUND;
}

/* static */PlatformKind PlatformUtil::getPlatformKind()
{
#if SLANG_WINRT
    return PlatformKind::WinRT;
#elif SLANG_XBOXONE
    return PlatformKind::XBoxOne;
#elif SLANG_WIN64
    return PlatformKind::Win64;
#elif SLANG_X360
    return PlatformKind::X360;
#elif SLANG_WIN32
    return PlatformKind::Win32;
#elif SLANG_ANDROID
    return PlatformKind::Android;
#elif SLANG_LINUX
    return PlatformKind::Linux;
#elif SLANG_IOS
    return PlatformKind::IOS;
#elif SLANG_OSX
    return PlatformKind::OSX;
#elif SLANG_PS3
    return PlatformKind::PS3;
#elif SLANG_SLANG_PS4
    return PlatformKind::PS4;
#elif SLANG_PSP2
    return PlatformKind::PSP2;
#elif SLANG_WIIU
    return PlatformKind::WIIU;
#else
    return PlatformKind::Unknown;
#endif
}

static const PlatformFlags s_familyFlags[int(PlatformFamily::CountOf)] =
{
    0,                                                                                                                  // Unknown
    PlatformFlag::WinRT | PlatformFlag::Win32 | PlatformFlag::Win64,                                                    // Windows
    PlatformFlag::WinRT | PlatformFlag::Win32 | PlatformFlag::Win64 | PlatformFlag::X360 | PlatformFlag::XBoxOne,       // Microsoft
    PlatformFlag::Linux | PlatformFlag::Android,                                                                        // Linux
    PlatformFlag::IOS | PlatformFlag::OSX,                                                                              // Apple
    PlatformFlag::Linux | PlatformFlag::Android | PlatformFlag::IOS | PlatformFlag::OSX,                                // Unix
};

/* static */PlatformFlags PlatformUtil::getPlatformFlags(PlatformFamily family)
{
    return s_familyFlags[int(family)];
}

}
