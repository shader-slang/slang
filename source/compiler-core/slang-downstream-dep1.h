#ifndef SLANG_DOWNSTREAM_DEP1_H
#define SLANG_DOWNSTREAM_DEP1_H


#include "slang-downstream-compiler.h"

namespace Slang
{

// (DEPRECIATED)
class DownstreamCompiler_Dep1: public RefObject
{
public:
    typedef RefObject Super;

        /// Get the desc of this compiler
    const DownstreamCompilerDesc& getDesc() const { return m_desc;  }
        /// Compile using the specified options. The result is in resOut
    virtual SlangResult compile(const DownstreamCompileOptions& options, RefPtr<DownstreamCompileResult>& outResult) = 0;
        /// Some compilers have support converting a binary blob into disassembly. Output disassembly is held in the output blob
    virtual SlangResult disassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out);

        /// True if underlying compiler uses file system to communicate source
    virtual bool isFileBased() = 0;

protected:

    DownstreamCompilerDesc m_desc;
};

class DownstreamCompilerAdapter_Dep1 : public DownstreamCompilerBase
{
public:
    // IDownstreamCompiler
    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() SLANG_OVERRIDE { return m_dep->getDesc(); }
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL compile(const CompileOptions& options, RefPtr<DownstreamCompileResult>& outResult) SLANG_OVERRIDE { return m_dep->compile(options, outResult); }
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL disassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out) SLANG_OVERRIDE { return m_dep->disassemble(sourceBlobTarget, blob, blobSize, out); }
    virtual SLANG_NO_THROW bool SLANG_MCALL isFileBased() SLANG_OVERRIDE { return m_dep->isFileBased(); }

    DownstreamCompilerAdapter_Dep1(DownstreamCompiler_Dep1* dep) :
        m_dep(dep)
    {
    }

protected:
    RefPtr<DownstreamCompiler_Dep1> m_dep;
};

struct DownstreamUtil_Dep1
{
    static SlangResult getDownstreamSharedLibrary(DownstreamCompileResult* downstreamResult, ComPtr<ISlangSharedLibrary>& outSharedLibrary);
};

}

#endif
