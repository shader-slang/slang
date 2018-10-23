#include "default-file-system.h"

#include "../../slang-com-ptr.h"
#include "../core/slang-io.h"
#include "compiler.h"

namespace Slang
{

// Allocate static const storage for the various interface IDs that the Slang API needs to expose
static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Guid IID_ISlangFileSystem = SLANG_UUID_ISlangFileSystem;
static const Guid IID_ISlangFileSystemExt = SLANG_UUID_ISlangFileSystemExt;

/* !!!!!!!!!!!!!!!!!!!!!!!!!! IncludeFileSystem !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* static */DefaultFileSystem DefaultFileSystem::s_singleton; 

ISlangUnknown* DefaultFileSystem::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangFileSystem || guid == IID_ISlangFileSystemExt) ? static_cast<ISlangFileSystemExt*>(this) : nullptr;
}

SlangResult DefaultFileSystem::getCanoncialPath(const char* path, ISlangBlob** canonicalPathOut)
{
    String canonicalPath;
    SLANG_RETURN_ON_FAIL(Path::GetCanonical(path, canonicalPath));
    
    *canonicalPathOut = createStringBlob(canonicalPath).detach();
    return SLANG_OK;
}

SlangResult DefaultFileSystem::calcRelativePath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** pathOut)
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

SlangResult SLANG_MCALL DefaultFileSystem::getPathType(
    const char* path,
    SlangPathType* pathTypeOut)
{
    return Path::GetPathType(path, pathTypeOut);   
}

SlangResult DefaultFileSystem::loadFile(char const* path, ISlangBlob** outBlob)
{
    // Default implementation that uses the `core` libraries facilities for talking to the OS filesystem.
    //
    // TODO: we might want to conditionally compile these in, so that
    // a user could create a build of Slang that doesn't include any OS
    // filesystem calls.

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

/* !!!!!!!!!!!!!!!!!!!!!!!!!! WrapFileSystem !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

ISlangUnknown* WrapFileSystem::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangFileSystem || guid == IID_ISlangFileSystemExt) ? static_cast<ISlangFileSystemExt*>(this) : nullptr;
}

SlangResult WrapFileSystem::loadFile(char const* path, ISlangBlob** outBlob)
{
    // Used the wrapped file system to do the loading
    return m_fileSystem->loadFile(path, outBlob);
}

SlangResult WrapFileSystem::getCanoncialPath(const char* path, ISlangBlob** canonicalPathOut)
{
    // This isn't a very good 'canonical path' because the same file might be referenced 
    // multiple ways - for example by using relative paths. 
    // But it's simple and matches slangs previous behavior. 
    String canonicalPath(path);
    *canonicalPathOut = createStringBlob(canonicalPath).detach();
    return SLANG_OK;
}

SlangResult WrapFileSystem::calcRelativePath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** pathOut)
{
    // Just defer to the default implementation
    return DefaultFileSystem::getSingleton()->calcRelativePath(fromPathType, fromPath, path, pathOut);
}

SlangResult WrapFileSystem::getPathType(const char* path, SlangPathType* pathTypeOut)
{
    // TODO:
    // This might be undesirable in the longer term because it means that ISlangFileSystem will not be used 
    // to test file existence - but the file system will be.
    // 
    // It would probably be better to use some kind of cache that uses 'loadFile' to load files, but also 
    // to test for existence.
    return DefaultFileSystem::getSingleton()->getPathType(path, pathTypeOut); 
}

} 