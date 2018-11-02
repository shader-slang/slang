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
    SharedLibrary lib = SharedLibrary::load(path);

    if (!lib.isLoaded())
    {
        return SLANG_E_NOT_FOUND;
    }

    ComPtr<ISlangSharedLibrary> sharedLib( new DefaultSharedLibrary(lib));
    *sharedLibraryOut = sharedLib.detach();
    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!! DefaultSharedLibrary !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

ISlangUnknown* DefaultSharedLibrary::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangSharedLibrary) ? static_cast<ISlangSharedLibrary*>(this) : nullptr;
}

bool DefaultSharedLibrary::isLoaded()
{
    return m_sharedLibrary.isLoaded();
}

void DefaultSharedLibrary::unload()
{
    m_sharedLibrary.unload();
}

SlangFuncPtr DefaultSharedLibrary::findFuncByName(char const* name)
{
    return m_sharedLibrary.isLoaded() ? m_sharedLibrary.findFuncByName(name) : nullptr;
}

} 