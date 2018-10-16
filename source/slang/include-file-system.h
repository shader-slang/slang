#ifndef SLANG_INCLUDE_FILE_SYSTEM_H_INCLUDED
#define SLANG_INCLUDE_FILE_SYSTEM_H_INCLUDED

#include "../../slang.h"
#include "../../slang-com-helper.h"

namespace Slang
{

class IncludeFileSystem : public ISlangFileSystem
{
public:
    // ISlangUnknown
    SLANG_IUNKNOWN_ALL
        
    // ISlangFileSystem
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getCanoncialPath(
            const char* path,
            ISlangBlob** canonicalPathOut) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL calcRelativePath(
        SlangPathType fromPathType,
        const char* fromPath,
        const char* path,
        ISlangBlob** pathOut) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadFile(
        char const*     path,
        ISlangBlob**    outBlob) SLANG_OVERRIDE;

        /// Get a default instance
    static ISlangFileSystem* getDefault();

protected:
  
        /// If no ref, add one to the ref
    void ensureRef() { m_refCount += (m_refCount == 0); }

    ISlangUnknown* getInterface(const Guid& guid);
    uint32_t m_refCount = 0;
};

}

#endif