// slang-ir-metadata.h
#pragma once

#include "slang-target.h"

namespace Slang
{

class ArtifactPostEmitMetadata;
struct IRModule;

void collectMetadata(
    const IRModule* irModule,
    ArtifactPostEmitMetadata& outMetadata,
    CodeGenTarget target);

} // namespace Slang
