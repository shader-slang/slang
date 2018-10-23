#ifndef SLANG_DEFAULT_FILE_SYSTEM_H_INCLUDED
#define SLANG_DEFAULT_FILE_SYSTEM_H_INCLUDED

#include "../../slang.h"
#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

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

    ISlangUnknown* getInterface(const Guid& guid);

    static DefaultFileSystem s_singleton;
};

/* Wraps an ISlangFileSystem, and provides the extra methods required to make a ISlangFileSystemExt 
interface, deferring to the contained file system to do reading. 

NOTE! That this behavior is the same as previously in that.... 
1) calcRelativePath, just returns the path as processed by the Path:: methods 
2) getCanonicalPath, just returns the input path as the 'canonical' path. This will be wrong with a file multiply referenced through paths with .. and or . but 
doing it this way means it works as before and requires no new functions.
*/
class WrapFileSystem : public ISlangFileSystemExt
{
public:
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
    WrapFileSystem(ISlangFileSystem* fileSystem):
        m_fileSystem(fileSystem)
    {
    }

protected:
    ISlangUnknown* getInterface(const Guid& guid);

    ComPtr<ISlangFileSystem> m_fileSystem;                  ///< The wrapped file system
    uint32_t m_refCount = 0;
};

class CacheFileSystem: public ISlangFileSystemExt
{
    public:
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
    CacheFileSystem(ISlangFileSystemExt* fileSystem) :
        m_fileSystem(fileSystem)
    {
    }

protected:
    ISlangUnknown* getInterface(const Guid& guid);

    struct Info
    {
        Info()
        {
            m_loadFileResult = SLANG_E_UNINITIALIZED;
            m_getPathTypeResult = SLANG_E_UNINITIALIZED;
        }
        SlangResult m_loadFileResult;   
        SlangResult m_getPathTypeResult;    
        SlangPathType m_pathType;
        ComPtr<ISlangBlob> m_fileBlob;
    };

        /// For a given canonical path return the associated info (by getting or creating and getting)
    Info* _getInfoForCanonicalPath(const String& canonicalPath);
    SlangResult _getCanonicalPath(const String& path, String& canonicalOut);
    
    /* TODO: This may be improved by mapping to a ISlangBlob. This makes output fast and easy, and if constructed 
    as a StringBlob, we can just static_cast to get as a string to use internally, instead of constantly converting. 
    It is probably the case we cannot do dynamic_cast on ISlangBlob if we don't know where constructed -> if outside of slang codebase 
    doing such a cast can cause an exception. So we *never* want to do dynamic cast from blobs which could be created by external code. */
    Dictionary<String, String> m_pathToCanonicalMap;        ///< Maps a path to a canonical path
    Dictionary<String, Info> m_canonicalToInfoMap;          ///< Maps a canonical path to a files contents

    ComPtr<ISlangFileSystemExt> m_fileSystem;               ///< Underlying file system
    uint32_t m_refCount = 0;
};

}

#endif