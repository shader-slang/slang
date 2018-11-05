#include "slang-shared-library.h"

#include "../../slang-com-ptr.h"
#include "../core/slang-io.h"
#include "../core/slang-string-util.h"

namespace Slang
{

// Allocate static const storage for the various interface IDs that the Slang API needs to expose
static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Guid IID_ISlangSharedLibrary = SLANG_UUID_ISlangSharedLibrary;
static const Guid IID_ISlangSharedLibraryLoader = SLANG_UUID_ISlangSharedLibraryLoader;

/* !!!!!!!!!!!!!!!!!!!!!!!!!! DefaultSharedLibraryLoader !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* static */DefaultSharedLibraryLoader DefaultSharedLibraryLoader::s_singleton;

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

ISlangUnknown* DefaultSharedLibrary::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangSharedLibrary) ? static_cast<ISlangSharedLibrary*>(this) : nullptr;
}

DefaultSharedLibrary::~DefaultSharedLibrary()
{
    SharedLibrary::unload(m_sharedLibraryHandle);
}

SlangFuncPtr DefaultSharedLibrary::findFuncByName(char const* name)
{
    return SharedLibrary::findFuncByName(m_sharedLibraryHandle, name); 
}

} 