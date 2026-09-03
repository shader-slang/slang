#pragma once

#include "slang-emit-nvvm-plan.h"

namespace Slang
{

struct CodeGenContext;

/// Checks whether linked Slang IR is in the exact direct-NVVM subset.
SlangResult validateNVVMSupportedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    NVVMOperationRequirements& outRequirements);

/// Emits verified LLVM 7-compatible NVVM IR 2.0 assembly from already-validated linked IR.
SlangResult emitNVVMIRFromLinkedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    const NVVMIRBuilder& builder,
    const NVVMOperationRequirements& requirements,
    ComPtr<IArtifact>& outArtifact);

} // namespace Slang
