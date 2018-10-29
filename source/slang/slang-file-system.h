#ifndef SLANG_FILE_SYSTEM_H_INCLUDED
#define SLANG_FILE_SYSTEM_H_INCLUDED

#include "../../slang.h"
#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-string-util.h"
#include "../core/dictionary.h"

namespace Slang
{

class DefaultFileSystem : public ISlangFileSystemExt
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
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getCanoncialPath(
            const char* path,
            ISlangBlob** canonicalPathOut) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL calcRelativePath(
        SlangPathType fromPathType,
        const char* fromPath,
        const char* path,
        ISlangBlob** pathOut) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getPathType(
        const char* path,
        SlangPathType* pathTypeOut) SLANG_OVERRIDE;

        /// Get a default instance
    static ISlangFileSystemExt* getSingleton() { return &s_singleton; }

private:
        /// Make so not constructible
    DefaultFileSystem() {}
    virtual ~DefaultFileSystem() {}

    ISlangUnknown* getInterface(const Guid& guid);

    static DefaultFileSystem s_singleton;
};

/* Wraps an underlying ISlangFileSystem or ISlangFileSystemExt and provides caching, 
as well as emulation of methods if only has ISlangFileSystem interface. Will query capabilities
of the interface on the constructor.

NOTE! That this behavior is the same as previously in that.... 
1) calcRelativePath, just returns the path as processed by the Path:: methods 
2) getCanonicalPath, just returns the input path as the 'canonical' path. This will be wrong with a file multiply referenced through paths with .. and or . but 
doing it this way means it works as before and requires no new functions.

You can use a more sophisticated canonical style if you pass true to  useSimplifyForCanonicalPath. This will simplify relative path to create a canonical path.
*/
class CacheFileSystem: public ISlangFileSystemExt
{
    public:

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
    SLANG_IUNKNOWN_ALL

    // ISlangFileSystem
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadFile(
        char const*     path,
        ISlangBlob**    outBlob) SLANG_OVERRIDE;

    // ISlangFileSystemExt
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getCanoncialPath(
        const char* path,
        ISlangBlob** canonicalPathOut) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL calcRelativePath(
        SlangPathType fromPathType,
        const char* fromPath,
        const char* path,
        ISlangBlob** pathOut) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getPathType(
        const char* path,
        SlangPathType* pathTypeOut) SLANG_OVERRIDE;

        /// Ctor
    CacheFileSystem(ISlangFileSystem* fileSystem, bool useSimplifyForCanonicalPath = false);
        /// Dtor
    virtual ~CacheFileSystem();

    static CompressedResult toCompressedResult(Result res);
    static Result toResult(CompressedResult compRes) { return s_compressedResultToResult[int(compRes)]; } 
    static const Result s_compressedResultToResult[int(CompressedResult::CountOf)];

protected:
    ISlangUnknown* getInterface(const Guid& guid);

    struct PathInfo
    {
        PathInfo(const String& canonicalPath)
        {
            m_canonicalPath = new StringBlob(canonicalPath);
            m_canonicalPath->addRef();

            m_loadFileResult = CompressedResult::Uninitialized;
            m_getPathTypeResult = CompressedResult::Uninitialized;
        }
        ~PathInfo()
        {
            m_canonicalPath->release();
        }
            /// Get the canonical path as a string
        const String& getCanonicalPath() const { SLANG_ASSERT(m_canonicalPath); return m_canonicalPath->getString(); }

        StringBlob*  m_canonicalPath;                         ///< The canonical path
        CompressedResult m_loadFileResult;              
        CompressedResult m_getPathTypeResult;    
        SlangPathType m_pathType;
        ComPtr<ISlangBlob> m_fileBlob;
    };

        /// For a given relPath gets a PathInfo
    PathInfo* _getPathInfo(const String& relPath);

    /* TODO: This may be improved by mapping to a ISlangBlob. This makes output fast and easy, and if constructed 
    as a StringBlob, we can just static_cast to get as a string to use internally, instead of constantly converting. 
    It is probably the case we cannot do dynamic_cast on ISlangBlob if we don't know where constructed -> if outside of slang codebase 
    doing such a cast can cause an exception. So we *never* want to do dynamic cast from blobs which could be created by external code. */

    Dictionary<String, PathInfo*> m_pathMap;        ///< Maps a path to a canonical path
    Dictionary<String, PathInfo*> m_canonicalMap;   ///< Maps a canonical path to a files contents. This OWNs the PathInfo.

    bool m_useSimplifyForCanonicalPath;             ///< If set will use Path::Simplify to create 'canonical' paths

    ComPtr<ISlangFileSystem> m_fileSystem;          ///< Must always be set
    ComPtr<ISlangFileSystemExt> m_fileSystemExt;    ///< Optionally set -> if not will fall back on the m_fileSystem
    uint32_t m_refCount = 0;
};

}

#endif // SLANG_FILE_SYSTEM_H_INCLUDED