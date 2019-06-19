#ifndef SLANG_FILE_SYSTEM_H_INCLUDED
#define SLANG_FILE_SYSTEM_H_INCLUDED

#include "../../slang.h"
#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-string-util.h"
#include "../core/slang-dictionary.h"

namespace Slang
{

class OSFileSystem : public ISlangFileSystemExt
{
public:
    // ISlangUnknown 
    // override ref counting, as DefaultFileSystem is singleton
    SLANG_IUNKNOWN_QUERY_INTERFACE 
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; } 

    // ISlangFileSystem
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadFile(
        char const*     path,
        ISlangBlob**    outBlob) SLANG_OVERRIDE;

    // ISlangFileSystemExt
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getFileUniqueIdentity(
            const char* path,
            ISlangBlob** uniqueIdentityOut) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL calcCombinedPath(
        SlangPathType fromPathType,
        const char* fromPath,
        const char* path,
        ISlangBlob** pathOut) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getPathType(
        const char* path,
        SlangPathType* pathTypeOut) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getSimplifiedPath(
        const char* path,
        ISlangBlob** outSimplifiedPath) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getCanonicalPath(
        const char* path,
        ISlangBlob** outCanonicalPath) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW void SLANG_MCALL clearCache() SLANG_OVERRIDE {}

        /// Get a default instance
    static ISlangFileSystemExt* getSingleton() { return &s_singleton; }

private:
        /// Make so not constructible
    OSFileSystem() {}
    virtual ~OSFileSystem() {}

    ISlangUnknown* getInterface(const Guid& guid);

    static OSFileSystem s_singleton;
};

/* Wraps an underlying ISlangFileSystem or ISlangFileSystemExt and provides caching, 
as well as emulation of methods if only has ISlangFileSystem interface. Will query capabilities
of the interface on the constructor.

NOTE! That this behavior is the same as previously in that.... 
1) calcRelativePath, just returns the path as processed by the Path:: methods 
2) getUniqueIdentity behavior depends on the UniqueIdentityMode.
*/
class CacheFileSystem: public ISlangFileSystemExt, public RefObject
{
    public:

    enum class PathStyle
    {
        Default,                    ///< Pass to say use the default 
        Unknown,                    ///< It's an unknown type of path
        Simplifiable,               ///< It can be simplified by Path::Simplify
    };

    enum UniqueIdentityMode
    {
        Default,                    ///< If passed, will default to the others depending on what kind of ISlangFileSystem is passed in
        Path,                       ///< Just use the path as is (old style slang behavior)
        SimplifyPath,               ///< Use the input path 'simplified' (ie removing . and .. aspects)
        Hash,                       ///< Use hashing
        SimplifyPathAndHash,        ///< Tries simplifying path first, and if that doesn't work it hashes
        FileSystemExt,              ///< Use the file system extended interface. 
    };

    /* Cannot change order/add members without changing s_compressedResultToResult */
    enum class CompressedResult: uint8_t
    {   
        Uninitialized,                          ///< Holds no value
        Ok,                                     ///< Ok
        NotFound,                               ///< File not found
        CannotOpen,                             ///< Cannot open
        Fail,                                   ///< Generic failure
        CountOf,
    };

    // ISlangUnknown 
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    // ISlangFileSystem
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadFile(
        char const*     path,
        ISlangBlob**    outBlob) SLANG_OVERRIDE;

    // ISlangFileSystemExt
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getFileUniqueIdentity(
        const char* path,
        ISlangBlob** outUniqueIdentity) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL calcCombinedPath(
        SlangPathType fromPathType,
        const char* fromPath,
        const char* path,
        ISlangBlob** pathOut) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getPathType(
        const char* path,
        SlangPathType* outPathType) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getSimplifiedPath(
        const char* path,
        ISlangBlob** outSimplifiedPath) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getCanonicalPath(
        const char* path,
        ISlangBlob** outCanonicalPath) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW void SLANG_MCALL clearCache() SLANG_OVERRIDE;

        /// Ctor
    CacheFileSystem(ISlangFileSystem* fileSystem, UniqueIdentityMode uniqueIdentityMode = UniqueIdentityMode::Default, PathStyle pathStyle = PathStyle::Default);
        /// Dtor
    virtual ~CacheFileSystem();

    static CompressedResult toCompressedResult(Result res);
    static Result toResult(CompressedResult compRes) { return s_compressedResultToResult[int(compRes)]; } 
    static const Result s_compressedResultToResult[int(CompressedResult::CountOf)];

protected:
    ISlangUnknown* getInterface(const Guid& guid);

    struct PathInfo
    {
        PathInfo(const String& uniqueIdentity)
        {
            m_uniqueIdentity = new StringBlob(uniqueIdentity);
            m_uniqueIdentity->addRef();

            m_loadFileResult = CompressedResult::Uninitialized;
            m_getPathTypeResult = CompressedResult::Uninitialized;
            m_getCanonicalPathResult = CompressedResult::Uninitialized;

            m_pathType = SLANG_PATH_TYPE_FILE;
        }
        
            /// Get the unique identity path as a string
        const String& getUniqueIdentity() const { SLANG_ASSERT(m_uniqueIdentity); return m_uniqueIdentity->getString(); }

        RefPtr<StringBlob> m_uniqueIdentity;
        CompressedResult m_loadFileResult;              
        CompressedResult m_getPathTypeResult;
        CompressedResult m_getCanonicalPathResult;

        SlangPathType m_pathType;
        ComPtr<ISlangBlob> m_fileBlob;
        RefPtr<StringBlob> m_canonicalPath;
    };
        /// Given a path, works out a uniqueIdentity, based on the uniqueIdentityMode. outFileContents will be set if file had to be read to produce the uniqueIdentity (ie with Hash)
    SlangResult _calcUniqueIdentity(const String& path, String& outUniqueIdentity, ComPtr<ISlangBlob>& outFileContents);

        /// For a given path gets a PathInfo. Can return nullptr, if it is not possible to create the PathInfo for some reason
    PathInfo* _resolvePathCacheInfo(const String& path);
        /// Turns the path into a uniqueIdentity, and then tries to look up in the uniqueIdentityMap.
    PathInfo* _resolveUniqueIdentityCacheInfo(const String& path);
        /// Will simplify the path (if possible) to lookup on the pathCache else will create on uniqueIdentityMap
    PathInfo* _resolveSimplifiedPathCacheInfo(const String& path);

    /* TODO: This may be improved by mapping to a ISlangBlob. This makes output fast and easy, and if constructed 
    as a StringBlob, we can just static_cast to get as a string to use internally, instead of constantly converting. 
    It is probably the case we cannot do dynamic_cast on ISlangBlob if we don't know where constructed -> if outside of slang codebase 
    doing such a cast can cause an exception. So we *never* want to do dynamic cast from blobs which could be created by external code. */

    Dictionary<String, PathInfo*> m_pathMap;            ///< Maps a path to a PathInfo (and unique identity)
    Dictionary<String, PathInfo*> m_uniqueIdentityMap;  ///< Maps a unique identity for a file to its contents. This OWNs the PathInfo.

    UniqueIdentityMode m_uniqueIdentityMode;            ///< Determines how the 'uniqueIdentity' is produced. Cannot be Default in usage.
    PathStyle m_pathStyle;                              ///< Style of paths

    ComPtr<ISlangFileSystem> m_fileSystem;              ///< Must always be set
    ComPtr<ISlangFileSystemExt> m_fileSystemExt;        ///< Optionally set -> if nullptr will fall back on the m_fileSystem and emulate all the other methods of ISlangFileSystemExt
};

}

#endif // SLANG_FILE_SYSTEM_H_INCLUDED
