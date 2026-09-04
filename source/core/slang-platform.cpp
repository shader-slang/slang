// slang-platform.cpp

#define _CRT_SECURE_NO_WARNINGS

#include "slang-platform.h"

#include "slang-common.h"
#include "slang-io.h"

#ifdef _WIN32
#include <windows.h>
#else
#include "slang-string.h"

#include <dlfcn.h>
#endif

#if SLANG_HAS_BACKTRACE
#include <execinfo.h>
#endif

#if SLANG_LINUX_FAMILY
#include <limits.h>
#include <unistd.h>
#endif

namespace Slang
{
// SharedLibrary

/* static */ SlangResult SharedLibrary::load(const char* path, SharedLibrary::Handle& handleOut)
{
    StringBuilder builder;
    calcPlatformPath(UnownedStringSlice(path), builder);
    return loadWithPlatformPath(builder.begin(), handleOut);
}

/* static */ void SharedLibrary::calcPlatformPath(
    const UnownedStringSlice& path,
    StringBuilder& outPath)
{
    // Work out the shared library name
    String parent = Path::getParentDirectory(path);
    String filename = Path::getFileName(path);

    if (parent.getLength() > 0)
    {
        // Work out the filename platform name (as in add .dll say on windows)
        StringBuilder platformFileNameBuilder;
        SharedLibrary::appendPlatformFileName(filename.getUnownedSlice(), platformFileNameBuilder);

        Path::combineIntoBuilder(
            parent.getUnownedSlice(),
            platformFileNameBuilder.getUnownedSlice(),
            outPath);
    }
    else if (filename.getLength() > 0)
    {
        appendPlatformFileName(filename.getUnownedSlice(), outPath);
    }
}

/* static */ String SharedLibrary::calcPlatformPath(const UnownedStringSlice& path)
{
    StringBuilder builder;
    calcPlatformPath(path, builder);
    return builder.toString();
}

/// The libraries that must never be unmapped, and why each one is here. See the declaration in
/// `slang-platform.h` for what this predicate is for.
///
/// * `slang-llvm` statically links two allocator runtimes that install a thread-exit destructor:
///   LLVM's vendored rpmalloc, which the official LLVM Windows packages enable and ship inside
///   `LLVMSupport.lib`, and Slang's own mimalloc. On Windows both register their destructor with
///   `FlsAlloc`, and Windows calls FLS destructors from `ntdll!RtlpFlsDataCleanup` inside
///   `LdrShutdownProcess` -- that is, during process exit, after every module has already been
///   unloaded. Neither allocator frees its FLS index when the module goes away, so once
///   `slang-llvm.dll` has been unloaded, process exit jumps into the hole it left behind and
///   raises an execute access violation (issue #12292). There is no earlier point at which
///   unloading would be safe, because the destructor is required to remain callable until the very
///   end of process shutdown.
/// * `libdxcompiler` invokes undefined behaviour on `dlclose`, see
///   https://github.com/microsoft/DirectXShaderCompiler/issues/5119.
/// * `libdxvk_d3d11` and `libdxvk_dxgi` break GDB when closed, see
///   https://github.com/doitsujin/dxvk/issues/3330.
/* static */ bool SharedLibrary::isUnclosable(const UnownedStringSlice& platformPath)
{
    // These are *platform* file name prefixes, not unadorned library names, so an entry that does
    // not match how a platform names its libraries simply never fires there. That is what scopes
    // the `libdxcompiler` and `libdxvk_*` entries to the POSIX platforms they were added for, and
    // it is why slang-llvm needs one entry per naming convention.
    static const char* const unclosableLibNames[] = {
        "slang-llvm",    // Windows
        "libslang-llvm", // POSIX
        "libdxcompiler",
        "libdxvk_d3d11",
        "libdxvk_dxgi",
    };

    // Compare against the file name so that a library is recognized whether it was loaded by bare
    // name or through a path, and compare by prefix so that the extension and any version suffix
    // (`.dll`, `.dylib`, `.so.3.7`, ...) do not have to be spelled out here.
    //
    // The comparison follows the file system's own notion of identity: on Windows
    // `SLANG-LLVM.DLL` names the very same file as `slang-llvm.dll`, so a case-sensitive
    // comparison there could let the module be unmapped after all, whereas on the POSIX
    // platforms those are two different files.
    const String fileName = Path::getFileName(platformPath);
    for (auto name : unclosableLibNames)
    {
#if SLANG_WINDOWS_FAMILY
        if (fileName.getUnownedSlice().startsWithCaseInsensitive(UnownedStringSlice(name)))
#else
        if (fileName.getUnownedSlice().startsWith(UnownedStringSlice(name)))
#endif
        {
            return true;
        }
    }
    return false;
}

#ifdef _WIN32

// Make sure SlangResult match for common standard window HRESULT
SLANG_COMPILE_TIME_ASSERT(E_FAIL == SLANG_FAIL);
SLANG_COMPILE_TIME_ASSERT(E_NOINTERFACE == SLANG_E_NO_INTERFACE);
SLANG_COMPILE_TIME_ASSERT(E_HANDLE == SLANG_E_INVALID_HANDLE);
SLANG_COMPILE_TIME_ASSERT(E_NOTIMPL == SLANG_E_NOT_IMPLEMENTED);
SLANG_COMPILE_TIME_ASSERT(E_INVALIDARG == SLANG_E_INVALID_ARG);
SLANG_COMPILE_TIME_ASSERT(E_OUTOFMEMORY == SLANG_E_OUT_OF_MEMORY);

/* static */ SlangResult PlatformUtil::getInstancePath(StringBuilder& out)
{
    wchar_t path[_MAX_PATH];
    ::GetModuleFileName(::GetModuleHandle(NULL), path, SLANG_COUNT_OF(path));
    String pathString = String::fromWString(path);

    // We don't want the instance name, just the path to it
    out.clear();
    out.append(Path::getParentDirectory(pathString));

    return out.getLength() > 0 ? SLANG_OK : SLANG_FAIL;
}

/* static */ SlangResult PlatformUtil::appendResult(SlangResult res, StringBuilder& builderOut)
{
    if (SLANG_FAILED(res) && res != SLANG_FAIL)
    {
        LPWSTR buffer = nullptr;
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
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
            builderOut.append(String::fromWString(buffer));
            LocalFree(buffer);
            return SLANG_OK;
        }
    }
    return SLANG_FAIL;
}

/* static */ SlangResult SharedLibrary::loadWithPlatformPath(
    char const* platformFileName,
    SharedLibrary::Handle& handleOut)
{
    handleOut = nullptr;
    if (!platformFileName || strlen(platformFileName) == 0)
    {
        if (!GetModuleHandleExW(0, nullptr, (HMODULE*)&handleOut))
            return SLANG_FAIL;
        return SLANG_OK;
    }

    // We try to search the DLL in two different attempts.
    // First attempt - LoadLibraryExW()
    // If it failed to find one, we will use LoadLibraryW() to search over all PATH.
    // Search order: 1) The directory that contains the DLL (LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR).
    //                  This directory is searched only for dependencies of the DLL being loaded.
    //               2) Application directory
    //               3) User directories (AddDllDirectory/SetDllDirectory)
    //               4) System32
    //               5) PATH environment variable (by the 2nd attempt with LoadLibraryW())
    // https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibraryexw
    // https://docs.microsoft.com/en-us/windows/desktop/api/libloaderapi/nf-libloaderapi-loadlibraryw
    String platformFileNameStr(platformFileName);
    OSString wideFileName = platformFileNameStr.toWString();
    HMODULE handle = LoadLibraryExW(wideFileName, nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

    if (!handle)
        handle = LoadLibraryW(wideFileName);
    // If still not found, return an error.
    if (!handle)
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
        default:
            break;
        }
        // Turn to Result, if not one of the well known errors
        return HRESULT_FROM_WIN32(lastError);
    }

    if (SharedLibrary::isUnclosable(UnownedStringSlice(platformFileName)))
    {
        // Pin the module, which raises its reference count to a value the loader never decrements.
        // A later `FreeLibrary` then still balances our own `LoadLibrary`, but can no longer unmap
        // the module. This is the Windows counterpart of `RTLD_NODELETE` on the POSIX path.
        //
        // Pinning by address rather than by name so that we pin exactly the module we just loaded,
        // even if another module with the same base name is also loaded.
        HMODULE pinned = nullptr;
        const BOOL isPinned = ::GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
            (LPCWSTR)handle,
            &pinned);
        // `handle` is a module this function has just loaded successfully, so the loader can
        // always resolve it and there is no legitimate input that reaches this failing.
        SLANG_ASSERT(isPinned && pinned == handle);
        // `pinned` only exists because GetModuleHandleExW requires the out parameter; the pin is
        // recorded in the loader's own reference count, not in the handle it hands back.
        SLANG_UNUSED(pinned);
        if (!isPinned)
        {
            // Returning the library anyway would hand the caller a module that `unload` can still
            // unmap, which is exactly the teardown crash this is here to prevent. Report the
            // failure instead, so the library is reported as unavailable rather than becoming a
            // crash at process exit.
            const DWORD lastError = GetLastError();
            ::FreeLibrary(handle);
            return HRESULT_FROM_WIN32(lastError);
        }
    }

    handleOut = (Handle)handle;
    return SLANG_OK;
}

/* static */ void SharedLibrary::unload(Handle handle)
{
    SLANG_ASSERT(handle);
    ::FreeLibrary((HMODULE)handle);
}

/* static */ void* SharedLibrary::findSymbolAddressByName(Handle handle, char const* name)
{
    SLANG_ASSERT(handle);
    return reinterpret_cast<void*>(GetProcAddress((HMODULE)handle, name));
}

/* static */ void SharedLibrary::appendPlatformFileName(
    const UnownedStringSlice& name,
    StringBuilder& dst)
{
    dst.append(name);
    dst.append(".dll");
}

#else // _WIN32
/* static */ SlangResult PlatformUtil::getInstancePath([[maybe_unused]] StringBuilder& out)
{
#if defined(__linux__) || defined(__CYGWIN__)
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1)
    {
        return SLANG_FAIL;
    }

    path[len] = '\0';
    String pathString(path);

    // We don't want the instance name, just the path to it
    out.clear();
    out.append(Path::getParentDirectory(pathString));

    return out.getLength() > 0 ? SLANG_OK : SLANG_FAIL;
#else
    return SLANG_E_NOT_IMPLEMENTED;
#endif
}

/* static */ SlangResult PlatformUtil::appendResult(
    [[maybe_unused]] SlangResult res,
    [[maybe_unused]] StringBuilder& builderOut)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

/* static */ SlangResult SharedLibrary::loadWithPlatformPath(
    char const* platformFileName,
    Handle& handleOut)
{
    handleOut = nullptr;
    // `RTLD_NODELETE` keeps the library mapped when it is later `dlclose`d, for the libraries
    // whose callbacks have to outlive their own module. See `SharedLibrary::isUnclosable`.
    const bool isUnclosable = SharedLibrary::isUnclosable(UnownedStringSlice(platformFileName));
    if (strlen(platformFileName) == 0)
        platformFileName = nullptr;
    const auto mode = RTLD_NOW | RTLD_LOCAL | (isUnclosable ? RTLD_NODELETE : 0);
    void* h = dlopen(platformFileName, mode);
    if (!h)
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

/* static */ void SharedLibrary::unload(Handle handle)
{
    SLANG_ASSERT(handle);
    dlclose(handle);
}

/* static */ void* SharedLibrary::findSymbolAddressByName(Handle handle, char const* name)
{
    return dlsym((void*)handle, name);
}

/* static */ void SharedLibrary::appendPlatformFileName(
    const UnownedStringSlice& name,
    StringBuilder& dst)
{
#if __CYGWIN__
    dst.append(name);
    dst.append(".dll");
#elif SLANG_APPLE_FAMILY
    dst.append("lib");
    dst.append(name);
    dst.append(".dylib");
#elif SLANG_LINUX_FAMILY
    if (!name.startsWith("lib"))
        dst.append("lib");
    dst.append(name);
    if (name.indexOf(UnownedStringSlice(".so.")) == -1)
        dst.append(".so");
#else
    // Just guess we can do with the name on it's own
    dst.append(name);
#endif
}

#endif // _WIN32


/* static */ SlangResult PlatformUtil::setEnvironmentVariable(
    const UnownedStringSlice& name,
    const UnownedStringSlice* value)
{
    const String nameStr(name);
#ifdef _WIN32
    // _putenv_s removes the variable when handed an empty string, which is the same
    // observable state as never having set it.
    const String valueStr = value ? String(*value) : String();
    return _putenv_s(nameStr.getBuffer(), valueStr.getBuffer()) == 0 ? SLANG_OK : SLANG_FAIL;
#else
    if (!value)
        return ::unsetenv(nameStr.getBuffer()) == 0 ? SLANG_OK : SLANG_FAIL;
    const String valueStr(*value);
    return ::setenv(nameStr.getBuffer(), valueStr.getBuffer(), 1) == 0 ? SLANG_OK : SLANG_FAIL;
#endif
}

/* static */ SlangResult PlatformUtil::getEnvironmentVariable(
    const UnownedStringSlice& name,
    StringBuilder& out)
{
#ifdef _WIN32
    char* value = nullptr;
    size_t len = 0;
    errno_t err = _dupenv_s(&value, &len, String(name).getBuffer());
    if (err == 0 && value != nullptr)
    {
        out.append(value);
        free(value);
        return SLANG_OK;
    }
    return SLANG_E_NOT_FOUND;
#else
    const char* value = getenv(String(name).getBuffer());
    if (value)
    {
        out.append(value);
        return SLANG_OK;
    }
    return SLANG_E_NOT_FOUND;
#endif
}

/* static */ PlatformKind PlatformUtil::getPlatformKind()
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

static const PlatformFlags s_familyFlags[int(PlatformFamily::CountOf)] = {
    0,                                                               // Unknown
    PlatformFlag::WinRT | PlatformFlag::Win32 | PlatformFlag::Win64, // Windows
    PlatformFlag::WinRT | PlatformFlag::Win32 | PlatformFlag::Win64 | PlatformFlag::X360 |
        PlatformFlag::XBoxOne,                   // Microsoft
    PlatformFlag::Linux | PlatformFlag::Android, // Linux
    PlatformFlag::IOS | PlatformFlag::OSX,       // Apple
    PlatformFlag::Linux | PlatformFlag::Android | PlatformFlag::IOS | PlatformFlag::OSX, // Unix
};

/* static */ PlatformFlags PlatformUtil::getPlatformFlags(PlatformFamily family)
{
    return s_familyFlags[int(family)];
}

/* static */ SlangResult PlatformUtil::outputDebugMessage([[maybe_unused]] const char* text)
{
#ifdef _WIN32
    String textStr(text);
    OutputDebugStringW(textStr.toWString());
    return SLANG_OK;
#else
    return SLANG_E_NOT_AVAILABLE;
#endif
}

/* static */ void PlatformUtil::backtrace()
{
#if SLANG_HAS_BACKTRACE
    // Print stack trace for debugging assistance
    void* stackTrace[64];
    int stackDepth = ::backtrace(stackTrace, 64);
    char** symbols = ::backtrace_symbols(stackTrace, stackDepth);
    if (symbols)
    {
        for (int i = 0; i < stackDepth; ++i)
        {
            fprintf(stdout, "%s\n", symbols[i]);
        }
        free(symbols);
    }
    fprintf(stdout, "\n");
#else
    fprintf(stdout, "Stack trace not available on this platform.\n");
#endif
}

} // namespace Slang
