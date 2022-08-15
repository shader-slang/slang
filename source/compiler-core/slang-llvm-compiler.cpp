// slang-llvm-compiler.cpp
#include "slang-llvm-compiler.h"

#include "../core/slang-common.h"
#include "../../slang-com-helper.h"

#include "../core/slang-blob.h"

#include "../core/slang-string-util.h"
#include "../core/slang-string-slice-pool.h"

#include "../core/slang-io.h"
#include "../core/slang-shared-library.h"
#include "../core/slang-semantic-version.h"
#include "../core/slang-char-util.h"

#include "slang-include-system.h"
#include "slang-source-loc.h"

#include "../core/slang-shared-library.h"

namespace Slang
{

class DownstreamCompilerAdapter_Dep1 : public DownstreamCompilerBase
{
public:
    // IDownstreamCompiler
    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() SLANG_OVERRIDE { return m_dep->getDesc(); }
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL compile(const CompileOptions& options, RefPtr<DownstreamCompileResult>& outResult) SLANG_OVERRIDE { return m_dep->compile(options, outResult); }
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL disassemble(SlangCompileTarget sourceBlobTarget, const void* blob, size_t blobSize, ISlangBlob** out) SLANG_OVERRIDE { return m_dep->disassemble(sourceBlobTarget, blob, blobSize, out); }
    virtual SLANG_NO_THROW bool SLANG_MCALL isFileBased() SLANG_OVERRIDE { return m_dep->isFileBased(); }

    DownstreamCompilerAdapter_Dep1(DownstreamCompiler_Dep1* dep): 
        m_dep(dep)
    {
    }

protected:
    RefPtr<DownstreamCompiler_Dep1> m_dep;
};

/* static */SlangResult LLVMDownstreamCompilerUtil::locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
{
    ComPtr<ISlangSharedLibrary> library;

    SLANG_RETURN_ON_FAIL(DownstreamCompilerUtil::loadSharedLibrary(path, loader, nullptr, "slang-llvm", library));

    SLANG_ASSERT(library);
    if (!library)
    {
        return SLANG_FAIL;
    }

    typedef SlangResult(*CreateDownstreamCompilerFunc_Dep1)(RefPtr<DownstreamCompiler_Dep1>& out);

    auto fn = (CreateDownstreamCompilerFunc_Dep1)library->findFuncByName("createLLVMDownstreamCompiler");
    if (!fn)
    {
        return SLANG_FAIL;
    }

    RefPtr<DownstreamCompiler_Dep1> downstreamCompilerDep1;

    SLANG_RETURN_ON_FAIL(fn(downstreamCompilerDep1));

    ComPtr<IDownstreamCompiler> downstreamCompiler(new DownstreamCompilerAdapter_Dep1(downstreamCompilerDep1));

    set->addSharedLibrary(library);
    set->addCompiler(downstreamCompiler);
    return SLANG_OK;
}

}
