#pragma once

#include "compiler-core/slang-nvvm-ir-builder.h"
#include "core/slang-dictionary.h"
#include "core/slang-list.h"
#include "slang-ir-link.h"

namespace Slang
{

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

struct NVVMPlannedValueOperation
{
    IRInst* source = nullptr;
    NVVMValueOperationRequirement operation;
};

struct NVVMPlannedUInt64WordConstruction
{
    IRInst* source = nullptr;
    IRInst* lowWord = nullptr;
    IRInst* highWord = nullptr;
    NVVMValueRecipeStep wordConversion;
    NVVMValueRecipeStep highWordShift;
    NVVMValueRecipeStep combine;
};

struct NVVMPlannedNumericTruthiness
{
    IRInst* source = nullptr;
    IRInst* value = nullptr;
    SlangNVVMValueTypeDesc valueType = {};
    NVVMValueRecipeStep comparison;
};

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

enum class NVVMPlannedDefaultResourceValueKind
{
    RawStructuredBuffer,
    DescriptorHandle,
};

struct NVVMPlannedDefaultResourceValue
{
    IRInst* source = nullptr;
    IRType* resultType = nullptr;
    IRType* structuredElementType = nullptr;
    NVVMPlannedDefaultResourceValueKind kind =
        NVVMPlannedDefaultResourceValueKind::RawStructuredBuffer;
};

enum class NVVMPlannedEphemeralValueKind
{
    ChosenUndefined,
    StableStringHash,
    IgnoredDebugNoScope,
};

struct NVVMPlannedEphemeralValue
{
    IRInst* source = nullptr;
    NVVMPlannedEphemeralValueKind kind = NVVMPlannedEphemeralValueKind::ChosenUndefined;
    IRType* valueType = nullptr;
    IRStringLit* stringLiteral = nullptr;
};

struct NVVMPlannedSurfaceOperation
{
    IRInst* source = nullptr;
    SlangNVVMSurfaceOperationDesc desc = {};
    IRInst* surface = nullptr;
    IRInst* coordinate = nullptr;
    IRInst* value = nullptr;
    const char* diagnosticName = nullptr;
};

struct NVVMPlannedAtomicOperation
{
    IRInst* source = nullptr;
    SlangNVVMAtomicOperationDesc desc = {};
    IRInst* pointer = nullptr;
    IRInst* values[2] = {};
    uint32_t valueCount = 0;
    NVVMValueRecipeStep valueNegation;
    int64_t implicitValue = 0;
    bool hasImplicitValue = false;
    bool negatesValue = false;
    const char* diagnosticName = nullptr;
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
    List<NVVMPlannedDefaultResourceValue> defaultResourceValues;
    List<NVVMPlannedEphemeralValue> ephemeralValues;
    List<NVVMPlannedSurfaceOperation> surfaceOperations;
    List<NVVMPlannedAtomicOperation> atomicOperations;
};

struct NVVMAtomicOperationRequirement
{
    SlangNVVMAtomicOperationDesc desc = {};
    const char* diagnosticName = nullptr;
};

struct NVVMSurfaceOperationRequirement
{
    IRInst* source = nullptr;
    SlangNVVMSurfaceOperationDesc desc = {};
    const char* diagnosticName = nullptr;
};

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

/// Indexes one immutable plan and rejects duplicate or missing source keys at initialization.
class NVVMEmissionPlanIndex
{
public:
    void initialize(const NVVMEmissionPlan& plan);

    const NVVMPlannedValueOperation* findValueOperation(IRInst* source) const;
    const NVVMPlannedUInt64WordConstruction* findUInt64WordConstruction(IRInst* source) const;
    const NVVMPlannedNumericTruthiness* findNumericTruthiness(IRInst* source) const;
    const NVVMPlannedFloatingRemainder* findFloatingRemainder(IRInst* source) const;
    const NVVMPlannedBitfieldOperation* findBitfieldOperation(IRInst* source) const;
    const NVVMPlannedDefaultResourceValue* findDefaultResourceValue(IRInst* source) const;
    const NVVMPlannedEphemeralValue* findEphemeralValue(IRInst* source) const;
    const NVVMPlannedSurfaceOperation* findSurfaceOperation(IRInst* source) const;
    const NVVMPlannedAtomicOperation* findAtomicOperation(IRInst* source) const;

private:
    const NVVMEmissionPlan* m_plan = nullptr;
    Dictionary<IRInst*, Index> m_valueOperations;
    Dictionary<IRInst*, Index> m_uint64WordConstructions;
    Dictionary<IRInst*, Index> m_numericTruthinessOperations;
    Dictionary<IRInst*, Index> m_floatingRemainderOperations;
    Dictionary<IRInst*, Index> m_bitfieldOperations;
    Dictionary<IRInst*, Index> m_defaultResourceValues;
    Dictionary<IRInst*, Index> m_ephemeralValues;
    Dictionary<IRInst*, Index> m_surfaceOperations;
    Dictionary<IRInst*, Index> m_atomicOperations;
};

} // namespace Slang
