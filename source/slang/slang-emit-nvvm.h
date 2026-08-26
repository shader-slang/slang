#pragma once

#include "compiler-core/slang-nvvm-ir-builder.h"
#include "slang-ir-link.h"

namespace Slang
{

struct CodeGenContext;

/// Checks whether linked Slang IR is in the exact empty-compute subset owned by Slice 6.
SlangResult validateNVVMMinimalComputeIR(CodeGenContext* codeGenContext, const LinkedIR& linkedIR);

/// Emits verified LLVM 14 NVVM bitcode from already-validated minimal compute IR.
SlangResult emitNVVMIRFromLinkedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    const NVVMIRBuilder& builder,
    ComPtr<IArtifact>& outArtifact);

} // namespace Slang
