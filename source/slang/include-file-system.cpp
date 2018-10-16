#include "include-file-system.h"

#include "../../slang-com-ptr.h"
#include "../core/slang-io.h"
#include "compiler.h"

namespace Slang
{

// Allocate static const storage for the various interface IDs that the Slang API needs to expose
static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Guid IID_ISlangFileSystem = SLANG_UUID_ISlangFileSystem;

/* static */ISlangFileSystem* IncludeFileSystem::getDefault()
{
    static IncludeFileSystem s_includeFileSystem;
    s_includeFileSystem.ensureRef();
    return &s_includeFileSystem;
}

ISlangUnknown* IncludeFileSystem::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangFileSystem) ? static_cast<ISlangFileSystem*>(this) : nullptr;
}

SlangResult IncludeFileSystem::getCanoncialPath(const char* path, ISlangBlob** canonicalPathOut)
{
    String canonicalPath;
    SLANG_RETURN_ON_FAIL(Path::GetCanonical(path, canonicalPath));
    
    *canonicalPathOut = createStringBlob(canonicalPath).detach();
    return SLANG_OK;
}

SlangResult IncludeFileSystem::calcRelativePath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** pathOut)
{
    String relPath;
    switch (fromPathType)
    {
        case SLANG_PATH_TYPE_FILE:
        {
            relPath = Path::Combine(Path::GetDirectoryName(fromPath), path);
            break;
        }
        case SLANG_PATH_TYPE_DIRECTORY:
        {
            relPath = Path::Combine(fromPath, path);
            break;
        }
    }

    *pathOut = createStringBlob(relPath).detach();
    return SLANG_OK;
}

SlangResult SLANG_MCALL IncludeFileSystem::loadFile(char const* path, ISlangBlob** outBlob)
{
    // Otherwise, fall back to a default implementation that uses the `core`
    // libraries facilities for talking to the OS filesystem.
    //
    // TODO: we might want to conditionally compile these in, so that
    // a user could create a build of Slang that doesn't include any OS
    // filesystem calls.
    //

    if (!File::Exists(path))
    {
        return SLANG_E_NOT_FOUND;
    }

    try
    {
        String sourceString = File::ReadAllText(path);
        ComPtr<ISlangBlob> sourceBlob = createStringBlob(sourceString);
        *outBlob = sourceBlob.detach();
        return SLANG_OK;
    }
    catch (...)
    {
    }
    return SLANG_E_CANNOT_OPEN;
}

} 