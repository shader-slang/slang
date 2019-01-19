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

SlangResult OSFileSystem::getCanoncialPath(const char* pathIn, ISlangBlob** canonicalPathOut)
{
    String canonicalPath;
    SLANG_RETURN_ON_FAIL(Path::GetCanonical(_fixPathDelimiters(pathIn), canonicalPath));
    
    *canonicalPathOut = StringUtil::createStringBlob(canonicalPath).detach();
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
            m_canonicalMode = m_fileSystemExt ? CanonicalMode::FileSystemExt : CanonicalMode::SimplifyPathAndHash;
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

// Determines if we can simplify a path for a given mode
static bool _canSimplifyPath(CacheFileSystem::CanonicalMode mode)
{
    typedef CacheFileSystem::CanonicalMode CanonicalMode;
    switch (mode)
    {
        case CanonicalMode::SimplifyPath:
        case CanonicalMode::SimplifyPathAndHash:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

SlangResult CacheFileSystem::_calcCanonicalPath(const String& path, String& outCanonicalPath, ComPtr<ISlangBlob>& outFileContents)
{
    switch (m_canonicalMode)
    {
        case CanonicalMode::FileSystemExt:
        {
            // Try getting the canonical path
            // Okay request from the underlying file system the canonical path
            ComPtr<ISlangBlob> canonicalBlob;
            SLANG_RETURN_ON_FAIL(m_fileSystemExt->getCanoncialPath(path.Buffer(), canonicalBlob.writeRef()));
            // Get the path as a string
            outCanonicalPath = StringUtil::getString(canonicalBlob);
            return SLANG_OK;
        }
        case CanonicalMode::Path:
        {
            outCanonicalPath = path;
            return SLANG_OK;
        }
        case CanonicalMode::SimplifyPath:
        {
            outCanonicalPath = Path::Simplify(path);
            // If it still has relative elements can't uniquely identify, so give up
            return Path::IsRelative(outCanonicalPath) ? SLANG_FAIL : SLANG_OK;
        }
        case CanonicalMode::SimplifyPathAndHash:
        case CanonicalMode::Hash:
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

            // The canonical name is.. combination of name and hash
            hashString.append(hash, 16);

            outCanonicalPath = hashString;
            return SLANG_OK;
        }
    }

    return SLANG_FAIL;
}

CacheFileSystem::PathInfo* CacheFileSystem::_getOrCreatePathInfo(const String& path)
{
    // First try with the name as it is
    PathInfo** infoPtr = m_pathMap.TryGetValue(path);
    if (infoPtr)
    {
        return *infoPtr;
    }

    // If we can simplify try that
    if (_canSimplifyPath(m_canonicalMode))
    {
        // Try to lookup with the simplified path, if it's different
        const String simplifiedPath = Path::Simplify(path);
        // Only lookup if the path is different - because otherwise will recurse forever...
        if (simplifiedPath != path)
        {
            // This is a recursive call, that will mean the original path and the simplified path will be added to cache
            return _getOrCreatePathInfo(simplifiedPath);
        }
    }

    // Okay we'll need to look up canonically then...
    ComPtr<ISlangBlob> fileContents;
    String canonicalPath;

    SlangResult res = _calcCanonicalPath(path, canonicalPath, fileContents);

    if (SLANG_FAILED(res))
    {
        // Was not able to create a canonical path.. so mark in path map the problem
        m_pathMap.Add(path, nullptr);
        return nullptr;
    }

    // Now try looking up by canonical path -> add if not found
    PathInfo* pathInfo;
    {
        // First see if we have it.. if not add it
        infoPtr = m_canonicalMap.TryGetValue(canonicalPath);
        if (infoPtr)
        {
            pathInfo = *infoPtr;
        }
        else
        {
            // Create with found canonicalPath
            pathInfo = new PathInfo(canonicalPath);
            m_canonicalMap.Add(canonicalPath, pathInfo);
        }
    }
    
    // At this point they must have same canonicalPath
    SLANG_ASSERT(StringUtil::getString(pathInfo->m_canonicalPath) == canonicalPath);

    // Add to the path map
    m_pathMap.Add(path, pathInfo);

    // If we have the file contents (because of calcing canonical), and there isn't a read fileblob already
    // store the data as if read, so doesn't get read again
    if (fileContents && !pathInfo->m_fileBlob)
    {
        pathInfo->m_fileBlob = fileContents;
        pathInfo->m_loadFileResult = CompressedResult::Ok;
    }
    
    return pathInfo;
}

SlangResult CacheFileSystem::loadFile(char const* pathIn, ISlangBlob** blobOut)
{
    *blobOut = nullptr;
    String path(pathIn);
    PathInfo* info = _getOrCreatePathInfo(path);
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
    PathInfo* info = _getOrCreatePathInfo(path);
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
    PathInfo* info = _getOrCreatePathInfo(path);
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
