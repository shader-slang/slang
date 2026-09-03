#pragma once

#include "compiler-core/slang-nvvm-ir-builder.h"
#include "core/slang-list.h"
#include "slang-ir-link.h"

namespace Slang
{

struct CodeGenContext;

/// Owns one queried scalar operation in a compiler-owned compound emission recipe.
struct NVVMValueRecipeStep
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

/// Owns the resolved direct provider operation for one canonical value-producing instruction.
struct NVVMPlannedValueOperation
{
    IRInst* source = nullptr;
    NVVMValueOperationRequirement operation;
};

/// Owns the canonical low-word/high-word reconstruction selected for one source instruction.
struct NVVMPlannedUInt64WordConstruction
{
    IRInst* source = nullptr;
    IRInst* lowWord = nullptr;
    IRInst* highWord = nullptr;
    NVVMValueRecipeStep wordConversion;
    NVVMValueRecipeStep highWordShift;
    NVVMValueRecipeStep combine;
};

/// Owns the numeric-to-Boolean comparison selected for one canonical cast.
struct NVVMPlannedNumericTruthiness
{
    IRInst* source = nullptr;
    IRInst* value = nullptr;
    SlangNVVMValueTypeDesc valueType = {};
    NVVMValueRecipeStep comparison;
};

/// Owns the CUDA floating-remainder recipe selected for one canonical `FRem`.
struct NVVMPlannedFloatingRemainder
{
    IRInst* source = nullptr;
    IRInst* operands[2] = {};
    bool operandIsVector[2] = {};
    IRType* resultType = nullptr;
    IRType* scalarType = nullptr;
    uint32_t laneCount = 0;
    NVVMValueRecipeStep scalarStep;
};

enum class NVVMPlannedBitfieldOperationKind
{
    None,
    Extract,
    Insert,
};

/// Owns the finite typed recipe selected for one canonical bitfield instruction.
struct NVVMPlannedBitfieldOperation
{
    IRInst* source = nullptr;
    NVVMPlannedBitfieldOperationKind kind = NVVMPlannedBitfieldOperationKind::None;
    IRInst* value = nullptr;
    IRInst* insertedValue = nullptr;
    IRInst* offset = nullptr;
    IRInst* count = nullptr;
    IRType* dataIRType = nullptr;
    SlangNVVMValueTypeDesc dataType = {};
    SlangNVVMValueTypeDesc unsignedDataType = {};
    SlangNVVMValueTypeDesc unsignedScalarType = {};
    bool needsCountConversion = false;
    bool isSigned = false;
    NVVMValueRecipeStep countConversion;
    NVVMValueRecipeStep toUnsigned;
    NVVMValueRecipeStep toSigned;
    NVVMValueRecipeStep subtract;
    NVVMValueRecipeStep shiftLeft;
    NVVMValueRecipeStep logicalShiftRight;
    NVVMValueRecipeStep signedShiftRight;
    NVVMValueRecipeStep bitAnd;
    NVVMValueRecipeStep bitOr;
    NVVMValueRecipeStep bitNot;
};

/// Owns stable module decisions produced by preflight and consumed without reclassification.
struct NVVMEmissionPlan
{
    List<IRFunc*> functions;
    List<String> functionNames;
    List<NVVMPlannedValueOperation> valueOperations;
    List<NVVMPlannedUInt64WordConstruction> uint64WordConstructions;
    List<NVVMPlannedNumericTruthiness> numericTruthinessOperations;
    List<NVVMPlannedFloatingRemainder> floatingRemainderOperations;
    List<NVVMPlannedBitfieldOperation> bitfieldOperations;
};

/// Owns one exact typed atomic-operation overload required by accepted linked IR.
struct NVVMAtomicOperationRequirement
{
    SlangNVVMAtomicOperationDesc desc = {};
    const char* diagnosticName = nullptr;
};

/// Owns one exact typed surface-operation overload required by accepted linked IR.
struct NVVMSurfaceOperationRequirement
{
    IRInst* source = nullptr;
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
    List<NVVMAtomicOperationRequirement> atomicOperations;
    List<NVVMSurfaceOperationRequirement> surfaceOperations;
    List<NVVMTextureOperationRequirement> textureOperations;
    bool requiresCUDADeviceLibrary = false;
    NVVMEmissionPlan emissionPlan;
};

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
