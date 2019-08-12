#include "slang-shared-library.h"

#include "../../slang-com-ptr.h"

#include "slang-io.h"
#include "slang-string-util.h"

namespace Slang
{

// Allocate static const storage for the various interface IDs that the Slang API needs to expose
static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Guid IID_ISlangSharedLibrary = SLANG_UUID_ISlangSharedLibrary;
static const Guid IID_ISlangSharedLibraryLoader = SLANG_UUID_ISlangSharedLibraryLoader;

/* !!!!!!!!!!!!!!!!!!!!!!!!!! DefaultSharedLibraryLoader !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* static */const char* DefaultSharedLibraryLoader::s_libraryNames[int(SharedLibraryType::CountOf)] =
{
    nullptr,                    // SharedLibraryType::Unknown 
    "dxcompiler",               // SharedLibraryType::Dxc 
    "d3dcompiler_47",           // SharedLibraryType::Fxc
    "slang-glslang",            // SharedLibraryType::Glslang  
    "dxil",                     // SharedLibraryType::Dxil
};

/* static */DefaultSharedLibraryLoader DefaultSharedLibraryLoader::s_singleton;

/* static */SharedLibraryType DefaultSharedLibraryLoader::getSharedLibraryTypeFromName(const UnownedStringSlice& name)
{
    // Start from 1 to skip Unknown
    for (int i = 1; i < SLANG_COUNT_OF(s_libraryNames); ++i)
    {
        if (name == s_libraryNames[i])
        {
            return SharedLibraryType(i);
        }
    }
    return SharedLibraryType::Unknown;
}

ISlangUnknown* DefaultSharedLibraryLoader::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangSharedLibraryLoader) ? static_cast<ISlangSharedLibraryLoader*>(this) : nullptr;
}

SlangResult DefaultSharedLibraryLoader::loadSharedLibrary(const char* path, ISlangSharedLibrary** sharedLibraryOut)
{
    *sharedLibraryOut = nullptr;
    // Try loading
    SharedLibrary::Handle handle;
    SLANG_RETURN_ON_FAIL(SharedLibrary::load(path, handle));
    *sharedLibraryOut = ComPtr<ISlangSharedLibrary>(new DefaultSharedLibrary(handle)).detach();
    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!! DefaultSharedLibrary !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

TemporarySharedLibrary::~TemporarySharedLibrary()
{
    if (m_sharedLibraryHandle)
    {
        // We have to unload if we want to be able to remove
        SharedLibrary::unload(m_sharedLibraryHandle);
        m_sharedLibraryHandle = nullptr;
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!! DefaultSharedLibrary !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

ISlangUnknown* DefaultSharedLibrary::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangSharedLibrary) ? static_cast<ISlangSharedLibrary*>(this) : nullptr;
}

DefaultSharedLibrary::~DefaultSharedLibrary()
{
    if (m_sharedLibraryHandle)
    {
        SharedLibrary::unload(m_sharedLibraryHandle);
    }
}

SlangFuncPtr DefaultSharedLibrary::findFuncByName(char const* name)
{
    return SharedLibrary::findFuncByName(m_sharedLibraryHandle, name); 
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!! ConfigurableSharedLibraryLoader !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

ISlangUnknown* ConfigurableSharedLibraryLoader::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangSharedLibraryLoader) ? static_cast<ISlangSharedLibraryLoader*>(this) : nullptr;
}

SlangResult ConfigurableSharedLibraryLoader::loadSharedLibrary(const char* path, ISlangSharedLibrary** sharedLibraryOut)
{
    Entry* entry = m_entryMap.TryGetValue(String(path));
    if (entry)
    {
        SharedLibrary::Handle handle;
        SLANG_RETURN_ON_FAIL(entry->func(path, entry->entryString, handle));
        SLANG_ASSERT(handle);

        ComPtr<ISlangSharedLibrary> sharedLib(new DefaultSharedLibrary(handle));
        *sharedLibraryOut = sharedLib.detach();
        return SLANG_OK;
    }

    return DefaultSharedLibraryLoader::getSingleton()->loadSharedLibrary(path, sharedLibraryOut);
}

/* static */Result ConfigurableSharedLibraryLoader::replace(const char* pathIn, const String& entryString, SharedLibrary::Handle& handleOut)
{
    SLANG_UNUSED(pathIn);
    // The replacement is the *whole* string
    return SharedLibrary::loadWithPlatformPath(entryString.begin(), handleOut);
}

/* static */Result ConfigurableSharedLibraryLoader::changePath(const char* pathIn, const String& entryString, SharedLibrary::Handle& handleOut )
{
    // Okay we need to reconstruct the name and insert the path
    StringBuilder builder;
    SharedLibrary::appendPlatformFileName(UnownedStringSlice(pathIn), builder);
    String path = Path::combine(entryString, builder);

    return SharedLibrary::loadWithPlatformPath(path.begin(), handleOut);
}


} 
