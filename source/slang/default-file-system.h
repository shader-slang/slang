#ifndef SLANG_DEFAULT_FILE_SYSTEM_H_INCLUDED
#define SLANG_DEFAULT_FILE_SYSTEM_H_INCLUDED

#include "../../slang.h"
#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

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

        /// Get a default instance
    static ISlangFileSystemExt* getSingleton() { return &s_singleton; }

private:
        /// Make so not constructable
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

}

#endif