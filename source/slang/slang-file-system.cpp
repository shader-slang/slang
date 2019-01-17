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

// Cacluate a combined path, just using Path:: string processing
static SlangResult _calcCombinedPath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** pathOut)
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

    *pathOut = StringUtil::createStringBlob(relPath).detach();
    return SLANG_OK;
}

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
    
    *canonicalPathOut = StringUtil::createStringBlob(canonicalPath).detach();
    return SLANG_OK;
}

SlangResult DefaultFileSystem::calcCombinedPath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** pathOut)
{
    return _calcCombinedPath(fromPathType, fromPath, path, pathOut);
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
        *outBlob = StringUtil::createStringBlob(sourceString).detach();
        return SLANG_OK;
    }
    catch (...)
    {
    }
    return SLANG_E_CANNOT_OPEN;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CacheFileSystem !!!!!!!!!!!!!!!!!!!!!!!!!!!

/* static */ const Result CacheFileSystem::s_compressedResultToResult[] = 
{
    SLANG_E_UNINITIALIZED, 
    SLANG_OK,               ///< Ok
    SLANG_E_NOT_FOUND,      ///< File not found
    SLANG_E_CANNOT_OPEN,    ///< CannotOpen,
    SLANG_FAIL,             ///< Fail
};

/* static */CacheFileSystem::CompressedResult CacheFileSystem::toCompressedResult(Result res)
{
    if (SLANG_SUCCEEDED(res))
    {
        return CompressedResult::Ok;
    }
    switch (res)
    {
        case SLANG_E_CANNOT_OPEN:   return CompressedResult::CannotOpen;
        case SLANG_E_NOT_FOUND:     return CompressedResult::NotFound;
        default:                    return CompressedResult::Fail;
    }
}

ISlangUnknown* CacheFileSystem::getInterface(const Guid& guid)
{
    return _getInterface(this, guid);
}

CacheFileSystem::CacheFileSystem(ISlangFileSystem* fileSystem, CanonicalMode canonicalMode) :
    m_fileSystem(fileSystem),
    m_canonicalMode(canonicalMode)
{
    // Try to get the more sophisticated interface
    fileSystem->queryInterface(IID_ISlangFileSystemExt, (void**)m_fileSystemExt.writeRef());

    switch (canonicalMode)
    {
        case CanonicalMode::Default:
        case CanonicalMode::FileSystemExt:
        {
            m_canonicalMode = m_fileSystemExt ? CanonicalMode::FileSystemExt : CanonicalMode::Hash;
            break;
        }
        default: break;
    }
    // It can't be default
    SLANG_ASSERT(m_canonicalMode != CanonicalMode::Default);
}

CacheFileSystem::~CacheFileSystem()
{
    for (const auto& pair : m_canonicalMap)
    {
        delete pair.Value;
    }
}

CacheFileSystem::PathInfo* CacheFileSystem::_getPathInfoFromCanonical(const String& canonicalPath)
{
    // First see if we have it.. if not add it
    PathInfo** infoPtr = m_canonicalMap.TryGetValue(canonicalPath);
    if (infoPtr)
    {
        return *infoPtr;
    }
    else
    {
        // Create and add to canonical path
        PathInfo* pathInfo  = new PathInfo(canonicalPath);
        m_canonicalMap.Add(canonicalPath, pathInfo);
        return pathInfo;
    }
}

CacheFileSystem::PathInfo* CacheFileSystem::_getPathInfo(const String& relPath)
{
    PathInfo** infoPtr = m_pathMap.TryGetValue(relPath);
    if (infoPtr)
    {
        return *infoPtr;
    }

    PathInfo* pathInfo = nullptr;
    switch (m_canonicalMode)
    {
        case CanonicalMode::FileSystemExt:
        {
            // Try getting the canonical path
            // Okay request from the underlying file system the canonical path
            ComPtr<ISlangBlob> canonicalBlob;
            if (SLANG_FAILED(m_fileSystemExt->getCanoncialPath(relPath.Buffer(), canonicalBlob.writeRef())))
            {
                // Write in result as being null ptr so not tried again
                m_pathMap.Add(relPath, nullptr);
                return nullptr;
            }
            // Get the path as a string
            String canonicalPath = StringUtil::getString(canonicalBlob);
            pathInfo = _getPathInfoFromCanonical(canonicalPath);
            break;
        }
        case CanonicalMode::Path:
        {
            pathInfo = _getPathInfoFromCanonical(relPath);
            break;
        }
        case CanonicalMode::SimplifiedPath:
        {
            pathInfo = _getPathInfoFromCanonical(Path::Simplify(relPath.getUnownedSlice()));
            break;
        }
        case CanonicalMode::Hash:
        {
            // I can only see if this is the same file as already loaded by loading the file and doing a hash
            ComPtr<ISlangBlob> fileBlob;
            Result res = m_fileSystem->loadFile(relPath.Buffer(), fileBlob.writeRef());
            if (SLANG_FAILED(res) || fileBlob == nullptr)
            {
                // Write in result as being null ptr so not tried again
                m_pathMap.Add(relPath, nullptr);
                return nullptr;
            }

            // Calculate the hash on the contents
            const uint64_t hash = GetHashCode64((const char*)fileBlob->getBufferPointer(), fileBlob->getBufferSize());

            String hashString = Path::GetFileName(relPath);
            hashString = hashString.ToLower();

            hashString.append(':');

            // The canonical name is.. combination of name and hash
            hashString.append(hash, 16);
            // We'll use the 'hashString' as the canonical path
            pathInfo = _getPathInfoFromCanonical(hashString);

            // We have the contents, so store it on the PathInfo, along with the result
            if (pathInfo->m_loadFileResult == CompressedResult::Uninitialized)
            {
                // Save the contents of the saved file
                pathInfo->m_loadFileResult = toCompressedResult(res);
                pathInfo->m_fileBlob = fileBlob;
            }

            break;
        }
    }

    // Add the relPath
    m_pathMap.Add(relPath, pathInfo);
    return pathInfo;
}

SlangResult CacheFileSystem::loadFile(char const* pathIn, ISlangBlob** blobOut)
{
    *blobOut = nullptr;
    String path(pathIn);
    PathInfo* info = _getPathInfo(path);
    if (!info)
    {
        return SLANG_FAIL;
    }
    
    if (info->m_loadFileResult == CompressedResult::Uninitialized)
    {
        info->m_loadFileResult = toCompressedResult(m_fileSystem->loadFile(path.Buffer(), info->m_fileBlob.writeRef()));
    }

    *blobOut = info->m_fileBlob;
    if (*blobOut)
    {
        (*blobOut)->addRef();
    }
    return toResult(info->m_loadFileResult);
}

SlangResult CacheFileSystem::getCanoncialPath(const char* path, ISlangBlob** canonicalPathOut)
{
    PathInfo* info = _getPathInfo(path);
    if (!info)
    {
        return SLANG_E_NOT_FOUND;
    }
    info->m_canonicalPath->addRef();
    *canonicalPathOut = info->m_canonicalPath;
    return SLANG_OK;
}

SlangResult CacheFileSystem::calcCombinedPath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** pathOut)
{
    // Just defer to contained implementation
    if (m_fileSystemExt)
    {
        return m_fileSystemExt->calcCombinedPath(fromPathType, fromPath, path, pathOut);
    }
    else
    {
        // Just use the default implementation
        return _calcCombinedPath(fromPathType, fromPath, path, pathOut);
    }
}

SlangResult CacheFileSystem::getPathType(const char* pathIn, SlangPathType* pathTypeOut)
{
    String path(pathIn);
    PathInfo* info = _getPathInfo(path);
    if (!info)
    {
        return SLANG_E_NOT_FOUND;
    }

    if (info->m_getPathTypeResult == CompressedResult::Uninitialized)
    {
        if (m_fileSystemExt)
        {
            info->m_getPathTypeResult = toCompressedResult(m_fileSystemExt->getPathType(pathIn, &info->m_pathType));
        }
        else
        {
            // Okay try to load the file
            if (info->m_loadFileResult == CompressedResult::Uninitialized)
            {
                info->m_loadFileResult = toCompressedResult(m_fileSystem->loadFile(path.Buffer(), info->m_fileBlob.writeRef()));
            }

            // Make the getPathResult the same as the load result
            info->m_getPathTypeResult = info->m_loadFileResult;
            // Just set to file... the result is what matters in this case
            info->m_pathType = SLANG_PATH_TYPE_FILE; 
        }
    }

    *pathTypeOut = info->m_pathType;
    return toResult(info->m_getPathTypeResult);
}

} 
