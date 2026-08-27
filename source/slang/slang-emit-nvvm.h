#pragma once

#include "compiler-core/slang-nvvm-ir-builder.h"
#include "slang-ir-link.h"

namespace Slang
{

struct CodeGenContext;

/// Independent builder semantics needed by an accepted linked-IR module.
using NVVMIRFeatureSet = SlangNVVMBuilderFeatureSet_3;

/// Checks whether linked Slang IR is in the exact direct-NVVM subset implemented through Slice 26.
SlangResult validateNVVMSupportedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    NVVMIRFeatureSet& outFeatures);

/// Emits verified LLVM 14 NVVM bitcode from already-validated supported IR.
SlangResult emitNVVMIRFromLinkedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    const NVVMIRBuilder& builder,
    ComPtr<IArtifact>& outArtifact);

} // namespace Slang
