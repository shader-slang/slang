#include "slang-file-system.h"

#include "../../slang-com-ptr.h"
#include "../core/slang-io.h"
#include "../core/slang-string-util.h"

#include "compiler.h"

namespace Slang
{

// Allocate static const storage for the various interface IDs that the Slang API needs to expose
static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Guid IID_ISlangFileSystem = SLANG_UUID_ISlangFileSystem;
static const Guid IID_ISlangFileSystemExt = SLANG_UUID_ISlangFileSystemExt;

/* !!!!!!!!!!!!!!!!!!!!!!!!!! IncludeFileSystem !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* static */DefaultFileSystem DefaultFileSystem::s_singleton; 

template <typename T>
static ISlangFileSystemExt* _getInterface(T* ptr, const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangFileSystem || guid == IID_ISlangFileSystemExt) ? static_cast<ISlangFileSystemExt*>(ptr) : nullptr;
}

ISlangUnknown* DefaultFileSystem::getInterface(const Guid& guid)
{
    return _getInterface(this, guid); 
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
    return _getInterface(this, guid);
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

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CacheFileSystem !!!!!!!!!!!!!!!!!!!!!!!!!!!

ISlangUnknown* CacheFileSystem::getInterface(const Guid& guid)
{
    return _getInterface(this, guid);
}

SlangResult CacheFileSystem::_getCanonicalPath(const String& path, String& canonicalOut)
{
    const String* canonicalPathPtr = m_pathToCanonicalMap.TryGetValue(path);

    if (canonicalPathPtr)
    {
        if (canonicalPathPtr->getStringRepresentation() == nullptr)
        {
            return SLANG_E_NOT_FOUND;
        }
        else
        {
            canonicalOut = *canonicalPathPtr;
            return SLANG_OK;
        }
    }

    // Okay request from the underlying file system the canonical path
    ComPtr<ISlangBlob> canonicalBlob;
    if (SLANG_FAILED(m_fileSystem->getCanoncialPath(path.Buffer(), canonicalBlob.writeRef())))
    {
        // Write in result as being null ptr so not tried again
        m_pathToCanonicalMap.Add(path, String((StringRepresentation*)nullptr));
        return SLANG_E_NOT_FOUND;
    }
     
    String canonicalPath = StringUtil::getString(canonicalBlob);
    SLANG_ASSERT(canonicalPath.Length() > 0);

    // Add it to the cache
    m_pathToCanonicalMap.Add(path, canonicalPath);

    // A canonical path always maps to itself
    if (path != canonicalPath)
    {
        m_pathToCanonicalMap.AddIfNotExists(canonicalPath, canonicalPath);
    }

    canonicalOut = canonicalPath;
    return SLANG_OK;
}

CacheFileSystem::Info*  CacheFileSystem::_getInfoForCanonicalPath(const String& canonicalPath)
{
    Info* info = m_canonicalToInfoMap.TryGetValue(canonicalPath);
    if (info)
    {
        return info;
    }
    // Initialize info for this entry
    Info initInfo;
    m_canonicalToInfoMap.Add(canonicalPath, initInfo);
    return m_canonicalToInfoMap.TryGetValue(canonicalPath);
}

SlangResult CacheFileSystem::loadFile(char const* pathIn, ISlangBlob** blobOut)
{
    *blobOut = nullptr;

    String path(pathIn);
    String canonicalPath;
    SLANG_RETURN_ON_FAIL(_getCanonicalPath(path, canonicalPath));

    Info* info = _getInfoForCanonicalPath(canonicalPath);
    if (info->m_loadFileResult == SLANG_E_UNINITIALIZED)
    {
        info->m_loadFileResult = m_fileSystem->loadFile(path.Buffer(), info->m_fileBlob.writeRef());
        // Can't be in same state after the load
        SLANG_ASSERT(info->m_loadFileResult != SLANG_E_UNINITIALIZED);
    }

    *blobOut = info->m_fileBlob;
    if (*blobOut)
    {
        (*blobOut)->addRef();
    }
    return info->m_loadFileResult;
}

SlangResult CacheFileSystem::getCanoncialPath(const char* path, ISlangBlob** canonicalPathOut)
{
    String canonicalPath;
    SLANG_RETURN_ON_FAIL(_getCanonicalPath(path, canonicalPath));
    *canonicalPathOut = createStringBlob(canonicalPath).detach();
    return SLANG_OK;
}

SlangResult CacheFileSystem::calcRelativePath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** pathOut)
{
    // Just defer to contained implementation
    return m_fileSystem->calcRelativePath(fromPathType, fromPath, path, pathOut);
}

SlangResult CacheFileSystem::getPathType(const char* pathIn, SlangPathType* pathTypeOut)
{
    String path(pathIn);
    String canonicalPath;
    SLANG_RETURN_ON_FAIL(_getCanonicalPath(path, canonicalPath));
    // See if we have it in the cache
    Info* info = _getInfoForCanonicalPath(canonicalPath);
    if (info->m_getPathTypeResult == SLANG_E_UNINITIALIZED)
    {
        info->m_getPathTypeResult = m_fileSystem->getPathType(pathIn, &info->m_pathType);
        SLANG_ASSERT(info->m_getPathTypeResult != SLANG_E_UNINITIALIZED);
    }

    *pathTypeOut = info->m_pathType;
    return info->m_getPathTypeResult;
}

} 