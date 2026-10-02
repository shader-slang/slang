#pragma once

#include "compiler-core/slang-nvvm-ir-builder.h"
#include "core/slang-dictionary.h"
#include "core/slang-list.h"
#include "slang-emit-nvvm-type-lowering.h"
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

/// Retains the typed view's element stride; the source extent is measured in bytes.
struct NVVMPlannedEquivalentStructuredBuffer
{
    IRInst* buffer = nullptr;
    IRType* elementType = nullptr;
    uint32_t elementStride = 0;
};

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

enum class NVVMPlannedResourceBitCastKind
{
    OpaqueHandle64,
    RawBuffer,
};

/// Owns one exact AnyValue/resource bit-transport decision made during preflight.
struct NVVMPlannedResourceBitCast
{
    IRInst* source = nullptr;
    IRInst* value = nullptr;
    IRType* resourceValueType = nullptr;
    IRVectorType* payloadType = nullptr;
    IRType* rawBufferElementType = nullptr;
    NVVMPlannedResourceBitCastKind kind = NVVMPlannedResourceBitCastKind::OpaqueHandle64;
    bool rawBufferIsByteAddress = false;
    bool rawBufferElementUsesStructuredStorage = false;
    bool resultIsResourceValue = false;
    NVVMValueRecipeStep steps[3] = {};
    uint32_t stepCount = 0;
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

/// Records the physical conversion selected for one already-admitted memory operation.
/// Read-only access is independent of representation: a borrowed float3 keeps its native vector,
/// while a parameter-group float3 uses compact component storage.
enum class NVVMStorageConversionKind
{
    Identity,
    StructuredBuffer,
    BFloat16Vector,
    CompactVector,
    CompactHalfVector,
};

struct NVVMPlannedStorageConversion
{
    NVVMStorageConversionKind kind = NVVMStorageConversionKind::Identity;
    IRType* type = nullptr;
    uint32_t laneCount = 0;
    NVVMTypeUse resultUse = NVVMTypeUse::Value;
    Index structuredRecipe = -1;
};

enum class NVVMStructuredConversionKind
{
    Identity,
    Boolean,
    Elements,
};

/// Executes one checked external-storage boundary over canonical IR types. Child indices refer
/// to owned recipes, so recursive planning never retains references across list growth.
struct NVVMStructuredConversionRecipe
{
    IRType* type = nullptr;
    NVVMStructuredConversionKind kind = NVVMStructuredConversionKind::Identity;
    bool storageToValue = false;
    bool extractAggregate = false;
    bool constructAggregate = false;
    List<Index> children;
};

/// Retains the original resource view and the complete direct-load emission decision.
struct NVVMPlannedStructuredLoad
{
    IRInst* source = nullptr;
    IRInst* buffer = nullptr;
    IRInst* elementIndex = nullptr;
    NVVMRawBufferType bufferType;
    IRType* resultType = nullptr;
    uint32_t alignment = 0;
    SlangNVVMLoadFlags flags = SLANG_NVVM_LOAD_FLAG_NONE;
    NVVMPlannedStorageConversion conversion;
};

struct NVVMPlannedAggregateStorageConstruction
{
    IRInst* source = nullptr;
    Index elementRecipe = -1;
};

/// Owns the admitted local allocation role and its proven physical alignment.
struct NVVMPlannedLocalStorage
{
    IRInst* source = nullptr;
    IRType* valueType = nullptr;
    NVVMTypeUse valueUse = NVVMTypeUse::Value;
    uint32_t alignment = 0;
};

/// Selects the finite scalar lanes stored at a checked explicit-layout field address.
/// Logical Bool has a layout-selected integer width; vectors use scalar lanes without padding.
struct NVVMLayoutStorage
{
    IRType* valueType = nullptr;
    IRType* scalarType = nullptr;
    uint32_t scalarSize = 0;
    uint32_t alignment = 0;
    uint32_t laneCount = 0;
};

/// Owns a load's storage conversion, alignment, flags and resulting pointer provenance.
struct NVVMPlannedLoad
{
    NVVMCUDAValueLayout cudaValueLayout;
    IRInst* source = nullptr;
    IRInst* pointer = nullptr;
    NVVMPlannedStorageConversion conversion;
    NVVMLayoutStorage layoutStorage;
    NVVMValueRecipeStep layoutBoolConversion;
    uint32_t alignment = 0;
    SlangNVVMLoadFlags flags = SLANG_NVVM_LOAD_FLAG_NONE;
    bool isGlobalUserPointer = false;
    bool isLayoutPointerRoot = false;
    bool isScoped = false;
    SlangNVVMMemoryOperationDesc memoryOperation = {};
};

/// Owns a store's storage conversion and pointer-value ABI choice before provider mutation.
struct NVVMPlannedStore
{
    IRInst* source = nullptr;
    IRInst* pointer = nullptr;
    IRInst* value = nullptr;
    NVVMPlannedStorageConversion conversion;
    NVVMLayoutStorage layoutStorage;
    NVVMValueRecipeStep layoutBoolConversion;
    uint32_t alignment = 0;
    bool usesHelperPointerValue = false;
    bool isScoped = false;
    SlangNVVMMemoryOperationDesc memoryOperation = {};
};

/// Records a field selected by canonical IR key and the admitted storage/access roles of its root.
/// Read-only access does not imply parameter-group storage or immutable-location load metadata.
struct NVVMStructFieldSelection
{
    IRStructField* field = nullptr;
    uint32_t fieldIndex = 0;
    bool isConventionalGlobal = false;
    bool isMutable = false;
    bool isPhysicalStorage = false;
    bool isParameterGroupStorage = false;
    bool isLocalSubstandardRecordStorage = false;
};

/// Owns one checked field selection. The IR pointer type remains the address-space authority.
struct NVVMPlannedFieldAddress
{
    IRInst* source = nullptr;
    IRInst* base = nullptr;
    IRInst* root = nullptr;
    NVVMStructFieldSelection selection;
    bool isLayoutStorage = false;
    uint64_t byteOffset = 0;
    NVVMLayoutStorage layoutStorage;
};

struct NVVMRawBufferDataPointer
{
    IRInst* source = nullptr;
    IRInst* buffer = nullptr;
    NVVMRawBufferType bufferType;
    NVVMBufferDataPointerType resultType;
};

struct NVVMStructuredBufferElementPointer
{
    IRInst* source = nullptr;
    IRInst* buffer = nullptr;
    IRInst* elementIndex = nullptr;
    NVVMRawBufferType bufferType;
    IRPtrTypeBase* resultType = nullptr;
};

enum class NVVMElementAddressKind
{
    // Local construction state; only completed records enter the address plan.
    Pending,
    RawBuffer,
    Sequential,
    DeviceArray,
};

/// Owns one checked index relation and its provider recipe, retaining semantic storage provenance.
struct NVVMPlannedElementAddress
{
    IRInst* source = nullptr;
    IRInst* base = nullptr;
    IRInst* index = nullptr;
    IRInst* root = nullptr;
    IRType* aggregateType = nullptr;
    IRPtrTypeBase* resultType = nullptr;
    NVVMElementAddressKind kind = NVVMElementAddressKind::Pending;
    bool isLayoutStorage = false;
    uint64_t byteOffset = 0;
    NVVMLayoutStorage layoutStorage;
    bool isReadOnly = false;
    bool isParameterGroupStorage = false;
    bool isLocalSubstandardRecordStorage = false;
    bool propagatesGlobalUserPointer = false;
    const char* diagnosticName = nullptr;
};

/// Indexes address proofs as preflight records them, so ordinary pointer uses never scan the
/// module. Indices remain stable when the owned lists grow; canonical IR identity is the only
/// lookup key.
class NVVMAddressPlan
{
public:
    void addFieldAddress(const NVVMPlannedFieldAddress& address);
    void addElementAddress(const NVVMPlannedElementAddress& address);
    void addDataPointer(const NVVMRawBufferDataPointer& address);
    void addStructuredElement(const NVVMStructuredBufferElementPointer& address);
    const NVVMPlannedFieldAddress* findFieldAddress(IRInst* source) const;
    const NVVMPlannedElementAddress* findElementAddress(IRInst* source) const;
    const NVVMRawBufferDataPointer* findDataPointer(IRInst* source) const;
    const NVVMStructuredBufferElementPointer* findStructuredElement(IRInst* source) const;
    IRInst* getRoot(IRInst* source) const;
    const NVVMRawBufferType* findRootBuffer(IRInst* source) const;

private:
    List<NVVMPlannedFieldAddress> m_fieldAddresses;
    List<NVVMPlannedElementAddress> m_elementAddresses;
    List<NVVMRawBufferDataPointer> m_dataPointers;
    List<NVVMStructuredBufferElementPointer> m_structuredElements;
    Dictionary<IRInst*, Index> m_fieldAddressIndices;
    Dictionary<IRInst*, Index> m_elementAddressIndices;
    Dictionary<IRInst*, Index> m_dataPointerIndices;
    Dictionary<IRInst*, Index> m_structuredElementIndices;
};

/// Retains the exact source name and signature validated before module creation.
struct NVVMPlannedNamedIntrinsic
{
    bool isDeviceLibraryFunction = false;
    IRInst* source = nullptr;
    String name;
    SlangNVVMValueTypeDesc resultType = {};
    List<SlangNVVMNamedIntrinsicOperandDesc> operands;
    List<IRInst*> operandValues;

    // Construct the borrowed view at use time because moving a plan can move its owned storage.
    SlangNVVMNamedIntrinsicDesc getDesc() const
    {
        SLANG_ASSERT(operands.getCount() == operandValues.getCount());
        return {
            name.getBuffer(),
            size_t(name.getLength()),
            resultType,
            operands.getBuffer(),
            size_t(operands.getCount())};
    }
};

/// Retains the original payload admission and all scalar operands for one register trace.
struct NVVMPlannedTraceRay
{
    IRInst* source = nullptr;
    SlangNVVMTraceRayDesc desc = {};
    List<IRInst*> operands;
};

/// Retains a selected object reference and typed operands; SDK state stays provider-private.
struct NVVMPlannedHitObjectOperation
{
    IRInst* source = nullptr;
    IRType* payloadType = nullptr;
    SlangNVVMHitObjectOperationDesc desc = {};
    List<IRInst*> operands;
};

/// Retains the checked callable index and optional numeric copy-in/out payload.
struct NVVMPlannedCallable
{
    IRInst* source = nullptr;
    IRInst* index = nullptr;
    IRInst* payload = nullptr;
};

/// Retains a checked affine row; a null handle selects the current ray's complete transform list.
struct NVVMPlannedInstanceTransform
{
    IRInst* source = nullptr;
    IRInst* handle = nullptr;
    SlangNVVMInstanceTransformDesc desc = {};
};

/// Retains the authoritative storage stride and proven parameter/load root for an address offset.
struct NVVMPlannedLayoutPointerOffset
{
    IRInst* base = nullptr;
    IRInst* index = nullptr;
    IRInst* root = nullptr;
    IRPtrTypeBase* resultType = nullptr;
    uint64_t stride = 0;
    NVVMValueRecipeStep widenIndex;
    NVVMValueRecipeStep scaleIndex;
};

/// Owns stable module decisions produced by preflight and consumed without reclassification.
struct NVVMEmissionPlan
{
    NVVMAddressPlan addresses;
    Dictionary<IRInst*, NVVMCUDAValueLayout> entryValueParameters;
    Dictionary<IRInst*, NVVMPlannedEquivalentStructuredBuffer> equivalentStructuredBuffers;
    Dictionary<IRInst*, NVVMPlannedLayoutPointerOffset> layoutPointerOffsets;
    Dictionary<IRInst*, IRInst*> pointerToIntegerValues;
    List<NVVMPlannedLocalStorage> localStorage;
    List<NVVMPlannedLoad> loads;
    List<NVVMPlannedStore> stores;
    List<NVVMPlannedStructuredLoad> structuredLoads;
    List<NVVMStructuredConversionRecipe> structuredConversions;
    List<NVVMPlannedAggregateStorageConstruction> aggregateStorageConstructions;
    List<IRFunc*> functions;
    List<String> functionNames;
    List<NVVMPlannedValueOperation> valueOperations;
    List<NVVMPlannedNamedIntrinsic> namedIntrinsics;
    List<NVVMPlannedTraceRay> traceRays;
    List<NVVMPlannedHitObjectOperation> hitObjectOperations;
    List<NVVMPlannedInstanceTransform> instanceTransforms;
    List<NVVMPlannedCallable> callables;
    List<NVVMPlannedUInt64WordConstruction> uint64WordConstructions;
    List<NVVMPlannedNumericTruthiness> numericTruthinessOperations;
    List<NVVMPlannedFloatingRemainder> floatingRemainderOperations;
    List<NVVMPlannedBitfieldOperation> bitfieldOperations;
    List<NVVMPlannedResourceBitCast> resourceBitCasts;
    List<NVVMPlannedDefaultResourceValue> defaultResourceValues;
    List<NVVMPlannedEphemeralValue> ephemeralValues;
    List<NVVMPlannedSurfaceOperation> surfaceOperations;
    List<NVVMPlannedAtomicOperation> atomicOperations;
    // GetOffsetPtr has no field/element record. Retain its checked physical space at planning.
    Dictionary<IRInst*, SlangNVVMAddressSpace> scopedOffsetSpaces;
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
    IRInst* source = nullptr;
    IRInst* texture = nullptr;
    IRInst* coordinate = nullptr;
    IRInst* level = nullptr;
    SlangNVVMTextureOperationDesc operations[3] = {};
    uint32_t operationCount = 0;
    const char* diagnosticName = nullptr;
};

/// Owns every provider capability required before module creation.
struct NVVMOperationRequirements
{
    uint32_t optixVersion = 90000;
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

    const NVVMPlannedFieldAddress* findFieldAddress(IRInst* source) const;
    const NVVMPlannedElementAddress* findElementAddress(IRInst* source) const;
    const NVVMPlannedLocalStorage* findLocalStorage(IRInst* source) const;
    const NVVMPlannedLoad* findLoad(IRInst* source) const;
    const NVVMPlannedStore* findStore(IRInst* source) const;
    const NVVMPlannedStructuredLoad* findStructuredLoad(IRInst* source) const;
    const NVVMPlannedAggregateStorageConstruction* findAggregateStorageConstruction(
        IRInst* source) const;
    const NVVMPlannedNamedIntrinsic* findNamedIntrinsic(IRInst* source) const;
    const NVVMPlannedTraceRay* findTraceRay(IRInst* source) const;
    const NVVMPlannedHitObjectOperation* findHitObjectOperation(IRInst* source) const;
    const NVVMPlannedInstanceTransform* findInstanceTransform(IRInst* source) const;
    const NVVMPlannedCallable* findCallable(IRInst* source) const;
    const NVVMPlannedValueOperation* findValueOperation(IRInst* source) const;
    const NVVMPlannedUInt64WordConstruction* findUInt64WordConstruction(IRInst* source) const;
    const NVVMPlannedNumericTruthiness* findNumericTruthiness(IRInst* source) const;
    const NVVMPlannedFloatingRemainder* findFloatingRemainder(IRInst* source) const;
    const NVVMPlannedBitfieldOperation* findBitfieldOperation(IRInst* source) const;
    const NVVMPlannedResourceBitCast* findResourceBitCast(IRInst* source) const;
    const NVVMPlannedDefaultResourceValue* findDefaultResourceValue(IRInst* source) const;
    const NVVMPlannedEphemeralValue* findEphemeralValue(IRInst* source) const;
    const NVVMPlannedSurfaceOperation* findSurfaceOperation(IRInst* source) const;
    const NVVMPlannedAtomicOperation* findAtomicOperation(IRInst* source) const;

private:
    const NVVMEmissionPlan* m_plan = nullptr;
    Dictionary<IRInst*, Index> m_localStorage;
    Dictionary<IRInst*, Index> m_loads;
    Dictionary<IRInst*, Index> m_stores;
    Dictionary<IRInst*, Index> m_structuredLoads;
    Dictionary<IRInst*, Index> m_aggregateStorageConstructions;
    Dictionary<IRInst*, Index> m_valueOperations;
    Dictionary<IRInst*, Index> m_namedIntrinsics;
    Dictionary<IRInst*, Index> m_traceRays;
    Dictionary<IRInst*, Index> m_hitObjectOperations;
    Dictionary<IRInst*, Index> m_instanceTransforms;
    Dictionary<IRInst*, Index> m_callables;
    Dictionary<IRInst*, Index> m_uint64WordConstructions;
    Dictionary<IRInst*, Index> m_numericTruthinessOperations;
    Dictionary<IRInst*, Index> m_floatingRemainderOperations;
    Dictionary<IRInst*, Index> m_bitfieldOperations;
    Dictionary<IRInst*, Index> m_resourceBitCasts;
    Dictionary<IRInst*, Index> m_defaultResourceValues;
    Dictionary<IRInst*, Index> m_ephemeralValues;
    Dictionary<IRInst*, Index> m_surfaceOperations;
    Dictionary<IRInst*, Index> m_atomicOperations;
};

} // namespace Slang
