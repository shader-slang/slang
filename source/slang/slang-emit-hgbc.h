// slang-emit-hgbc.h
#pragma once

#include "slang-emit-base.h"

namespace Slang
{

SlangResult emitHGBCForEntryPoints(CodeGenContext* codeGenContext, ComPtr<IArtifact>& outArtifact);

}
