#pragma once

#include "compiler-core/slang-nvvm-ir-builder.h"
#include "core/slang-list.h"
#include "slang-ir-link.h"

namespace Slang
{

struct CodeGenContext;

/// Owns one exact typed value-operation overload required by accepted linked IR.
struct NVVMValueOperationRequirement
{
    SlangNVVMValueOperation operation = 0;
    SlangNVVMValueTypeDesc resultType = {};
    SlangNVVMValueTypeDesc operandTypes[3] = {};
    uint32_t operandCount = 0;
    const char* diagnosticName = nullptr;

    SlangNVVMValueOperationDesc getDesc() const
    {
        return {operation, resultType, operandCount ? operandTypes : nullptr, operandCount};
    }
};

using NVVMValueOperationRequirements = List<NVVMValueOperationRequirement>;

/// Replaces exact CUDA layout-query calls with constants and removes their compile-time-only IR.
SlangResult foldNVVMCompileTimeLayoutQueries(CodeGenContext* codeGenContext, LinkedIR& linkedIR);

/// Checks whether linked Slang IR is in the exact direct-NVVM subset.
SlangResult validateNVVMSupportedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    NVVMValueOperationRequirements& outRequirements);

/// Emits verified LLVM 7-compatible NVVM IR 2.0 assembly from already-validated linked IR.
SlangResult emitNVVMIRFromLinkedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    const NVVMIRBuilder& builder,
    const NVVMValueOperationRequirements& requirements,
    ComPtr<IArtifact>& outArtifact);

} // namespace Slang
