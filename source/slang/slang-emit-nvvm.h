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

/// Owns one exact typed surface-operation overload required by accepted linked IR.
struct NVVMSurfaceOperationRequirement
{
    IRFunc* function = nullptr;
    SlangNVVMSurfaceOperationDesc desc = {};
    const char* diagnosticName = nullptr;
};

/// Owns the ordered typed texture operations required by one accepted helper.
struct NVVMTextureOperationRequirement
{
    IRFunc* function = nullptr;
    SlangNVVMTextureOperationDesc operations[3] = {};
    uint32_t operationCount = 0;
    uint32_t outputParameterCount = 0;
    bool writesTrailingZero = false;
    const char* diagnosticName = nullptr;
};

/// Owns every provider capability required before module creation.
struct NVVMOperationRequirements
{
    NVVMValueOperationRequirements valueOperations;
    List<NVVMSurfaceOperationRequirement> surfaceOperations;
    List<NVVMTextureOperationRequirement> textureOperations;
};

/// Replaces exact CUDA layout-query calls with constants and removes their compile-time-only IR.
SlangResult foldNVVMCompileTimeLayoutQueries(CodeGenContext* codeGenContext, LinkedIR& linkedIR);

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
