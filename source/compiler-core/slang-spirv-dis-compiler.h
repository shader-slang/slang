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

    virtual SlangResult convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact) override;

    virtual bool isFileBased() override { return true; }
    virtual SlangResult compile(const CompileOptions& options, IArtifact** outArtifact) override;
};

}
