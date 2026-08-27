#pragma once

#include "compiler-core/slang-nvvm-ir-builder.h"
#include "slang-ir-link.h"

namespace Slang
{

struct CodeGenContext;

/// The smallest coherent builder prefix needed by an accepted linked-IR module.
enum class NVVMIRCapability
{
    Minimal,
    ScalarMemory,
    ScalarControlFlow,
    ScalarSSA,
    ScalarFunctions,
    ScalarPointerArithmetic,
    ScalarArrayAddressing,
    ScalarIntegerMultiply,
    ScalarIntegerBitAnd,
    ScalarIntegerBitOr,
    ScalarIntegerBitXor,
    ScalarIntegerBitNot,
    ScalarIntegerNegate,
    RelaxedGlobalI32AtomicAdd,
};

/// Checks whether linked Slang IR is in the exact relaxed-global-i32-atomic subset owned by
/// Slice 19.
SlangResult validateNVVMSupportedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    NVVMIRCapability& outCapability);

/// Emits verified LLVM 14 NVVM bitcode from already-validated supported IR.
SlangResult emitNVVMIRFromLinkedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    const NVVMIRBuilder& builder,
    ComPtr<IArtifact>& outArtifact);

} // namespace Slang
