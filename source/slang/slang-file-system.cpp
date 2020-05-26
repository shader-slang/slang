#include "slang-file-system.h"

#include "../../slang-com-ptr.h"
#include "../core/slang-io.h"
#include "../core/slang-string-util.h"

namespace Slang
{

// Allocate static const storage for the various interface IDs that the Slang API needs to expose
static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Guid IID_ISlangFileSystem = SLANG_UUID_ISlangFileSystem;
static const Guid IID_ISlangFileSystemExt = SLANG_UUID_ISlangFileSystemExt;
static const Guid IID_SlangCacheFileSystem = SLANG_UUID_CacheFileSystem;

// Cacluate a combined path, just using Path:: string processing
static SlangResult _calcCombinedPath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** pathOut)
{
    String relPath;
    switch (fromPathType)
    {
        case SLANG_PATH_TYPE_FILE:
        {
            relPath = Path::combine(Path::getParentDirectory(fromPath), path);
            break;
        }
        case SLANG_PATH_TYPE_DIRECTORY:
        {
            relPath = Path::combine(fromPath, path);
            break;
        }
    }

    *pathOut = StringUtil::createStringBlob(relPath).detach();
    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! OSFileSystem !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* static */OSFileSystem OSFileSystem::s_singleton;

ISlangUnknown* OSFileSystem::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangFileSystem ) ? static_cast<ISlangFileSystem*>(this) : nullptr;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!! OSFileSystemExt !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* static */OSFileSystemExt OSFileSystemExt::s_singleton; 

template <typename T>
static ISlangFileSystemExt* _getInterface(T* ptr, const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangFileSystem || guid == IID_ISlangFileSystemExt) ? static_cast<ISlangFileSystemExt*>(ptr) : nullptr;
}

ISlangUnknown* OSFileSystemExt::getInterface(const Guid& guid)
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
    return StringUtil::calcCharReplaced(pathIn, '\\', Path::kPathDelimiter);
#endif
}

SlangResult OSFileSystemExt::getFileUniqueIdentity(const char* pathIn, ISlangBlob** outUniqueIdentity)
{
    // By default we use the canonical path to uniquely identify a file
    return getCanonicalPath(pathIn, outUniqueIdentity);
}

SlangResult OSFileSystemExt::getCanonicalPath(const char* path, ISlangBlob** outCanonicalPath)
{
    String canonicalPath;
    SLANG_RETURN_ON_FAIL(Path::getCanonical(_fixPathDelimiters(path), canonicalPath));
    *outCanonicalPath = StringUtil::createStringBlob(canonicalPath).detach();
    return SLANG_OK;
}

SlangResult OSFileSystemExt::getSimplifiedPath(const char* pathIn, ISlangBlob** outSimplifiedPath)
{
    String simplifiedPath = Path::simplify(_fixPathDelimiters(pathIn));
    *outSimplifiedPath = StringUtil::createStringBlob(simplifiedPath).detach();
    return SLANG_OK;
}

SlangResult OSFileSystemExt::calcCombinedPath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** pathOut)
{
    // Don't need to fix delimiters - because combine path handles both path delimiter types
    return _calcCombinedPath(fromPathType, fromPath, path, pathOut);
}

SlangResult SLANG_MCALL OSFileSystemExt::getPathType(const char* pathIn, SlangPathType* pathTypeOut)
{
    return Path::getPathType(_fixPathDelimiters(pathIn), pathTypeOut);
}

SlangResult OSFileSystemExt::loadFile(char const* pathIn, ISlangBlob** outBlob)
{
    // Default implementation that uses the `core` libraries facilities for talking to the OS filesystem.
    //
    // TODO: we might want to conditionally compile these in, so that
    // a user could create a build of Slang that doesn't include any OS
    // filesystem calls.

    const String path = _fixPathDelimiters(pathIn);
    if (!File::exists(path))
    {
        return SLANG_E_NOT_FOUND;
    }

    try
    {
        String sourceString = File::readAllText(path);
        *outBlob = StringUtil::createStringBlob(sourceString).detach();
        return SLANG_OK;
    }
    catch (...)
    {
    }
    return SLANG_E_CANNOT_OPEN;
}

SLANG_NO_THROW SlangResult SLANG_MCALL OSFileSystemExt::saveFile(const char* pathIn, const void* data, size_t size)
{
    const String path = _fixPathDelimiters(pathIn);

    try
    {
        FileStream stream(pathIn, FileMode::Create, FileAccess::Write, FileShare::ReadWrite);

        int64_t numWritten = stream.write(data, size);

        if (numWritten != int64_t(size))
        {
            return SLANG_FAIL;
        }

    }
    catch (const IOException&)
    {
    	return SLANG_E_CANNOT_OPEN;
    }

    return SLANG_OK;
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

SLANG_NO_THROW SlangResult SLANG_MCALL CacheFileSystem::queryInterface(SlangUUID const& uuid, void** outObject)
{
    if (uuid == IID_SlangCacheFileSystem)
    {
        *outObject = this;
        return SLANG_OK;
    }

    ISlangUnknown* intf = getInterface(uuid);
    if (intf)
    {
        addReference();
        *outObject = intf;
        return SLANG_OK;
    }
    return SLANG_E_NO_INTERFACE;
}


CacheFileSystem::CacheFileSystem(ISlangFileSystem* fileSystem, UniqueIdentityMode uniqueIdentityMode, PathStyle pathStyle) 
{
    setInnerFileSystem(fileSystem, uniqueIdentityMode, pathStyle);
}

CacheFileSystem::~CacheFileSystem()
{
    for (const auto& pair : m_uniqueIdentityMap)
    {
        delete pair.Value;
    }
}

void CacheFileSystem::setInnerFileSystem(ISlangFileSystem* fileSystem, UniqueIdentityMode uniqueIdentityMode, PathStyle pathStyle)
{
    m_fileSystem = fileSystem;

    m_uniqueIdentityMode = uniqueIdentityMode;
    m_pathStyle = pathStyle;
    
    m_fileSystemExt.setNull();

    if (fileSystem)
    {
        // Try to get the more sophisticated interface
        fileSystem->queryInterface(IID_ISlangFileSystemExt, (void**)m_fileSystemExt.writeRef());
    }

    switch (m_uniqueIdentityMode)
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

    if (pathStyle == PathStyle::Default)
    {
        // We'll assume it's simplify-able 
        m_pathStyle = PathStyle::Simplifiable;
        // If we have fileSystemExt, we defer to that
        if (m_fileSystemExt)
        {
            // We just defer to the m_fileSystem
            m_pathStyle = PathStyle::FileSystemExt;
        }
    }

    // It can't be default
    SLANG_ASSERT(m_uniqueIdentityMode != UniqueIdentityMode::Default);
}

void CacheFileSystem::clearCache()
{
    for (const auto& pair : m_uniqueIdentityMap)
    {
        delete pair.Value;
    }

    m_uniqueIdentityMap.Clear();
    m_pathMap.Clear();

    if (m_fileSystemExt)
    {
        m_fileSystemExt->clearCache();
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
            SLANG_RETURN_ON_FAIL(m_fileSystemExt->getFileUniqueIdentity(path.getBuffer(), uniqueIdentity.writeRef()));
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
            outUniqueIdentity = Path::simplify(path);
            // If it still has relative elements can't uniquely identify, so give up
            return Path::hasRelativeElement(outUniqueIdentity) ? SLANG_FAIL : SLANG_OK;
        }
        case UniqueIdentityMode::SimplifyPathAndHash:
        case UniqueIdentityMode::Hash:
        {
            // If we don't have a file system -> assume cannot be found
            if (m_fileSystem == nullptr)
            {
                return SLANG_E_NOT_FOUND;
            }

            // I can only see if this is the same file as already loaded by loading the file and doing a hash
            Result res = m_fileSystem->loadFile(path.getBuffer(), outFileContents.writeRef());
            if (SLANG_FAILED(res) || outFileContents == nullptr)
            {
                return SLANG_FAIL;
            }
             
            // Calculate the hash on the contents
            const uint64_t hash = getHashCode64((const char*)outFileContents->getBufferPointer(), outFileContents->getBufferSize());

            String hashString = Path::getFileName(path);
            hashString = hashString.toLower();

            hashString.append(':');

            // The uniqueIdentity is a combination of name and hash
            hashString.append(hash, 16);

            outUniqueIdentity = hashString;
            return SLANG_OK;
        }
    }

    return SLANG_FAIL;
}

CacheFileSystem::PathInfo* CacheFileSystem::_resolveUniqueIdentityCacheInfo(const String& path)
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

    // If we have the file contents (because of calc-ing uniqueIdentity), and there isn't a read file blob already
    // store the data as if read, so doesn't get read again
    if (fileContents && !pathInfo->m_fileBlob)
    {
        pathInfo->m_fileBlob = fileContents;
        pathInfo->m_loadFileResult = CompressedResult::Ok;
    }

    return pathInfo;
}

CacheFileSystem::PathInfo* CacheFileSystem::_resolveSimplifiedPathCacheInfo(const String& path)
{
    // If we can simplify the path, try looking up in path cache with simplified path (as long as it's different!)
    if (_canSimplifyPath(m_uniqueIdentityMode))
    {
        const String simplifiedPath = Path::simplify(path);
        // Only lookup if the path is different - because otherwise will recurse forever...
        if (simplifiedPath != path)
        {
            // This is a recursive call - and will ensure the simplified path is added to the cache
            return _resolvePathCacheInfo(simplifiedPath);
        }
    }

    return  _resolveUniqueIdentityCacheInfo(path);
}

CacheFileSystem::PathInfo* CacheFileSystem::_resolvePathCacheInfo(const String& path)
{
    // Lookup in path cache
    PathInfo* pathInfo;
    if (m_pathMap.TryGetValue(path, pathInfo))
    {
        // Found so done
        return pathInfo;
    }

    // Try getting or creating taking into account possible path simplification
    pathInfo = _resolveSimplifiedPathCacheInfo(path);
    // Always add the result to the path cache (even if null)
    m_pathMap.Add(path, pathInfo);
    return pathInfo;
}

SlangResult CacheFileSystem::loadFile(char const* pathIn, ISlangBlob** blobOut)
{
    *blobOut = nullptr;
    String path(pathIn);
    PathInfo* info = _resolvePathCacheInfo(path);
    if (!info)
    {
        return SLANG_FAIL;
    }
    
    if (info->m_loadFileResult == CompressedResult::Uninitialized)
    {
        info->m_loadFileResult = toCompressedResult(m_fileSystem->loadFile(path.getBuffer(), info->m_fileBlob.writeRef()));
    }

    *blobOut = info->m_fileBlob;
    if (*blobOut)
    {
        (*blobOut)->addRef();
    }
    return toResult(info->m_loadFileResult);
}

SlangResult CacheFileSystem::getFileUniqueIdentity(const char* path, ISlangBlob** outUniqueIdentity)
{
    PathInfo* info = _resolvePathCacheInfo(path);
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
    switch (m_pathStyle)
    {
        case PathStyle::FileSystemExt:
        {
            return m_fileSystemExt->calcCombinedPath(fromPathType, fromPath, path, pathOut);
        }
        default:
        {
            // Just use the default implementation
            return _calcCombinedPath(fromPathType, fromPath, path, pathOut);
        }
    }
}

SlangResult CacheFileSystem::getPathType(const char* pathIn, SlangPathType* pathTypeOut)
{
    String path(pathIn);
    PathInfo* info = _resolvePathCacheInfo(path);
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
                info->m_loadFileResult = toCompressedResult(m_fileSystem->loadFile(path.getBuffer(), info->m_fileBlob.writeRef()));
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

SlangResult CacheFileSystem::getSimplifiedPath(const char* path, ISlangBlob** outSimplifiedPath)
{
    // If we have a ISlangFileSystemExt we can just pass on the request to it
    switch (m_pathStyle)
    {
        case PathStyle::FileSystemExt:
        {
            return m_fileSystemExt->getSimplifiedPath(path, outSimplifiedPath);
        }
        case PathStyle::Simplifiable:
        {
            String simplifiedPath = Path::simplify(_fixPathDelimiters(path));
            *outSimplifiedPath = StringUtil::createStringBlob(simplifiedPath).detach();
            return SLANG_OK;
        }
        default: return SLANG_E_NOT_IMPLEMENTED;
    }
}

SlangResult CacheFileSystem::getCanonicalPath(const char* path, ISlangBlob** outCanonicalPath)
{
    // A file must exist to get a canonical path... 
    PathInfo* info = _resolvePathCacheInfo(path);
    if (!info)
    {
        return SLANG_E_NOT_FOUND;
    }

    // We don't have this -> so read it ...
    if (info->m_getCanonicalPathResult == CompressedResult::Uninitialized)
    {
        if (!m_fileSystemExt)
        {
            return SLANG_E_NOT_IMPLEMENTED;
        }

        // Try getting the canonicalPath by asking underlying file system
        ComPtr<ISlangBlob> canonicalPathBlob;
        SlangResult res = m_fileSystemExt->getCanonicalPath(path, canonicalPathBlob.writeRef());

        if (SLANG_SUCCEEDED(res))
        {
            // Get the path as a string
            String canonicalPath = StringUtil::getString(canonicalPathBlob);
            if (canonicalPath.getLength() > 0)
            {
                info->m_canonicalPath = new StringBlob(canonicalPath);
            }
            else
            {
                res = SLANG_FAIL;
            }
        }

        // Save the result
        info->m_getCanonicalPathResult = toCompressedResult(res);
    }

    if (info->m_canonicalPath)
    {
        info->m_canonicalPath->addRef();
    }
    *outCanonicalPath = info->m_canonicalPath;
    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  RelativeFileSystem  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

ISlangUnknown* RelativeFileSystem::getInterface(const Guid& guid)
{
    return _getInterface(this, guid);
}

SlangResult RelativeFileSystem::_getFixedPath(const char* path, String& outPath)
{
    ComPtr<ISlangBlob> blob;
    if (m_stripPath)
    {
        String strippedPath = Path::getFileName(path);
        SLANG_RETURN_ON_FAIL(m_fileSystem->calcCombinedPath(SLANG_PATH_TYPE_DIRECTORY, m_relativePath.getBuffer(), strippedPath.getBuffer(), blob.writeRef()));
    }
    else
    {
        SLANG_RETURN_ON_FAIL(m_fileSystem->calcCombinedPath(SLANG_PATH_TYPE_DIRECTORY, m_relativePath.getBuffer(), path, blob.writeRef()));
    }

    outPath = StringUtil::getString(blob);
    return SLANG_OK;
}

SLANG_NO_THROW SlangResult SLANG_MCALL RelativeFileSystem::loadFile(char const* path, ISlangBlob**    outBlob)
{
    String fixedPath;
    SLANG_RETURN_ON_FAIL(_getFixedPath(path, fixedPath));
    return m_fileSystem->loadFile(fixedPath.getBuffer(), outBlob);
}

SLANG_NO_THROW SlangResult SLANG_MCALL RelativeFileSystem::getFileUniqueIdentity(const char* path, ISlangBlob** outUniqueIdentity) 
{
    String fixedPath;
    SLANG_RETURN_ON_FAIL(_getFixedPath(path, fixedPath));
    return m_fileSystem->getFileUniqueIdentity(fixedPath.getBuffer(), outUniqueIdentity);
}

SLANG_NO_THROW SlangResult SLANG_MCALL RelativeFileSystem::calcCombinedPath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** outPath)
{
    String fixedFromPath;
    SLANG_RETURN_ON_FAIL(_getFixedPath(fromPath, fixedFromPath));

    return m_fileSystem->calcCombinedPath(fromPathType, fixedFromPath.getBuffer(), path, outPath);
}

SLANG_NO_THROW SlangResult SLANG_MCALL RelativeFileSystem::getPathType(const char* path, SlangPathType* outPathType)
{
    String fixedPath;
    SLANG_RETURN_ON_FAIL(_getFixedPath(path, fixedPath));
    return m_fileSystem->getPathType(fixedPath.getBuffer(), outPathType);
}

SLANG_NO_THROW SlangResult SLANG_MCALL RelativeFileSystem::getSimplifiedPath(const char* path, ISlangBlob** outSimplifiedPath)
{
    return m_fileSystem->getSimplifiedPath(path, outSimplifiedPath);
}

SLANG_NO_THROW SlangResult SLANG_MCALL RelativeFileSystem::getCanonicalPath(const char* path, ISlangBlob** outCanonicalPath)
{
    String fixedPath;
    SLANG_RETURN_ON_FAIL(_getFixedPath(path, fixedPath));
    return m_fileSystem->getCanonicalPath(fixedPath.getBuffer(), outCanonicalPath);
}

SLANG_NO_THROW void SLANG_MCALL RelativeFileSystem::clearCache()
{
    m_fileSystem->clearCache();
}

SLANG_NO_THROW SlangResult SLANG_MCALL RelativeFileSystem::saveFile(const char* path, const void* data, size_t size)
{
    String fixedPath;
    SLANG_RETURN_ON_FAIL(_getFixedPath(path, fixedPath));
    return m_fileSystem->saveFile(fixedPath.getBuffer(), data, size);
}

} 
