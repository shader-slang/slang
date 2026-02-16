// slang-ir-metadata.h
#pragma once

namespace Slang
{

class ArtifactPostEmitMetadata;
struct IRModule;

void collectCooperativeMetadata(const IRModule* irModule, ArtifactPostEmitMetadata& outMetadata);

void collectMetadata(const IRModule* irModule, ArtifactPostEmitMetadata& outMetadata);

} // namespace Slang
