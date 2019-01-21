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

/* static */OSFileSystem OSFileSystem::s_singleton; 

template <typename T>
static ISlangFileSystemExt* _getInterface(T* ptr, const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangFileSystem || guid == IID_ISlangFileSystemExt) ? static_cast<ISlangFileSystemExt*>(ptr) : nullptr;
}

ISlangUnknown* OSFileSystem::getInterface(const Guid& guid)
{
    return _getInterface(this, guid); 
}

static String _fixPathDelimiters(const char* pathIn)
{
#if SLANG_WINDOWS_FAMILY
    return pathIn;
#else
    // To allow windows style \ delimiters on other platforms, we convert to our standard delimiter
    String path(pathIn);
    return StringUtil::calcCharReplaced(pathIn, '\\', Path::PathDelimiter);
#endif
}

SlangResult OSFileSystem::getUniqueIdentity(const char* pathIn, ISlangBlob** outUniqueIdentity)
{
    // By default we use the canonical path to uniquely identify a file
    String canonicalPath;
    SLANG_RETURN_ON_FAIL(Path::GetCanonical(_fixPathDelimiters(pathIn), canonicalPath));
    
    *outUniqueIdentity = StringUtil::createStringBlob(canonicalPath).detach();
    return SLANG_OK;
}

SlangResult OSFileSystem::calcCombinedPath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** pathOut)
{
    // Don't need to fix delimiters - because combine path handles both path delimiter types
    return _calcCombinedPath(fromPathType, fromPath, path, pathOut);
}

SlangResult SLANG_MCALL OSFileSystem::getPathType(const char* pathIn, SlangPathType* pathTypeOut)
{
    return Path::GetPathType(_fixPathDelimiters(pathIn), pathTypeOut);
}

SlangResult OSFileSystem::loadFile(char const* pathIn, ISlangBlob** outBlob)
{
    // Default implementation that uses the `core` libraries facilities for talking to the OS filesystem.
    //
    // TODO: we might want to conditionally compile these in, so that
    // a user could create a build of Slang that doesn't include any OS
    // filesystem calls.

    const String path = _fixPathDelimiters(pathIn);
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

CacheFileSystem::CacheFileSystem(ISlangFileSystem* fileSystem, UniqueIdentityMode uniqueIdentityMode) :
    m_fileSystem(fileSystem),
    m_uniqueIdentityMode(uniqueIdentityMode)
{
    // Try to get the more sophisticated interface
    fileSystem->queryInterface(IID_ISlangFileSystemExt, (void**)m_fileSystemExt.writeRef());

    switch (uniqueIdentityMode)
    {
        case UniqueIdentityMode::Default:
        case UniqueIdentityMode::FileSystemExt:
        {
            // If it's not a complete file system, we will default to SimplifyAndHash style by default
            m_uniqueIdentityMode = m_fileSystemExt ? UniqueIdentityMode::FileSystemExt : UniqueIdentityMode::SimplifyPathAndHash;
            break;
        }
        default: break;
    }

    // It can't be default
    SLANG_ASSERT(m_uniqueIdentityMode != UniqueIdentityMode::Default);
}

CacheFileSystem::~CacheFileSystem()
{
    for (const auto& pair : m_uniqueIdentityMap)
    {
        delete pair.Value;
    }
}

// Determines if we can simplify a path for a given mode
static bool _canSimplifyPath(CacheFileSystem::UniqueIdentityMode mode)
{
    typedef CacheFileSystem::UniqueIdentityMode UniqueIdentityMode;
    switch (mode)
    {
        case UniqueIdentityMode::SimplifyPath:
        case UniqueIdentityMode::SimplifyPathAndHash:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

SlangResult CacheFileSystem::_calcUniqueIdentity(const String& path, String& outUniqueIdentity, ComPtr<ISlangBlob>& outFileContents)
{
    switch (m_uniqueIdentityMode)
    {
        case UniqueIdentityMode::FileSystemExt:
        {
            // Try getting the uniqueIdentity by asking underlying file system
            ComPtr<ISlangBlob> uniqueIdentity;
            SLANG_RETURN_ON_FAIL(m_fileSystemExt->getUniqueIdentity(path.Buffer(), uniqueIdentity.writeRef()));
            // Get the path as a string
            outUniqueIdentity = StringUtil::getString(uniqueIdentity);
            return SLANG_OK;
        }
        case UniqueIdentityMode::Path:
        {
            outUniqueIdentity = path;
            return SLANG_OK;
        }
        case UniqueIdentityMode::SimplifyPath:
        {
            outUniqueIdentity = Path::Simplify(path);
            // If it still has relative elements can't uniquely identify, so give up
            return Path::IsRelative(outUniqueIdentity) ? SLANG_FAIL : SLANG_OK;
        }
        case UniqueIdentityMode::SimplifyPathAndHash:
        case UniqueIdentityMode::Hash:
        {
            // I can only see if this is the same file as already loaded by loading the file and doing a hash
            Result res = m_fileSystem->loadFile(path.Buffer(), outFileContents.writeRef());
            if (SLANG_FAILED(res) || outFileContents == nullptr)
            {
                return SLANG_FAIL;
            }
             
            // Calculate the hash on the contents
            const uint64_t hash = GetHashCode64((const char*)outFileContents->getBufferPointer(), outFileContents->getBufferSize());

            String hashString = Path::GetFileName(path);
            hashString = hashString.ToLower();

            hashString.append(':');

            // The uniqueIdentity is a combination of name and hash
            hashString.append(hash, 16);

            outUniqueIdentity = hashString;
            return SLANG_OK;
        }
    }

    return SLANG_FAIL;
}

CacheFileSystem::PathInfo* CacheFileSystem::_getOrCreateUniqueIdentityCacheInfo(const String& path)
{
    // Use the path to produce uniqueIdentity information
    ComPtr<ISlangBlob> fileContents;
    String uniqueIdentity;

    SlangResult res = _calcUniqueIdentity(path, uniqueIdentity, fileContents);
    if (SLANG_FAILED(res))
    {
        // Was not able to create a uniqueIdentity - return failure as nullptr
        return nullptr;
    }

    // Now try looking up by uniqueIdentity path. If not found, add a new result
    PathInfo* pathInfo = nullptr;
    if (!m_uniqueIdentityMap.TryGetValue(uniqueIdentity, pathInfo))
    {
        // Create with found uniqueIdentity
        pathInfo = new PathInfo(uniqueIdentity);
        m_uniqueIdentityMap.Add(uniqueIdentity, pathInfo);
    }

    // At this point they must have same uniqueIdentity
    SLANG_ASSERT(pathInfo->getUniqueIdentity() == uniqueIdentity);

    // If we have the file contents (because of calcing uniqueIdentity), and there isn't a read fileblob already
    // store the data as if read, so doesn't get read again
    if (fileContents && !pathInfo->m_fileBlob)
    {
        pathInfo->m_fileBlob = fileContents;
        pathInfo->m_loadFileResult = CompressedResult::Ok;
    }

    return pathInfo;
}

CacheFileSystem::PathInfo* CacheFileSystem::_getOrCreateSimplifiedPathCacheInfo(const String& path)
{
    // If we can simplify the path, try looking up in path cache with simplified path (as long as it's different!)
    if (_canSimplifyPath(m_uniqueIdentityMode))
    {
        const String simplifiedPath = Path::Simplify(path);
        // Only lookup if the path is different - because otherwise will recurse forever...
        if (simplifiedPath != path)
        {
            // This is a recursive call - and will ensure the simplified path is added to the cache
            return _getOrCreatePathCacheInfo(simplifiedPath);
        }
    }

    return  _getOrCreateUniqueIdentityCacheInfo(path);
}

CacheFileSystem::PathInfo* CacheFileSystem::_getOrCreatePathCacheInfo(const String& path)
{
    // Lookup in path cache
    PathInfo* pathInfo;
    if (m_pathMap.TryGetValue(path, pathInfo))
    {
        // Found so done
        return pathInfo;
    }

    // Try getting or creating taking into account possible path simplification
    pathInfo = _getOrCreateSimplifiedPathCacheInfo(path);
    // Always add the result to the path cache (even if null)
    m_pathMap.Add(path, pathInfo);
    return pathInfo;
}

SlangResult CacheFileSystem::loadFile(char const* pathIn, ISlangBlob** blobOut)
{
    *blobOut = nullptr;
    String path(pathIn);
    PathInfo* info = _getOrCreatePathCacheInfo(path);
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

SlangResult CacheFileSystem::getUniqueIdentity(const char* path, ISlangBlob** outUniqueIdentity)
{
    PathInfo* info = _getOrCreatePathCacheInfo(path);
    if (!info)
    {
        return SLANG_E_NOT_FOUND;
    }
    info->m_uniqueIdentity->addRef();
    *outUniqueIdentity = info->m_uniqueIdentity;
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
    PathInfo* info = _getOrCreatePathCacheInfo(path);
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
