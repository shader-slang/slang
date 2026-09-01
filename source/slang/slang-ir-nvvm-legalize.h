#pragma once

#include "slang-ir-link.h"

namespace Slang
{

struct CodeGenContext;

/// Reduces linked CUDA IR to the canonical operation set accepted by direct NVVM preflight.
SlangResult legalizeIRForNVVM(CodeGenContext* codeGenContext, LinkedIR& linkedIR);

} // namespace Slang
