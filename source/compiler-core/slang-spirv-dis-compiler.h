#pragma once

#include "slang-downstream-compiler-util.h"

#include "../core/slang-platform.h"

namespace Slang
{

struct SPIRVDisDownstreamCompilerUtil
{
    static SlangResult locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set);
};

class SPIRVDisDownstreamCompiler : public DownstreamCompilerBase
{
public:
    SPIRVDisDownstreamCompiler(const Desc& desc) : DownstreamCompilerBase(desc) {}

    virtual SlangResult SLANG_MCALL convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact) noexcept override;

    virtual bool SLANG_MCALL isFileBased() noexcept override { return true; }
    virtual SlangResult SLANG_MCALL compile(const CompileOptions& options, IArtifact** outArtifact) noexcept override;
};

}
