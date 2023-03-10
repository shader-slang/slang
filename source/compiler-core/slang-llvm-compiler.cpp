// slang-llvm-compiler.cpp
#include "slang-llvm-compiler.h"

#include "../core/slang-common.h"

namespace Slang
{

class AliasDepreciatedDownstreamCompiler : public DownstreamCompilerBase
{
public:

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL compile(const CompileOptions& options, IArtifact** outArtifact) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW bool SLANG_MCALL canConvert(const ArtifactDesc& from, const ArtifactDesc& to) SLANG_OVERRIDE { return m_inner->canConvert(from, to); }
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact) SLANG_OVERRIDE { return m_inner->convert(from, to, outArtifact); }
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getVersionString(slang::IBlob** outVersionString) { return m_inner->getVersionString(outVersionString); }
    virtual SLANG_NO_THROW bool SLANG_MCALL isFileBased() { return m_inner->isFileBased(); }

    template <typename T>
    void initCompileOptionsDepreciated()
    {
        m_compileOptionsOffset = T::kStart;
    }

    AliasDepreciatedDownstreamCompiler(IDownstreamCompiler* inner) :
        m_inner(inner)
    {
        m_desc = inner->getDesc();
    }

    ComPtr<IDownstreamCompiler> m_inner;
    ptrdiff_t m_compileOptionsOffset = 0;
};

SlangResult AliasDepreciatedDownstreamCompiler::compile(const CompileOptions& options, IArtifact** outArtifact)
{
    if (m_compileOptionsOffset == 0)
    {
        return m_inner->compile(options, outArtifact);
    }
    const uint8_t* ptr = ((const uint8_t*)&options) + m_compileOptionsOffset;
    return m_inner->compile(*(const CompileOptions*)ptr, outArtifact);
}

/* static */SlangResult LLVMDownstreamCompilerUtil::locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
{
    ComPtr<ISlangSharedLibrary> library;

    SLANG_RETURN_ON_FAIL(DownstreamCompilerUtil::loadSharedLibrary(path, loader, nullptr, "slang-llvm", library));

    SLANG_ASSERT(library);
    if (!library)
    {
        return SLANG_FAIL;
    }

    typedef SlangResult(*CreateDownstreamCompilerFunc)(const Guid& intf, IDownstreamCompiler** outCompiler);

    ComPtr<IDownstreamCompiler> downstreamCompiler;

    if (auto fnV2 = (CreateDownstreamCompilerFunc)library->findFuncByName("createLLVMDownstreamCompiler_V2"))
    {
        ComPtr<IDownstreamCompiler> innerDownstreamCompiler;

        SLANG_RETURN_ON_FAIL(fnV2(IDownstreamCompiler::getTypeGuid(), innerDownstreamCompiler.writeRef()));

        // We then need to wrap
        AliasDepreciatedDownstreamCompiler* fix = new AliasDepreciatedDownstreamCompiler(innerDownstreamCompiler);
        downstreamCompiler = fix;
        fix->initCompileOptionsDepreciated<DownstreamCompileOptions_AliasDepreciated1>();
    }
    else if (auto fnV3 = (CreateDownstreamCompilerFunc)library->findFuncByName("createLLVMDownstreamCompiler_V3"))
    {
        SLANG_RETURN_ON_FAIL(fnV3(IDownstreamCompiler::getTypeGuid(), downstreamCompiler.writeRef()));
    }
    else
    {
        return SLANG_FAIL;
    }

    set->addSharedLibrary(library);
    set->addCompiler(downstreamCompiler);
    return SLANG_OK;
}

}
