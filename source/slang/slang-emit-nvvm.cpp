#include "slang-emit-nvvm.h"

#include "compiler-core/slang-artifact-impl.h"
#include "compiler-core/slang-artifact-util.h"
#include "compiler-core/slang-nvvm-semantic-catalog.h"
#include "core/slang-dictionary.h"
#include "core/slang-math.h"
#include "slang-code-gen.h"
#include "slang-diagnostics.h"
#include "slang-emit-nvvm-type-lowering.h"
#include "slang-ir-dce.h"
#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-legalize-varying-params.h"
#include "slang-ir-lower-buffer-element-type.h"
#include "slang-ir-string-hash.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
namespace
{

static const uint32_t kNVVMScalar32Alignment = 4;
static const IRIntegerValue kNVVMI32Min = -2147483647 - 1;
static const IRIntegerValue kNVVMI32Max = 2147483647;
static const IRIntegerValue kNVVMUInt32Max = 4294967295;
static const uint32_t kNVVMPointerAlignment = 8;
static const SlangNVVMValueTypeDesc kNVVMUnsignedI8 = {
    SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
    8,
    1,
};
static const SlangNVVMValueTypeDesc kNVVMStructuredBoolLoadOperands[] = {
    kNVVMUnsignedI8,
    kNVVMUnsignedI8,
};
static const SlangNVVMValueOperationDesc kNVVMStructuredBoolLoadOperation = {
    SLANG_NVVM_VALUE_OP_NOT_EQUAL,
    NVVMSemantics::kBool,
    kNVVMStructuredBoolLoadOperands,
    SLANG_COUNT_OF(kNVVMStructuredBoolLoadOperands),
};
static const SlangNVVMValueTypeDesc kNVVMStructuredBoolStoreOperands[] = {
    NVVMSemantics::kBool,
};
static const SlangNVVMValueOperationDesc kNVVMStructuredBoolStoreOperation = {
    SLANG_NVVM_VALUE_OP_INTEGER_CONVERT,
    kNVVMUnsignedI8,
    kNVVMStructuredBoolStoreOperands,
    SLANG_COUNT_OF(kNVVMStructuredBoolStoreOperands),
};

// Identifies a bare local BF16 vector or a field in a qualified local BF16 record. A field must
// retain its admitted producer; a device/resource pointer cannot acquire storage by pointee alone.
IRVectorType* _getNVVMLocalBFloat16VectorPointer(const NVVMAddressPlan& addresses, IRInst* pointer);

// Matches the prelude's native BF2 and component-struct BF3/BF4 storage. This alignment describes
// memory only; BF3/BF4 register vectors have stronger LLVM allocation alignment.
uint32_t _getNVVMBFloat16VectorStorageAlignment(IRType* type)
{
    uint32_t count = 0;
    SLANG_RELEASE_ASSERT(asNVVMBFloat16VectorType(type, &count));
    return count == 2 ? 4 : 2;
}

// Returns the natural alignment of every first-class value admitted by the direct backend.
uint32_t _getNVVMExecutableValueAlignment(IRInst* type)
{
    if (isNVVMHitObjectStorageType(type))
        return 1;
    if (asNVVMHitObjectPointerType(type))
        return kNVVMPointerAlignment;
    if (const uint32_t helperAlignment = getNVVMHelperValueAlignment(type))
        return helperAlignment;
    if (auto arrayType = asNVVMSupportedLocalSubstandardRecordArrayType(type))
        return getNVVMHelperValueAlignment(arrayType->getElementType());
    return getNVVMResourceValueAlignment(type);
}
// Owns the exact scalar/vector bitcast descriptor used by both preflight and emission.
// The operand descriptor remains valid while getDesc() is consumed by either caller.
struct NVVMHalfHelperABIOperation
{
    NVVMHalfHelperABIOperation(IRType* canonicalType, bool toPhysical)
    {
        const uint32_t laneCount = getNVVMHalfHelperABILaneCount(canonicalType);
        SLANG_RELEASE_ASSERT(laneCount);
        operandType = toPhysical ? NVVMSemantics::kFloat16 : NVVMSemantics::kUnsignedI16;
        resultType = toPhysical ? NVVMSemantics::kUnsignedI16 : NVVMSemantics::kFloat16;
        operandType.laneCount = laneCount;
        resultType.laneCount = laneCount;
    }

    SlangNVVMValueOperationDesc getDesc() const
    {
        return {SLANG_NVVM_VALUE_OP_BIT_REINTERPRET, resultType, &operandType, 1};
    }

    SlangNVVMValueTypeDesc operandType;
    SlangNVVMValueTypeDesc resultType;
};
static const SlangNVVMValueTypeDesc kNVVMRawBufferCountConversionOperands[] = {
    NVVMSemantics::kUnsignedI64,
};
static const SlangNVVMValueOperationDesc kNVVMRawBufferCountConversion = {
    SLANG_NVVM_VALUE_OP_INTEGER_CONVERT,
    NVVMSemantics::kUnsignedI32,
    kNVVMRawBufferCountConversionOperands,
    SLANG_COUNT_OF(kNVVMRawBufferCountConversionOperands),
};

// Byte views store byte extents; typed views store floor(byteExtent / elementStride).
static const SlangNVVMValueTypeDesc kNVVMByteViewCountOperands[] = {
    NVVMSemantics::kUnsignedI64,
    NVVMSemantics::kUnsignedI64,
};
static const SlangNVVMValueOperationDesc kNVVMByteViewCountDivision = {
    SLANG_NVVM_VALUE_OP_DIVIDE,
    NVVMSemantics::kUnsignedI64,
    kNVVMByteViewCountOperands,
    SLANG_COUNT_OF(kNVVMByteViewCountOperands),
};

bool _getNVVMStructuredBufferStorageLayout(
    CodeGenContext* codeGenContext,
    IRType* type,
    IRSizeAndAlignment& outLayout);

struct NVVMConventionalGlobalParams
{
    IRGlobalParam* globalParam = nullptr;
    IRStructType* elementType = nullptr;
};

// Recognizes the canonical collected CUDA parameter block by its producer-owned shape. Field
// support is validated separately: an unsupported sibling must not make an otherwise canonical
// field address lose its provenance before module validation can name that sibling's exact type.
bool _getNVVMConventionalGlobalParams(IRInst* inst, NVVMConventionalGlobalParams& outParams)
{
    outParams = {};
    auto globalParam = as<IRGlobalParam>(inst);
    auto constantBufferType =
        globalParam ? as<IRConstantBufferType>(globalParam->getDataType()) : nullptr;
    auto elementType =
        constantBufferType ? as<IRStructType>(constantBufferType->getElementType()) : nullptr;
    if (!globalParam || !elementType ||
        !elementType->findDecoration<IRSynthesizedParameterGroupDecoration>())
    {
        return false;
    }

    bool hasField = false;
    for (auto field : elementType->getFields())
    {
        SLANG_UNUSED(field);
        hasField = true;
    }
    if (!hasField)
        return false;

    outParams = {globalParam, elementType};
    return true;
}

// Finds a field by semantic key and returns its actual ABI position. The global collector can move
// CUDA fields, so every aggregate address uses key identity instead of source declaration order.
bool _findNVVMStructField(
    IRStructType* structType,
    IRInst* key,
    IRStructField*& outField,
    uint32_t& outFieldIndex)
{
    outField = nullptr;
    outFieldIndex = 0;
    uint32_t fieldIndex = 0;
    for (auto field : structType->getFields())
    {
        if (field->getKey() == key)
        {
            outField = field;
            outFieldIndex = fieldIndex;
            return true;
        }
        ++fieldIndex;
    }
    return false;
}

// Returns whether a retained struct declaration is an exact storage type owned by the accepted
// conventional CUDA parameter block.
bool _isNVVMConventionalGlobalStorageType(const NVVMConventionalGlobalParams& params, IRInst* inst)
{
    // A raw CUDA kernel can retain by-value struct declarations without having a collected global
    // parameter block. In that case there are no conventional-global storage types to recognize.
    if (!params.elementType)
        return false;

    if (inst == params.elementType)
        return true;
    for (auto field : params.elementType->getFields())
    {
        IRType* parameterGroupElementType = nullptr;
        if (asNVVMSupportedParameterGroupType(field->getFieldType(), &parameterGroupElementType) &&
            inst == parameterGroupElementType)
        {
            return true;
        }
    }
    return false;
}

// Proves that this array pointer comes from mutable local storage or an internal out/inout/readonly
// parameter. For example, forwarding `out Payload values[2]` to an inout helper preserves the
// same storage; a matching pointer on a global, block parameter or external function does not.
IRPtrTypeBase* _getNVVMLocalSubstandardRecordArrayPointer(IRInst* value)
{
    if (auto parameter = as<IRParam>(value))
    {
        auto block = as<IRBlock>(parameter->getParent());
        auto function = block ? as<IRFunc>(block->getParent()) : nullptr;
        if (!function || !function->isDefinition() || block != function->getFirstBlock() ||
            function->findDecoration<IREntryPointDecoration>() ||
            function->findDecoration<IRCudaKernelDecoration>() ||
            function->findDecorationImpl(kIROp_CudaDeviceExportDecoration))
            return nullptr;
        return asNVVMSupportedLocalRecordArrayReferenceType(parameter->getDataType());
    }
    auto pointerType =
        value && value->getOp() == kIROp_Var ? as<IRPtrTypeBase>(value->getDataType()) : nullptr;
    return pointerType && pointerType->getOp() == kIROp_PtrType &&
                   pointerType->getOperandCount() == 1 &&
                   pointerType->getAddressSpace() == AddressSpace::Generic &&
                   pointerType->getAccessQualifier() == AccessQualifier::ReadWrite &&
                   asNVVMSupportedLocalSubstandardRecordArrayType(pointerType->getValueType())
               ? pointerType
               : nullptr;
}

// A noncopyable result is a caller-owned out parameter, not a global handle. Descendants
// use existing checked element records; no arbitrary pointer graph can manufacture a root.
IRPtrTypeBase* _getNVVMHitObjectPointer(const NVVMAddressPlan& addresses, IRInst* value)
{
    auto pointer = value ? asNVVMHitObjectPointerType(value->getDataType()) : nullptr;
    if (!pointer)
        return nullptr;
    auto root = addresses.getRoot(value);
    auto block = root ? as<IRBlock>(root->getParent()) : nullptr;
    auto function = block ? as<IRFunc>(block->getParent()) : nullptr;
    if (!function || !function->isDefinition())
        return nullptr;
    if (root->getOp() == kIROp_Var)
        return pointer;
    if (as<IRParam>(root) && block == function->getFirstBlock() &&
        !function->findDecoration<IREntryPointDecoration>() &&
        !function->findDecoration<IRCudaKernelDecoration>() &&
        !function->findDecorationImpl(kIROp_CudaDeviceExportDecoration))
        return pointer;
    return nullptr;
}

// Returns a value reference from a variable, helper parameter/result, or its checked child.
// For `initialize(out wrapper.textures[i])`, the explicit child pointer spelling is not sufficient:
// the address plan must retain the local wrapper root and its access restriction.
IRPtrTypeBase* _getNVVMLocalValueReference(
    const NVVMAddressPlan& addresses,
    IRInst* value,
    bool* outIsReadOnly = nullptr)
{
    if (!value)
        return nullptr;
    auto field = addresses.findFieldAddress(value);
    auto element = addresses.findElementAddress(value);
    if ((value->getOp() == kIROp_FieldAddress && !field) ||
        (value->getOp() == kIROp_GetElementPtr && !element))
        return nullptr;
    auto root = addresses.getRoot(value);
    auto rootPointer = asNVVMSupportedLocalResourceValuePointerType(root->getDataType());
    if (!rootPointer)
        rootPointer = asNVVMSupportedLocalHelperValuePointerType(root->getDataType());
    if (!rootPointer)
        rootPointer = asNVVMSupportedLocalCopyableValuePointerType(root->getDataType());
    if (!rootPointer)
        rootPointer = asNVVMSupportedHelperReferencePointerType(root->getDataType());
    auto block = as<IRBlock>(root->getParent());
    auto function = block ? as<IRFunc>(block->getParent()) : nullptr;
    auto call = as<IRCall>(root);
    auto callee = call ? as<IRFunc>(call->getCallee()) : nullptr;
    const bool isHelperResult =
        callee && callee->getFirstBlock() && !callee->findDecoration<IREntryPointDecoration>() &&
        !callee->findDecoration<IRCudaKernelDecoration>() &&
        isTypeEqual(root->getDataType(), callee->getResultType()) &&
        classifyNVVMType(root->getDataType()).supports(NVVMTypeUse::HelperResult);
    if (!rootPointer || !function ||
        (root->getOp() != kIROp_Var && root->getOp() != kIROp_PtrCast && !isHelperResult &&
         (!as<IRParam>(root) || block != function->getFirstBlock() ||
          function->findDecoration<IREntryPointDecoration>() ||
          function->findDecoration<IRCudaKernelDecoration>())))
        return nullptr;
    if (outIsReadOnly)
        *outIsReadOnly = rootPointer->getAccessQualifier() == AccessQualifier::Read ||
                         (field && !field->selection.isMutable) || (element && element->isReadOnly);
    return as<IRPtrTypeBase>(value->getDataType());
}

struct NVVMSequentialElementPointer
{
    IRInst* base = nullptr;
    IRInst* index = nullptr;
    IRType* aggregateType = nullptr;
    IRPtrTypeBase* resultType = nullptr;
    bool isImmutable = false;
    bool isParameterGroupStorage = false;
    bool isLocalSubstandardRecordStorage = false;
};

bool _getNVVMSequentialElementPointer(
    const NVVMAddressPlan& addresses,
    IRInst* inst,
    NVVMSequentialElementPointer& outPointer);

// Resolves the aggregate-address shapes with executable representations. These include fields in
// collected CUDA parameters, loaded parameter groups, local/helper storage, and fields selected
// after an already-proved sequential aggregate element. Every child inherits its root access.
bool _getNVVMStructFieldAddress(
    const NVVMAddressPlan& addresses,
    IRFieldAddress* fieldAddress,
    NVVMStructFieldSelection& outAddress)
{
    outAddress = {};
    if (!fieldAddress)
        return false;

    IRStructType* structType = nullptr;
    NVVMConventionalGlobalParams globalParams;
    NVVMSharedGlobal sharedGlobal;
    const bool isConventionalGlobal =
        _getNVVMConventionalGlobalParams(fieldAddress->getBase(), globalParams);
    if (isConventionalGlobal)
    {
        structType = globalParams.elementType;
        outAddress.isConventionalGlobal = true;
    }
    else if (getNVVMSupportedSharedGlobal(fieldAddress->getBase(), &sharedGlobal))
    {
        structType = asNVVMSupportedHelperStructType(sharedGlobal.storageType);
        if (!structType)
            return false;
        outAddress.isMutable = true;
    }
    else if (auto parentFieldAddress = as<IRFieldAddress>(fieldAddress->getBase()))
    {
        // A nested field address carries the complete pointer spelling produced for its parent
        // field, which is intentionally more explicit than a local `Ptr<T>`. Reuse its checked
        // selection and preserve its root role: selecting a child cannot make immutable storage
        // mutable or detach conventional-global pointer provenance.
        const auto parent = addresses.findFieldAddress(parentFieldAddress);
        if (!parent)
            return false;
        const auto& parentAddress = parent->selection;
        auto basePointerType = as<IRPtrTypeBase>(parentFieldAddress->getDataType());
        structType = basePointerType
                         ? asNVVMSupportedHelperStructType(basePointerType->getValueType())
                         : nullptr;
        if (!structType && basePointerType && parentAddress.isConventionalGlobal)
        {
            // The checked parent owns CUDA binding storage. Descending into a pointer-bearing
            // record preserves that role without admitting a whole record as a helper value.
            structType = asNVVMSupportedAggregateStorageStructType(basePointerType->getValueType());
        }
        if (!structType && basePointerType && parentAddress.isLocalSubstandardRecordStorage)
        {
            // For `outer.inner.pair[index]`, the parent field address proves that inner belongs
            // to qualified local storage. Its explicit derived pointer spelling alone does not
            // grant that role. Retain the proof through each field so BF2 component selection
            // observes the same local root without admitting device or shared record storage.
            structType = asNVVMSupportedSubstandardRecordType(basePointerType->getValueType());
        }
        if (!structType && basePointerType)
            structType = asNVVMSupportedResourceStructType(basePointerType->getValueType());
        if (!structType && basePointerType)
        {
            structType =
                asNVVMSupportedPhysicalAggregateStorageStructType(basePointerType->getValueType());
        }
        if (!structType || !isTypeEqual(parentAddress.field->getFieldType(), structType))
        {
            return false;
        }
        outAddress.isConventionalGlobal = parentAddress.isConventionalGlobal;
        outAddress.isMutable = parentAddress.isMutable;
        outAddress.isPhysicalStorage = parentAddress.isPhysicalStorage;
        outAddress.isParameterGroupStorage = parentAddress.isParameterGroupStorage;
        outAddress.isLocalSubstandardRecordStorage = parentAddress.isLocalSubstandardRecordStorage;
    }
    else if (auto resourceElement = addresses.findStructuredElement(fieldAddress->getBase()))
    {
        auto resourceElementPointer = resourceElement->resultType;
        outAddress.isMutable = resourceElement->bufferType.access == NVVMBufferAccess::ReadWrite;
        structType = asNVVMSupportedPhysicalArrayStructType(resourceElementPointer->getValueType());
        if (!structType)
        {
            structType = asNVVMSupportedResourceStructType(resourceElementPointer->getValueType());
            if (!structType)
                return false;
        }
    }
    else if (
        auto resourceReference =
            asNVVMSupportedLocalResourceValuePointerType(fieldAddress->getBase()->getDataType()))
    {
        structType = as<IRStructType>(resourceReference->getValueType());
        if (!structType)
            return false;
        outAddress.isMutable =
            resourceReference->getAccessQualifier() == AccessQualifier::ReadWrite;
    }
    else
    {
        IRStructType* physicalStorageReferenceType = nullptr;
        auto physicalStorageReference = as<IRParam>(fieldAddress->getBase())
                                            ? asNVVMSupportedPhysicalStorageReferencePointerType(
                                                  fieldAddress->getBase()->getDataType(),
                                                  &physicalStorageReferenceType)
                                            : nullptr;
        IRStructType* localPhysicalStorageType = nullptr;
        auto localPhysicalStoragePointer = asNVVMSupportedLocalPhysicalStoragePointerType(
            fieldAddress->getBase()->getDataType(),
            &localPhysicalStorageType);
        if (physicalStorageReference)
        {
            structType = physicalStorageReferenceType;
            outAddress.isPhysicalStorage = true;
            outAddress.isParameterGroupStorage = true;
        }
        else if (localPhysicalStoragePointer)
        {
            structType = localPhysicalStorageType;
            outAddress.isMutable = true;
            outAddress.isPhysicalStorage = true;
            outAddress.isParameterGroupStorage = true;
        }
        IRType* copyableValueType = nullptr;
        auto localCopyablePointer = structType ? nullptr
                                               : asNVVMSupportedLocalCopyableValuePointerType(
                                                     fieldAddress->getBase()->getDataType(),
                                                     &copyableValueType);
        IRType* helperReferenceValueType = nullptr;
        auto helperReference = as<IRParam>(fieldAddress->getBase())
                                   ? asNVVMSupportedHelperReferencePointerType(
                                         fieldAddress->getBase()->getDataType(),
                                         &helperReferenceValueType)
                                   : nullptr;
        if (!structType)
        {
            structType = localCopyablePointer ? asNVVMSupportedCopyableStructType(copyableValueType)
                                              : nullptr;
        }
        if (localCopyablePointer && structType)
        {
            // Consider `void initialize(out Payload value) { value.count = 1; }`. The helper ABI
            // already proves the exact `OutParam<Payload>` representation; field selection only
            // needs to retain that mutable root role.
            outAddress.isMutable = true;
        }
        else if (physicalStorageReference || localPhysicalStoragePointer)
        {
            // The exact physical-storage classifier above already selected the struct and root
            // access. Keep that producer-owned immutable/local distinction unchanged.
        }
        else if (helperReference)
        {
            // Consider `int read(__constref Payload value) { return value.count; }`. Only an exact
            // helper parameter can own this canonical borrow, and selecting a field cannot turn
            // its read-only pointer into writable storage.
            structType = asNVVMSupportedHelperStructType(helperReferenceValueType);
            if (!structType)
                return false;
            outAddress.isMutable =
                helperReference->getAccessQualifier() == AccessQualifier::ReadWrite;
        }
        else if (fieldAddress->getBase()->getOp() == kIROp_GetElementPtr)
        {
            // Consider `Payload values[2]; values[index].count = 1;`. The element resolver proves
            // the array, index, result pointee, and access before this field resolver composes the
            // next canonical selection.
            const auto parentElement = addresses.findElementAddress(fieldAddress->getBase());
            if (!parentElement || parentElement->kind != NVVMElementAddressKind::Sequential)
                return false;
            structType = asNVVMSupportedHelperStructType(parentElement->resultType->getValueType());
            if (!structType && parentElement->isLocalSubstandardRecordStorage)
            {
                structType =
                    asNVVMSupportedSubstandardRecordType(parentElement->resultType->getValueType());
                outAddress.isLocalSubstandardRecordStorage = structType != nullptr;
            }
            if (!structType)
            {
                structType =
                    asNVVMSupportedResourceStructType(parentElement->resultType->getValueType());
            }
            if (!structType)
                return false;
            outAddress.isMutable = !parentElement->isReadOnly;
            outAddress.isParameterGroupStorage = parentElement->isParameterGroupStorage;
        }
        else
        {
            IRType* helperValueType = nullptr;
            if (asNVVMSupportedLocalHelperValuePointerType(
                    fieldAddress->getBase()->getDataType(),
                    &helperValueType))
            {
                structType = asNVVMSupportedHelperStructType(helperValueType);
                if (!structType)
                {
                    structType = asNVVMSupportedLocalSubstandardRecordType(helperValueType);
                    outAddress.isLocalSubstandardRecordStorage = structType != nullptr;
                }
                if (!structType)
                    return false;
                outAddress.isMutable = true;
            }
            else if (
                auto devicePointer = asNVVMSupportedDeviceCopyableValuePointerType(
                    fieldAddress->getBase()->getDataType(),
                    &helperValueType))
            {
                SLANG_UNUSED(devicePointer);
                structType = asNVVMSupportedHelperStructType(helperValueType);
                if (!structType)
                    return false;
                outAddress.isMutable = true;
            }
            else if (asNVVMSupportedSharedHelperPointerType(
                         fieldAddress->getBase()->getDataType(),
                         &helperValueType))
            {
                structType = asNVVMSupportedHelperStructType(helperValueType);
                if (!structType)
                    return false;
                outAddress.isMutable = true;
            }
            else
            {
                IRType* parameterGroupElementType = nullptr;
                if (!asNVVMSupportedParameterGroupType(
                        fieldAddress->getBase()->getDataType(),
                        &parameterGroupElementType) ||
                    !(structType = as<IRStructType>(parameterGroupElementType)))
                {
                    return false;
                }
                outAddress.isParameterGroupStorage = true;
            }
        }
    }
    if (!_findNVVMStructField(
            structType,
            fieldAddress->getField(),
            outAddress.field,
            outAddress.fieldIndex))
    {
        return false;
    }

    auto pointerType = as<IRPtrTypeBase>(fieldAddress->getDataType());
    if (!pointerType || !isTypeEqual(outAddress.field->getFieldType(), pointerType->getValueType()))
    {
        return false;
    }

    IRType* fieldType = outAddress.field->getFieldType();
    if (isConventionalGlobal)
    {
        // Global storage also contains comparison samplers and unsized sampler arrays. Those
        // storage-only fields are not executable values, but all other fields share admission.
        return isNVVMSupportedConventionalGlobalFieldType(outAddress.field) &&
               (!asNVVMSupportedSamplerStorageType(fieldType) ||
                asNVVMSupportedSamplerValueType(fieldType)) &&
               !asNVVMSupportedUnsizedSamplerArrayStorageType(fieldType);
    }

    // A physical record keeps its storage role even when the record is a mutable local
    // temporary. Packed numeric fields and nested parameter groups need not be helper values.
    if (outAddress.isParameterGroupStorage)
        return isNVVMSupportedParameterGroupElementStorageType(fieldType);

    if (outAddress.isMutable || outAddress.isLocalSubstandardRecordStorage ||
        addresses.findRootBuffer(fieldAddress->getBase()) ||
        _getNVVMLocalValueReference(addresses, fieldAddress->getBase()))
    {
        return _getNVVMExecutableValueAlignment(fieldType) != 0 ||
               (outAddress.isLocalSubstandardRecordStorage && asNVVMBFloat16VectorType(fieldType));
    }

    return isNVVMSupportedAggregateStorageType(fieldType);
}

IRVectorType* _getNVVMLocalBFloat16VectorPointer(const NVVMAddressPlan& addresses, IRInst* pointer)
{
    IRType* valueType = nullptr;
    if (pointer && asNVVMSupportedLocalHelperValuePointerType(pointer->getDataType(), &valueType))
        return asNVVMBFloat16VectorType(valueType);

    // FieldAddress retains the exact record and key. For example, `record.value` in an inout
    // Record helper has an explicit pointer spelling, so its producer proves the local role.
    const auto field = addresses.findFieldAddress(pointer);
    return field && field->selection.isLocalSubstandardRecordStorage
               ? asNVVMBFloat16VectorType(field->selection.field->getFieldType())
               : nullptr;
}

bool _getNVVMStructFieldValue(IRFieldExtract* fieldExtract, NVVMStructFieldSelection& outField);

// Resolves one canonical parameter-group pointer value. Consider these examples:
//
//     struct Globals { ParameterBlock<Material> material; }
//     Globals globals;
//     Material value = globals.material;
//
//     struct Scene { ParameterBlock<Material> material; }
//     Material nested = scene.material;
//
//     void kernel(uniform ParameterBlock<Material> material) { use(material); }
//
// Entry-point-uniform lowering first emits `fieldAddress(globals, material)`, then loads the
// pointer. A first-class Scene uses `fieldExtract`, while a raw launch parameter uses its exact
// `IRParam`. Each value has parameter-group type rather than `Ptr<Material>`. Preserve these three
// producers instead of admitting arbitrary load results as device pointers.
bool _getNVVMParameterGroupPointer(
    const NVVMAddressPlan& addresses,
    IRInst* inst,
    IRType*& outElementType)
{
    outElementType = nullptr;
    IRType* elementType = nullptr;
    auto parameterGroupType =
        inst ? asNVVMSupportedParameterGroupType(inst->getDataType(), &elementType) : nullptr;
    if (!parameterGroupType)
        return false;

    if (as<IRParam>(inst))
    {
        outElementType = elementType;
        return true;
    }

    if (auto fieldExtract = as<IRFieldExtract>(inst))
    {
        NVVMStructFieldSelection valueField;
        if (!_getNVVMStructFieldValue(fieldExtract, valueField) ||
            !isTypeEqual(valueField.field->getFieldType(), parameterGroupType))
        {
            return false;
        }
        outElementType = elementType;
        return true;
    }

    auto load = as<IRLoad>(inst);
    auto fieldAddress = load ? as<IRFieldAddress>(load->getPtr()) : nullptr;
    const auto storageField = addresses.findFieldAddress(fieldAddress);
    if (!storageField ||
        !isTypeEqual(storageField->selection.field->getFieldType(), parameterGroupType))
    {
        return false;
    }

    outElementType = elementType;
    return true;
}

// Resolves one resource-capable field extraction by canonical struct key and exact result type.
bool _getNVVMStructFieldValue(IRFieldExtract* fieldExtract, NVVMStructFieldSelection& outField)
{
    outField = {};
    auto structType = fieldExtract
                          ? asNVVMSupportedHelperStructType(fieldExtract->getBase()->getDataType())
                          : nullptr;
    if (!structType && fieldExtract)
        structType = asNVVMSupportedSubstandardRecordType(fieldExtract->getBase()->getDataType());
    if (!structType && fieldExtract)
        structType = asNVVMSupportedResourceStructType(fieldExtract->getBase()->getDataType());
    if (!structType || !_findNVVMStructField(
                           structType,
                           fieldExtract->getField(),
                           outField.field,
                           outField.fieldIndex))
    {
        return false;
    }
    return isTypeEqual(outField.field->getFieldType(), fieldExtract->getDataType());
}


// Resolves the canonical operation that exposes field zero of an admitted raw buffer view.
bool _getNVVMRawBufferDataPointer(IRInst* inst, NVVMRawBufferDataPointer& outPointer)
{
    outPointer = {};
    if (!inst || inst->getOperandCount() != 1 ||
        (inst->getOp() != kIROp_GetStructuredBufferPtr &&
         inst->getOp() != kIROp_GetUntypedBufferPtr))
    {
        return false;
    }

    IRInst* buffer = inst->getOperand(0);
    NVVMRawBufferType bufferType;
    NVVMBufferDataPointerType resultType;
    if (!buffer || !getNVVMSupportedRawBufferType(buffer->getDataType(), bufferType) ||
        !getNVVMSupportedBufferDataPointerType(inst->getDataType(), resultType) ||
        !isNVVMRawBufferElementType(bufferType, resultType.elementType))
    {
        return false;
    }

    const bool isStructured = bufferType.kind == NVVMRawBufferKind::Structured;
    if ((inst->getOp() == kIROp_GetStructuredBufferPtr) != isStructured)
        return false;

    outPointer.source = inst;
    outPointer.buffer = buffer;
    outPointer.bufferType = bufferType;
    outPointer.resultType = resultType;
    return true;
}


struct NVVMStructuredBufferDimensions
{
    IRInst* buffer = nullptr;
    NVVMRawBufferType bufferType;
    IRVectorType* resultType = nullptr;
    uint32_t elementStride = 0;
};

// Resolves the canonical structured-buffer query against the existing `{data, count}` raw view.
// The count is runtime data carried by the view; the CUDA element stride is a compile-time fact of
// the exact selected storage type and never needs to be recovered from source syntax.
bool _getNVVMStructuredBufferDimensions(
    CodeGenContext* codeGenContext,
    IRInst* inst,
    NVVMStructuredBufferDimensions& outDimensions)
{
    outDimensions = {};
    auto query = as<IRStructuredBufferGetDimensions>(inst);
    IRInst* buffer = query ? query->getBuffer() : nullptr;
    NVVMRawBufferType bufferType;
    bool isSigned = false;
    uint32_t elementCount = 0;
    auto resultType =
        query ? asNVVMSupportedI32VectorType(query->getDataType(), &isSigned, &elementCount)
              : nullptr;
    IRSizeAndAlignment elementLayout;
    if (!query || query->getOperandCount() != 1 || !buffer || !resultType || isSigned ||
        elementCount != 2 || !getNVVMSupportedRawBufferType(buffer->getDataType(), bufferType) ||
        bufferType.kind != NVVMRawBufferKind::Structured ||
        !_getNVVMStructuredBufferStorageLayout(
            codeGenContext,
            bufferType.structuredElementType,
            elementLayout) ||
        elementLayout.size <= 0 || elementLayout.size > kNVVMUInt32Max)
    {
        return false;
    }

    outDimensions.buffer = buffer;
    outDimensions.bufferType = bufferType;
    outDimensions.resultType = resultType;
    outDimensions.elementStride = uint32_t(elementLayout.size);
    return true;
}


// Resolves the pointer-form structured-buffer access retained for mutable elements and physical
// read-only storage. The buffer owns access; the pointer spelling alone cannot make a read-only
// StructuredBuffer writable.
bool _getNVVMStructuredBufferElementPointer(
    IRInst* inst,
    NVVMStructuredBufferElementPointer& outPointer)
{
    outPointer = {};
    if (!inst || inst->getOp() != kIROp_RWStructuredBufferGetElementPtr ||
        inst->getOperandCount() != 2)
    {
        return false;
    }

    IRInst* buffer = inst->getOperand(0);
    IRInst* elementIndex = inst->getOperand(1);
    NVVMRawBufferType bufferType;
    auto resultType = asNVVMSupportedRWStructuredBufferElementPointerType(inst->getDataType());
    if (!buffer || !elementIndex || !isNVVMInteger32Type(elementIndex->getDataType()) ||
        !getNVVMSupportedRawBufferType(buffer->getDataType(), bufferType) ||
        bufferType.kind != NVVMRawBufferKind::Structured || !resultType ||
        !isNVVMRawBufferElementType(bufferType, resultType->getValueType()))
    {
        return false;
    }

    outPointer.source = inst;
    outPointer.buffer = buffer;
    outPointer.elementIndex = elementIndex;
    outPointer.bufferType = bufferType;
    outPointer.resultType = resultType;
    return true;
}

// Resolves a canonical structured-buffer value load and its exact physical load contract.
bool _getNVVMStructuredBufferLoad(IRInst* inst, NVVMPlannedStructuredLoad& outLoad)
{
    outLoad = {};
    if (!inst ||
        (inst->getOp() != kIROp_StructuredBufferLoad &&
         inst->getOp() != kIROp_RWStructuredBufferLoad) ||
        inst->getOperandCount() != 2)
    {
        return false;
    }

    IRInst* buffer = inst->getOperand(0);
    IRInst* elementIndex = inst->getOperand(1);
    NVVMRawBufferType bufferType;
    const NVVMBufferAccess expectedAccess = inst->getOp() == kIROp_StructuredBufferLoad
                                                ? NVVMBufferAccess::ReadOnly
                                                : NVVMBufferAccess::ReadWrite;
    IRType* resultType = inst->getDataType();
    const uint32_t alignment = _getNVVMExecutableValueAlignment(resultType);
    if (!buffer || !elementIndex || !isNVVMInteger32Type(elementIndex->getDataType()) ||
        !getNVVMSupportedRawBufferType(buffer->getDataType(), bufferType) ||
        bufferType.kind != NVVMRawBufferKind::Structured || bufferType.access != expectedAccess ||
        !isNVVMRawBufferElementType(bufferType, resultType) ||
        (!alignment && !isNVVMSupportedStructuredBufferStorageType(resultType)))
    {
        return false;
    }

    outLoad.buffer = buffer;
    outLoad.elementIndex = elementIndex;
    outLoad.bufferType = bufferType;
    outLoad.resultType = resultType;
    outLoad.flags = expectedAccess == NVVMBufferAccess::ReadOnly ? SLANG_NVVM_LOAD_FLAG_INVARIANT
                                                                 : SLANG_NVVM_LOAD_FLAG_NONE;
    return true;
}

struct NVVMByteAddressAccess
{
    IRInst* buffer = nullptr;
    IRInst* byteOffset = nullptr;
    IRInst* value = nullptr;
    IRType* valueType = nullptr;
    NVVMRawBufferType bufferType;
    uint32_t alignment = 0;
    bool isStore = false;
};

// Returns the implicit alignment of one value retained by byte-address legalization. Consider
// `struct Data { int16_t a; int16_t b; }`: legalization emits scalar Int16 accesses at offsets zero
// and two, with an omitted/zero alignment operand. The producer guarantees at most four-byte
// alignment, reduced to the physical value alignment for narrower values.
uint32_t _getNVVMByteAddressValueAlignment(IRType* type)
{
    uint32_t naturalAlignment = getNVVMNumericValueAlignment(type);
    if (!naturalAlignment)
    {
        auto arrayType = asNVVMSupportedNumericArrayType(type);
        naturalAlignment =
            arrayType ? getNVVMNumericValueAlignment(arrayType->getElementType()) : 0;
    }
    return naturalAlignment < 4 ? naturalAlignment : 4;
}

// Resolves the canonical selected numeric scalar/vector byte-address load and store family. A zero
// or omitted alignment carries the ordinary at-most-four-byte contract; an explicit alignment is
// a power-of-two promise that can be forwarded unchanged to LLVM.
bool _getNVVMByteAddressAccess(IRInst* inst, NVVMByteAddressAccess& outAccess)
{
    outAccess = {};
    if (!inst)
        return false;

    const bool isLoad = inst->getOp() == kIROp_ByteAddressBufferLoad;
    const bool isStore = inst->getOp() == kIROp_ByteAddressBufferStore;
    if ((!isLoad && !isStore) ||
        (isLoad && inst->getOperandCount() != 2 && inst->getOperandCount() != 3) ||
        (isStore && inst->getOperandCount() != 4))
    {
        return false;
    }

    IRInst* buffer = inst->getOperand(0);
    IRInst* byteOffset = inst->getOperand(1);
    IRInst* alignmentOperand =
        isLoad && inst->getOperandCount() == 2 ? nullptr : inst->getOperand(2);
    IRInst* value = isStore ? inst->getOperand(3) : nullptr;
    IRType* valueType = isStore && value ? value->getDataType() : inst->getDataType();
    NVVMRawBufferType bufferType;
    if (!buffer || !byteOffset || !isNVVMUnsignedI32Type(byteOffset->getDataType()) || !valueType ||
        !isNVVMSupportedByteAddressValueType(valueType) ||
        !getNVVMSupportedRawBufferType(buffer->getDataType(), bufferType) ||
        bufferType.kind != NVVMRawBufferKind::ByteAddress ||
        (isStore && (bufferType.access != NVVMBufferAccess::ReadWrite ||
                     !as<IRVoidType>(inst->getDataType()))))
    {
        return false;
    }

    uint32_t alignment = _getNVVMByteAddressValueAlignment(valueType);
    if (!alignment)
        return false;
    if (alignmentOperand)
    {
        auto alignmentLiteral = as<IRIntLit>(alignmentOperand);
        if (!alignmentLiteral || !isNVVMUnsignedI32Type(alignmentLiteral->getDataType()) ||
            alignmentLiteral->getValue() < 0 || alignmentLiteral->getValue() > UINT32_MAX)
        {
            return false;
        }
        const uint32_t literalAlignment = uint32_t(alignmentLiteral->getValue());
        if (literalAlignment)
        {
            if (literalAlignment & (literalAlignment - 1))
                return false;
            alignment = literalAlignment;
        }
    }

    outAccess.buffer = buffer;
    outAccess.byteOffset = byteOffset;
    outAccess.value = value;
    outAccess.valueType = valueType;
    outAccess.bufferType = bufferType;
    outAccess.alignment = alignment;
    outAccess.isStore = isStore;
    return true;
}

struct NVVMEquivalentStructuredBuffer
{
    IRInst* buffer = nullptr;
    NVVMRawBufferType sourceType;
    NVVMRawBufferType resultType;
};

// Resolves the selected scalar structured views used by byte-address legalization.
bool _getNVVMEquivalentStructuredBuffer(IRInst* inst, NVVMEquivalentStructuredBuffer& outConversion)
{
    outConversion = {};
    if (!inst || inst->getOp() != kIROp_GetEquivalentStructuredBuffer ||
        inst->getOperandCount() != 1)
    {
        return false;
    }

    IRInst* buffer = inst->getOperand(0);
    if (!buffer ||
        !getNVVMSupportedRawBufferType(buffer->getDataType(), outConversion.sourceType) ||
        !getNVVMSupportedRawBufferType(inst->getDataType(), outConversion.resultType) ||
        outConversion.sourceType.kind != NVVMRawBufferKind::ByteAddress ||
        outConversion.resultType.kind != NVVMRawBufferKind::Structured ||
        outConversion.sourceType.access != outConversion.resultType.access)
    {
        return false;
    }

    IRType* physicalElementType = outConversion.resultType.structuredElementType;
    uint32_t integerBitWidth = 0;
    const bool isSelectedInteger =
        isNVVMSupportedIntegerScalarType(physicalElementType, &integerBitWidth) &&
        (integerBitWidth == 16 || integerBitWidth == 32 || integerBitWidth == 64);
    if (!isSelectedInteger && !isNVVMFloat16Type(physicalElementType) &&
        !isNVVMFloat32Type(physicalElementType))
    {
        return false;
    }

    outConversion.buffer = buffer;
    return true;
}

struct NVVMRawBufferElementPointer
{
    IRInst* base = nullptr;
    IRInst* index = nullptr;
    IRPtrTypeBase* resultType = nullptr;
};

// Resolves one scalar element address rooted directly in an admitted raw-buffer data pointer.
bool _getNVVMRawBufferElementPointer(
    const NVVMAddressPlan& addresses,
    IRInst* inst,
    NVVMRawBufferElementPointer& outPointer)
{
    outPointer = {};
    if (!inst || inst->getOp() != kIROp_GetElementPtr || inst->getOperandCount() != 2)
        return false;

    IRInst* base = inst->getOperand(0);
    IRInst* index = inst->getOperand(1);
    const auto baseProducer = addresses.findDataPointer(base);
    auto resultType = asNVVMSupportedDeviceScalarPointerType(inst->getDataType());
    IRType* resultLayout = resultType ? resultType->getDataLayout() : nullptr;
    if (!base || !index || !baseProducer || !resultType || resultType->getOperandCount() != 4 ||
        !resultLayout ||
        (resultLayout->getOp() != kIROp_ScalarBufferLayoutType &&
         resultLayout->getOp() != kIROp_CUDABufferLayoutType) ||
        !isTypeEqual(resultType->getValueType(), baseProducer->resultType.elementType) ||
        resultType->getAccessQualifier() !=
            baseProducer->resultType.pointerType->getAccessQualifier() ||
        resultType->getAddressSpace() != baseProducer->resultType.pointerType->getAddressSpace() ||
        !isNVVMInteger32Type(index->getDataType()))
    {
        return false;
    }

    outPointer.base = base;
    outPointer.index = index;
    outPointer.resultType = resultType;
    return true;
}

// Resolves one selected element address rooted in an admitted fixed array or vector. Immutable
// parameter-group and physical-storage roots retain that property through every nested index.
bool _getNVVMSequentialElementPointer(
    const NVVMAddressPlan& addresses,
    IRInst* inst,
    NVVMSequentialElementPointer& outPointer)
{
    outPointer = {};
    if (!inst || inst->getOp() != kIROp_GetElementPtr || inst->getOperandCount() != 2)
        return false;

    IRInst* base = inst->getOperand(0);
    IRInst* index = inst->getOperand(1);
    IRArrayType* arrayType = nullptr;
    IRPtrTypeBase* baseType = nullptr;
    NVVMSharedGlobal sharedGlobal;
    bool isSharedGlobalBase = false;
    bool isImmutable = false;
    bool hasImmutablePhysicalStorageFieldBase = false;
    bool isParameterGroupStorage = false;
    const auto resourceElement = addresses.findStructuredElement(base);
    const bool hasResourceElementBase = resourceElement != nullptr;
    if (base && getNVVMSupportedSharedGlobal(base, &sharedGlobal))
    {
        arrayType = asNVVMSupportedHelperArrayType(sharedGlobal.storageType);
        baseType = arrayType ? as<IRPtrTypeBase>(base->getDataType()) : nullptr;
        isSharedGlobalBase = baseType != nullptr;
    }
    if (!baseType && base)
        baseType = asNVVMSupportedLocalNumericArrayPointerType(base->getDataType(), &arrayType);
    if (!baseType && base)
        baseType = asNVVMSupportedLocalCopyableArrayPointerType(base->getDataType(), &arrayType);
    if (!baseType && base)
    {
        IRType* helperValueType = nullptr;
        baseType =
            asNVVMSupportedLocalHelperValuePointerType(base->getDataType(), &helperValueType);
        arrayType = baseType ? asNVVMSupportedHelperArrayType(helperValueType) : nullptr;
        if (!arrayType)
            baseType = nullptr;
    }
    if (!baseType && base)
    {
        auto resource = asNVVMSupportedLocalResourceValuePointerType(base->getDataType());
        if (resource && (arrayType = asNVVMSupportedResourceArrayType(resource->getValueType())))
        {
            baseType = resource;
            isImmutable = resource->getAccessQualifier() == AccessQualifier::Read;
        }
    }
    if (!baseType)
    {
        auto object = _getNVVMHitObjectPointer(addresses, base);
        if (object && as<IRArrayType>(object->getValueType()))
        {
            baseType = object;
            arrayType = cast<IRArrayType>(object->getValueType());
            isImmutable = object->getAccessQualifier() == AccessQualifier::Read;
        }
    }
    bool isLocalSubstandardRecordStorage = false;
    if (!baseType)
    {
        baseType = _getNVVMLocalSubstandardRecordArrayPointer(base);
        if (baseType)
        {
            arrayType = asNVVMSupportedLocalSubstandardRecordArrayType(baseType->getValueType());
            isLocalSubstandardRecordStorage = true;
            isImmutable = baseType->getAccessQualifier() == AccessQualifier::Read;
        }
    }
    if (!baseType && hasResourceElementBase)
    {
        baseType = resourceElement->resultType;
        arrayType = baseType ? asNVVMSupportedHelperArrayType(baseType->getValueType()) : nullptr;
        if (!arrayType)
            baseType = nullptr;
        else
            isImmutable = resourceElement->bufferType.access == NVVMBufferAccess::ReadOnly;
    }
    if (!baseType && base)
    {
        IRType* valueType = nullptr;
        auto pointer = asNVVMSupportedDeviceHelperValuePointerType(base->getDataType(), &valueType);
        if (pointer && (arrayType = asNVVMSupportedHelperArrayType(valueType)))
        {
            baseType = pointer;
            isImmutable = pointer->getAccessQualifier() == AccessQualifier::Read;
        }
    }
    IRType* aggregateType = arrayType;

    if (!arrayType && base)
    {
        const auto parentElement = addresses.findElementAddress(base);
        if (parentElement && parentElement->kind == NVVMElementAddressKind::Sequential)
        {
            arrayType = asNVVMSupportedHelperArrayType(parentElement->resultType->getValueType());
            if (!arrayType)
                arrayType =
                    asNVVMSupportedResourceArrayType(parentElement->resultType->getValueType());
            if (arrayType)
            {
                baseType = parentElement->resultType;
                aggregateType = arrayType;
                isImmutable = parentElement->isReadOnly;
                isParameterGroupStorage = parentElement->isParameterGroupStorage;
            }
        }
    }

    if (!arrayType && base)
    {
        IRType* parameterGroupElementType = nullptr;
        const bool hasParameterGroupBase =
            asNVVMSupportedParameterGroupType(base->getDataType(), &parameterGroupElementType);
        arrayType = hasParameterGroupBase
                        ? asNVVMSupportedAggregateStorageArrayType(parameterGroupElementType)
                        : nullptr;
        isParameterGroupStorage = arrayType != nullptr;
        if (!arrayType && base->getOp() == kIROp_FieldAddress)
        {
            const auto field = addresses.findFieldAddress(base);
            if (field)
            {
                arrayType =
                    (field->selection.isMutable || addresses.findRootBuffer(base) ||
                     _getNVVMLocalValueReference(addresses, base))
                        ? asNVVMSupportedHelperArrayType(field->selection.field->getFieldType())
                        : asNVVMSupportedAggregateStorageArrayType(
                              field->selection.field->getFieldType());
                if (!arrayType && _getNVVMLocalValueReference(addresses, base))
                    arrayType =
                        asNVVMSupportedResourceArrayType(field->selection.field->getFieldType());
                baseType = as<IRPtrTypeBase>(base->getDataType());
                isImmutable = !field->selection.isMutable;
                isParameterGroupStorage = field->selection.isParameterGroupStorage;
                hasImmutablePhysicalStorageFieldBase =
                    isImmutable && field->selection.isPhysicalStorage && arrayType;
            }
        }
        if (arrayType)
        {
            aggregateType = arrayType;
        }
    }

    IRVectorType* vectorType = nullptr;
    if (!arrayType && base)
    {
        IRType* valueType = nullptr;
        IRPtrTypeBase* numericPointer = nullptr;
        if (hasResourceElementBase)
        {
            numericPointer = resourceElement->resultType;
            valueType = numericPointer ? numericPointer->getValueType() : nullptr;
            if (numericPointer)
                isImmutable = resourceElement->bufferType.access == NVVMBufferAccess::ReadOnly;
        }
        if (!numericPointer)
            numericPointer =
                asNVVMSupportedLocalNumericPointerType(base->getDataType(), &valueType);
        if (!numericPointer)
        {
            numericPointer =
                asNVVMSupportedDeviceHelperValuePointerType(base->getDataType(), &valueType);
            if (numericPointer)
                isImmutable = numericPointer->getAccessQualifier() == AccessQualifier::Read;
        }
        if (!numericPointer && base->getOp() == kIROp_FieldAddress)
        {
            const auto field = addresses.findFieldAddress(base);
            if (field && !field->selection.isConventionalGlobal &&
                (field->selection.isMutable || field->selection.isLocalSubstandardRecordStorage ||
                 (field->selection.isParameterGroupStorage &&
                  asNVVMSupportedNumericVectorType(field->selection.field->getFieldType())) ||
                 asNVVMSupported32BitNumericVectorType(field->selection.field->getFieldType())))
            {
                numericPointer = as<IRPtrTypeBase>(base->getDataType());
                valueType = numericPointer ? numericPointer->getValueType() : nullptr;
                isImmutable = !field->selection.isMutable;
                isParameterGroupStorage = field->selection.isParameterGroupStorage;
                isLocalSubstandardRecordStorage = field->selection.isLocalSubstandardRecordStorage;
            }
        }
        const auto parentElement = addresses.findElementAddress(base);
        if (!numericPointer && parentElement &&
            parentElement->kind == NVVMElementAddressKind::Sequential)
        {
            numericPointer = parentElement->resultType;
            valueType = numericPointer->getValueType();
            isImmutable = parentElement->isReadOnly;
            isParameterGroupStorage = parentElement->isParameterGroupStorage;
        }
        vectorType = asNVVMSupportedNumericVectorType(valueType);
        if (!vectorType && numericPointer && as<IRFieldAddress>(base))
        {
            // AnyValue unpacking stores each BF2 component through its canonical field address.
            // The local record proof owns this memory; neither a bare vector pointer nor a
            // resource field acquires admission from the component type alone.
            uint32_t count = 0;
            auto localVector = _getNVVMLocalBFloat16VectorPointer(addresses, base);
            if (localVector && asNVVMBFloat16VectorType(localVector, &count) && count == 2)
                vectorType = localVector;
        }
        if (numericPointer && vectorType)
        {
            baseType = numericPointer;
            aggregateType = vectorType;
        }
    }
    auto resultType = as<IRPtrTypeBase>(inst->getDataType());
    IRType* resultLayout = resultType ? resultType->getDataLayout() : nullptr;
    IRType* expectedElementType = arrayType    ? arrayType->getElementType()
                                  : vectorType ? vectorType->getElementType()
                                               : nullptr;
    // Physical storage field lowering keeps the immutable root in the field resolver but emits
    // the canonical GetElementPtr result with the ordinary ReadWrite access operand. Preserve
    // immutability through `isImmutable`; the result spelling is a producer-owned pointer detail.
    const AccessQualifier expectedAccess = hasImmutablePhysicalStorageFieldBase
                                               ? AccessQualifier::ReadWrite
                                           : baseType ? baseType->getAccessQualifier()
                                                      : AccessQualifier::ReadWrite;
    const AddressSpace expectedAddressSpace = isSharedGlobalBase ? AddressSpace::GroupShared
                                              : baseType         ? baseType->getAddressSpace()
                                                                 : AddressSpace::Generic;
    const bool hasCanonicalLocalLayout =
        (resultType && (resultType->getOperandCount() == 1 || resultType->getOperandCount() == 3) &&
         !resultLayout) ||
        (resultType && resultType->getOperandCount() == 4 && resultLayout &&
         (resultLayout->getOp() == kIROp_ScalarBufferLayoutType ||
          (baseType && resultLayout->getOp() == kIROp_CUDABufferLayoutType &&
           (baseType->getAddressSpace() == AddressSpace::UserPointer ||
            (addresses.findRootBuffer(base) && baseType->getDataLayout() == resultLayout))) ||
          ((isLocalSubstandardRecordStorage && isImmutable ||
            isNVVMHitObjectStorageType(resultType->getValueType())) &&
           resultLayout->getOp() == kIROp_DefaultBufferLayoutType)));
    if (!aggregateType || !resultType || resultType->getOp() != kIROp_PtrType ||
        !hasCanonicalLocalLayout || resultType->getAddressSpace() != expectedAddressSpace ||
        resultType->getAccessQualifier() != expectedAccess ||
        !_getNVVMExecutableValueAlignment(resultType->getValueType()) ||
        !isTypeEqual(expectedElementType, resultType->getValueType()) ||
        !isNVVMInteger32Type(index->getDataType()))
    {
        return false;
    }

    outPointer.base = base;
    outPointer.index = index;
    outPointer.aggregateType = aggregateType;
    outPointer.resultType = resultType;
    outPointer.isImmutable = isImmutable;
    outPointer.isParameterGroupStorage = isParameterGroupStorage;
    outPointer.isLocalSubstandardRecordStorage = isLocalSubstandardRecordStorage;
    return true;
}

// Gets the natural CUDA alignment carried by one physical LLVM `byval` entry parameter.
bool _getNVVMByValueParameterAlignment(
    CodeGenContext* codeGenContext,
    IRType* type,
    uint32_t& outAlignment)
{
    outAlignment = 0;
    if (!codeGenContext || !isNVVMSupportedResourceAggregateType(type))
        return false;

    IRSizeAndAlignment layout;
    if (SLANG_FAILED(getSizeAndAlignment(
            codeGenContext->getTargetReq(),
            IRTypeLayoutRules::getCUDA(),
            type,
            &layout)) ||
        layout.alignment <= 0 || layout.alignment > UINT32_MAX)
    {
        return false;
    }
    outAlignment = uint32_t(layout.alignment);
    return true;
}

bool _hasNVVMCompatibleHelperValueLayout(CodeGenContext* codeGenContext, IRType* type);

// Verifies that a selected copyable struct can use unpadded LLVM structs for storage. Consider
// `Thing { uint pos; float radius; half4 color; }`: CUDA and LLVM give its fields offsets 0, 4, and
// 8 and the same 16-byte stride, even though their preferred aggregate alignment differs. Matching
// offsets and size are the actual memory contract; a mismatch must be handled by layout lowering
// rather than by silently indexing a different LLVM representation.
bool _hasNVVMCompatibleStructLayout(CodeGenContext* codeGenContext, IRStructType* type)
{
    if (!codeGenContext || !type)
        return false;

    IRSizeAndAlignment cudaLayout;
    IRSizeAndAlignment llvmLayout;
    if (SLANG_FAILED(getSizeAndAlignment(
            codeGenContext->getTargetReq(),
            IRTypeLayoutRules::getCUDA(),
            type,
            &cudaLayout)) ||
        SLANG_FAILED(getSizeAndAlignment(
            codeGenContext->getTargetReq(),
            IRTypeLayoutRules::getLLVM(),
            type,
            &llvmLayout)) ||
        cudaLayout.size <= 0 || cudaLayout.size != llvmLayout.size)
    {
        return false;
    }

    for (auto field : type->getFields())
    {
        IRIntegerValue cudaOffset = 0;
        IRIntegerValue llvmOffset = 0;
        if (SLANG_FAILED(getOffset(
                codeGenContext->getTargetReq(),
                IRTypeLayoutRules::getCUDA(),
                field,
                &cudaOffset)) ||
            SLANG_FAILED(getOffset(
                codeGenContext->getTargetReq(),
                IRTypeLayoutRules::getLLVM(),
                field,
                &llvmOffset)) ||
            cudaOffset < 0 || cudaOffset != llvmOffset)
        {
            return false;
        }
        IRType* fieldType = field->getFieldType();
        if ((asNVVMSupportedResourceStructType(fieldType) ||
             asNVVMSupportedHelperStructType(fieldType) ||
             asNVVMSupportedHelperArrayType(fieldType) ||
             asNVVMSupportedResourceArrayType(fieldType)) &&
            !_hasNVVMCompatibleHelperValueLayout(codeGenContext, fieldType))
            return false;
    }
    return true;
}

// Verifies every node in one recursive helper value. A typed user pointer is the fixed-size leaf;
// matching the complete array size proves that an explicit canonical stride, when present, agrees
// with LLVM's element stride, and the struct check above proves every CUDA field offset.
bool _hasNVVMCompatibleHelperValueLayout(CodeGenContext* codeGenContext, IRType* type)
{
    if (!codeGenContext || !type)
        return false;
    IRType* atomicValueType = nullptr;
    if (asNVVMSupportedAtomicType(type, &atomicValueType))
        return _hasNVVMCompatibleHelperValueLayout(codeGenContext, atomicValueType);
    if (auto structType = asNVVMSupportedHelperStructType(type))
        return _hasNVVMCompatibleStructLayout(codeGenContext, structType);
    if (auto structType = asNVVMSupportedResourceStructType(type))
        return _hasNVVMCompatibleStructLayout(codeGenContext, structType);
    auto arrayType = asNVVMSupportedHelperArrayType(type);
    if (!arrayType)
        arrayType = asNVVMSupportedResourceArrayType(type);
    if (arrayType)
    {
        IRSizeAndAlignment cudaLayout;
        IRSizeAndAlignment llvmLayout;
        return SLANG_SUCCEEDED(getSizeAndAlignment(
                   codeGenContext->getTargetReq(),
                   IRTypeLayoutRules::getCUDA(),
                   arrayType,
                   &cudaLayout)) &&
               SLANG_SUCCEEDED(getSizeAndAlignment(
                   codeGenContext->getTargetReq(),
                   IRTypeLayoutRules::getLLVM(),
                   arrayType,
                   &llvmLayout)) &&
               cudaLayout.size > 0 && cudaLayout.size == llvmLayout.size &&
               _hasNVVMCompatibleHelperValueLayout(codeGenContext, arrayType->getElementType());
    }
    if (asNVVMSupportedDeviceHelperValuePointerType(type))
        return true;
    NVVMRawBufferType rawBuffer;
    NVVMReadOnlyTextureType texture;
    NVVMSurfaceType surface;
    if (!isNVVMSupportedNumericValueType(type) && !isNVVMBFloat16Type(type) &&
        !isNVVMAccelerationStructureType(type) && !asNVVMSupportedDescriptorHandleType(type) &&
        !getNVVMSupportedRawBufferType(type, rawBuffer) &&
        !getNVVMSupportedReadOnlyTextureType(type, texture) &&
        !getNVVMSupportedSurfaceType(type, surface) && !asNVVMSupportedSamplerValueType(type))
        return false;

    // Structured-buffer and aggregate-storage pointer arithmetic use the provider type's physical
    // stride. A selected numeric leaf may cross either storage boundary only when CUDA and LLVM
    // agree on both size and alignment. Register-only helper values do not use this predicate.
    IRSizeAndAlignment cudaLayout;
    IRSizeAndAlignment llvmLayout;
    return SLANG_SUCCEEDED(getSizeAndAlignment(
               codeGenContext->getTargetReq(),
               IRTypeLayoutRules::getCUDA(),
               type,
               &cudaLayout)) &&
           SLANG_SUCCEEDED(getSizeAndAlignment(
               codeGenContext->getTargetReq(),
               IRTypeLayoutRules::getLLVM(),
               type,
               &llvmLayout)) &&
           cudaLayout.size > 0 && cudaLayout.size == llvmLayout.size &&
           cudaLayout.alignment == llvmLayout.alignment;
}

IRIntegerValue _alignNVVMStorageSize(IRIntegerValue size, IRIntegerValue alignment)
{
    SLANG_RELEASE_ASSERT(alignment > 0 && (alignment & (alignment - 1)) == 0);
    return (size + alignment - 1) & ~(alignment - 1);
}

// Computes the physical provider representation selected at a structured-buffer boundary. The
// final IR type remains the semantic source of truth: this routine merely proves that the derived
// UInt8 Boolean, scalar-array vector3, fixed-array, and direct-field struct representation has the
// exact pointer stride and field offsets required by external storage. Loads and stores carry the
// canonical CUDA alignment explicitly, so a stronger provider-preferred root alignment is valid.
bool _getNVVMStructuredBufferStorageLayout(
    CodeGenContext* codeGenContext,
    IRType* type,
    IRSizeAndAlignment& outLayout)
{
    outLayout = {};
    if (!codeGenContext || !isNVVMSupportedStructuredBufferStorageType(type))
        return false;

    if (isNVVMBoolType(type))
    {
        outLayout.size = 1;
        outLayout.alignment = 1;
        return true;
    }

    uint32_t laneCount = 0;
    if (auto vectorType = asNVVMSupportedValueVectorType(type, &laneCount))
    {
        if (isNVVMBoolType(vectorType->getElementType()) || laneCount == 3)
        {
            IRSizeAndAlignment elementLayout;
            if (!_getNVVMStructuredBufferStorageLayout(
                    codeGenContext,
                    vectorType->getElementType(),
                    elementLayout))
            {
                return false;
            }
            const IRIntegerValue stride =
                _alignNVVMStorageSize(elementLayout.size, elementLayout.alignment);
            outLayout.size = laneCount * stride;
            outLayout.alignment = isNVVMBoolType(vectorType->getElementType())
                                      ? (laneCount == 2   ? 2
                                         : laneCount == 3 ? 1
                                                          : 4)
                                      : elementLayout.alignment;
            return true;
        }
    }

    if (auto arrayType = as<IRArrayType>(type))
    {
        auto count = as<IRIntLit>(arrayType->getElementCount());
        IRSizeAndAlignment elementLayout;
        if (!count || !_getNVVMStructuredBufferStorageLayout(
                          codeGenContext,
                          arrayType->getElementType(),
                          elementLayout))
        {
            return false;
        }
        const IRIntegerValue stride =
            _alignNVVMStorageSize(elementLayout.size, elementLayout.alignment);
        if (IRInst* explicitStride = arrayType->getArrayStride())
        {
            auto strideValue = as<IRIntLit>(explicitStride);
            if (!strideValue || strideValue->getValue() != stride)
                return false;
        }
        outLayout.size = count->getValue() * stride;
        outLayout.alignment = elementLayout.alignment;
        return true;
    }

    if (auto structType = as<IRStructType>(type))
    {
        IRIntegerValue size = 0;
        int alignment = 1;
        for (auto field : structType->getFields())
        {
            IRSizeAndAlignment fieldLayout;
            IRIntegerValue cudaOffset = 0;
            if (!_getNVVMStructuredBufferStorageLayout(
                    codeGenContext,
                    field->getFieldType(),
                    fieldLayout) ||
                SLANG_FAILED(getOffset(
                    codeGenContext->getTargetReq(),
                    IRTypeLayoutRules::getCUDA(),
                    field,
                    &cudaOffset)))
            {
                return false;
            }
            size = _alignNVVMStorageSize(size, fieldLayout.alignment);
            if (cudaOffset != size)
                return false;
            size += fieldLayout.size;
            alignment = Math::Max(alignment, fieldLayout.alignment);
        }
        outLayout.size = _alignNVVMStorageSize(size, alignment);
        outLayout.alignment = alignment;
        return true;
    }

    return SLANG_SUCCEEDED(getSizeAndAlignment(
        codeGenContext->getTargetReq(),
        IRTypeLayoutRules::getLLVM(),
        type,
        &outLayout));
}

// Verifies the external element representation selected by one canonical structured-buffer view.
// Byte-address views keep their fixed UInt32 storage contract and need no element comparison.
bool _hasNVVMCompatibleRawBufferElementLayout(CodeGenContext* codeGenContext, IRType* type)
{
    NVVMRawBufferType rawBufferType;
    if (!getNVVMSupportedRawBufferType(type, rawBufferType))
        return false;
    if (rawBufferType.kind == NVVMRawBufferKind::ByteAddress)
        return true;

    // Resource-containing structured-buffer elements were already supported through their exact
    // ordinary value representation. They are a distinct canonical family from the recursively
    // converted numeric/Boolean storage algebra introduced here, so keep proving them with the
    // established CUDA/LLVM value-layout contract.
    if (!isNVVMSupportedStructuredBufferStorageType(rawBufferType.structuredElementType))
    {
        return _hasNVVMCompatibleHelperValueLayout(
            codeGenContext,
            rawBufferType.structuredElementType);
    }

    IRSizeAndAlignment providerLayout;
    IRSizeAndAlignment cudaLayout;
    const bool providerOK = _getNVVMStructuredBufferStorageLayout(
        codeGenContext,
        rawBufferType.structuredElementType,
        providerLayout);
    const bool cudaOK = SLANG_SUCCEEDED(getSizeAndAlignment(
        codeGenContext->getTargetReq(),
        IRTypeLayoutRules::getCUDA(),
        rawBufferType.structuredElementType,
        &cudaLayout));
    // Pointer stride and every nested field offset are fixed by the provider type, while each
    // load/store carries CUDA's explicit conservative alignment. LLVM may prefer a stronger
    // aggregate alignment without changing either memory fact; `Thing { uint, float, half4 }` is
    // the established example (16-byte size, offsets 0/4/8, CUDA alignment 4, LLVM alignment 8).
    return providerOK && cudaOK && providerLayout.size == cudaLayout.size;
}

uint32_t _getNVVMStructuredBufferMemoryAlignment(CodeGenContext* codeGenContext, IRType* type)
{
    IRSizeAndAlignment cudaLayout;
    if (!codeGenContext ||
        SLANG_FAILED(getSizeAndAlignment(
            codeGenContext->getTargetReq(),
            IRTypeLayoutRules::getCUDA(),
            type,
            &cudaLayout)) ||
        cudaLayout.alignment <= 0 || cudaLayout.alignment > UINT32_MAX)
    {
        return 0;
    }
    return uint32_t(cudaLayout.alignment);
}

// Reads the finite byte layout retained on a canonical IR type layout. Collected conventional
// globals carry this metadata after target layout and global-parameter collection, including for
// opaque resource fields that the context-free CUDA layout query cannot inspect.
bool _getNVVMCanonicalByteLayout(IRTypeLayout* typeLayout, IRSizeAndAlignment& outLayout)
{
    outLayout = {};
    if (!typeLayout)
        return false;

    auto size = typeLayout->getSizeInBytes();
    auto alignment = typeLayout->getAlignmentInBytes();
    if (!size.isFinite() || alignment <= 0 || alignment > INT_MAX)
        return false;
    outLayout.size = IRIntegerValue(size.getFiniteValue().getValidValue());
    outLayout.alignment = int(alignment);
    return true;
}

// Looks up one field by its semantic key. Struct-layout entries are key/value metadata, so their
// order is not assumed to match the declaration even though current producers usually preserve it.
IRVarLayout* _findNVVMCanonicalFieldLayout(IRStructTypeLayout* structLayout, IRStructKey* fieldKey)
{
    if (!structLayout || !fieldKey)
        return nullptr;
    for (auto fieldLayoutAttr : structLayout->getFieldLayoutAttrs())
    {
        if (fieldLayoutAttr->getFieldKey() == fieldKey)
            return fieldLayoutAttr->getLayout();
    }
    return nullptr;
}

// Computes the provider layout selected for one aggregate-storage type. Compact vectors use their
// exact CUDA size and alignment, raw-buffer views are pointer/count pairs, and every other leaf
// keeps its ordinary LLVM representation. When target layout has already produced canonical
// metadata, the recursive walk proves provider offsets and strides against that metadata instead
// of trying to reconstruct the layout of opaque resource fields. Local BF16 records explicitly
// enable their qualified leaves here; the default global/resource storage proof stays unchanged.
bool _getNVVMAggregateStorageLayout(
    CodeGenContext* codeGenContext,
    IRType* type,
    IRSizeAndAlignment& outLayout,
    IRTypeLayout* canonicalTypeLayout = nullptr,
    bool isParameterGroupStorage = false,
    bool allowLocalSubstandardRecords = false)
{
    outLayout = {};
    if (!codeGenContext || !type)
        return false;

    if (allowLocalSubstandardRecords && asNVVMBFloat16VectorType(type))
    {
        uint32_t count = 0;
        asNVVMBFloat16VectorType(type, &count);
        outLayout.size = count * 2;
        outLayout.alignment = int(_getNVVMBFloat16VectorStorageAlignment(type));
        IRSizeAndAlignment canonicalLayout;
        return !canonicalTypeLayout ||
               (_getNVVMCanonicalByteLayout(canonicalTypeLayout, canonicalLayout) &&
                canonicalLayout.size == outLayout.size &&
                canonicalLayout.alignment == outLayout.alignment);
    }

    if (asNVVMSupportedCompactParameterGroupVectorType(type))
    {
        if (SLANG_FAILED(getSizeAndAlignment(
                codeGenContext->getTargetReq(),
                IRTypeLayoutRules::getCUDA(),
                type,
                &outLayout)))
        {
            return false;
        }
        if (canonicalTypeLayout)
        {
            IRSizeAndAlignment canonicalLayout;
            return _getNVVMCanonicalByteLayout(canonicalTypeLayout, canonicalLayout) &&
                   canonicalLayout.size == outLayout.size &&
                   canonicalLayout.alignment == outLayout.alignment;
        }
        return true;
    }

    IRType* parameterGroupElementType = nullptr;
    if (asNVVMSupportedParameterGroupType(type, &parameterGroupElementType))
    {
        SLANG_UNUSED(parameterGroupElementType);
        outLayout.size = kNVVMPointerAlignment;
        outLayout.alignment = kNVVMPointerAlignment;
        return true;
    }

    IRArrayType* arrayType =
        isParameterGroupStorage && isNVVMSupportedParameterGroupElementStorageType(type)
            ? as<IRArrayType>(type)
            : asNVVMSupportedAggregateStorageArrayType(type);
    if (!arrayType && allowLocalSubstandardRecords)
        arrayType = asNVVMSupportedLocalSubstandardRecordArrayType(type);
    if (arrayType)
    {
        auto canonicalArrayLayout = as<IRArrayTypeLayout>(canonicalTypeLayout);
        if (canonicalTypeLayout && !canonicalArrayLayout)
            return false;
        if (asNVVMSupportedCompactParameterGroupVectorType(arrayType->getElementType()))
        {
            IRSizeAndAlignment elementLayout;
            if (!_getNVVMAggregateStorageLayout(
                    codeGenContext,
                    arrayType->getElementType(),
                    elementLayout,
                    canonicalArrayLayout ? canonicalArrayLayout->getElementTypeLayout() : nullptr,
                    isParameterGroupStorage,
                    allowLocalSubstandardRecords))
            {
                return false;
            }
            auto elementCount = cast<IRIntLit>(arrayType->getElementCount())->getValue();
            outLayout.size = elementCount * elementLayout.size;
            outLayout.alignment = elementLayout.alignment;
            IRSizeAndAlignment canonicalLayout;
            return !canonicalArrayLayout ||
                   (_getNVVMCanonicalByteLayout(canonicalArrayLayout, canonicalLayout) &&
                    canonicalLayout.size == outLayout.size &&
                    canonicalLayout.alignment == outLayout.alignment &&
                    canonicalArrayLayout->getElementStrideInBytes().isFinite() &&
                    canonicalArrayLayout->getElementStrideInBytes()
                            .getFiniteValue()
                            .getValidValue() == uint64_t(elementLayout.size));
        }

        IRSizeAndAlignment elementLayout;
        if (!_getNVVMAggregateStorageLayout(
                codeGenContext,
                arrayType->getElementType(),
                elementLayout,
                canonicalArrayLayout ? canonicalArrayLayout->getElementTypeLayout() : nullptr,
                isParameterGroupStorage,
                allowLocalSubstandardRecords) ||
            (elementLayout.size <= 0 && !(isParameterGroupStorage && elementLayout.size == 0)) ||
            elementLayout.alignment <= 0)
        {
            return false;
        }
        const auto elementCount = cast<IRIntLit>(arrayType->getElementCount())->getValue();
        const auto elementStride =
            _alignNVVMStorageSize(elementLayout.size, elementLayout.alignment);
        outLayout.size = elementCount * elementStride;
        outLayout.alignment = elementLayout.alignment;
        if (canonicalArrayLayout)
        {
            IRSizeAndAlignment canonicalLayout;
            auto canonicalStride = canonicalArrayLayout->getElementStrideInBytes();
            if (!_getNVVMCanonicalByteLayout(canonicalArrayLayout, canonicalLayout) ||
                !canonicalStride.isFinite() ||
                IRIntegerValue(canonicalStride.getFiniteValue().getValidValue()) != elementStride ||
                canonicalLayout.size != outLayout.size ||
                canonicalLayout.alignment != outLayout.alignment)
            {
                return false;
            }
        }
        return true;
    }

    IRStructType* structType =
        isParameterGroupStorage && isNVVMSupportedParameterGroupElementStorageType(type)
            ? as<IRStructType>(type)
            : asNVVMSupportedAggregateStorageStructType(type);
    if (!structType && allowLocalSubstandardRecords)
        structType = asNVVMSupportedLocalSubstandardRecordType(type);
    if (structType)
    {
        auto canonicalStructLayout = as<IRStructTypeLayout>(canonicalTypeLayout);
        if (canonicalTypeLayout && !canonicalStructLayout)
            return false;
        IRIntegerValue size = 0;
        int alignment = 1;
        for (auto field : structType->getFields())
        {
            IRVarLayout* canonicalFieldLayout =
                canonicalStructLayout
                    ? _findNVVMCanonicalFieldLayout(canonicalStructLayout, field->getKey())
                    : nullptr;
            if (canonicalStructLayout && !canonicalFieldLayout)
                return false;
            IRSizeAndAlignment fieldLayout;
            IRIntegerValue cudaOffset = 0;
            if (!_getNVVMAggregateStorageLayout(
                    codeGenContext,
                    field->getFieldType(),
                    fieldLayout,
                    canonicalFieldLayout ? canonicalFieldLayout->getTypeLayout() : nullptr,
                    isParameterGroupStorage,
                    allowLocalSubstandardRecords) ||
                (fieldLayout.size <= 0 && !(isParameterGroupStorage && fieldLayout.size == 0)) ||
                fieldLayout.alignment <= 0)
            {
                return false;
            }
            size = _alignNVVMStorageSize(size, fieldLayout.alignment);
            if (canonicalFieldLayout)
            {
                auto offsetAttr = canonicalFieldLayout->findOffsetAttr(LayoutResourceKind::Uniform);
                if (!offsetAttr)
                    return false;
                cudaOffset = IRIntegerValue(offsetAttr->getOffset());
            }
            else if (SLANG_FAILED(getOffset(
                         codeGenContext->getTargetReq(),
                         IRTypeLayoutRules::getCUDA(),
                         field,
                         &cudaOffset)))
            {
                return false;
            }
            if (cudaOffset != size)
                return false;
            size += fieldLayout.size;
            alignment = Math::Max(alignment, fieldLayout.alignment);
        }
        outLayout.size = _alignNVVMStorageSize(size, alignment);
        outLayout.alignment = alignment;
        if (canonicalStructLayout)
        {
            IRSizeAndAlignment canonicalLayout;
            if (!_getNVVMCanonicalByteLayout(canonicalStructLayout, canonicalLayout) ||
                canonicalLayout.size != outLayout.size ||
                canonicalLayout.alignment != outLayout.alignment)
            {
                return false;
            }
        }
        return true;
    }

    NVVMRawBufferType rawBufferType;
    if (getNVVMSupportedRawBufferType(type, rawBufferType))
    {
        outLayout.size = 16;
        outLayout.alignment = 8;
        return true;
    }

    NVVMSurfaceType surfaceType;
    NVVMReadOnlyTextureType sampledTextureType;
    if (getNVVMSupportedSurfaceType(type, surfaceType) || isNVVMAccelerationStructureType(type) ||
        getNVVMSupportedReadOnlyTextureType(type, sampledTextureType) ||
        asNVVMSupportedSamplerStorageType(type) ||
        asNVVMSupportedDeviceCopyableValuePointerType(type) ||
        asNVVMSupportedDevicePhysicalStoragePointerType(type) ||
        (isParameterGroupStorage && asNVVMSupportedLayoutTransportPointerType(type)))
    {
        outLayout.size = kNVVMPointerAlignment;
        outLayout.alignment = kNVVMPointerAlignment;
        return true;
    }

    if (!isNVVMSupportedNumericValueType(type) && !isNVVMBoolType(type) &&
        !(allowLocalSubstandardRecords && (isNVVMBFloat16Type(type) || isNVVMFloat8Type(type))))
    {
        return false;
    }
    return SLANG_SUCCEEDED(getSizeAndAlignment(
        codeGenContext->getTargetReq(),
        IRTypeLayoutRules::getLLVM(),
        type,
        &outLayout));
}

bool _hasNVVMCompatibleAggregateStorageLayout(
    CodeGenContext* codeGenContext,
    IRType* type,
    IRTypeLayout* canonicalTypeLayout = nullptr,
    bool isParameterGroupStorage = false,
    bool allowLocalSubstandardRecords = false)
{
    IRSizeAndAlignment providerLayout;
    if (canonicalTypeLayout)
        return _getNVVMAggregateStorageLayout(
            codeGenContext,
            type,
            providerLayout,
            canonicalTypeLayout,
            isParameterGroupStorage,
            allowLocalSubstandardRecords);

    IRSizeAndAlignment cudaLayout;
    return _getNVVMAggregateStorageLayout(
               codeGenContext,
               type,
               providerLayout,
               nullptr,
               isParameterGroupStorage,
               allowLocalSubstandardRecords) &&
           SLANG_SUCCEEDED(getSizeAndAlignment(
               codeGenContext->getTargetReq(),
               IRTypeLayoutRules::getCUDA(),
               type,
               &cudaLayout)) &&
           providerLayout.size == cudaLayout.size &&
           providerLayout.alignment == cudaLayout.alignment;
}

uint32_t _getNVVMPhysicalAggregateStorageAlignment(CodeGenContext* codeGenContext, IRType* type)
{
    if (!asNVVMSupportedPhysicalAggregateStorageStructType(type))
        return 0;

    IRSizeAndAlignment layout;
    if (!_getNVVMAggregateStorageLayout(codeGenContext, type, layout) || layout.alignment <= 0 ||
        layout.alignment > UINT32_MAX)
    {
        return 0;
    }
    return uint32_t(layout.alignment);
}

// Retains the canonical declaration closure of a selected struct. Consider a
// `ParameterBlock<Parameters>` where `Parameters` contains a zero-field `Empty` struct. Global-
// parameter collection retains a typed pointer to `Parameters`, and type lowering therefore
// visits both declarations even when optimized code never loads the handle. The explicit
// parameter-group role carries permission for that finite zero-state child through the closure;
// ordinary signatures, locals, and storage roots continue using their nonempty classifiers.
void _addNVVMReachableStructTypes(
    IRType* type,
    HashSet<IRInst*>& reachableTypes,
    bool allowZeroStateStructs = false)
{
    IRType* pointerValueType = nullptr;
    if (asNVVMSupportedDeviceCopyableValuePointerType(type, &pointerValueType))
        type = pointerValueType;
    else if (auto pointer = asNVVMSupportedLayoutTransportPointerType(type))
        type = pointer->getValueType();
    IRType* parameterGroupElementType = nullptr;
    if (asNVVMSupportedParameterGroupType(type, &parameterGroupElementType))
    {
        type = parameterGroupElementType;
        allowZeroStateStructs = true;
    }
    // A buffer inside a value record still owns its element declaration, even when
    // constant folding leaves only scalar stores and no whole-element value in a function.
    NVVMRawBufferType bufferType;
    if (getNVVMSupportedRawBufferType(type, bufferType) &&
        bufferType.kind == NVVMRawBufferKind::Structured)
    {
        _addNVVMReachableStructTypes(
            bufferType.structuredElementType,
            reachableTypes,
            allowZeroStateStructs);
        return;
    }
    if (auto arrayType = as<IRArrayType>(type))
    {
        if (!asNVVMSupportedHelperArrayType(arrayType) &&
            !asNVVMSupportedResourceArrayType(arrayType) &&
            !asNVVMSupportedAggregateStorageArrayType(arrayType) &&
            !asNVVMSupportedLocalSubstandardRecordArrayType(arrayType) &&
            !(allowZeroStateStructs &&
              isNVVMSupportedParameterGroupElementStorageType(arrayType)) &&
            !isNVVMSupportedStructuredBufferStorageType(arrayType))
            return;
        _addNVVMReachableStructTypes(
            arrayType->getElementType(),
            reachableTypes,
            allowZeroStateStructs);
        return;
    }
    auto structType = as<IRStructType>(type);
    if (!structType ||
        (!asNVVMSupportedHelperStructType(structType) &&
         !asNVVMSupportedResourceStructType(structType) &&
         !asNVVMSupportedAggregateStorageStructType(structType) &&
         !asNVVMSupportedLocalSubstandardRecordType(structType) &&
         !(allowZeroStateStructs && isNVVMSupportedParameterGroupElementStorageType(structType)) &&
         !isNVVMSupportedStructuredBufferStorageType(structType)) ||
        reachableTypes.contains(structType))
        return;
    reachableTypes.add(structType);
    for (auto field : structType->getFields())
    {
        _addNVVMReachableStructTypes(field->getFieldType(), reachableTypes, allowZeroStateStructs);
    }
}

IRIntLit* _asExecutableInteger32Constant(IRInst* value);

struct NVVMSequentialElement
{
    IRInst* base = nullptr;
    IRInst* index = nullptr;
};

// Resolves an integer-indexed element read from one accepted ordinary vector or copyable fixed-
// array value. Constant indices are checked here; a dynamic index retains the source IR value for
// provider lowering.
bool _getNVVMSequentialElement(IRInst* inst, NVVMSequentialElement& outElement)
{
    outElement = {};

    IRInst* base = nullptr;
    IRInst* elementIndex = nullptr;
    if (auto swizzle = as<IRSwizzle>(inst))
    {
        if (swizzle->getElementCount() != 1)
            return false;
        base = swizzle->getBase();
        elementIndex = swizzle->getElementIndex(0);
    }
    else if (auto getElement = as<IRGetElement>(inst))
    {
        base = getElement->getBase();
        elementIndex = getElement->getIndex();
    }
    else
    {
        return false;
    }

    uint32_t baseElementCount = 0;
    IRType* baseElementType = nullptr;
    if (auto baseVectorType =
            asNVVMRegisterVectorType(base ? base->getDataType() : nullptr, &baseElementCount))
    {
        baseElementType = baseVectorType->getElementType();
    }
    else if (!as<IRSwizzle>(inst))
    {
        auto baseArrayType =
            asNVVMSupportedHelperArrayType(base ? base->getDataType() : nullptr, &baseElementCount);
        if (!baseArrayType)
        {
            baseArrayType = asNVVMSupportedResourceArrayType(
                base ? base->getDataType() : nullptr,
                &baseElementCount);
        }
        if (!baseArrayType)
        {
            baseArrayType = asNVVMSupportedLocalSubstandardRecordArrayType(
                base ? base->getDataType() : nullptr,
                &baseElementCount);
        }
        if (baseArrayType)
        {
            baseElementType = baseArrayType->getElementType();
        }
    }
    if (!baseElementType || !isTypeEqual(inst->getDataType(), baseElementType) || !elementIndex ||
        !isNVVMSupportedIntegerScalarType(elementIndex->getDataType()))
    {
        return false;
    }
    if (auto constantIndex = _asExecutableInteger32Constant(elementIndex))
    {
        if (constantIndex->getValue() < 0 || constantIndex->getValue() >= baseElementCount)
            return false;
    }

    outElement.base = base;
    outElement.index = elementIndex;
    return true;
}

struct NVVMVectorConstructElement
{
    IRInst* value = nullptr;
    IRInst* extractedBase = nullptr;
    uint32_t extractedIndex = 0;
};

struct NVVMVectorConstruction
{
    IRVectorType* resultType = nullptr;
    NVVMVectorConstructElement elements[4];
    uint32_t elementCount = 0;
};

// Appends one canonical constructor operand to a flattened lane sequence. For example,
// `half4(half2Value, halfValue, halfValue)` contributes lanes 0-1 from `half2Value`, followed by
// the two scalar operands. The final provider operation can then consume one uniform lane list.
bool _appendNVVMVectorConstructOperand(
    IRInst* operand,
    IRType* resultElementType,
    uint32_t resultElementCount,
    NVVMVectorConstructElement* outElements,
    uint32_t& ioElementCount)
{
    if (!operand || !resultElementType || !outElements || ioElementCount > resultElementCount)
        return false;

    if (isTypeEqual(operand->getDataType(), resultElementType))
    {
        if (ioElementCount >= resultElementCount)
            return false;
        outElements[ioElementCount++].value = operand;
        return true;
    }

    uint32_t operandElementCount = 0;
    auto operandType = asNVVMRegisterVectorType(operand->getDataType(), &operandElementCount);
    if (!operandType || !isTypeEqual(operandType->getElementType(), resultElementType) ||
        operandElementCount > resultElementCount - ioElementCount)
    {
        return false;
    }
    for (uint32_t i = 0; i < operandElementCount; ++i)
    {
        NVVMVectorConstructElement& element = outElements[ioElementCount++];
        element.extractedBase = operand;
        element.extractedIndex = i;
    }
    return true;
}

// Resolves the canonical flat constructor, scalar splat, or multi-lane swizzle of one accepted
// ordinary value vector. Every output lane retains its exact scalar value or base/index source.
bool _getNVVMVectorConstruction(IRInst* inst, NVVMVectorConstruction& outConstruction)
{
    outConstruction = {};
    uint32_t elementCount = 0;
    auto resultType = asNVVMRegisterVectorType(inst ? inst->getDataType() : nullptr, &elementCount);
    if (!resultType)
        return false;

    if (inst->getOp() == kIROp_MakeVector)
    {
        uint32_t flattenedElementCount = 0;
        for (UInt i = 0; i < inst->getOperandCount(); ++i)
        {
            if (!_appendNVVMVectorConstructOperand(
                    inst->getOperand(i),
                    resultType->getElementType(),
                    elementCount,
                    outConstruction.elements,
                    flattenedElementCount))
            {
                return false;
            }
        }
        if (flattenedElementCount != elementCount)
            return false;
    }
    else if (inst->getOp() == kIROp_MakeVectorFromScalar)
    {
        if (inst->getOperandCount() != 1 || !inst->getOperand(0) ||
            !isTypeEqual(inst->getOperand(0)->getDataType(), resultType->getElementType()))
        {
            return false;
        }
        for (uint32_t i = 0; i < elementCount; ++i)
            outConstruction.elements[i].value = inst->getOperand(0);
    }
    else if (auto swizzle = as<IRSwizzle>(inst))
    {
        IRInst* base = swizzle->getBase();
        uint32_t baseElementCount = 0;
        auto baseType =
            asNVVMRegisterVectorType(base ? base->getDataType() : nullptr, &baseElementCount);
        if (!baseType || swizzle->getElementCount() != elementCount ||
            !isTypeEqual(baseType->getElementType(), resultType->getElementType()))
        {
            return false;
        }
        for (uint32_t i = 0; i < elementCount; ++i)
        {
            auto index = _asExecutableInteger32Constant(swizzle->getElementIndex(i));
            if (!index || index->getValue() < 0 || index->getValue() >= baseElementCount)
                return false;
            outConstruction.elements[i].extractedBase = base;
            outConstruction.elements[i].extractedIndex = uint32_t(index->getValue());
        }
    }
    else if (auto swizzleSet = as<IRSwizzleSet>(inst))
    {
        IRInst* base = swizzleSet->getBase();
        IRInst* source = swizzleSet->getSource();
        uint32_t baseElementCount = 0;
        auto baseType =
            asNVVMSupportedValueVectorType(base ? base->getDataType() : nullptr, &baseElementCount);
        const uint32_t sourceElementCount = uint32_t(swizzleSet->getElementCount());
        if (!baseType || !isTypeEqual(baseType, resultType) || !source || sourceElementCount == 0 ||
            sourceElementCount > elementCount)
        {
            return false;
        }

        IRVectorType* sourceType = nullptr;
        if (sourceElementCount == 1)
        {
            if (!isTypeEqual(source->getDataType(), resultType->getElementType()))
                return false;
        }
        else
        {
            uint32_t actualSourceElementCount = 0;
            sourceType =
                asNVVMSupportedValueVectorType(source->getDataType(), &actualSourceElementCount);
            if (!sourceType || actualSourceElementCount != sourceElementCount ||
                !isTypeEqual(sourceType->getElementType(), resultType->getElementType()))
            {
                return false;
            }
        }

        for (uint32_t i = 0; i < elementCount; ++i)
        {
            outConstruction.elements[i].extractedBase = base;
            outConstruction.elements[i].extractedIndex = i;
        }

        uint32_t updatedLaneMask = 0;
        for (uint32_t sourceIndex = 0; sourceIndex < sourceElementCount; ++sourceIndex)
        {
            auto destinationIndex =
                _asExecutableInteger32Constant(swizzleSet->getElementIndex(sourceIndex));
            if (!destinationIndex || destinationIndex->getValue() < 0 ||
                destinationIndex->getValue() >= elementCount)
            {
                return false;
            }
            const uint32_t destinationLane = uint32_t(destinationIndex->getValue());
            const uint32_t laneMask = 1u << destinationLane;
            if (updatedLaneMask & laneMask)
                return false;
            updatedLaneMask |= laneMask;

            NVVMVectorConstructElement& destination = outConstruction.elements[destinationLane];
            if (sourceElementCount == 1)
            {
                destination.value = source;
                destination.extractedBase = nullptr;
            }
            else
            {
                destination.extractedBase = source;
                destination.extractedIndex = sourceIndex;
            }
        }
    }
    else
    {
        return false;
    }

    outConstruction.resultType = resultType;
    outConstruction.elementCount = elementCount;
    return true;
}

struct NVVMAggregateConstruction
{
    IRType* resultType = nullptr;
    uint32_t elementCount = 0;
    bool repeatsSingleElement = false;
    NVVMTypeUse resultUse = NVVMTypeUse::Value;
};

// Resolves a canonical aggregate value whose complete ordered element sequence is explicit in
// final IR. Matrix legalization produces fixed arrays, while ordinary and resource-bearing
// structs preserve their declared field sequence. All map directly to the provider's
// aggregate-generic construction operation.
bool _getNVVMAggregateConstruction(IRInst* inst, NVVMAggregateConstruction& outConstruction)
{
    outConstruction = {};
    if (!inst)
        return false;

    if (inst->getOp() == kIROp_MakeArray)
    {
        uint32_t elementCount = 0;
        auto resultType = asNVVMSupportedHelperArrayType(inst->getDataType(), &elementCount);
        if (!resultType)
            resultType = asNVVMSupportedResourceArrayType(inst->getDataType(), &elementCount);
        if (!resultType)
        {
            resultType =
                asNVVMSupportedLocalSubstandardRecordArrayType(inst->getDataType(), &elementCount);
        }
        if (!resultType)
        {
            resultType =
                asNVVMSupportedAggregateStorageArrayType(inst->getDataType(), &elementCount);
            outConstruction.resultUse = NVVMTypeUse::Storage;
        }
        if (!resultType || inst->getOperandCount() != elementCount)
            return false;
        for (uint32_t i = 0; i < elementCount; ++i)
        {
            IRInst* element = inst->getOperand(i);
            if (!element || !isTypeEqual(element->getDataType(), resultType->getElementType()))
                return false;
        }
        outConstruction.resultType = resultType;
        outConstruction.elementCount = elementCount;
        return true;
    }

    if (inst->getOp() == kIROp_MakeArrayFromElement)
    {
        uint32_t elementCount = 0;
        auto resultType = asNVVMSupportedHelperArrayType(inst->getDataType(), &elementCount);
        if (!resultType)
            resultType = asNVVMSupportedResourceArrayType(inst->getDataType(), &elementCount);
        if (!resultType)
        {
            resultType =
                asNVVMSupportedLocalSubstandardRecordArrayType(inst->getDataType(), &elementCount);
        }
        if (!resultType)
        {
            resultType =
                asNVVMSupportedAggregateStorageArrayType(inst->getDataType(), &elementCount);
            outConstruction.resultUse = NVVMTypeUse::Storage;
        }
        IRInst* element = inst->getOperandCount() == 1 ? inst->getOperand(0) : nullptr;
        if (!resultType || !element ||
            !isTypeEqual(element->getDataType(), resultType->getElementType()))
            return false;
        outConstruction.resultType = resultType;
        outConstruction.elementCount = elementCount;
        outConstruction.repeatsSingleElement = true;
        return true;
    }

    auto resultType = inst->getOp() == kIROp_MakeStruct
                          ? asNVVMSupportedHelperStructType(inst->getDataType())
                          : nullptr;
    if (!resultType && inst->getOp() == kIROp_MakeStruct)
        resultType = asNVVMSupportedSubstandardRecordType(inst->getDataType());
    if (!resultType && inst->getOp() == kIROp_MakeStruct)
        resultType = asNVVMSupportedResourceStructType(inst->getDataType());
    if (!resultType && inst->getOp() == kIROp_MakeStruct)
        resultType = asNVVMSupportedPhysicalArrayStructType(inst->getDataType());
    if (!resultType)
        return false;

    uint32_t elementIndex = 0;
    for (auto field : resultType->getFields())
    {
        if (elementIndex >= inst->getOperandCount())
            return false;
        IRInst* element = inst->getOperand(elementIndex);
        if (!element || !isTypeEqual(element->getDataType(), field->getFieldType()))
            return false;
        ++elementIndex;
    }
    if (elementIndex != inst->getOperandCount())
        return false;

    outConstruction.resultType = resultType;
    outConstruction.elementCount = elementIndex;
    return true;
}

// Resolves the exact resource leaves retained when optional lowering constructs the irrelevant
// payload of `none`. Raw structured buffers use their established pointer/count view, while the
// selected texture and sampler descriptor handles are aliases of an i64 resource handle.
bool _resolveNVVMDefaultResourceValue(
    IRInst* inst,
    NVVMPlannedDefaultResourceValue& outDefaultValue)
{
    outDefaultValue = {};
    if (!as<IRDefaultConstruct>(inst) || inst->getOperandCount() != 0)
        return false;

    auto resultType = inst->getDataType();
    NVVMRawBufferType rawBufferType;
    if (getNVVMSupportedRawBufferType(resultType, rawBufferType) &&
        rawBufferType.kind == NVVMRawBufferKind::Structured)
    {
        outDefaultValue.resultType = resultType;
        outDefaultValue.kind = NVVMPlannedDefaultResourceValueKind::RawStructuredBuffer;
        outDefaultValue.structuredElementType = rawBufferType.structuredElementType;
        outDefaultValue.source = inst;
        return true;
    }

    IRType* resourceType = nullptr;
    NVVMReadOnlyTextureType textureType;
    if (asNVVMSupportedDescriptorHandleType(resultType, &resourceType) &&
        (getNVVMSupportedReadOnlyTextureType(resourceType, textureType) ||
         asNVVMSupportedSamplerValueType(resourceType)))
    {
        outDefaultValue.resultType = resultType;
        outDefaultValue.kind = NVVMPlannedDefaultResourceValueKind::DescriptorHandle;
        outDefaultValue.source = inst;
        return true;
    }
    return false;
}

struct NVVMAggregateElement
{
    IRInst* base = nullptr;
    IRArrayType* baseType = nullptr;
    uint32_t index = 0;
};

// Resolves one statically selected element from a canonical fixed-array value. LLVM aggregate
// extraction is structurally indexed, so dynamic source indexing remains outside this contract.
bool _getNVVMAggregateElement(IRInst* inst, NVVMAggregateElement& outElement)
{
    outElement = {};
    auto getElement = as<IRGetElement>(inst);
    IRInst* base = getElement ? getElement->getBase() : nullptr;
    uint32_t elementCount = 0;
    auto baseType =
        asNVVMSupportedHelperArrayType(base ? base->getDataType() : nullptr, &elementCount);
    if (!baseType)
    {
        baseType =
            asNVVMSupportedResourceArrayType(base ? base->getDataType() : nullptr, &elementCount);
    }
    auto index = getElement ? _asExecutableInteger32Constant(getElement->getIndex()) : nullptr;
    if (!baseType || !isTypeEqual(inst->getDataType(), baseType->getElementType()) || !index ||
        index->getValue() < 0 || index->getValue() >= elementCount)
    {
        return false;
    }
    outElement.base = base;
    outElement.baseType = baseType;
    outElement.index = uint32_t(index->getValue());
    return true;
}

struct NVVMVectorSwizzledStore
{
    IRInst* destination = nullptr;
    IRInst* source = nullptr;
    IRVectorType* destinationType = nullptr;
    IRType* elementType = nullptr;
    uint32_t sourceElementCount = 0;
    uint32_t destinationIndices[4] = {};
};

// Resolves the canonical constant-lane store to an accepted RWStructuredBuffer vector element.
// Final IR owns the exact destination mapping, so emission can consume it without reconstructing
// the source l-value swizzle.
bool _getNVVMVectorSwizzledStore(IRInst* inst, NVVMVectorSwizzledStore& outStore)
{
    outStore = {};

    auto swizzledStore = as<IRSwizzledStore>(inst);
    if (!swizzledStore || swizzledStore->getOperandCount() < 3)
        return false;

    IRInst* destination = swizzledStore->getOperand(0);
    IRInst* source = swizzledStore->getOperand(1);
    auto destinationPointerType =
        destination
            ? asNVVMSupportedRWStructuredBufferElementPointerType(destination->getDataType())
            : nullptr;
    uint32_t destinationElementCount = 0;
    auto destinationType = destinationPointerType ? asNVVMSupported32BitNumericVectorType(
                                                        destinationPointerType->getValueType(),
                                                        &destinationElementCount)
                                                  : nullptr;
    const uint32_t sourceElementCount = uint32_t(swizzledStore->getElementCount());
    if (!destinationType || !source || sourceElementCount == 0 ||
        sourceElementCount > destinationElementCount)
    {
        return false;
    }

    IRType* elementType = destinationType->getElementType();
    if (sourceElementCount == 1)
    {
        if (!isTypeEqual(source->getDataType(), elementType))
            return false;
    }
    else
    {
        uint32_t sourceVectorElementCount = 0;
        auto sourceType =
            asNVVMSupported32BitNumericVectorType(source->getDataType(), &sourceVectorElementCount);
        if (!sourceType || sourceVectorElementCount != sourceElementCount ||
            !isTypeEqual(sourceType->getElementType(), elementType))
        {
            return false;
        }
    }

    uint32_t usedDestinationLanes = 0;
    for (uint32_t sourceIndex = 0; sourceIndex < sourceElementCount; ++sourceIndex)
    {
        auto destinationIndex =
            _asExecutableInteger32Constant(swizzledStore->getElementIndex(sourceIndex));
        if (!destinationIndex || destinationIndex->getValue() < 0 ||
            destinationIndex->getValue() >= destinationElementCount)
        {
            return false;
        }
        const uint32_t lane = uint32_t(destinationIndex->getValue());
        const uint32_t laneMask = 1u << lane;
        if (usedDestinationLanes & laneMask)
            return false;
        usedDestinationLanes |= laneMask;
        outStore.destinationIndices[sourceIndex] = lane;
    }

    outStore.destination = destination;
    outStore.source = source;
    outStore.destinationType = destinationType;
    outStore.elementType = elementType;
    outStore.sourceElementCount = sourceElementCount;
    return true;
}

struct ScopedNVVMDeviceLibrary
{
    const NVVMIRBuilder* builder = nullptr;
    SlangNVVMDeviceLibraryHandle library = nullptr;
    ~ScopedNVVMDeviceLibrary()
    {
        if (library)
            builder->destroyDeviceLibrary(library);
    }
};

struct ScopedNVVMModule
{
    const NVVMIRBuilder* builder = nullptr;
    SlangNVVMModuleHandle module = nullptr;

    ~ScopedNVVMModule()
    {
        if (builder && module)
            builder->destroyModule(module);
    }
};

SlangResult _diagnoseUnsupportedIR(
    CodeGenContext* codeGenContext,
    const UnownedStringSlice& construct)
{
    codeGenContext->getSink()->diagnose(
        Diagnostics::NvvmUnsupportedIr{.construct = String(construct)});
    return SLANG_E_NOT_IMPLEMENTED;
}

// Appends a stable description of a canonical helper-boundary type. `getTypeNameHint` deliberately
// omits pointer wrappers, so spell their role, pointee, address space, and access contract here.
void _appendNVVMCanonicalTypeName(StringBuilder& out, IRInst* type)
{
    SLANG_RELEASE_ASSERT(type);
    if (auto arrayType = as<IRArrayType>(type))
    {
        out << "Array<";
        _appendNVVMCanonicalTypeName(out, arrayType->getElementType());
        out << ", ";
        if (auto elementCount = as<IRIntLit>(arrayType->getElementCount()))
            out << elementCount->getValue();
        else
            out << getIROpInfo(arrayType->getElementCount()->getOp()).name;
        out << ">";
        return;
    }
    if (auto pointerType = as<IRPtrTypeBase>(type))
    {
        out << getIROpInfo(type->getOp()).name << "<";
        _appendNVVMCanonicalTypeName(out, pointerType->getValueType());
        if (pointerType->getOperandCount() > 1)
        {
            out << ", addressSpace=" << uint64_t(pointerType->getAddressSpace())
                << ", access=" << uint64_t(pointerType->getAccessQualifier())
                << ", operands=" << pointerType->getOperandCount();
            if (IRType* dataLayout = pointerType->getDataLayout())
                out << ", layout=" << getIROpInfo(dataLayout->getOp()).name;
        }
        out << ">";
        return;
    }

    const Index typeNameStart = out.getLength();
    getTypeNameHint(out, type);
    if (out.getLength() == typeNameStart)
        out << getIROpInfo(type->getOp()).name;
}

// Reports the exact canonical type at a role-based preflight boundary. The final linked type is
// the source of truth here; source syntax may no longer describe the specialized helper contract.
SlangResult _diagnoseUnsupportedIRType(
    CodeGenContext* codeGenContext,
    const char* role,
    IRInst* type)
{
    SLANG_RELEASE_ASSERT(type);
    StringBuilder construct;
    construct << role << ": ";
    _appendNVVMCanonicalTypeName(construct, type);
    return _diagnoseUnsupportedIR(codeGenContext, construct.getUnownedSlice());
}

// Reports an exact canonical source/destination type relation. Call legality is relational, so a
// diagnostic naming only the argument or only the parameter would hide the invariant that failed.
SlangResult _diagnoseUnsupportedIRTypeRelation(
    CodeGenContext* codeGenContext,
    const char* role,
    IRInst* sourceType,
    IRInst* destinationType)
{
    SLANG_RELEASE_ASSERT(sourceType && destinationType);
    StringBuilder construct;
    construct << role << ": ";
    _appendNVVMCanonicalTypeName(construct, sourceType);
    construct << " -> ";
    _appendNVVMCanonicalTypeName(construct, destinationType);
    return _diagnoseUnsupportedIR(codeGenContext, construct.getUnownedSlice());
}

// Reports the canonical CUDA assembly template together with its final linked helper signature.
// The template and signature jointly identify one semantic overload; either value alone is
// insufficient. Escape diagnostic delimiters and controls without interpreting the assembly.
SlangResult _diagnoseUnsupportedGenericAsm(
    CodeGenContext* codeGenContext,
    IRGenericAsm* genericAsm,
    IRFunc* function)
{
    SLANG_RELEASE_ASSERT(genericAsm && function);
    StringBuilder construct;
    construct << "GenericAsm assembly=";
    for (const char* cursor = genericAsm->getAsm().begin(); cursor != genericAsm->getAsm().end();
         ++cursor)
    {
        switch (*cursor)
        {
        case '\\':
            construct << "\\\\";
            break;
        case '\'':
            construct << "\\x27";
            break;
        case '\r':
            construct << "\\r";
            break;
        case '\n':
            construct << "\\n";
            break;
        case '\t':
            construct << "\\t";
            break;
        default:
            construct.append(*cursor);
            break;
        }
    }
    construct << ", signature=";
    _appendNVVMCanonicalTypeName(construct, function->getResultType());
    construct << "(";
    for (UInt parameterIndex = 0; parameterIndex < function->getParamCount(); ++parameterIndex)
    {
        if (parameterIndex)
            construct << ", ";
        _appendNVVMCanonicalTypeName(construct, function->getParamType(parameterIndex));
    }
    construct << ")";
    return _diagnoseUnsupportedIR(codeGenContext, construct.getUnownedSlice());
}

SlangResult _requireBuilderOperation(
    CodeGenContext* codeGenContext,
    const char* operation,
    SlangResult result)
{
    if (SLANG_SUCCEEDED(result))
        return result;

    codeGenContext->getSink()->diagnose(Diagnostics::NvvmIrBuilderOperationFailed{
        .operation = String(operation),
        .resultCode = result,
    });
    return result;
}

// Crosses the physical i16 scalar/vector boundary selected for a canonical Half helper parameter
// or result. Only the LLVM call representation changes; helper bodies retain canonical Half values.
SlangResult _emitNVVMHalfHelperABIReinterpretation(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    NVVMTypeLoweringContext& typeContext,
    IRType* canonicalType,
    bool toPhysical,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle& outValue)
{
    if (!hasNVVMHalfHelperABITransport(canonicalType))
    {
        outValue = value;
        return SLANG_OK;
    }
    if (!getNVVMHalfHelperABILaneCount(canonicalType))
    {
        List<IRType*> elementTypes;
        uint32_t count = 0;
        if (auto array = as<IRArrayType>(canonicalType))
        {
            elementTypes.add(array->getElementType());
            count = uint32_t(cast<IRIntLit>(array->getElementCount())->getValue());
        }
        else
        {
            for (auto field : cast<IRStructType>(canonicalType)->getFields())
                elementTypes.add(field->getFieldType());
            count = uint32_t(elementTypes.getCount());
        }
        List<SlangNVVMValueHandle> elements;
        for (uint32_t i = 0; i < count; ++i)
        {
            SlangNVVMValueHandle element = nullptr, converted = nullptr;
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "Half helper aggregate element",
                builder.emitAggregateElementExtract(module, value, i, element)));
            SLANG_RETURN_ON_FAIL(_emitNVVMHalfHelperABIReinterpretation(
                codeGenContext,
                builder,
                module,
                typeContext,
                elementTypes[as<IRArrayType>(canonicalType) ? 0 : i],
                toPhysical,
                element,
                converted));
            elements.add(converted);
        }
        SlangNVVMTypeHandle resultType = nullptr;
        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
            canonicalType,
            toPhysical ? NVVMTypeUse::HelperParameter : NVVMTypeUse::Value,
            resultType));
        return _requireBuilderOperation(
            codeGenContext,
            "Half helper aggregate reconstruction",
            builder.emitAggregateConstruct(
                module,
                resultType,
                elements.getBuffer(),
                elements.getCount(),
                outValue));
    }
    const NVVMHalfHelperABIOperation operation(canonicalType, toPhysical);
    return _requireBuilderOperation(
        codeGenContext,
        toPhysical ? "physical Half helper ABI encoding" : "canonical Half helper ABI decoding",
        builder.emitValueOperation(module, operation.getDesc(), &value, 1, outValue));
}

// Widens one pointer whose producer proves global-memory provenance into the helper UserPointer
// representation. Helper values can also carry local addresses, so their pointer leaves use LLVM
// generic pointers even though ordinary kernel execution and conventional-global storage keep the
// producer-proven AS1 representation.
SlangResult _emitNVVMExecutableUserPointer(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    IRType* canonicalType,
    SlangNVVMValueHandle globalPointer,
    NVVMTypeLoweringContext& typeContext,
    SlangNVVMValueHandle& outPointer)
{
    SLANG_RELEASE_ASSERT(asNVVMSupportedDeviceCopyableValuePointerType(canonicalType));
    SlangNVVMTypeHandle executableType = nullptr;
    SLANG_RETURN_ON_FAIL(typeContext.lowerType(canonicalType, NVVMTypeUse::Value, executableType));
    return _requireBuilderOperation(
        codeGenContext,
        "global-to-generic UserPointer conversion",
        builder.emitPointerAddressSpaceCast(module, executableType, globalPointer, outPointer));
}

// Returns an executable signed-i32 literal, excluding layout and other module constants.
IRIntLit* _asExecutableI32Constant(IRInst* value)
{
    auto intLit = as<IRIntLit>(value);
    if (!intLit || !isNVVMSignedI32Type(intLit->getDataType()))
        return nullptr;

    const IRIntegerValue intValue = intLit->getValue();
    return intValue >= kNVVMI32Min && intValue <= kNVVMI32Max ? intLit : nullptr;
}

// Returns an executable signed or unsigned 32-bit literal, excluding module/layout constants.
IRIntLit* _asExecutableInteger32Constant(IRInst* value)
{
    if (auto intLit = _asExecutableI32Constant(value))
        return intLit;

    auto intLit = as<IRIntLit>(value);
    if (!intLit || !isNVVMUnsignedI32Type(intLit->getDataType()))
        return nullptr;

    const IRIntegerValue intValue = intLit->getValue();
    return intValue >= 0 && intValue <= kNVVMUInt32Max ? intLit : nullptr;
}

// Returns an executable literal in one selected integer width. Canonical UInt64 uses the signed
// storage bits of IRIntegerValue when its high bit is set; the provider preserves that bit pattern.
IRIntLit* _asExecutableSelectedIntegerConstant(IRInst* value)
{
    auto intLit = as<IRIntLit>(value);
    uint32_t bitWidth = 0;
    bool isSigned = false;
    if (!intLit || !isNVVMSupportedIntegerScalarType(intLit->getDataType(), &bitWidth, &isSigned))
    {
        return nullptr;
    }

    const IRIntegerValue integerValue = intLit->getValue();
    if (bitWidth == 64)
        return intLit;
    if (isSigned)
    {
        const IRIntegerValue minimum = -(IRIntegerValue(1) << (bitWidth - 1));
        const IRIntegerValue maximum = (IRIntegerValue(1) << (bitWidth - 1)) - 1;
        return integerValue >= minimum && integerValue <= maximum ? intLit : nullptr;
    }
    const IRIntegerValue maximum = (IRIntegerValue(1) << bitWidth) - 1;
    return integerValue >= 0 && integerValue <= maximum ? intLit : nullptr;
}

// Returns the canonical null literal for an admitted complete CUDA device-pointer type.
IRPtrLit* _asExecutableNullDevicePointer(IRInst* value)
{
    auto pointerLiteral = as<IRPtrLit>(value);
    return pointerLiteral && !pointerLiteral->getValue() &&
                   asNVVMSupportedDeviceCopyableValuePointerType(pointerLiteral->getDataType())
               ? pointerLiteral
               : nullptr;
}

// Returns a canonical executable Boolean literal.
IRBoolLit* _asExecutableBoolConstant(IRInst* value)
{
    auto boolLit = as<IRBoolLit>(value);
    return boolLit && isNVVMBoolType(boolLit->getDataType()) ? boolLit : nullptr;
}

// Returns an executable selected floating-point literal, excluding layout and module constants.
IRFloatLit* _asExecutableFloatingPointConstant(IRInst* value)
{
    auto floatLit = as<IRFloatLit>(value);
    return floatLit && (isNVVMSupportedFloatingPointScalarType(floatLit->getDataType()) ||
                        isNVVMBFloat16Type(floatLit->getDataType()) ||
                        isNVVMFloat8Type(floatLit->getDataType()))
               ? floatLit
               : nullptr;
}

// Recognizes one finite module-owned constant value tree. Consider this example:
//
//     static const float3x2 values[2] = { ... };
//
// Matrix legalization retains module-scope `makeVector` leaves, `makeArray` matrix rows, and one
// outer `makeArray`. These are immutable SSA values rather than storage declarations. Prove the
// complete literal/construction tree here so a function can materialize it with the same generic
// operations used for an equivalent local expression. Arbitrary module operations and cyclic
// graphs remain outside the contract.
bool _isNVVMSupportedModuleConstantValue(IRInst* value, HashSet<IRInst*>& activeValues)
{
    if (!value || !value->getModule() || value->getParent() != value->getModule()->getModuleInst())
    {
        return false;
    }
    if (_asExecutableSelectedIntegerConstant(value) || _asExecutableBoolConstant(value) ||
        _asExecutableFloatingPointConstant(value) || _asExecutableNullDevicePointer(value))
    {
        return true;
    }
    if (activeValues.contains(value))
        return false;

    activeValues.add(value);
    NVVMVectorConstruction vectorConstruction;
    if (_getNVVMVectorConstruction(value, vectorConstruction))
    {
        for (uint32_t i = 0; i < vectorConstruction.elementCount; ++i)
        {
            IRInst* element = vectorConstruction.elements[i].value;
            if (!element || !_isNVVMSupportedModuleConstantValue(element, activeValues))
            {
                activeValues.remove(value);
                return false;
            }
        }
        activeValues.remove(value);
        return true;
    }

    NVVMAggregateConstruction aggregateConstruction;
    if (_getNVVMAggregateConstruction(value, aggregateConstruction) &&
        aggregateConstruction.resultUse == NVVMTypeUse::Value)
    {
        for (uint32_t i = 0; i < aggregateConstruction.elementCount; ++i)
        {
            IRInst* element = value->getOperand(aggregateConstruction.repeatsSingleElement ? 0 : i);
            if (!_isNVVMSupportedModuleConstantValue(element, activeValues))
            {
                activeValues.remove(value);
                return false;
            }
        }
        activeValues.remove(value);
        return true;
    }

    activeValues.remove(value);
    return false;
}

bool _isNVVMSupportedModuleConstantValue(IRInst* value)
{
    HashSet<IRInst*> activeValues;
    return _isNVVMSupportedModuleConstantValue(value, activeValues);
}

// Resolves canonical values and markers that code generation consumes without a source-level CUDA
// expression. Every accepted form has one upstream semantic source of truth: SSA construction
// chooses an arbitrary value, GPU string validation proves the literal, and inlining owns the
// debug-scope marker.
bool _resolveNVVMEphemeralValue(IRInst* inst, NVVMPlannedEphemeralValue& outValue)
{
    outValue = {};
    if (!inst)
        return false;

    switch (inst->getOp())
    {
    case kIROp_LoadFromUninitializedMemory:
        if (!isNVVMSupportedCopyableValueType(inst->getDataType()) &&
            !asNVVMSupportedSamplerValueType(inst->getDataType()))
        {
            return false;
        }
        outValue.kind = NVVMPlannedEphemeralValueKind::ChosenUndefined;
        outValue.valueType = inst->getDataType();
        outValue.source = inst;
        return true;

    case kIROp_GetStringHash:
        if (inst->getOperandCount() != 1 || !isNVVMSignedI32Type(inst->getDataType()))
            return false;
        outValue.stringLiteral = as<IRStringLit>(inst->getOperand(0));
        if (!outValue.stringLiteral)
            return false;
        outValue.kind = NVVMPlannedEphemeralValueKind::StableStringHash;
        outValue.valueType = inst->getDataType();
        outValue.source = inst;
        return true;

    case kIROp_DebugNoScope:
        if (!as<IRVoidType>(inst->getDataType()))
            return false;
        outValue.kind = NVVMPlannedEphemeralValueKind::IgnoredDebugNoScope;
        outValue.source = inst;
        return true;

    default:
        return false;
    }
}

bool _getNVVMSemanticType(IRType* type, SlangNVVMValueTypeDesc& outType);

// Validates the physical operation contract produced by surface legalization. Format provenance,
// conversion, and byte-coordinate arithmetic have already become explicit IR at this boundary.
bool _resolveNVVMPhysicalSurfaceOperation(IRInst* inst, NVVMPlannedSurfaceOperation& outOperation)
{
    outOperation = {};
    const bool isLoad = inst->getOp() == kIROp_NVVMSurfaceLoad;
    const bool isStore = inst->getOp() == kIROp_NVVMSurfaceStore;
    if ((!isLoad && !isStore) || inst->getOperandCount() != (isLoad ? 2u : 3u))
        return false;
    IRInst* surface = inst->getOperand(0);
    IRInst* coordinate = inst->getOperand(1);
    IRInst* value = isStore ? inst->getOperand(2) : nullptr;
    NVVMSurfaceType surfaceType;
    SlangNVVMValueTypeDesc physicalType = {};
    if (!surface || !coordinate ||
        !getNVVMSupportedSurfaceType(surface->getDataType(), surfaceType) ||
        !_getNVVMSemanticType(isLoad ? inst->getDataType() : value->getDataType(), physicalType))
        return false;
    // Normalized legalization has already selected and made the floating/integer conversion
    // explicit. This boundary validates the physical operation, without rediscovering formats.
    const bool normalizedStorage =
        surfaceType.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
        (surfaceType.elementType.bitWidth == 16 || surfaceType.elementType.bitWidth == 32) &&
        (physicalType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
         physicalType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER) &&
        (physicalType.bitWidth == 8 || physicalType.bitWidth == 16);
    if ((physicalType.kind != surfaceType.elementType.kind && !normalizedStorage) ||
        physicalType.laneCount != surfaceType.elementType.laneCount ||
        (physicalType.laneCount != 1 && physicalType.laneCount != 2 &&
         physicalType.laneCount != 4) ||
        (physicalType.bitWidth != surfaceType.elementType.bitWidth && !normalizedStorage &&
         !(physicalType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
           physicalType.bitWidth == 16 && surfaceType.elementType.bitWidth == 32) &&
         !((physicalType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
            physicalType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER) &&
           (physicalType.bitWidth == 8 || physicalType.bitWidth == 16) &&
           surfaceType.elementType.bitWidth == 32)) ||
        (isStore && !as<IRVoidType>(inst->getDataType())))
        return false;

    if (!isNVVMSignedI32Type(getIRVectorBaseType(coordinate->getDataType())) ||
        UInt(getIRVectorElementSize(coordinate->getDataType())) != surfaceType.coordinateLaneCount)
        return false;
    outOperation.desc = {
        isLoad ? SLANG_NVVM_SURFACE_OP_LOAD : SLANG_NVVM_SURFACE_OP_STORE,
        surfaceType.shape,
        surfaceType.isArray,
        physicalType,
        SLANG_NVVM_SURFACE_BOUNDARY_ZERO,
    };
    outOperation.surface = surface;
    outOperation.coordinate = coordinate;
    outOperation.value = value;
    outOperation.source = inst;
    outOperation.diagnosticName = isLoad ? "physical surface load" : "physical surface store";
    return true;
}

void _requireSurfaceOperation(
    List<NVVMSurfaceOperationRequirement>& requirements,
    IRInst* source,
    const SlangNVVMSurfaceOperationDesc& desc,
    const char* diagnosticName)
{
    for (const auto& requirement : requirements)
    {
        if (requirement.source != source)
            continue;
        const auto& existing = requirement.desc;
        SLANG_RELEASE_ASSERT(
            existing.operation == desc.operation && existing.shape == desc.shape &&
            existing.isArray == desc.isArray && existing.boundaryMode == desc.boundaryMode &&
            NVVMSemantics::areSameType(existing.elementType, desc.elementType));
        return;
    }
    requirements.add({source, desc, diagnosticName});
}

// Resolves logical texture instructions while resource types still carry shape and access.
// Consider `texture.Load(int3(x, y, mip))`: core splits the packed location into an integer
// coordinate and a level before constructing TextureFetch. This boundary validates that typed
// contract once; the provider receives the same descriptors used by sampling and queries.
bool _resolveNVVMTextureOperation(IRInst* inst, NVVMTextureOperationRequirement& outOperation)
{
    outOperation = {};
    const IROp op = inst->getOp();
    const bool isLayerQuery = op == kIROp_TextureQueryLayerCount;
    const bool isQuery = op == kIROp_TextureQuerySize || isLayerQuery;
    const bool isFetch = op == kIROp_TextureFetch;
    const bool isGather = op == kIROp_TextureGather;
    const bool isLevel = op == kIROp_SampleLevel;
    const UInt operandCount = isQuery ? 1 : (isFetch || op == kIROp_Sample) ? 3 : 4;
    if (inst->getOperandCount() != operandCount)
        return false;

    IRInst* texture = inst->getOperand(0);
    NVVMReadOnlyTextureType textureType;
    NVVMSurfaceType surfaceType;
    const bool isSurfaceQuery =
        isQuery && getNVVMSupportedSurfaceType(texture->getDataType(), surfaceType);
    if (isSurfaceQuery)
    {
        textureType.textureType = surfaceType.textureType;
        textureType.shape = SlangNVVMTextureShape(surfaceType.shape);
        textureType.isArray = surfaceType.isArray;
        textureType.coordinateLaneCount = surfaceType.coordinateLaneCount;
        textureType.elementType = surfaceType.elementType;
    }
    else if (!getNVVMSupportedReadOnlyTextureType(texture->getDataType(), textureType))
        return false;
    outOperation.source = inst;
    outOperation.texture = texture;
    outOperation.operationCount = 1;
    auto& operation = outOperation.operations[0];
    operation.shape = textureType.shape;
    operation.isArray = textureType.isArray ? 1u : 0u;
    operation.elementType = textureType.elementType;

    if (isLayerQuery)
    {
        if (!textureType.isArray ||
            (textureType.shape != SLANG_NVVM_TEXTURE_SHAPE_1D &&
             textureType.shape != SLANG_NVVM_TEXTURE_SHAPE_2D) ||
            !isNVVMUnsignedI32Type(inst->getDataType()))
            return false;
        operation.operation = isSurfaceQuery ? SLANG_NVVM_TEXTURE_OP_SURFACE_QUERY_ARRAY_SIZE
                              : textureType.shape == SLANG_NVVM_TEXTURE_SHAPE_1D
                                  ? SLANG_NVVM_TEXTURE_OP_QUERY_HEIGHT
                                  : SLANG_NVVM_TEXTURE_OP_QUERY_DEPTH;
        outOperation.diagnosticName =
            isSurfaceQuery ? "surface array size" : "sampled texture array layer count";
        return true;
    }
    if (isQuery)
    {
        const UInt rank = textureType.shape == SLANG_NVVM_TEXTURE_SHAPE_1D   ? 1
                          : textureType.shape == SLANG_NVVM_TEXTURE_SHAPE_3D ? 3
                                                                             : 2;
        if (!isNVVMUnsignedI32Type(getIRVectorBaseType(inst->getDataType())) ||
            UInt(getIRVectorElementSize(inst->getDataType())) != rank)
            return false;
        const SlangNVVMTextureOperation queryOperations[] = {
            SLANG_NVVM_TEXTURE_OP_QUERY_WIDTH,
            SLANG_NVVM_TEXTURE_OP_QUERY_HEIGHT,
            SLANG_NVVM_TEXTURE_OP_QUERY_DEPTH,
        };
        outOperation.operationCount = uint32_t(rank);
        outOperation.diagnosticName =
            isSurfaceQuery ? "surface dimension query" : "sampled texture dimension query";
        for (UInt i = 0; i < rank; ++i)
        {
            outOperation.operations[i] = operation;
            const SlangNVVMTextureOperation surfaceQueries[] = {
                SLANG_NVVM_TEXTURE_OP_SURFACE_QUERY_WIDTH,
                SLANG_NVVM_TEXTURE_OP_SURFACE_QUERY_HEIGHT,
                SLANG_NVVM_TEXTURE_OP_SURFACE_QUERY_DEPTH,
            };
            outOperation.operations[i].operation =
                isSurfaceQuery ? surfaceQueries[i] : queryOperations[i];
        }
        return true;
    }

    if (!isFetch && !asNVVMSupportedSamplerValueType(inst->getOperand(1)->getDataType()))
        return false;
    IRInst* coordinate = inst->getOperand(isFetch ? 1 : 2);
    if (UInt(getIRVectorElementSize(coordinate->getDataType())) != textureType.coordinateLaneCount)
        return false;
    IRType* coordinateScalar = getIRVectorBaseType(coordinate->getDataType());
    if (isFetch ? !isNVVMSignedI32Type(coordinateScalar) : !isNVVMFloat32Type(coordinateScalar))
        return false;
    outOperation.coordinate = coordinate;

    if (isGather)
    {
        auto component = as<IRIntLit>(inst->getOperand(3));
        IRType* scalarType = getIRVectorBaseType(textureType.textureType->getElementType());
        if (textureType.shape != SLANG_NVVM_TEXTURE_SHAPE_2D || textureType.isArray || !component ||
            component->getValue() < 0 || component->getValue() > 3 ||
            !isNVVMSignedI32Type(component->getDataType()) ||
            getIRVectorElementSize(inst->getDataType()) != 4 ||
            !isTypeEqual(getIRVectorBaseType(inst->getDataType()), scalarType))
            return false;
        operation.operation = SLANG_NVVM_TEXTURE_OP_GATHER;
        operation.component = uint32_t(component->getValue());
        operation.elementType.laneCount = 4;
        outOperation.diagnosticName = "ordinary Texture2D gather";
        return true;
    }
    if (!isTypeEqual(inst->getDataType(), textureType.textureType->getElementType()))
        return false;
    if (isFetch)
    {
        if ((textureType.shape != SLANG_NVVM_TEXTURE_SHAPE_2D &&
             textureType.shape != SLANG_NVVM_TEXTURE_SHAPE_3D &&
             !(textureType.shape == SLANG_NVVM_TEXTURE_SHAPE_1D && textureType.isArray)) ||
            !isNVVMSignedI32Type(inst->getOperand(2)->getDataType()))
            return false;
        operation.operation = SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL;
        outOperation.level = inst->getOperand(2);
        outOperation.diagnosticName = "integer-coordinate sampled texture fetch";
        return true;
    }
    if (textureType.elementType.kind != SLANG_NVVM_VALUE_TYPE_FLOATING_POINT ||
        textureType.elementType.bitWidth != 32 ||
        (textureType.elementType.laneCount != 1 && textureType.elementType.laneCount != 2 &&
         textureType.elementType.laneCount != 4))
        return false;
    if (isLevel)
    {
        if (!isNVVMFloat32Type(inst->getOperand(3)->getDataType()))
            return false;
        outOperation.level = inst->getOperand(3);
    }
    operation.operation =
        isLevel ? SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL : SLANG_NVVM_TEXTURE_OP_SAMPLE;
    outOperation.diagnosticName =
        isLevel ? "sampled texture level operation" : "implicit sampled texture operation";
    return true;
}

const NVVMTextureOperationRequirement* _findTextureOperationRequirement(
    const List<NVVMTextureOperationRequirement>& requirements,
    IRInst* source)
{
    for (const auto& requirement : requirements)
        if (requirement.source == source)
            return &requirement;
    return nullptr;
}

// Converts one canonical Slang type to its stable provider semantic role.
bool _getNVVMSemanticType(IRType* type, SlangNVVMValueTypeDesc& outType)
{
    if (as<IRVoidType>(type))
        outType = NVVMSemantics::kVoid;
    else if (auto vectorType = asNVVMRegisterVectorType(type))
    {
        IRType* elementType = vectorType->getElementType();
        uint32_t bitWidth = 0;
        bool isSigned = false;
        uint32_t elementCount = 0;
        SLANG_RELEASE_ASSERT(asNVVMRegisterVectorType(type, &elementCount));
        if (isNVVMSupportedIntegerScalarType(elementType, &bitWidth, &isSigned))
            outType = {
                isSigned ? SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER
                         : SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
                bitWidth,
                elementCount,
            };
        else if (uint32_t floatingPointBitWidth = 0;
                 isNVVMSupportedFloatingPointScalarType(elementType, &floatingPointBitWidth))
            outType = {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, floatingPointBitWidth, elementCount};
        else if (isNVVMBFloat16Type(elementType))
            outType = {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 16, elementCount};
        else
        {
            SLANG_RELEASE_ASSERT(isNVVMBoolType(elementType));
            outType = {SLANG_NVVM_VALUE_TYPE_BOOL, 1, elementCount};
        }
    }
    else if (isNVVMFloat8Type(type))
        outType = type->getOp() == kIROp_FloatE4M3Type ? NVVMSemantics::kFloatE4M3
                                                       : NVVMSemantics::kFloatE5M2;
    else if (isNVVMBFloat16Type(type))
        outType = NVVMSemantics::kBFloat16;
    else if (isNVVMBoolType(type))
        outType = NVVMSemantics::kBool;
    else
    {
        uint32_t bitWidth = 0;
        bool isSigned = false;
        if (isNVVMSupportedIntegerScalarType(type, &bitWidth, &isSigned))
        {
            outType = {
                isSigned ? SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER
                         : SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
                bitWidth,
                1,
            };
        }
        else if (uint32_t floatingPointBitWidth = 0;
                 isNVVMSupportedFloatingPointScalarType(type, &floatingPointBitWidth))
            outType = {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, floatingPointBitWidth, 1};
        else
            return false;
    }
    return true;
}

// Returns whether `genericAsm` is the complete executable body of one linked value helper. CUDA
// target specialization produces this exact shape after selecting an intrinsic-asm case. Named
// call admission must not infer a complete helper from a fragment in an arbitrary function.
bool _isCanonicalNVVMIntrinsicValueHelper(IRInst* terminator, IRFunc* function)
{
    IRBlock* block = function ? function->getFirstBlock() : nullptr;
    if (!terminator || !block || block->getNextBlock() || terminator->getParent() != block)
    {
        return false;
    }
    for (auto inst : block->getOrdinaryInsts())
    {
        if (inst != terminator)
            return false;
    }
    return true;
}

// OptiX execution primitives are tied to the selected entry's complete reachable closure.
// A helper containing a payload access is still illegal when called by a compute entry.
bool _isNVVMOptixStage(Stage stage)
{
    return stage == Stage::RayGeneration || stage == Stage::Miss || stage == Stage::ClosestHit ||
           stage == Stage::AnyHit || stage == Stage::Intersection || stage == Stage::Callable;
}

bool _isNVVMOptixPrimitiveStage(UnownedStringSlice name, Stage stage)
{
    const char* anyHitPrimitives[] = {
        "_optix_get_object_ray_origin_x",
        "_optix_get_object_ray_origin_y",
        "_optix_get_object_ray_origin_z",
        "_optix_get_object_ray_direction_x",
        "_optix_get_object_ray_direction_y",
        "_optix_get_object_ray_direction_z",
        "_optix_ignore_intersection",
        "_optix_terminate_ray",
    };
    for (UInt i = 0; i < SLANG_COUNT_OF(anyHitPrimitives); ++i)
        if (name == UnownedStringSlice(anyHitPrimitives[i]))
            return stage == Stage::AnyHit || (i < 6 && stage == Stage::Intersection);
    // Ray state exists only while servicing a ray in the selected callback stages.
    // Match the complete SDK names so launch queries retain their separate stage contract.
    const char* rayStateQueries[] = {
        "_optix_get_world_ray_origin_x",
        "_optix_get_world_ray_origin_y",
        "_optix_get_world_ray_origin_z",
        "_optix_get_world_ray_direction_x",
        "_optix_get_world_ray_direction_y",
        "_optix_get_world_ray_direction_z",
        "_optix_get_ray_tmin",
        "_optix_get_ray_tmax",
        "_optix_get_ray_flags",
    };
    for (auto query : rayStateQueries)
        if (name == UnownedStringSlice(query))
            return stage == Stage::Miss || stage == Stage::ClosestHit || stage == Stage::AnyHit ||
                   stage == Stage::Intersection;
    // Hit properties describe the current candidate in AnyHit or the selected hit in ClosestHit.
    // A miss has only ray state.
    if (name == toSlice("_optix_read_primitive_idx") ||
        name == toSlice("_optix_read_instance_idx") || name == toSlice("_optix_read_instance_id") ||
        name == toSlice("_optix_get_transform_list_size") ||
        name == toSlice("_optix_get_transform_list_handle"))
        return stage == Stage::ClosestHit || stage == Stage::AnyHit || stage == Stage::Intersection;
    if (name == toSlice("_optix_get_hit_kind"))
        return stage == Stage::ClosestHit || stage == Stage::AnyHit;
    if (name == toSlice("_optix_get_payload") || name == toSlice("_optix_set_payload"))
        return stage == Stage::Miss || stage == Stage::ClosestHit || stage == Stage::AnyHit ||
               stage == Stage::Intersection;
    if (name.startsWith(toSlice("_optix_get_attribute_")))
        return stage == Stage::ClosestHit || stage == Stage::AnyHit;
    return _isNVVMOptixStage(stage);
}

// The varying-parameter pass retains unsupported payloads as its existing pointer fallback.
// Only canonical register/attribute instructions reach this exact SDK signature mapping.
bool _planNVVMOptixRegister(IRInst* inst, Stage stage, NVVMPlannedNamedIntrinsic& plan)
{
    plan = {};
    plan.source = inst;
    plan.resultType = NVVMSemantics::kUnsignedI32;
    const bool attribute = inst->getOp() == kIROp_GetOptiXHitAttribute;
    const bool write = inst->getOp() == kIROp_SetOptiXPayloadRegister;
    if (inst->getOperandCount() != (attribute || write ? 2u : 1u))
        return false;
    auto index = as<IRIntLit>(inst->getOperand(attribute ? 1 : 0));
    if (!index || index->getValue() < 0 || index->getValue() >= (attribute ? 8 : 32))
        return false;
    if (attribute)
    {
        if ((stage != Stage::ClosestHit && stage != Stage::AnyHit) ||
            inst->getOperand(0) != inst->getDataType() ||
            inst->getDataType()->getOp() != kIROp_UIntType)
            return false;
        StringBuilder name;
        name << "_optix_get_attribute_" << index->getValue();
        plan.name = name.produceString();
        return true;
    }
    if (stage != Stage::Miss && stage != Stage::ClosestHit && stage != Stage::AnyHit &&
        stage != Stage::Intersection)
        return false;
    if (write)
    {
        if (inst->getDataType()->getOp() != kIROp_VoidType ||
            inst->getOperand(1)->getDataType()->getOp() != kIROp_UIntType)
            return false;
        plan.resultType = NVVMSemantics::kVoid;
    }
    else if (inst->getDataType()->getOp() != kIROp_UIntType)
        return false;
    plan.name = write ? "_optix_set_payload" : "_optix_get_payload";
    plan.operands.add(
        {NVVMSemantics::kUnsignedI32, SLANG_NVVM_NAMED_INTRINSIC_OPERAND_INTEGER_CONSTANT});
    plan.operandValues.add(index);
    if (write)
    {
        plan.operands.add({NVVMSemantics::kUnsignedI32, SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE});
        plan.operandValues.add(inst->getOperand(1));
    }
    return true;
}

// Core constructs a matrix from three literal row reads. Retain the exact selector and handle
// here so emission never reinterprets a handle as a general pointer or rechecks source syntax.
bool _planNVVMInstanceTransform(IRInst* inst, Stage stage, NVVMPlannedInstanceTransform& plan)
{
    plan = {};
    const bool current = inst->getOp() == kIROp_OptixCurrentTransformRow;
    const UInt start = current ? 0 : 1;
    if (current
            ? (stage != Stage::AnyHit && stage != Stage::ClosestHit && stage != Stage::Intersection)
            : !_isNVVMOptixStage(stage))
        return false;
    if (inst->getOperandCount() != start + 2)
        return false;
    auto result = as<IRVectorType>(inst->getDataType());
    auto row = as<IRIntLit>(inst->getOperand(start));
    auto inverse = _asExecutableBoolConstant(inst->getOperand(start + 1));
    if (!result || result->getElementType()->getOp() != kIROp_FloatType ||
        getIntVal(result->getElementCount()) != 4 ||
        (!current && inst->getOperand(0)->getDataType()->getOp() != kIROp_UInt64Type) || !row ||
        row->getDataType()->getOp() != kIROp_UIntType || row->getValue() < 0 ||
        row->getValue() > 2 || !inverse)
        return false;
    plan.source = inst;
    plan.handle = current ? nullptr : inst->getOperand(0);
    plan.desc = {uint32_t(row->getValue()), uint32_t(inverse->getValue())};
    return true;
}

// Core query IDs are explicit typed operands. They select a finite provider contract; the
// checked plan retains only its descriptor and executable values, never source text.
bool _planNVVMHitObject(IRInst* inst, Stage stage, NVVMPlannedHitObjectOperation& plan)
{
    plan = {};
    plan.source = inst;
    auto& desc = plan.desc;
    IROp expectedResult = kIROp_VoidType;
    UInt vectorLanes = 0;
    UInt start = 0;
    switch (inst->getOp())
    {
    case kIROp_AllocateOpaqueHandle:
    case kIROp_OptixHitObjectMakeNop:
        desc.operation = SLANG_NVVM_HIT_OBJECT_OP_MAKE_NOP;
        break;
    case kIROp_OptixHitObjectMakeMiss:
        desc.operation = SLANG_NVVM_HIT_OBJECT_OP_MAKE_MISS;
        break;
    case kIROp_OptixHitObjectSetSbt:
        desc.operation = SLANG_NVVM_HIT_OBJECT_OP_SET_SBT_INDEX;
        break;
    case kIROp_OptixHitObjectLoadSbt:
        desc.operation = SLANG_NVVM_HIT_OBJECT_OP_LOAD_SBT_U32;
        expectedResult = kIROp_UIntType;
        break;
    case kIROp_OptixHitObjectReorder:
        desc.operation = SLANG_NVVM_HIT_OBJECT_OP_REORDER;
        break;
    case kIROp_OptixReorderHint:
        desc.operation = SLANG_NVVM_HIT_OBJECT_OP_REORDER_HINT;
        break;
    case kIROp_OptixHitObjectPayload:
        {
            if (inst->getOperandCount() < 2)
                return false;
            plan.payloadType = as<IRType>(inst->getOperand(0));
            auto operation = as<IRIntLit>(inst->getOperand(1));
            UInt count = 0;
            if (!plan.payloadType || !operation ||
                operation->getDataType()->getOp() != kIROp_UIntType ||
                !(operation->getValue() == SLANG_NVVM_HIT_OBJECT_OP_REPORT_INTERSECTION
                      ? getNVVMOptixAttributeRegisterCount(plan.payloadType, count)
                      : getNVVMOptixHitObjectPayloadRegisterCount(plan.payloadType, count)))
                return false;
            auto code = operation->getValue();
            if (code != SLANG_NVVM_HIT_OBJECT_OP_TRAVERSE &&
                code != SLANG_NVVM_HIT_OBJECT_OP_INVOKE &&
                code != SLANG_NVVM_HIT_OBJECT_OP_REPORT_INTERSECTION)
                return false;
            desc.operation = SlangNVVMHitObjectOperation(code);
            desc.payloadCount = count;
            start = 2;
            if (desc.operation == SLANG_NVVM_HIT_OBJECT_OP_REPORT_INTERSECTION)
            {
                if (count > 8)
                    return false;
                expectedResult = kIROp_UIntType;
            }
            else if (count)
            {
                auto array = as<IRArrayType>(inst->getDataType());
                auto elements = array ? as<IRIntLit>(array->getElementCount()) : nullptr;
                if (!array || array->getArrayStride() || !elements ||
                    elements->getValue() != count ||
                    array->getElementType()->getOp() != kIROp_UIntType)
                    return false;
                expectedResult = kIROp_ArrayType;
            }
        }
        break;
    case kIROp_OptixCurrentHitQuery:
    case kIROp_OptixHitObjectQuery:
        {
            const bool current = inst->getOp() == kIROp_OptixCurrentHitQuery;
            const UInt first = current ? 0 : 1;
            if (inst->getOperandCount() != first + 2)
                return false;
            auto query = as<IRIntLit>(inst->getOperand(first));
            auto index = as<IRIntLit>(inst->getOperand(first + 1));
            if (!query || !index || query->getDataType()->getOp() != kIROp_UIntType ||
                index->getDataType()->getOp() != kIROp_UIntType || query->getValue() < 0 ||
                query->getValue() > SLANG_NVVM_HIT_OBJECT_QUERY_IS_LSS || index->getValue() < 0)
                return false;
            desc.operation =
                current ? SLANG_NVVM_HIT_OBJECT_OP_CURRENT_QUERY : SLANG_NVVM_HIT_OBJECT_OP_QUERY;
            desc.query = SlangNVVMHitObjectQuery(query->getValue());
            if (current && desc.query != SLANG_NVVM_HIT_OBJECT_QUERY_CLUSTER_ID &&
                desc.query != SLANG_NVVM_HIT_OBJECT_QUERY_SPHERE &&
                desc.query != SLANG_NVVM_HIT_OBJECT_QUERY_LSS &&
                desc.query != SLANG_NVVM_HIT_OBJECT_QUERY_IS_SPHERE &&
                desc.query != SLANG_NVVM_HIT_OBJECT_QUERY_IS_LSS)
                return false;
            desc.index = uint32_t(index->getValue());
            if (index->getValue() != desc.index)
                return false;
            UInt maxIndex = desc.query == SLANG_NVVM_HIT_OBJECT_QUERY_ATTRIBUTE ? 7
                            : desc.query == SLANG_NVVM_HIT_OBJECT_QUERY_LSS     ? 1
                            : desc.query == SLANG_NVVM_HIT_OBJECT_QUERY_MATRIX_OBJECT_TO_WORLD ||
                                    desc.query == SLANG_NVVM_HIT_OBJECT_QUERY_MATRIX_WORLD_TO_OBJECT
                                ? 2
                                : 0;
            if (desc.index > maxIndex)
                return false;
            expectedResult = kIROp_UIntType;
            if (desc.query >= SLANG_NVVM_HIT_OBJECT_QUERY_WORLD_ORIGIN &&
                desc.query <= SLANG_NVVM_HIT_OBJECT_QUERY_TIME)
                expectedResult = kIROp_FloatType;
            if (desc.query == SLANG_NVVM_HIT_OBJECT_QUERY_WORLD_ORIGIN ||
                desc.query == SLANG_NVVM_HIT_OBJECT_QUERY_WORLD_DIRECTION)
                vectorLanes = 3;
            if (desc.query >= SLANG_NVVM_HIT_OBJECT_QUERY_SPHERE &&
                desc.query <= SLANG_NVVM_HIT_OBJECT_QUERY_MATRIX_WORLD_TO_OBJECT)
            {
                expectedResult = kIROp_FloatType;
                vectorLanes = 4;
            }
            if (!current)
                plan.operands.add(inst->getOperand(0));
            start = first + 2;
        }
        break;
    default:
        return false;
    }
    for (UInt i = start; i < inst->getOperandCount(); ++i)
        plan.operands.add(inst->getOperand(i));
    const bool currentQuery = desc.operation == SLANG_NVVM_HIT_OBJECT_OP_CURRENT_QUERY;
    const bool report = desc.operation == SLANG_NVVM_HIT_OBJECT_OP_REPORT_INTERSECTION;
    const bool reorder = desc.operation == SLANG_NVVM_HIT_OBJECT_OP_REORDER ||
                         desc.operation == SLANG_NVVM_HIT_OBJECT_OP_REORDER_HINT;
    if (currentQuery ? (stage != Stage::AnyHit && stage != Stage::ClosestHit)
        : report     ? stage != Stage::Intersection
        : reorder
            ? stage != Stage::RayGeneration
            : stage != Stage::RayGeneration && stage != Stage::ClosestHit && stage != Stage::Miss)
        return false;
    IRType* result = inst->getDataType();
    if (vectorLanes)
    {
        auto vector = as<IRVectorType>(result);
        auto lanes = vector ? as<IRIntLit>(vector->getElementCount()) : nullptr;
        if (!lanes || lanes->getValue() != vectorLanes)
            return false;
        result = vector->getElementType();
    }
    if (result->getOp() != expectedResult)
        return false;
    const bool hasObject =
        !currentQuery && !report && desc.operation != SLANG_NVVM_HIT_OBJECT_OP_REORDER_HINT;
    UInt fixedCount = desc.operation == SLANG_NVVM_HIT_OBJECT_OP_MAKE_MISS  ? 11
                      : desc.operation == SLANG_NVVM_HIT_OBJECT_OP_TRAVERSE ? 15
                      : report || reorder                                   ? 2
                      : desc.operation == SLANG_NVVM_HIT_OBJECT_OP_SET_SBT_INDEX ||
                              desc.operation == SLANG_NVVM_HIT_OBJECT_OP_LOAD_SBT_U32
                          ? 1
                          : 0;
    if (plan.operands.getCount() != fixedCount + desc.payloadCount + UInt(hasObject))
        return false;
    for (UInt i = 0; i < plan.operands.getCount(); ++i)
    {
        auto type = plan.operands[i]->getDataType();
        if (hasObject && i == 0)
        {
            // SetShaderTableIndex is a non-mutating public method, so noncopyable `this` is
            // borrowed Read. Its typed operation owns opaque interior mutation; this does not
            // permit ordinary stores or constructors through that borrowed reference.
            auto pointer = asNVVMHitObjectPointerType(type);
            if (!pointer || pointer->getValueType()->getOp() != kIROp_HitObjectType ||
                ((desc.operation == SLANG_NVVM_HIT_OBJECT_OP_MAKE_NOP ||
                  desc.operation == SLANG_NVVM_HIT_OBJECT_OP_MAKE_MISS ||
                  desc.operation == SLANG_NVVM_HIT_OBJECT_OP_TRAVERSE) &&
                 pointer->getAccessQualifier() != AccessQualifier::ReadWrite))
                return false;
            continue;
        }
        UInt scalar = i - UInt(hasObject);
        IROp expected = kIROp_UIntType;
        if (desc.operation == SLANG_NVVM_HIT_OBJECT_OP_TRAVERSE)
            expected = scalar == 0   ? kIROp_RaytracingAccelerationStructureType
                       : scalar <= 9 ? kIROp_FloatType
                                     : kIROp_UIntType;
        else if (desc.operation == SLANG_NVVM_HIT_OBJECT_OP_MAKE_MISS)
            expected = scalar >= 1 && scalar <= 9 ? kIROp_FloatType : kIROp_UIntType;
        else if (report && scalar == 0)
            expected = kIROp_FloatType;
        if (!type || type->getOp() != expected)
            return false;
    }
    return true;
}

bool _planNVVMTraceRay(IRInst* inst, Stage stage, NVVMPlannedTraceRay& plan)
{
    plan = {};
    if (stage != Stage::RayGeneration || inst->getOperandCount() < 17)
        return false;
    auto payloadType = as<IRType>(inst->getOperand(0));
    UInt count = payloadType ? getNVVMOptixPayloadRegisterCount(payloadType) : 0;
    auto result = as<IRArrayType>(inst->getDataType());
    auto resultCount = result ? as<IRIntLit>(result->getElementCount()) : nullptr;
    if (!count || inst->getOperandCount() != 16 + count || !resultCount ||
        resultCount->getValue() != count || result->getElementType()->getOp() != kIROp_UIntType)
        return false;
    for (UInt i = 1; i < inst->getOperandCount(); ++i)
    {
        auto type = inst->getOperand(i)->getDataType();
        IROp expected = i == 1    ? kIROp_RaytracingAccelerationStructureType
                        : i <= 10 ? kIROp_FloatType
                                  : kIROp_UIntType;
        if (!type || type->getOp() != expected)
            return false;
        plan.operands.add(inst->getOperand(i));
    }
    plan.source = inst;
    plan.desc.payloadCount = count;
    return true;
}

// Captures the checked signature and explicit operands without interpreting the LLVM name.
// Consider `uint scan(uint x) { __intrinsic_asm "llvm.ctlz", x, false; }`: lowering stores
// the parameter and Boolean literal as GenericAsm operands. Preserve those values directly;
// helper parameters are not an implicit forwarding convention. The provider validates LLVM's
// signature and immediate-argument constraints before an output module exists.
bool _getNVVMNamedIntrinsicDesc(
    IRGenericAsm* genericAsm,
    IRFunc* function,
    NVVMPlannedNamedIntrinsic& outPlan)
{
    outPlan = {};
    outPlan.isDeviceLibraryFunction = genericAsm->getAsm().startsWith(toSlice("__nv_"));
    if ((!outPlan.isDeviceLibraryFunction && !genericAsm->getAsm().startsWith(toSlice("llvm.")) &&
         !genericAsm->getAsm().startsWith(toSlice("_optix_"))) ||
        !_isCanonicalNVVMIntrinsicValueHelper(genericAsm, function) ||
        !_getNVVMSemanticType(function->getResultType(), outPlan.resultType))
        return false;
    outPlan.source = genericAsm;
    outPlan.name = genericAsm->getAsm();
    for (UInt i = 1; i < genericAsm->getOperandCount(); ++i)
    {
        IRInst* value = genericAsm->getOperand(i);
        SlangNVVMNamedIntrinsicOperandDesc operand = {};
        if (!value)
            return false;
        IRType* pointedToType = nullptr;
        if (outPlan.isDeviceLibraryFunction &&
            asNVVMSupportedLocalNumericPointerType(value->getDataType(), &pointedToType))
        {
            if (!_getNVVMSemanticType(pointedToType, operand.type))
                return false;
            operand.kind = SLANG_NVVM_NAMED_INTRINSIC_OPERAND_OUT_POINTER;
        }
        else
        {
            if (!_getNVVMSemanticType(value->getDataType(), operand.type))
                return false;
            operand.kind =
                _asExecutableSelectedIntegerConstant(value) || _asExecutableBoolConstant(value)
                    ? SLANG_NVVM_NAMED_INTRINSIC_OPERAND_INTEGER_CONSTANT
                    : SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE;
        }
        outPlan.operands.add(operand);
        outPlan.operandValues.add(value);
    }
    return true;
}

// Describes one queried scalar operation in a compiler-owned value conversion recipe. The
// conversion emitter decides how intermediate values flow between the checked operations.
void _setNVVMValueRecipeStep(
    NVVMValueRecipeStep& step,
    SlangNVVMValueOperation operation,
    const SlangNVVMValueTypeDesc& resultType,
    const SlangNVVMValueTypeDesc* operandTypes,
    uint32_t operandCount,
    const char* diagnosticName)
{
    SLANG_ASSERT(operandCount <= SLANG_COUNT_OF(step.operandTypes));
    step = {};
    step.operation = operation;
    step.resultType = resultType;
    step.operandCount = operandCount;
    step.diagnosticName = diagnosticName;
    for (uint32_t i = 0; i < operandCount; ++i)
        step.operandTypes[i] = operandTypes[i];
}

bool _setNVVMSupportedValueRecipeStep(
    NVVMValueRecipeStep& step,
    SlangNVVMValueOperation operation,
    const SlangNVVMValueTypeDesc& resultType,
    const SlangNVVMValueTypeDesc* operandTypes,
    uint32_t operandCount,
    const char* diagnosticName)
{
    _setNVVMValueRecipeStep(
        step,
        operation,
        resultType,
        operandTypes,
        operandCount,
        diagnosticName);
    return NVVMSemantics::isSupported(step.getDesc());
}

// Describes the canonical low-word/high-word reconstruction emitted by AnyValue unmarshalling.
// Both inputs are semantic UInt32 values; zero extension makes the provider's signless i64
// representation independent of the source words' high bits.
bool _resolveNVVMUInt64WordConstruction(
    IRInst* inst,
    NVVMPlannedUInt64WordConstruction& outConstruction)
{
    outConstruction = {};
    if (!inst || inst->getOp() != kIROp_MakeUInt64 || inst->getOperandCount() != 2 ||
        inst->getDataType()->getOp() != kIROp_UInt64Type)
    {
        return false;
    }

    IRInst* lowWord = inst->getOperand(0);
    IRInst* highWord = inst->getOperand(1);
    if (!lowWord || !highWord || !isNVVMUnsignedI32Type(lowWord->getDataType()) ||
        !isNVVMUnsignedI32Type(highWord->getDataType()))
    {
        return false;
    }

    const SlangNVVMValueTypeDesc conversionOperands[] = {NVVMSemantics::kUnsignedI32};
    const SlangNVVMValueTypeDesc binaryOperands[] = {
        NVVMSemantics::kUnsignedI64,
        NVVMSemantics::kUnsignedI64,
    };
    if (!_setNVVMSupportedValueRecipeStep(
            outConstruction.wordConversion,
            SLANG_NVVM_VALUE_OP_INTEGER_CONVERT,
            NVVMSemantics::kUnsignedI64,
            conversionOperands,
            SLANG_COUNT_OF(conversionOperands),
            "UInt64 word zero extension") ||
        !_setNVVMSupportedValueRecipeStep(
            outConstruction.highWordShift,
            SLANG_NVVM_VALUE_OP_SHIFT_LEFT,
            NVVMSemantics::kUnsignedI64,
            binaryOperands,
            SLANG_COUNT_OF(binaryOperands),
            "UInt64 high-word shift") ||
        !_setNVVMSupportedValueRecipeStep(
            outConstruction.combine,
            SLANG_NVVM_VALUE_OP_BIT_OR,
            NVVMSemantics::kUnsignedI64,
            binaryOperands,
            SLANG_COUNT_OF(binaryOperands),
            "UInt64 word combination"))
    {
        return false;
    }

    outConstruction.lowWord = lowWord;
    outConstruction.highWord = highWord;
    outConstruction.source = inst;
    return true;
}

bool _getNVVMValueOperation(IROp op, SlangNVVMValueOperation& outOperation)
{
    switch (op)
    {
    case kIROp_Add:
        outOperation = SLANG_NVVM_VALUE_OP_ADD;
        return true;
    case kIROp_Sub:
        outOperation = SLANG_NVVM_VALUE_OP_SUBTRACT;
        return true;
    case kIROp_Fma:
        outOperation = SLANG_NVVM_VALUE_OP_FMA;
        return true;
    case kIROp_Mul:
        outOperation = SLANG_NVVM_VALUE_OP_MULTIPLY;
        return true;
    case kIROp_Div:
        outOperation = SLANG_NVVM_VALUE_OP_DIVIDE;
        return true;
    case kIROp_IRem:
        outOperation = SLANG_NVVM_VALUE_OP_REMAINDER;
        return true;
    case kIROp_Lsh:
        outOperation = SLANG_NVVM_VALUE_OP_SHIFT_LEFT;
        return true;
    case kIROp_Rsh:
        outOperation = SLANG_NVVM_VALUE_OP_SHIFT_RIGHT;
        return true;
    case kIROp_BitAnd:
        outOperation = SLANG_NVVM_VALUE_OP_BIT_AND;
        return true;
    case kIROp_BitOr:
        outOperation = SLANG_NVVM_VALUE_OP_BIT_OR;
        return true;
    case kIROp_BitXor:
        outOperation = SLANG_NVVM_VALUE_OP_BIT_XOR;
        return true;
    case kIROp_BitNot:
        outOperation = SLANG_NVVM_VALUE_OP_BIT_NOT;
        return true;
    case kIROp_And:
        outOperation = SLANG_NVVM_VALUE_OP_BIT_AND;
        return true;
    case kIROp_Or:
        outOperation = SLANG_NVVM_VALUE_OP_BIT_OR;
        return true;
    case kIROp_Not:
        outOperation = SLANG_NVVM_VALUE_OP_BIT_NOT;
        return true;
    case kIROp_Neg:
        outOperation = SLANG_NVVM_VALUE_OP_NEGATE;
        return true;
    case kIROp_Eql:
        outOperation = SLANG_NVVM_VALUE_OP_EQUAL;
        return true;
    case kIROp_Neq:
        outOperation = SLANG_NVVM_VALUE_OP_NOT_EQUAL;
        return true;
    case kIROp_Less:
        outOperation = SLANG_NVVM_VALUE_OP_LESS_THAN;
        return true;
    case kIROp_Greater:
        outOperation = SLANG_NVVM_VALUE_OP_GREATER_THAN;
        return true;
    case kIROp_Leq:
        outOperation = SLANG_NVVM_VALUE_OP_LESS_EQUAL;
        return true;
    case kIROp_Geq:
        outOperation = SLANG_NVVM_VALUE_OP_GREATER_EQUAL;
        return true;
    case kIROp_IntCast:
        outOperation = SLANG_NVVM_VALUE_OP_INTEGER_CONVERT;
        return true;
    case kIROp_CastIntToFloat:
        outOperation = SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT;
        return true;
    case kIROp_CastFloatToInt:
        outOperation = SLANG_NVVM_VALUE_OP_FLOAT_TO_INTEGER;
        return true;
    case kIROp_FloatCast:
        outOperation = SLANG_NVVM_VALUE_OP_FLOAT_CONVERT;
        return true;
    case kIROp_BitCast:
        outOperation = SLANG_NVVM_VALUE_OP_BIT_REINTERPRET;
        return true;
    case kIROp_Select:
        outOperation = SLANG_NVVM_VALUE_OP_SELECT;
        return true;
    case kIROp_WaveGetConvergedMask:
        outOperation = SLANG_NVVM_VALUE_OP_WAVE_ACTIVE_MASK;
        return true;
    case kIROp_WaveMaskBallot:
        outOperation = SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT;
        return true;
    case kIROp_WaveMaskMatch:
        outOperation = SLANG_NVVM_VALUE_OP_WAVE_MASK_MATCH;
        return true;
    default:
        return false;
    }
}

struct NVVMResolvedValueOperation
{
    SlangNVVMValueTypeDesc operandTypes[3] = {};
    SlangNVVMValueOperationDesc desc = {};
    const NVVMSemantics::CatalogEntry* staticEntry = nullptr;
    NVVMSemantics::ValueOperationFamilyResolution family;
    const char* diagnosticName = nullptr;
};

// Describes the CUDA floating-remainder recipe selected for one canonical `kIROp_FRem`.
// LLVM `frem` is not a semantic substitute for CUDA `fmod` near an exactly representable
// multiple, so selected vectors are explicitly decomposed into scalar libdevice operations.
struct NVVMPointerConversion
{
    IRInst* value = nullptr;
    IRPtrTypeBase* pointerType = nullptr;
    bool resultIsPointer = false;
};

// Classifies the selected resource side of an AnyValue bit transport by the physical CUDA value
// established by NVVM type lowering. Consider the two producers covered by
// `anyvalue-layout.slang` and `reinterpret-structured-buffer.slang`:
//
//     uint2 bits = reinterpret<uint2>(foo.texHandle);
//     RWStructuredBuffer<half2> pairs =
//         reinterpret<RWStructuredBuffer<half2>>(*inputBuffer);
//
// DescriptorHandle<T> deliberately has T's representation, so unwrap it only to select the
// representation while retaining the semantic handle type in the plan. The bare raw resource in
// the second example follows the same type-lowering contract. This accepts no arbitrary
// equal-sized aggregate.
bool _getNVVMResourceBitCastKind(
    IRType* type,
    NVVMPlannedResourceBitCastKind& outKind,
    NVVMRawBufferType& outRawBufferType)
{
    outKind = NVVMPlannedResourceBitCastKind::OpaqueHandle64;
    outRawBufferType = {};

    IRType* resourceType = nullptr;
    if (!asNVVMSupportedDescriptorHandleType(type, &resourceType))
        resourceType = type;

    if (getNVVMSupportedRawBufferType(resourceType, outRawBufferType))
    {
        outKind = NVVMPlannedResourceBitCastKind::RawBuffer;
        return true;
    }

    NVVMSurfaceType surfaceType;
    NVVMReadOnlyTextureType textureType;
    return getNVVMSupportedSurfaceType(resourceType, surfaceType) ||
           getNVVMSupportedReadOnlyTextureType(resourceType, textureType) ||
           asNVVMSupportedSamplerValueType(resourceType);
}

// Resolves the exact bit transport emitted by AnyValue and resource reinterpret lowering. CUDA
// owns two selected physical forms: an opaque 64-bit texture/sampler/surface handle transported as
// `uint2`, or a 16-byte raw-buffer pointer/count view transported as `uint4`.
bool _resolveNVVMResourceBitCast(IRInst* inst, NVVMPlannedResourceBitCast& outCast)
{
    outCast = {};
    if (!inst || inst->getOp() != kIROp_BitCast || inst->getOperandCount() != 1)
        return false;

    IRInst* value = inst->getOperand(0);
    if (!value)
        return false;
    IRType* sourceType = as<IRType>(value->getDataType());
    IRType* resultType = as<IRType>(inst->getDataType());
    if (!sourceType || !resultType)
        return false;

    NVVMPlannedResourceBitCastKind kind = NVVMPlannedResourceBitCastKind::OpaqueHandle64;
    NVVMRawBufferType rawBufferType;
    IRType* resourceValueType = resultType;
    IRType* payloadType = sourceType;
    bool resultIsResourceValue = _getNVVMResourceBitCastKind(resultType, kind, rawBufferType);
    if (!resultIsResourceValue)
    {
        resourceValueType = sourceType;
        payloadType = resultType;
        if (!_getNVVMResourceBitCastKind(sourceType, kind, rawBufferType))
            return false;
    }

    bool payloadIsSigned = false;
    uint32_t payloadLaneCount = 0;
    auto payloadVector =
        asNVVMSupportedI32VectorType(payloadType, &payloadIsSigned, &payloadLaneCount);
    const uint32_t expectedLaneCount = kind == NVVMPlannedResourceBitCastKind::RawBuffer ? 4u : 2u;
    if (!payloadVector || payloadIsSigned || payloadLaneCount != expectedLaneCount)
    {
        return false;
    }

    if (kind == NVVMPlannedResourceBitCastKind::OpaqueHandle64)
    {
        SlangNVVMValueTypeDesc payloadSemantic = NVVMSemantics::kUnsignedI32;
        payloadSemantic.laneCount = 2;
        const SlangNVVMValueTypeDesc resultSemantic =
            resultIsResourceValue ? NVVMSemantics::kUnsignedI64 : payloadSemantic;
        const SlangNVVMValueTypeDesc operandSemantic =
            resultIsResourceValue ? payloadSemantic : NVVMSemantics::kUnsignedI64;
        if (!_setNVVMSupportedValueRecipeStep(
                outCast.steps[0],
                SLANG_NVVM_VALUE_OP_BIT_REINTERPRET,
                resultSemantic,
                &operandSemantic,
                1,
                "opaque resource bit transport"))
        {
            return false;
        }
        outCast.stepCount = 1;
    }
    else if (resultIsResourceValue)
    {
        const SlangNVVMValueTypeDesc conversionOperands[] = {NVVMSemantics::kUnsignedI32};
        const SlangNVVMValueTypeDesc binaryOperands[] = {
            NVVMSemantics::kUnsignedI64,
            NVVMSemantics::kUnsignedI64,
        };
        if (!_setNVVMSupportedValueRecipeStep(
                outCast.steps[0],
                SLANG_NVVM_VALUE_OP_INTEGER_CONVERT,
                NVVMSemantics::kUnsignedI64,
                conversionOperands,
                SLANG_COUNT_OF(conversionOperands),
                "descriptor count word zero extension") ||
            !_setNVVMSupportedValueRecipeStep(
                outCast.steps[1],
                SLANG_NVVM_VALUE_OP_SHIFT_LEFT,
                NVVMSemantics::kUnsignedI64,
                binaryOperands,
                SLANG_COUNT_OF(binaryOperands),
                "descriptor count high-word shift") ||
            !_setNVVMSupportedValueRecipeStep(
                outCast.steps[2],
                SLANG_NVVM_VALUE_OP_BIT_OR,
                NVVMSemantics::kUnsignedI64,
                binaryOperands,
                SLANG_COUNT_OF(binaryOperands),
                "descriptor count word combination"))
        {
            return false;
        }
        outCast.stepCount = 3;
    }
    else if (kind == NVVMPlannedResourceBitCastKind::RawBuffer)
    {
        const SlangNVVMValueTypeDesc conversionOperands[] = {NVVMSemantics::kUnsignedI64};
        const SlangNVVMValueTypeDesc shiftOperands[] = {
            NVVMSemantics::kUnsignedI64,
            NVVMSemantics::kUnsignedI64,
        };
        if (!_setNVVMSupportedValueRecipeStep(
                outCast.steps[0],
                SLANG_NVVM_VALUE_OP_INTEGER_CONVERT,
                NVVMSemantics::kUnsignedI32,
                conversionOperands,
                SLANG_COUNT_OF(conversionOperands),
                "descriptor count word extraction") ||
            !_setNVVMSupportedValueRecipeStep(
                outCast.steps[1],
                SLANG_NVVM_VALUE_OP_SHIFT_RIGHT,
                NVVMSemantics::kUnsignedI64,
                shiftOperands,
                SLANG_COUNT_OF(shiftOperands),
                "descriptor count high-word shift"))
        {
            return false;
        }
        outCast.stepCount = 2;
    }

    outCast.source = inst;
    outCast.value = value;
    outCast.resourceValueType = resourceValueType;
    outCast.payloadType = payloadVector;
    outCast.rawBufferElementType = rawBufferType.structuredElementType;
    outCast.kind = kind;
    outCast.rawBufferIsByteAddress = rawBufferType.kind == NVVMRawBufferKind::ByteAddress;
    outCast.rawBufferElementUsesStructuredStorage =
        rawBufferType.structuredElementType &&
        isNVVMSupportedStructuredBufferStorageType(rawBufferType.structuredElementType);
    outCast.resultIsResourceValue = resultIsResourceValue;
    return true;
}

bool _isNVVMPointerBitPatternType(IRInst* type)
{
    uint32_t bitWidth = 0;
    bool isSigned = false;
    if (isNVVMSupportedIntegerScalarType(type, &bitWidth, &isSigned))
        return !isSigned && bitWidth == 64;
    uint32_t laneCount = 0;
    return asNVVMSupportedI32VectorType(type, &isSigned, &laneCount) && !isSigned && laneCount == 2;
}

// CUDA pointer addresses are 64-bit integers regardless of signedness.
bool _isNVVMAddressIntegerType(IRType* type)
{
    uint32_t width = 0;
    return isNVVMSupportedIntegerScalarType(type, &width) && width == 64;
}

// Resolves explicit pointer/address conversions and the pointer bit transport used by AnyValue.
// CastIntToPtr carries a signed or unsigned 64-bit address; BitCast retains its
// established unsigned 64-bit or 2x32-bit payload contract.
bool _getNVVMPointerConversion(IRInst* inst, NVVMPointerConversion& outCast)
{
    outCast = {};
    if (!inst || inst->getOperandCount() != 1 ||
        (inst->getOp() != kIROp_BitCast && inst->getOp() != kIROp_CastIntToPtr))
        return false;

    IRInst* value = inst->getOperand(0);
    IRPtrTypeBase* resultPointer = asNVVMSupportedDeviceHelperValuePointerType(inst->getDataType());
    IRPtrTypeBase* valuePointer =
        value ? asNVVMSupportedDeviceHelperValuePointerType(value->getDataType()) : nullptr;
    const bool isBitCast = inst->getOp() == kIROp_BitCast;
    const bool hasResultBits = isBitCast ? _isNVVMPointerBitPatternType(inst->getDataType())
                                         : _isNVVMAddressIntegerType(inst->getDataType());
    const bool hasValueBits =
        value && (isBitCast ? _isNVVMPointerBitPatternType(value->getDataType())
                            : _isNVVMAddressIntegerType(value->getDataType()));
    if (inst->getOp() == kIROp_CastIntToPtr && !resultPointer)
        return false;
    if ((!resultPointer || !hasValueBits || valuePointer || hasResultBits) &&
        (!valuePointer || !hasResultBits || resultPointer || hasValueBits))
    {
        return false;
    }

    outCast.value = value;
    outCast.pointerType = resultPointer ? resultPointer : valuePointer;
    outCast.resultIsPointer = resultPointer != nullptr;
    return true;
}

// Resolves descriptor conversions that preserve the resource's CUDA representation. Consider:
//
//     float4 loadTexture(uint64_t handle, int2 coordinate)
//     {
//         Texture2D<float4> texture = DescriptorHandle<Texture2D<float4>>(handle);
//         return texture.Load(int3(coordinate, 0));
//     }
//
// The standard-library constructor produces CastUInt64ToDescriptorHandle, followed by
// CastDescriptorHandleToResource. CUDA layout gives the descriptor its resource's layout, and
// type lowering represents a selected read-only texture as i64. Both conversions therefore
// preserve the same provider value, as does the inverse conversion to UInt64. Buffer descriptors
// instead carry a pointer/count aggregate, so only their resource/descriptor casts are identities.
bool _getNVVMDescriptorHandleConversion(IRInst* inst, IRInst*& outValue)
{
    outValue = nullptr;
    if (!inst || inst->getOperandCount() != 1)
        return false;

    IRInst* value = inst->getOperand(0);
    if (!value)
        return false;

    IRType* resourceType = nullptr;
    switch (inst->getOp())
    {
    case kIROp_CombinedTextureSamplerGetSampler:
        {
            NVVMReadOnlyTextureType texture;
            if (!getNVVMSupportedReadOnlyTextureType(value->getDataType(), texture) ||
                !texture.textureType->isCombined() ||
                !asNVVMSupportedSamplerValueType(inst->getDataType()))
                return false;
            // CUDA embeds sampling state in the texture object. The ordinary sampler operand
            // is an opaque placeholder and never becomes a separate hardware sampler reference.
        }
        break;

    case kIROp_CastDescriptorHandleToResource:
        if (!asNVVMSupportedDescriptorHandleType(value->getDataType(), &resourceType) ||
            inst->getDataType() != resourceType)
        {
            return false;
        }
        break;

    case kIROp_CastResourceToDescriptorHandle:
        if (!asNVVMSupportedDescriptorHandleType(inst->getDataType(), &resourceType) ||
            value->getDataType() != resourceType)
        {
            return false;
        }
        break;

    case kIROp_CastUInt64ToDescriptorHandle:
    case kIROp_CastDescriptorHandleToUInt64:
        {
            const bool toHandle = inst->getOp() == kIROp_CastUInt64ToDescriptorHandle;
            auto handleType =
                as<IRDescriptorHandleType>(toHandle ? inst->getDataType() : value->getDataType());
            IRType* bitsType = toHandle ? value->getDataType() : inst->getDataType();
            NVVMReadOnlyTextureType textureType;
            NVVMSurfaceType surfaceType;
            if (!handleType || bitsType->getOp() != kIROp_UInt64Type ||
                (!getNVVMSupportedReadOnlyTextureType(handleType->getResourceType(), textureType) &&
                 !getNVVMSupportedSurfaceType(handleType->getResourceType(), surfaceType)))
            {
                return false;
            }
        }
        break;

    default:
        return false;
    }

    outValue = value;
    return true;
}

// Resolves one canonical scalar atomic to the complete descriptor consumed by both preflight and
// emission. Memory-order literals are semantic metadata and are not provider SSA operands.
bool _resolveNVVMAtomicOperation(IRInst* inst, NVVMPlannedAtomicOperation& outOperation)
{
    outOperation = {};
    if (!inst)
        return false;

    SlangNVVMAtomicOperation providerOperation = 0;
    uint32_t valueCount = 0;
    uint32_t successOrderIndex = 0;
    uint32_t failureOrderIndex = 0;
    bool hasFailureOrder = false;
    switch (inst->getOp())
    {
    case kIROp_AtomicLoad:
        providerOperation = SLANG_NVVM_ATOMIC_OP_LOAD;
        successOrderIndex = 1;
        break;
    case kIROp_AtomicStore:
        providerOperation = SLANG_NVVM_ATOMIC_OP_STORE;
        valueCount = 1;
        successOrderIndex = 2;
        break;
    case kIROp_AtomicExchange:
        providerOperation = SLANG_NVVM_ATOMIC_OP_EXCHANGE;
        valueCount = 1;
        successOrderIndex = 2;
        break;
    case kIROp_AtomicCompareExchange:
        providerOperation = SLANG_NVVM_ATOMIC_OP_COMPARE_EXCHANGE;
        valueCount = 2;
        successOrderIndex = 3;
        failureOrderIndex = 4;
        hasFailureOrder = true;
        break;
    case kIROp_AtomicAdd:
        providerOperation = SLANG_NVVM_ATOMIC_OP_ADD;
        valueCount = 1;
        successOrderIndex = 2;
        break;
    case kIROp_AtomicSub:
        providerOperation = SLANG_NVVM_ATOMIC_OP_ADD;
        valueCount = 1;
        successOrderIndex = 2;
        outOperation.negatesValue = true;
        break;
    case kIROp_AtomicAnd:
        providerOperation = SLANG_NVVM_ATOMIC_OP_BIT_AND;
        valueCount = 1;
        successOrderIndex = 2;
        break;
    case kIROp_AtomicOr:
        providerOperation = SLANG_NVVM_ATOMIC_OP_BIT_OR;
        valueCount = 1;
        successOrderIndex = 2;
        break;
    case kIROp_AtomicXor:
        providerOperation = SLANG_NVVM_ATOMIC_OP_BIT_XOR;
        valueCount = 1;
        successOrderIndex = 2;
        break;
    case kIROp_AtomicMin:
        providerOperation = SLANG_NVVM_ATOMIC_OP_MIN;
        valueCount = 1;
        successOrderIndex = 2;
        break;
    case kIROp_AtomicMax:
        providerOperation = SLANG_NVVM_ATOMIC_OP_MAX;
        valueCount = 1;
        successOrderIndex = 2;
        break;
    case kIROp_AtomicInc:
    case kIROp_AtomicDec:
        providerOperation = SLANG_NVVM_ATOMIC_OP_ADD;
        successOrderIndex = 1;
        outOperation.hasImplicitValue = true;
        outOperation.implicitValue = inst->getOp() == kIROp_AtomicInc ? 1 : -1;
        break;
    default:
        return false;
    }
    const uint32_t expectedOperandCount = successOrderIndex + 1 + (hasFailureOrder ? 1 : 0);
    if (inst->getOperandCount() != expectedOperandCount)
        return false;

    IRInst* pointer = inst->getOperand(0);
    auto memoryOrder = _asExecutableI32Constant(inst->getOperand(successOrderIndex));
    auto failureMemoryOrder = hasFailureOrder
                                  ? _asExecutableI32Constant(inst->getOperand(failureOrderIndex))
                                  : memoryOrder;
    auto pointerType = as<IRPtrTypeBase>(pointer->getDataType());
    SlangNVVMValueTypeDesc valueType = {};
    IRType* physicalValueType = nullptr;
    if (!pointerType || pointerType->getAccessQualifier() != AccessQualifier::ReadWrite ||
        !memoryOrder || !failureMemoryOrder)
    {
        return false;
    }
    physicalValueType = pointerType->getValueType();
    IRType* atomicValueType = nullptr;
    if (asNVVMSupportedAtomicType(physicalValueType, &atomicValueType))
        physicalValueType = atomicValueType;
    if (memoryOrder->getValue() != kIRMemoryOrder_Relaxed ||
        failureMemoryOrder->getValue() != kIRMemoryOrder_Relaxed ||
        !_getNVVMSemanticType(physicalValueType, valueType))
    {
        return false;
    }
    const bool returnsVoid = providerOperation == SLANG_NVVM_ATOMIC_OP_STORE;
    if ((returnsVoid && inst->getDataType()->getOp() != kIROp_VoidType) ||
        (!returnsVoid && !isTypeEqual(physicalValueType, inst->getDataType())))
    {
        return false;
    }
    for (uint32_t i = 0; i < valueCount; ++i)
    {
        IRInst* value = inst->getOperand(i + 1);
        if (!value || !isTypeEqual(value->getDataType(), physicalValueType))
            return false;
        outOperation.values[i] = value;
    }

    outOperation.desc = {
        providerOperation,
        valueType,
        SLANG_NVVM_ADDRESS_SPACE_GENERIC,
        SLANG_NVVM_MEMORY_ORDER_RELAXED,
        SLANG_NVVM_MEMORY_ORDER_RELAXED,
    };
    if (!NVVMSemantics::isSupported(outOperation.desc))
        return false;
    if (outOperation.negatesValue)
    {
        const SlangNVVMValueTypeDesc operandTypes[] = {valueType};
        if (!_setNVVMSupportedValueRecipeStep(
                outOperation.valueNegation,
                SLANG_NVVM_VALUE_OP_NEGATE,
                valueType,
                operandTypes,
                SLANG_COUNT_OF(operandTypes),
                "atomic subtract value negation"))
        {
            return false;
        }
    }
    outOperation.pointer = pointer;
    outOperation.valueCount = valueCount;
    outOperation.diagnosticName = "relaxed scalar atomic operation";
    outOperation.source = inst;
    return true;
}

// Records one exact atomic overload, deduplicating identical semantic descriptors.
void _requireAtomicOperation(
    List<NVVMAtomicOperationRequirement>& requirements,
    const SlangNVVMAtomicOperationDesc& desc,
    const char* diagnosticName)
{
    for (const auto& requirement : requirements)
    {
        if (requirement.desc.operation == desc.operation &&
            NVVMSemantics::areSameType(requirement.desc.valueType, desc.valueType) &&
            requirement.desc.addressSpace == desc.addressSpace &&
            requirement.desc.memoryOrder == desc.memoryOrder &&
            requirement.desc.failureMemoryOrder == desc.failureMemoryOrder)
        {
            return;
        }
    }
    requirements.add({desc, diagnosticName});
}

// Records one exact typed provider operation, deduplicating identical overloads.
void _requireValueOperation(
    NVVMValueOperationRequirements& requirements,
    const SlangNVVMValueOperationDesc& desc,
    const char* diagnosticName)
{
    for (const auto& requirement : requirements)
    {
        const SlangNVVMValueOperationDesc existing = requirement.getDesc();
        if (existing.operation != desc.operation || existing.operandCount != desc.operandCount ||
            !NVVMSemantics::areSameType(existing.resultType, desc.resultType))
        {
            continue;
        }

        bool operandsMatch = true;
        for (uint32_t i = 0; i < existing.operandCount; ++i)
        {
            operandsMatch =
                operandsMatch &&
                NVVMSemantics::areSameType(existing.operandTypes[i], desc.operandTypes[i]);
        }
        if (operandsMatch)
            return;
    }

    NVVMValueOperationRequirement requirement;
    requirement.operation = desc.operation;
    requirement.resultType = desc.resultType;
    requirement.operandCount = uint32_t(desc.operandCount);
    requirement.diagnosticName = diagnosticName;
    for (uint32_t i = 0; i < requirement.operandCount; ++i)
        requirement.operandTypes[i] = desc.operandTypes[i];
    requirements.add(requirement);
}

// Records the exact provider descriptor selected for one canonical ordinary value instruction.
// Capability requirements remain deduplicated by overload, while the emission plan retains one
// source-keyed record so emission never has to classify the instruction again.
void _planNVVMValueOperation(
    NVVMOperationRequirements& requirements,
    IRInst* source,
    const NVVMResolvedValueOperation& operation)
{
    SLANG_ASSERT(source);
    _requireValueOperation(requirements.valueOperations, operation.desc, operation.diagnosticName);

    NVVMPlannedValueOperation planned;
    planned.source = source;
    planned.operation.operation = operation.desc.operation;
    planned.operation.resultType = operation.desc.resultType;
    planned.operation.operandCount = uint32_t(operation.desc.operandCount);
    planned.operation.diagnosticName = operation.diagnosticName;
    for (uint32_t i = 0; i < planned.operation.operandCount; ++i)
        planned.operation.operandTypes[i] = operation.desc.operandTypes[i];
    requirements.emissionPlan.valueOperations.add(planned);
}

const NVVMPlannedValueOperation* _findPlannedNVVMValueOperation(
    const NVVMOperationRequirements& requirements,
    IRInst* source)
{
    for (const auto& operation : requirements.emissionPlan.valueOperations)
    {
        if (operation.source == source)
            return &operation;
    }
    return nullptr;
}

template<typename T>
const T* _findPlannedNVVMOperation(const List<T>& operations, IRInst* source)
{
    for (const auto& operation : operations)
    {
        if (operation.source == source)
            return &operation;
    }
    return nullptr;
}

// Allows the availability pass to complete a selected record with checked producer facts.
template<typename T>
T* _findPlannedNVVMOperation(List<T>& operations, IRInst* source)
{
    for (auto& operation : operations)
        if (operation.source == source)
            return &operation;
    return nullptr;
}

void _requireNVVMAtomicOperations(
    NVVMOperationRequirements& requirements,
    const NVVMPlannedAtomicOperation& operation)
{
    _requireAtomicOperation(
        requirements.atomicOperations,
        operation.desc,
        operation.diagnosticName);
    if (operation.negatesValue)
    {
        _requireValueOperation(
            requirements.valueOperations,
            operation.valueNegation.getDesc(),
            operation.valueNegation.diagnosticName);
    }
}

// Plans the physical/value boundary once, while collecting its provider requirements. Types
// have already passed storage admission, which excludes recursive aggregates. A recipe keeps
// canonical type identity and the exact extraction/construction strategy selected here.
Index _planNVVMStructuredBufferStorageConversion(
    NVVMOperationRequirements& requirements,
    IRType* type,
    bool storageToValue)
{
    SLANG_RELEASE_ASSERT(isNVVMSupportedStructuredBufferStorageType(type));
    NVVMStructuredConversionRecipe recipe;
    recipe.type = type;
    recipe.storageToValue = storageToValue;
    if (asNVVMSupportedPhysicalArrayStructType(type))
    {
        // The explicit-stride array constructor already supplies this wrapper's physical value.
    }
    else if (isNVVMBoolType(type))
    {
        recipe.kind = NVVMStructuredConversionKind::Boolean;
        _requireValueOperation(
            requirements.valueOperations,
            storageToValue ? kNVVMStructuredBoolLoadOperation : kNVVMStructuredBoolStoreOperation,
            storageToValue ? "structured-buffer Boolean load conversion"
                           : "structured-buffer Boolean store conversion");
    }
    else
    {
        uint32_t laneCount = 0;
        if (auto vectorType = asNVVMSupportedValueVectorType(type, &laneCount))
        {
            if (isNVVMBoolType(vectorType->getElementType()) || laneCount == 3)
            {
                recipe.kind = NVVMStructuredConversionKind::Elements;
                recipe.extractAggregate = storageToValue && laneCount == 3;
                recipe.constructAggregate = !storageToValue && laneCount == 3;
                const Index child = _planNVVMStructuredBufferStorageConversion(
                    requirements,
                    vectorType->getElementType(),
                    storageToValue);
                for (uint32_t i = 0; i < laneCount; ++i)
                    recipe.children.add(child);
            }
        }
        else if (auto arrayType = as<IRArrayType>(type))
        {
            recipe.kind = NVVMStructuredConversionKind::Elements;
            recipe.extractAggregate = recipe.constructAggregate = true;
            const Index child = _planNVVMStructuredBufferStorageConversion(
                requirements,
                arrayType->getElementType(),
                storageToValue);
            const auto count = cast<IRIntLit>(arrayType->getElementCount())->getValue();
            for (IRIntegerValue i = 0; i < count; ++i)
                recipe.children.add(child);
        }
        else if (auto structType = as<IRStructType>(type))
        {
            recipe.kind = NVVMStructuredConversionKind::Elements;
            recipe.extractAggregate = recipe.constructAggregate = true;
            for (auto field : structType->getFields())
                recipe.children.add(_planNVVMStructuredBufferStorageConversion(
                    requirements,
                    field->getFieldType(),
                    storageToValue));
        }
    }
    auto& recipes = requirements.emissionPlan.structuredConversions;
    const Index index = recipes.getCount();
    recipes.add(_Move(recipe));
    return index;
}

// A checked resource root owns the external-storage role independently of the leaf pointer's
// spelling. Borrowed/local pointers with the same pointee never acquire that role.
IRType* _getNVVMStructuredBufferStoragePointerValueType(
    const NVVMAddressPlan& addresses,
    IRInst* pointer)
{
    auto pointerType = pointer ? as<IRPtrTypeBase>(pointer->getDataType()) : nullptr;
    IRType* valueType = pointerType ? pointerType->getValueType() : nullptr;
    if (!valueType || !isNVVMSupportedStructuredBufferStorageType(valueType))
        return nullptr;
    const auto buffer = addresses.findRootBuffer(pointer);
    return buffer && buffer->kind == NVVMRawBufferKind::Structured ? valueType : nullptr;
}

// Records both directions of the bit-preserving Half helper ABI boundary. A Half parameter uses
// the physical-to-canonical direction at helper entry and the canonical-to-physical direction at
// each call; a Half result uses the same pair in the opposite locations.
void _requireNVVMHalfHelperABIOperations(
    NVVMValueOperationRequirements& requirements,
    IRType* canonicalType)
{
    if (!hasNVVMHalfHelperABITransport(canonicalType))
        return;
    if (auto array = as<IRArrayType>(canonicalType))
    {
        _requireNVVMHalfHelperABIOperations(requirements, array->getElementType());
        return;
    }
    if (auto record = as<IRStructType>(canonicalType))
    {
        for (auto field : record->getFields())
            _requireNVVMHalfHelperABIOperations(requirements, field->getFieldType());
        return;
    }
    const NVVMHalfHelperABIOperation encode(canonicalType, true);
    const NVVMHalfHelperABIOperation decode(canonicalType, false);
    _requireValueOperation(requirements, encode.getDesc(), "physical Half helper ABI encoding");
    _requireValueOperation(requirements, decode.getDesc(), "canonical Half helper ABI decoding");
}

// Registers all scalar decode operations before a CUDA memory carrier is read. The same checked
// layout serves launch aggregates and pointer-bearing values copied from parameter groups.
void _requireNVVMCUDAValueDecodeOperations(
    NVVMValueOperationRequirements& requirements,
    const NVVMCUDAValueLayout& layout)
{
    List<const NVVMCUDAValueLayout*> pending;
    pending.add(&layout);
    while (pending.getCount())
    {
        auto node = pending.getLast();
        pending.removeLast();
        if (node->isBoolean)
            _requireValueOperation(
                requirements,
                kNVVMStructuredBoolLoadOperation,
                "numeric entry Boolean decode");
        if (node->isHalf)
            _requireNVVMHalfHelperABIOperations(requirements, node->scalarType);
        for (const auto& child : node->children)
            pending.add(&child);
    }
}


void _requireNVVMUInt64WordConstructionOperations(
    NVVMValueOperationRequirements& requirements,
    const NVVMPlannedUInt64WordConstruction& construction)
{
    const NVVMValueRecipeStep* steps[] = {
        &construction.wordConversion,
        &construction.highWordShift,
        &construction.combine,
    };
    for (auto step : steps)
        _requireValueOperation(requirements, step->getDesc(), step->diagnosticName);
}

void _requireNVVMResourceBitCastOperations(
    NVVMValueOperationRequirements& requirements,
    const NVVMPlannedResourceBitCast& bitCast)
{
    SLANG_ASSERT(bitCast.stepCount >= 1 && bitCast.stepCount <= SLANG_COUNT_OF(bitCast.steps));
    for (uint32_t i = 0; i < bitCast.stepCount; ++i)
    {
        const auto& step = bitCast.steps[i];
        _requireValueOperation(requirements, step.getDesc(), step.diagnosticName);
    }
}

bool _resolveNVVMFloatingRemainderOperation(
    IRInst* inst,
    NVVMPlannedFloatingRemainder& outOperation)
{
    outOperation = {};
    if (!inst || inst->getOp() != kIROp_FRem || inst->getOperandCount() != 2)
        return false;

    IRType* resultType = inst->getDataType();
    SlangNVVMValueTypeDesc resultSemantic = {};
    SlangNVVMValueTypeDesc operandSemantics[2] = {};
    if (!_getNVVMSemanticType(resultType, resultSemantic) ||
        !_getNVVMSemanticType(inst->getOperand(0)->getDataType(), operandSemantics[0]) ||
        !_getNVVMSemanticType(inst->getOperand(1)->getDataType(), operandSemantics[1]))
    {
        return false;
    }

    const SlangNVVMValueOperationDesc componentwiseDesc = {
        SLANG_NVVM_VALUE_OP_REMAINDER,
        resultSemantic,
        operandSemantics,
        SLANG_COUNT_OF(operandSemantics),
    };
    NVVMSemantics::ValueOperationFamilyResolution componentwiseFamily;
    if (!NVVMSemantics::resolveValueOperationFamily(componentwiseDesc, componentwiseFamily) ||
        componentwiseFamily.family != NVVMSemantics::ValueOperationFamily::FloatBinary)
    {
        return false;
    }

    IRType* scalarType = resultType;
    uint32_t laneCount = 1;
    if (auto vectorType = asNVVMSupportedValueVectorType(resultType, &laneCount))
        scalarType = vectorType->getElementType();

    uint32_t bitWidth = 0;
    if (!isNVVMSupportedFloatingPointScalarType(scalarType, &bitWidth) ||
        (bitWidth != 32 && bitWidth != 64))
    {
        return false;
    }

    SlangNVVMValueTypeDesc scalarSemantic = {};
    SLANG_RELEASE_ASSERT(_getNVVMSemanticType(scalarType, scalarSemantic));
    const SlangNVVMValueTypeDesc operandTypes[] = {scalarSemantic, scalarSemantic};
    _setNVVMValueRecipeStep(
        outOperation.scalarStep,
        SLANG_NVVM_VALUE_OP_FMOD,
        scalarSemantic,
        operandTypes,
        SLANG_COUNT_OF(operandTypes),
        "CUDA scalar floating-point remainder");

    NVVMSemantics::ValueOperationFamilyResolution family;
    if (!NVVMSemantics::resolveValueOperationFamily(outOperation.scalarStep.getDesc(), family))
        return false;
    SLANG_RELEASE_ASSERT(family.requiresCUDADeviceLibrary);

    outOperation.operands[0] = inst->getOperand(0);
    outOperation.operands[1] = inst->getOperand(1);
    outOperation.operandIsVector[0] = operandSemantics[0].laneCount > 1;
    outOperation.operandIsVector[1] = operandSemantics[1].laneCount > 1;
    outOperation.resultType = resultType;
    outOperation.scalarType = scalarType;
    outOperation.laneCount = laneCount;
    outOperation.scalarStep.diagnosticName = family.diagnosticName;
    outOperation.source = inst;
    return true;
}

// Resolves numeric operations through their family and hardware-wave operations by exact signature.
bool _resolveNVVMValueOperation(IRInst* inst, NVVMResolvedValueOperation& outOperation)
{
    outOperation = {};
    if (!inst || inst->getOperandCount() > 3)
        return false;

    SlangNVVMValueOperation operation = 0;
    SlangNVVMValueTypeDesc resultType = {};
    if (!_getNVVMValueOperation(inst->getOp(), operation) ||
        !_getNVVMSemanticType(inst->getDataType(), resultType))
    {
        return false;
    }
    for (UInt i = 0; i < inst->getOperandCount(); ++i)
    {
        IRInst* operand = inst->getOperand(i);
        if (!operand || !_getNVVMSemanticType(operand->getDataType(), outOperation.operandTypes[i]))
            return false;
    }

    outOperation.desc = {
        operation,
        resultType,
        inst->getOperandCount() ? outOperation.operandTypes : nullptr,
        inst->getOperandCount(),
    };
    if (NVVMSemantics::resolveValueOperationFamily(outOperation.desc, outOperation.family))
    {
        outOperation.diagnosticName = outOperation.family.diagnosticName;
        return true;
    }
    outOperation.staticEntry = NVVMSemantics::find(outOperation.desc);
    if (!outOperation.staticEntry)
        return false;
    outOperation.diagnosticName = outOperation.staticEntry->diagnosticName;
    return true;
}

// Resolves a canonical checked numeric-to-Boolean cast as truthiness rather than a width
// conversion. Integer lowering uses `IntCast`, while floating lowering uses `CastFloatToInt`;
// their complete scalar source and Bool result types prove the shared nonzero comparison semantic.
bool _resolveNVVMNumericTruthiness(IRInst* inst, NVVMPlannedNumericTruthiness& outOperation)
{
    outOperation = {};
    if (!inst || (inst->getOp() != kIROp_IntCast && inst->getOp() != kIROp_CastFloatToInt) ||
        inst->getOperandCount() != 1)
        return false;

    IRInst* value = inst->getOperand(0);
    SlangNVVMValueTypeDesc resultType = {};
    if (!value || !_getNVVMSemanticType(inst->getDataType(), resultType) ||
        !_getNVVMSemanticType(value->getDataType(), outOperation.valueType) ||
        !NVVMSemantics::isSelectedBoolValue(resultType) || resultType.laneCount != 1 ||
        outOperation.valueType.laneCount != 1 ||
        (inst->getOp() == kIROp_IntCast
             ? !NVVMSemantics::isSelectedIntegerValue(outOperation.valueType)
             : !NVVMSemantics::isSelectedFloatValue(outOperation.valueType)))
    {
        return false;
    }

    const SlangNVVMValueTypeDesc operands[] = {
        outOperation.valueType,
        outOperation.valueType,
    };
    if (!_setNVVMSupportedValueRecipeStep(
            outOperation.comparison,
            SLANG_NVVM_VALUE_OP_NOT_EQUAL,
            resultType,
            operands,
            SLANG_COUNT_OF(operands),
            inst->getOp() == kIROp_IntCast ? "integer truthiness comparison"
                                           : "floating-point truthiness comparison"))
    {
        return false;
    }
    outOperation.value = value;
    outOperation.source = inst;
    return true;
}

void _requireNVVMNumericTruthinessOperations(
    NVVMValueOperationRequirements& requirements,
    const NVVMPlannedNumericTruthiness& operation)
{
    _requireValueOperation(
        requirements,
        operation.comparison.getDesc(),
        operation.comparison.diagnosticName);
}

// Checks that an executable operand has an accepted definition that dominates its use.
SlangResult _validateAvailableValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    // Canonical module-owned storage values exist before every function body
    // and therefore do not participate in instruction dominance. All other executable values
    // remain SSA-ordered.
    NVVMConventionalGlobalParams globalParams;
    if (value && consumer && value->getModule() == consumer->getModule() &&
        (getNVVMSupportedSharedGlobal(value) ||
         _getNVVMConventionalGlobalParams(value, globalParams) ||
         _isNVVMSupportedModuleConstantValue(value)))
    {
        return SLANG_OK;
    }
    if (value && consumer && dominatorTree && availableValues.contains(value) &&
        dominatorTree->dominates(value, consumer))
    {
        return SLANG_OK;
    }

    return _diagnoseUnsupportedIR(
        codeGenContext,
        value ? UnownedStringSlice(getIROpInfo(value->getOp()).name) : toSlice("missing operand"));
}

// Checks that an executable operand is an available signed 32-bit value.
SlangResult _validateI32Value(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (!value || !isNVVMSignedI32Type(value->getDataType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 value"));

    if (_asExecutableI32Constant(value))
    {
        return SLANG_OK;
    }

    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks sign-independent transport of a canonical 32-bit integer value.
SlangResult _validateInteger32Value(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (value && isNVVMSignedI32Type(value->getDataType()))
    {
        return _validateI32Value(codeGenContext, value, consumer, availableValues, dominatorTree);
    }
    if (!value || !isNVVMUnsignedI32Type(value->getDataType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("32-bit integer value"));
    if (_asExecutableSelectedIntegerConstant(value))
        return SLANG_OK;
    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks one selected integer value, including an exact-width executable literal.
SlangResult _validateSelectedIntegerValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (!value || !isNVVMSupportedIntegerScalarType(value->getDataType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("selected integer value"));
    if (_asExecutableSelectedIntegerConstant(value))
    {
        return SLANG_OK;
    }
    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks a canonical UInt value, including its operation-defined 32-bit literal form.
SlangResult _validateUnsignedI32Value(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree,
    UnownedStringSlice diagnosticRole)
{
    if (!value || !isNVVMUnsignedI32Type(value->getDataType()))
        return _diagnoseUnsupportedIR(codeGenContext, diagnosticRole);
    if (_asExecutableInteger32Constant(value))
    {
        return SLANG_OK;
    }
    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks a canonical UInt wave mask, including its operation-defined 32-bit literal form.
SlangResult _validateWaveMaskValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    return _validateUnsignedI32Value(
        codeGenContext,
        value,
        consumer,
        availableValues,
        dominatorTree,
        toSlice("wave mask value"));
}

// Checks transport of a canonical Boolean value or materializes its literal through i1.
SlangResult _validateBooleanValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (!value || !isNVVMBoolType(value->getDataType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("Boolean value"));
    if (_asExecutableBoolConstant(value))
    {
        return SLANG_OK;
    }
    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks that an executable operand is an available selected floating-point value.
SlangResult _validateFloatingPointValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (!value ||
        (!isNVVMSupportedFloatingPointScalarType(value->getDataType()) &&
         !isNVVMBFloat16Type(value->getDataType()) && !isNVVMFloat8Type(value->getDataType())))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("floating-point value"));

    if (auto literal = _asExecutableFloatingPointConstant(value))
    {
        if (isNVVMFloat8Type(literal->getDataType()) &&
            (Math::IsNaN(literal->getValue()) || Math::IsInf(literal->getValue())))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("nonfinite FP8 literal"));
        return SLANG_OK;
    }

    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks an available canonical scalar value using its semantic type.
SlangResult _validateScalarValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (value && isNVVMBoolType(value->getDataType()))
    {
        return _validateBooleanValue(
            codeGenContext,
            value,
            consumer,
            availableValues,
            dominatorTree);
    }
    if (value &&
        (isNVVMSupportedFloatingPointScalarType(value->getDataType()) ||
         isNVVMBFloat16Type(value->getDataType()) || isNVVMFloat8Type(value->getDataType())))
    {
        return _validateFloatingPointValue(
            codeGenContext,
            value,
            consumer,
            availableValues,
            dominatorTree);
    }
    if (value && isNVVMSupportedIntegerScalarType(value->getDataType()))
    {
        return _validateSelectedIntegerValue(
            codeGenContext,
            value,
            consumer,
            availableValues,
            dominatorTree);
    }
    return _diagnoseUnsupportedIR(codeGenContext, toSlice("scalar value"));
}

// Checks a selected scalar or an available first-class resource-capable value admitted by
// preflight. Scalars may be executable constants; every vector, aggregate, pointer-backed resource
// view, and CUDA handle must already have a dominating producer.
SlangResult _validateSelectedValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (_asExecutableNullDevicePointer(value))
        return SLANG_OK;
    IRType* valueType = value ? value->getDataType() : nullptr;
    const bool requiresAvailability =
        valueType &&
        (asNVVMRegisterVectorType(valueType) ||
         ((as<IRArrayType>(valueType) || as<IRStructType>(valueType)) &&
          isNVVMSupportedStructuredBufferStorageType(valueType)) ||
         (_getNVVMExecutableValueAlignment(valueType) &&
          !isNVVMSupportedIntegerScalarType(valueType) &&
          !isNVVMSupportedFloatingPointScalarType(valueType) && !isNVVMBFloat16Type(valueType) &&
          !isNVVMFloat8Type(valueType) && !isNVVMBoolType(valueType)));
    if (requiresAvailability)
    {
        return _validateAvailableValue(
            codeGenContext,
            value,
            consumer,
            availableValues,
            dominatorTree);
    }
    return _validateScalarValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks a selected byte payload. A fixed array is already an exact first-class SSA value, so it
// needs availability validation rather than the scalar/vector semantic validator.
SlangResult _validateByteAddressValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (value && asNVVMSupportedNumericArrayType(value->getDataType()))
        return _validateAvailableValue(
            codeGenContext,
            value,
            consumer,
            availableValues,
            dominatorTree);
    return _validateSelectedValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Finds only completed address records. A matching pointer type alone never grants layout
// memory access, and nested record addresses remain navigable without whole-record loads.
const NVVMLayoutStorage* _findNVVMLayoutStorage(const NVVMAddressPlan& addresses, IRInst* pointer)
{
    if (auto field = addresses.findFieldAddress(pointer))
        return field->isLayoutStorage ? &field->layoutStorage : nullptr;
    if (auto element = addresses.findElementAddress(pointer))
        return element->isLayoutStorage ? &element->layoutStorage : nullptr;
    return nullptr;
}

// Checks an available scalar pointer and enforces the source access qualifier for stores.
SlangResult _validatePointerValue(
    CodeGenContext* codeGenContext,
    const NVVMOperationRequirements& requirements,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree,
    bool requireWriteAccess,
    IRType* expectedPointeeType)
{
    if (auto pointer = _getNVVMHitObjectPointer(requirements.emissionPlan.addresses, value))
    {
        if (!expectedPointeeType || !isTypeEqual(pointer->getValueType(), expectedPointeeType) ||
            (requireWriteAccess && pointer->getAccessQualifier() != AccessQualifier::ReadWrite))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("HitObject storage access"));
        return _validateAvailableValue(
            codeGenContext,
            value,
            consumer,
            availableValues,
            dominatorTree);
    }
    if (auto storage = _findNVVMLayoutStorage(requirements.emissionPlan.addresses, value))
    {
        auto pointerType = as<IRPtrTypeBase>(value->getDataType());
        if (!storage->valueType || !pointerType ||
            !isTypeEqual(storage->valueType, expectedPointeeType) ||
            (consumer->getOp() != kIROp_Load && consumer->getOp() != kIROp_Store) ||
            (requireWriteAccess && pointerType->getAccessQualifier() != AccessQualifier::ReadWrite))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("layout field memory access"));
        return _validateAvailableValue(
            codeGenContext,
            value,
            consumer,
            availableValues,
            dominatorTree);
    }

    IRType* loadedParameterGroupElementType = nullptr;
    if (_getNVVMParameterGroupPointer(
            requirements.emissionPlan.addresses,
            value,
            loadedParameterGroupElementType))
    {
        if (!hasNVVMParameterGroupStorageValueRepresentation(loadedParameterGroupElementType))
        {
            return _diagnoseUnsupportedIR(
                codeGenContext,
                toSlice("loaded parameter-group value representation"));
        }
        if (!consumer || consumer->getOp() != kIROp_Load || requireWriteAccess)
        {
            StringBuilder construct;
            construct << "immutable loaded parameter-group access: consumer="
                      << (consumer ? getIROpInfo(consumer->getOp()).name : "missing");
            return _diagnoseUnsupportedIR(codeGenContext, construct.getUnownedSlice());
        }
        if (!expectedPointeeType ||
            !isTypeEqual(loadedParameterGroupElementType, expectedPointeeType))
        {
            return _diagnoseUnsupportedIR(
                codeGenContext,
                toSlice("loaded parameter-group pointee type"));
        }
        return _validateAvailableValue(
            codeGenContext,
            value,
            consumer,
            availableValues,
            dominatorTree);
    }

    auto numericPtrType =
        value ? asNVVMSupportedDeviceNumericPointerType(value->getDataType()) : nullptr;
    auto deviceCopyablePtrType =
        value ? asNVVMSupportedDeviceCopyableValuePointerType(value->getDataType()) : nullptr;
    auto deviceHelperPtrType =
        value ? asNVVMSupportedDeviceHelperValuePointerType(value->getDataType()) : nullptr;
    auto devicePhysicalStoragePtrType =
        value ? asNVVMSupportedDevicePhysicalStoragePointerType(value->getDataType()) : nullptr;
    const auto plannedElement = requirements.emissionPlan.addresses.findElementAddress(value);
    const auto structuredBufferElementPointer =
        requirements.emissionPlan.addresses.findStructuredElement(value);
    const bool hasStructuredBufferElementProducer = structuredBufferElementPointer != nullptr;
    const bool hasResourceElementProducer =
        hasStructuredBufferElementProducer ||
        (plannedElement && plannedElement->kind == NVVMElementAddressKind::RawBuffer);
    auto resourceElementPtrType =
        hasResourceElementProducer
            ? asNVVMSupportedRWStructuredBufferElementPointerType(value->getDataType())
            : nullptr;
    auto sharedElementPtrType =
        value ? asNVVMSupportedSharedElementPointerType(value->getDataType()) : nullptr;
    NVVMSharedGlobal sharedGlobal;
    auto sharedGlobalPtrType = value && getNVVMSupportedSharedGlobal(value, &sharedGlobal)
                                   ? as<IRPtrTypeBase>(value->getDataType())
                                   : nullptr;
    auto localRecordArrayPtrType = _getNVVMLocalSubstandardRecordArrayPointer(value);
    auto localStructPtrType =
        _getNVVMLocalValueReference(requirements.emissionPlan.addresses, value);
    auto localHelperPtrType =
        value ? asNVVMSupportedLocalHelperValuePointerType(value->getDataType()) : nullptr;
    // A local `var T` and module-scope groupshared storage can both expose `Ptr<T>`. Only a local
    // variable or helper parameter proves the generic local-copyable role at this boundary.
    auto localCopyablePtrType =
        value && (value->getOp() == kIROp_Var || as<IRParam>(value))
            ? asNVVMSupportedLocalCopyableValuePointerType(value->getDataType())
            : nullptr;
    auto localPhysicalStoragePtrType =
        value && value->getOp() == kIROp_Var
            ? asNVVMSupportedLocalPhysicalStoragePointerType(value->getDataType())
            : nullptr;
    auto helperReferencePtrType =
        value && as<IRParam>(value)
            ? asNVVMSupportedHelperReferencePointerType(value->getDataType())
            : nullptr;
    auto physicalStorageReferencePtrType =
        value && as<IRParam>(value)
            ? asNVVMSupportedPhysicalStorageReferencePointerType(value->getDataType())
            : nullptr;
    auto sharedHelperPtrType =
        value ? asNVVMSupportedSharedHelperPointerType(value->getDataType()) : nullptr;
    auto sequentialElementPtrType =
        plannedElement && plannedElement->kind == NVVMElementAddressKind::Sequential
            ? plannedElement->resultType
            : nullptr;
    auto fieldPtrType = value ? as<IRPtrTypeBase>(value->getDataType()) : nullptr;
    const auto plannedField = requirements.emissionPlan.addresses.findFieldAddress(value);
    if (!fieldPtrType || !plannedField)
    {
        fieldPtrType = nullptr;
    }
    IRPtrTypeBase* devicePtrType = numericPtrType;
    if (!devicePtrType)
        devicePtrType = deviceCopyablePtrType;
    if (!devicePtrType)
        devicePtrType = deviceHelperPtrType;
    if (!devicePtrType)
        devicePtrType = devicePhysicalStoragePtrType;
    IRPtrTypeBase* acceptedPtrType = devicePtrType                 ? devicePtrType
                                     : sharedElementPtrType        ? sharedElementPtrType
                                     : sharedGlobalPtrType         ? sharedGlobalPtrType
                                     : resourceElementPtrType      ? resourceElementPtrType
                                     : localCopyablePtrType        ? localCopyablePtrType
                                     : localPhysicalStoragePtrType ? localPhysicalStoragePtrType
                                     : sequentialElementPtrType    ? sequentialElementPtrType
                                     : localRecordArrayPtrType     ? localRecordArrayPtrType
                                     : localStructPtrType          ? localStructPtrType
                                     : localHelperPtrType          ? localHelperPtrType
                                     : helperReferencePtrType      ? helperReferencePtrType
                                     : physicalStorageReferencePtrType
                                         ? physicalStorageReferencePtrType
                                     : sharedHelperPtrType ? sharedHelperPtrType
                                                           : fieldPtrType;
    if (!acceptedPtrType)
    {
        StringBuilder construct;
        construct << "device scalar pointer: producer="
                  << (value ? getIROpInfo(value->getOp()).name : "missing")
                  << ", consumer=" << (consumer ? getIROpInfo(consumer->getOp()).name : "missing");
        return _diagnoseUnsupportedIR(codeGenContext, construct.getUnownedSlice());
    }
    IRType* actualPointeeType = acceptedPtrType->getValueType();
    if (!expectedPointeeType || !isTypeEqual(actualPointeeType, expectedPointeeType))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("device pointer pointee type"));
    const auto atomicOperation =
        _findPlannedNVVMOperation(requirements.emissionPlan.atomicOperations, consumer);
    const bool isAtomicConsumer = atomicOperation && atomicOperation->pointer == value;
    const auto plannedChild = requirements.emissionPlan.addresses.findElementAddress(consumer);
    const bool hasSequentialChild = plannedChild && plannedChild->base == value &&
                                    plannedChild->kind == NVVMElementAddressKind::Sequential;
    if (resourceElementPtrType && consumer->getOp() != kIROp_Load &&
        consumer->getOp() != kIROp_Store && consumer->getOp() != kIROp_SwizzledStore &&
        !isAtomicConsumer && consumer->getOp() != kIROp_Call && !hasSequentialChild)
    {
        return _diagnoseUnsupportedIR(
            codeGenContext,
            toSlice("raw RWStructuredBuffer numeric load or store consumer"));
    }
    const auto childField = requirements.emissionPlan.addresses.findFieldAddress(consumer);
    const bool hasCheckedChild = hasSequentialChild || (childField && childField->base == value);
    if (hasStructuredBufferElementProducer &&
        structuredBufferElementPointer->bufferType.access == NVVMBufferAccess::ReadOnly &&
        consumer->getOp() != kIROp_Load && !hasCheckedChild)
    {
        return _diagnoseUnsupportedIR(
            codeGenContext,
            toSlice("read-only structured-buffer element access"));
    }
    const auto rootBuffer = requirements.emissionPlan.addresses.findRootBuffer(value);
    bool isReadOnlyResourceUse = rootBuffer && hasCheckedChild;
    if (_getNVVMLocalValueReference(requirements.emissionPlan.addresses, value))
    {
        auto childField = requirements.emissionPlan.addresses.findFieldAddress(consumer);
        isReadOnlyResourceUse = hasSequentialChild || (childField && childField->base == value);
        if (auto call = as<IRCall>(consumer))
        {
            if (auto callee = as<IRFunc>(call->getCallee()))
            {
                isReadOnlyResourceUse = true;
                for (UInt i = 0; i < call->getArgCount(); ++i)
                    if (call->getArg(i) == value)
                    {
                        auto parameter =
                            asNVVMSupportedLocalResourceValuePointerType(callee->getParamType(i));
                        if (!parameter)
                            parameter =
                                asNVVMSupportedHelperReferencePointerType(callee->getParamType(i));
                        if (!parameter || parameter->getAccessQualifier() != AccessQualifier::Read)
                            isReadOnlyResourceUse = false;
                    }
            }
        }
    }
    if (fieldPtrType && !plannedField->selection.isMutable &&
        ((consumer->getOp() != kIROp_Load && !isReadOnlyResourceUse) || requireWriteAccess))
    {
        StringBuilder construct;
        construct << "immutable struct field access: consumer="
                  << getIROpInfo(consumer->getOp()).name;
        return _diagnoseUnsupportedIR(codeGenContext, construct.getUnownedSlice());
    }
    if (sequentialElementPtrType && plannedElement->isReadOnly &&
        ((consumer->getOp() != kIROp_Load && !isReadOnlyResourceUse) || requireWriteAccess))
    {
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("read-only sequential element load"));
    }
    if (requireWriteAccess && acceptedPtrType->getAccessQualifier() != AccessQualifier::ReadWrite)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("read-only pointer store"));
    if (_asExecutableNullDevicePointer(value))
        return SLANG_OK;
    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks that a branch destination is a block declared by the selected function.
SlangResult _validateBlockTarget(
    CodeGenContext* codeGenContext,
    IRBlock* block,
    const HashSet<IRBlock*>& functionBlocks)
{
    if (block && functionBlocks.contains(block))
        return SLANG_OK;
    return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch target"));
}

// Orders reachable bodies by CFG dominance, then preserves physical order for unreachable bodies.
List<IRBlock*> _getNVVMBodyOrder(IRFunc* function, IRDominatorTree* dominatorTree)
{
    List<IRBlock*> result;
    HashSet<IRBlock*> addedBlocks;
    for (auto block : getReversePostorder(function))
    {
        if (!dominatorTree->isUnreachable(block) && addedBlocks.add(block))
            result.add(block);
    }
    for (auto block : function->getBlocks())
    {
        if (addedBlocks.add(block))
            result.add(block);
    }
    return result;
}

// Counts the positional SSA values a branch to `block` must provide.
UInt _getBlockParamCount(IRBlock* block)
{
    UInt count = 0;
    for (auto param : block->getParams())
    {
        SLANG_UNUSED(param);
        ++count;
    }
    return count;
}

// Validates the positional SSA values carried by an actual branch edge.
SlangResult _validateBranchArguments(
    CodeGenContext* codeGenContext,
    IRUnconditionalBranch* branch,
    IRBlock* entryBlock,
    const HashSet<IRBlock*>& functionBlocks,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    IRBlock* targetBlock = branch->getTargetBlock();
    SLANG_RETURN_ON_FAIL(_validateBlockTarget(codeGenContext, targetBlock, functionBlocks));
    if (targetBlock == entryBlock)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-block branch target"));

    const UInt argumentCount = branch->getArgCount();
    if (argumentCount != _getBlockParamCount(targetBlock))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch argument count"));

    IRParam* targetParam = targetBlock->getFirstParam();
    for (UInt argumentIndex = 0; argumentIndex < argumentCount;
         ++argumentIndex, targetParam = targetParam->getNextParam())
    {
        IRInst* argument = branch->getArg(argumentIndex);
        SLANG_ASSERT(targetParam);
        if (!argument || !isTypeEqual(argument->getDataType(), targetParam->getDataType()))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch argument type"));
        SLANG_RETURN_ON_FAIL(_validateSelectedValue(
            codeGenContext,
            argument,
            branch,
            availableValues,
            dominatorTree));
    }
    return SLANG_OK;
}

// Returns the LLVM symbol chosen from the canonical linked IR for an accepted function.
String _getNVVMFunctionName(IRFunc* function, IRFunc* entryPoint)
{
    if (function == entryPoint)
    {
        auto entryPointDecoration = function->findDecoration<IREntryPointDecoration>();
        SLANG_RELEASE_ASSERT(entryPointDecoration);
        String name(entryPointDecoration->getName()->getStringSlice());
        switch (entryPointDecoration->getProfile().getStage())
        {
        case Stage::RayGeneration:
            return String("__raygen__") + name;
        case Stage::Miss:
            return String("__miss__") + name;
        case Stage::ClosestHit:
            return String("__closesthit__") + name;
        case Stage::AnyHit:
            return String("__anyhit__") + name;
        case Stage::Intersection:
            return String("__intersection__") + name;
        case Stage::Callable:
            return String("__direct_callable__") + name;
        default:
            break;
        }
        return name;
    }
    if (auto exportDecoration = function->findDecorationImpl(kIROp_CudaDeviceExportDecoration))
    {
        SLANG_RELEASE_ASSERT(exportDecoration->getOperandCount() == 1);
        auto exportName = as<IRStringLit>(exportDecoration->getOperand(0));
        SLANG_RELEASE_ASSERT(exportName);
        return String(exportName->getStringSlice());
    }
    return String(getMangledName(function));
}

// Returns whether a type is an accepted canonical value in a helper result.
bool _isSupportedNVVMHelperResultType(IRType* type)
{
    return classifyNVVMType(type).supports(NVVMTypeUse::HelperResult);
}

// Returns whether one exact canonical type can cross a selected helper parameter boundary.
bool _isSupportedNVVMHelperParameterType(IRType* type)
{
    return classifyNVVMType(type).supports(NVVMTypeUse::HelperParameter);
}

// Returns the exact group-shared pointer values whose source type is allowed to omit address-space
// provenance. A module global carries that provenance on its GroupShared rate. Explicit helper and
// element pointers preserve address space 3 directly in their type.
IRPtrTypeBase* _asNVVMSupportedSharedPointerValue(IRInst* value)
{
    auto pointerType = value ? as<IRPtrTypeBase>(value->getDataType()) : nullptr;
    if (!pointerType)
        return nullptr;
    if (asNVVMSupportedSharedHelperPointerType(pointerType) ||
        asNVVMSupportedSharedElementPointerType(pointerType))
    {
        return pointerType;
    }

    NVVMSharedGlobal sharedGlobal;
    return getNVVMSupportedSharedGlobal(value, &sharedGlobal) &&
                   isNVVMSupportedHelperValueType(pointerType->getValueType())
               ? pointerType
               : nullptr;
}

// Returns whether one canonical call argument satisfies an exact helper parameter. A mutable
// borrow, a group-shared root, and an explicit thread-local context parameter deliberately have
// distinct source types from the pointer parameter, while each retains exact producer provenance
// and lowers to one typed pointer in the corresponding provider address space.
bool _isSupportedNVVMHelperArgument(
    CodeGenContext* codeGenContext,
    const NVVMAddressPlan& addresses,
    IRInst* argument,
    IRType* parameterType)
{
    IRType* argumentType = argument ? argument->getDataType() : nullptr;
    if (!argumentType)
        return false;
    if (auto reference = asNVVMSupportedLocalRecordArrayReferenceType(parameterType))
    {
        auto actual = _getNVVMLocalSubstandardRecordArrayPointer(argument);
        return actual &&
               (reference->getAccessQualifier() == AccessQualifier::Read ||
                actual->getAccessQualifier() == AccessQualifier::ReadWrite) &&
               isTypeEqual(actual->getValueType(), reference->getValueType());
    }
    if (auto parameter = asNVVMHitObjectPointerType(parameterType))
    {
        auto actual = _getNVVMHitObjectPointer(addresses, argument);
        return actual && isTypeEqual(actual->getValueType(), parameter->getValueType()) &&
               (parameter->getAccessQualifier() == AccessQualifier::Read ||
                actual->getAccessQualifier() == AccessQualifier::ReadWrite);
    }
    if (isTypeEqual(argumentType, parameterType))
        return true;

    // Storage legalization can retain CUDALayout on an address while the unchanged helper
    // parameter retains DefaultLayout. Exact pointee identity and CUDA/LLVM size/alignment prove
    // that this qualification preserves the pointed-to object; differing physical types reject.
    auto argumentDevice = asNVVMSupportedDeviceHelperValuePointerType(argumentType);
    auto parameterDevice = asNVVMSupportedDeviceHelperValuePointerType(parameterType);
    if (argumentDevice && parameterDevice &&
        isTypeEqual(argumentDevice->getValueType(), parameterDevice->getValueType()) &&
        _hasNVVMCompatibleHelperValueLayout(codeGenContext, argumentDevice->getValueType()))
        return true;


    // A returned reference uses the ordinary generic pointer spelling. Converting a checked
    // local reference or device pointer to that spelling preserves storage and access.
    if (auto parameter = asNVVMSupportedLocalCopyableValuePointerType(parameterType))
    {
        bool isReadOnly = false;
        auto local = _getNVVMLocalValueReference(addresses, argument, &isReadOnly);
        if (local && !isReadOnly && isTypeEqual(local->getValueType(), parameter->getValueType()))
            return true;
        auto actual = asNVVMSupportedDeviceCopyableValuePointerType(argumentType);
        if (actual && actual->getAccessQualifier() == AccessQualifier::ReadWrite &&
            isTypeEqual(actual->getValueType(), parameter->getValueType()) &&
            (!actual->getDataLayout() ||
             actual->getDataLayout()->getOp() == kIROp_DefaultBufferLayoutType))
            return true;
    }

    IRType* parameterSharedValueType = nullptr;
    auto parameterSharedPointer =
        asNVVMSupportedSharedHelperPointerType(parameterType, &parameterSharedValueType);
    auto argumentSharedPointer = _asNVVMSupportedSharedPointerValue(argument);
    if (parameterSharedPointer && argumentSharedPointer &&
        isTypeEqual(argumentSharedPointer->getValueType(), parameterSharedValueType))
    {
        return true;
    }

    IRType* parameterReferenceValueType = nullptr;
    auto parameterReference =
        asNVVMSupportedHelperReferencePointerType(parameterType, &parameterReferenceValueType);
    if (parameterReference)
    {
        // A method such as `params.getValue()` borrows its existing constant-buffer record.
        // CUDA storage legalization can make that record an ordinary helper value type rather
        // than an IRPhysicalType. Preserve the readonly borrow only when both storage roles
        // already have the same representation; pointer-bearing or repacked values still reject.
        IRType* groupElementType = nullptr;
        if (parameterReference->getAccessQualifier() == AccessQualifier::Read &&
            _getNVVMParameterGroupPointer(addresses, argument, groupElementType) &&
            isTypeEqual(groupElementType, parameterReferenceValueType) &&
            hasNVVMParameterGroupStorageValueRepresentation(groupElementType) &&
            _hasNVVMCompatibleHelperValueLayout(codeGenContext, groupElementType))
            return true;

        IRType* argumentValueType = nullptr;
        IRPtrTypeBase* argumentPointer =
            asNVVMSupportedLocalCopyableValuePointerType(argumentType, &argumentValueType);
        if (!argumentPointer)
            argumentPointer =
                asNVVMSupportedLocalHelperValuePointerType(argumentType, &argumentValueType);
        if (!argumentPointer)
            argumentPointer =
                asNVVMSupportedDerivedCopyableValuePointerType(argumentType, &argumentValueType);
        if (!argumentPointer)
        {
            argumentPointer = asNVVMSupportedRWStructuredBufferElementPointerType(argumentType);
            argumentValueType = argumentPointer ? argumentPointer->getValueType() : nullptr;
        }
        bool isReadOnlyLocal = false;
        if (!argumentPointer)
        {
            argumentPointer = _getNVVMLocalValueReference(addresses, argument, &isReadOnlyLocal);
            argumentValueType = argumentPointer ? argumentPointer->getValueType() : nullptr;
        }
        const bool hasRequiredAccess =
            parameterReference->getAccessQualifier() == AccessQualifier::Read ||
            (argumentPointer && !isReadOnlyLocal &&
             argumentPointer->getAccessQualifier() == AccessQualifier::ReadWrite);
        if (argumentPointer && hasRequiredAccess &&
            isTypeEqual(argumentValueType, parameterReferenceValueType))
        {
            return true;
        }
    }

    IRStructType* parameterPhysicalStorageType = nullptr;
    auto parameterPhysicalStorageReference = asNVVMSupportedPhysicalStorageReferencePointerType(
        parameterType,
        &parameterPhysicalStorageType);
    if (parameterPhysicalStorageReference)
    {
        IRType* argumentParameterGroupElementType = nullptr;
        if (asNVVMSupportedParameterGroupType(argumentType, &argumentParameterGroupElementType) &&
            isTypeEqual(argumentParameterGroupElementType, parameterPhysicalStorageType))
        {
            return true;
        }

        IRStructType* argumentPhysicalStorageType = nullptr;
        if (asNVVMSupportedLocalPhysicalStoragePointerType(
                argumentType,
                &argumentPhysicalStorageType) &&
            isTypeEqual(argumentPhysicalStorageType, parameterPhysicalStorageType))
        {
            return true;
        }
    }

    IRType* parameterValueType = nullptr;
    bool isReadOnlyResource = false;
    auto argumentPointer = _getNVVMLocalValueReference(addresses, argument, &isReadOnlyResource);
    IRType* argumentValueType = argumentPointer ? argumentPointer->getValueType() : nullptr;
    auto parameterPointer =
        asNVVMSupportedLocalResourceValuePointerType(parameterType, &parameterValueType);
    if (argumentPointer && parameterPointer &&
        (parameterPointer->getAccessQualifier() == AccessQualifier::Read || !isReadOnlyResource) &&
        isTypeEqual(argumentValueType, parameterValueType))
    {
        return true;
    }

    IRType* argumentCopyableType = nullptr;
    IRType* parameterCopyableType = nullptr;
    auto argumentCopyablePointer =
        asNVVMSupportedLocalCopyableValuePointerType(argumentType, &argumentCopyableType);
    if (!argumentCopyablePointer)
    {
        argumentCopyablePointer =
            asNVVMSupportedDerivedCopyableValuePointerType(argumentType, &argumentCopyableType);
    }
    auto parameterCopyablePointer =
        asNVVMSupportedLocalCopyableValuePointerType(parameterType, &parameterCopyableType);
    const bool isMutableCopyableParameter =
        parameterCopyablePointer &&
        (parameterCopyablePointer->getOp() == kIROp_OutParamType ||
         parameterCopyablePointer->getOp() == kIROp_BorrowInOutParamType);
    // Consider `void initialize(out Payload value) { value.setLayer(); }`. Parameter lowering
    // preserves OutParam for value and BorrowInOutParam for the mutating method's this parameter.
    // addArg forwards the existing address, so the direction wrappers differ while the pointee
    // and generic pointer representation remain identical. The canonical local classifiers already
    // accept both mutable parameter roles; keep their storage checks and exact pointee identity.
    if (argumentCopyablePointer && isMutableCopyableParameter &&
        isTypeEqual(argumentCopyableType, parameterCopyableType))
    {
        return true;
    }

    IRType* argumentHelperType = nullptr;
    IRType* parameterHelperType = nullptr;
    auto argumentHelperPointer =
        asNVVMSupportedLocalHelperValuePointerType(argumentType, &argumentHelperType);
    bool isReadOnlyHelper = false;
    if (!argumentHelperPointer)
    {
        argumentHelperPointer = _getNVVMLocalValueReference(addresses, argument, &isReadOnlyHelper);
        argumentHelperType =
            argumentHelperPointer ? argumentHelperPointer->getValueType() : nullptr;
    }
    auto parameterHelperPointer =
        asNVVMSupportedLocalHelperValuePointerType(parameterType, &parameterHelperType);
    const bool isMutableHelperParameter =
        parameterHelperPointer && (parameterHelperPointer->getOp() == kIROp_OutParamType ||
                                   parameterHelperPointer->getOp() == kIROp_BorrowInOutParamType);
    // Pointer-bearing helper aggregates use the same forwarding rule. A derived child retains
    // its checked local root and access; its explicit pointer spelling alone is insufficient.
    if (argumentHelperPointer && isMutableHelperParameter && !isReadOnlyHelper &&
        argumentHelperPointer->getAccessQualifier() == AccessQualifier::ReadWrite &&
        isTypeEqual(argumentHelperType, parameterHelperType))
    {
        return true;
    }

    IRType* parameterDeviceValueType = nullptr;
    auto parameterDevicePointer =
        asNVVMSupportedDeviceHelperValuePointerType(parameterType, &parameterDeviceValueType);
    IRPtrTypeBase* argumentLocalPointer =
        argumentCopyablePointer ? argumentCopyablePointer : argumentHelperPointer;
    IRType* argumentLocalValueType =
        argumentCopyablePointer ? argumentCopyableType : argumentHelperType;
    bool isReadOnlyLocal = false;
    auto checkedLocal = _getNVVMLocalValueReference(addresses, argument, &isReadOnlyLocal);
    if (checkedLocal)
    {
        argumentLocalPointer = checkedLocal;
        argumentLocalValueType = checkedLocal->getValueType();
    }
    else if (argumentLocalPointer && argumentLocalPointer->getOperandCount() != 1)
        return false;
    return argumentLocalPointer &&
           (argumentLocalPointer->getOp() == kIROp_PtrType || checkedLocal) && !isReadOnlyLocal &&
           !isReadOnlyHelper && parameterDevicePointer &&
           argumentLocalPointer->getAccessQualifier() == AccessQualifier::ReadWrite &&
           isTypeEqual(argumentLocalValueType, parameterDeviceValueType);
}

// Returns whether a canonical helper-reference argument is physically produced in CUDA global
// memory. Its source type remains generic because resource layout is semantic metadata; the
// producer is the source of truth for the provider address space.
bool _isNVVMGlobalHelperReferenceArgument(const NVVMAddressPlan& addresses, IRInst* argument)
{
    if (!argument)
        return false;
    IRType* groupElementType = nullptr;
    if (asNVVMSupportedDeviceHelperValuePointerType(argument->getDataType()) ||
        _getNVVMParameterGroupPointer(addresses, argument, groupElementType))
        return true;
    const auto element = addresses.findElementAddress(argument);
    return addresses.findStructuredElement(argument) ||
           (element && element->kind == NVVMElementAddressKind::RawBuffer);
}

// Returns whether a canonical helper signature needs the generic construction path.
bool _usesGenericNVVMFunctions(IRFunc* helper)
{
    SLANG_RELEASE_ASSERT(helper);
    SLANG_RELEASE_ASSERT(_isSupportedNVVMHelperResultType(helper->getResultType()));
    if (!isNVVMSignedI32Type(helper->getResultType()))
        return true;
    for (UInt parameterIndex = 0; parameterIndex < helper->getParamCount(); ++parameterIndex)
    {
        IRType* parameterType = helper->getParamType(parameterIndex);
        SLANG_RELEASE_ASSERT(_isSupportedNVVMHelperParameterType(parameterType));
        if (!isNVVMSignedI32Type(parameterType))
            return true;
    }
    return false;
}

// Emits one non-void helper return through its complete target ABI boundary. All canonical helper
// producers, including specialized GenericAsm bodies, must use this path so a physical Half result
// cannot diverge from an ordinary IR `return`.
SlangResult _emitNVVMFunctionValueReturn(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    NVVMTypeLoweringContext& typeContext,
    IRFunc* function,
    const char* diagnosticName,
    SlangNVVMValueHandle value)
{
    SLANG_RELEASE_ASSERT(function && !as<IRVoidType>(function->getResultType()));
    if (hasNVVMHalfHelperABITransport(function->getResultType()))
    {
        SlangNVVMValueHandle physicalValue = nullptr;
        SLANG_RETURN_ON_FAIL(_emitNVVMHalfHelperABIReinterpretation(
            codeGenContext,
            builder,
            module,
            typeContext,
            function->getResultType(),
            true,
            value,
            physicalValue));
        value = physicalValue;
    }
    return _requireBuilderOperation(
        codeGenContext,
        diagnosticName,
        builder.emitValueReturn(module, value));
}

// Checks the exact helper ABI before adding a direct callee to the accepted closure.
SlangResult _validateNVVMHelperTarget(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    IRFunc* entryPoint,
    IRFunc* helper)
{
    if (!helper)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("call"));
    if (helper == entryPoint || helper->findDecoration<IREntryPointDecoration>() ||
        helper->findDecoration<IRCudaKernelDecoration>())
    {
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("call"));
    }
    if (helper->getParent() != linkedIR.module->getModuleInst() || !helper->isDefinition())
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("call"));
    // Internal callers and callees agree on the qualified LLVM register ABI. An exported
    // CUDA helper instead promises an external ABI: for example, BF3 is six bytes/alignment
    // two in CUDA but eight bytes/alignment eight in the provider. Keep that contract closed.
    const bool isCUDAExport =
        helper->findDecorationImpl(kIROp_CudaDeviceExportDecoration) != nullptr;
    IRType* localResultType = nullptr;
    if (isCUDAExport &&
        asNVVMSupportedLocalHelperValuePointerType(helper->getResultType(), &localResultType) &&
        asNVVMSupportedLocalSubstandardRecordType(localResultType))
        return _diagnoseUnsupportedIRType(
            codeGenContext,
            "exported substandard record helper reference result",
            helper->getResultType());
    if (isCUDAExport && asNVVMSupportedSubstandardRecordType(helper->getResultType()))
        return _diagnoseUnsupportedIRType(
            codeGenContext,
            "exported substandard record helper result",
            helper->getResultType());
    if (isCUDAExport && isNVVMFloat8Type(helper->getResultType()))
        return _diagnoseUnsupportedIRType(
            codeGenContext,
            "exported FP8 helper result",
            helper->getResultType());
    if (isCUDAExport && asNVVMBFloat16VectorType(helper->getResultType()))
        return _diagnoseUnsupportedIRType(
            codeGenContext,
            "exported BF16 vector helper result",
            helper->getResultType());
    if (!_isSupportedNVVMHelperResultType(helper->getResultType()))
        return _diagnoseUnsupportedIRType(
            codeGenContext,
            "helper function result type",
            helper->getResultType());
    for (UInt parameterIndex = 0; parameterIndex < helper->getParamCount(); ++parameterIndex)
    {
        if (isCUDAExport &&
            asNVVMSupportedLayoutTransportPointerType(helper->getParamType(parameterIndex)))
            return _diagnoseUnsupportedIRType(
                codeGenContext,
                "exported layout pointer helper parameter",
                helper->getParamType(parameterIndex));
        if (isCUDAExport &&
            (asNVVMSupportedLocalSubstandardRecordArrayType(helper->getParamType(parameterIndex)) ||
             asNVVMSupportedLocalRecordArrayReferenceType(helper->getParamType(parameterIndex))))
            return _diagnoseUnsupportedIRType(
                codeGenContext,
                "exported substandard record array helper parameter",
                helper->getParamType(parameterIndex));
        if (isCUDAExport &&
            asNVVMSupportedSubstandardRecordType(helper->getParamType(parameterIndex)))
            return _diagnoseUnsupportedIRType(
                codeGenContext,
                "exported substandard record helper parameter",
                helper->getParamType(parameterIndex));
        if (isCUDAExport && isNVVMFloat8Type(helper->getParamType(parameterIndex)))
            return _diagnoseUnsupportedIRType(
                codeGenContext,
                "exported FP8 helper parameter",
                helper->getParamType(parameterIndex));
        if (isCUDAExport && asNVVMBFloat16VectorType(helper->getParamType(parameterIndex)))
            return _diagnoseUnsupportedIRType(
                codeGenContext,
                "exported BF16 vector helper parameter",
                helper->getParamType(parameterIndex));
        IRType* localValueType = nullptr;
        if (isCUDAExport &&
            asNVVMSupportedLocalHelperValuePointerType(
                helper->getParamType(parameterIndex),
                &localValueType) &&
            (asNVVMBFloat16VectorType(localValueType) ||
             asNVVMSupportedLocalSubstandardRecordType(localValueType)))
        {
            return _diagnoseUnsupportedIRType(
                codeGenContext,
                asNVVMBFloat16VectorType(localValueType) ? "exported BF16 vector helper reference"
                : asNVVMSupportedLocalBFloat16RecordType(localValueType)
                    ? "exported BF16 record helper reference"
                    : "exported substandard record helper reference",
                helper->getParamType(parameterIndex));
        }
        if (!_isSupportedNVVMHelperParameterType(helper->getParamType(parameterIndex)))
        {
            return _diagnoseUnsupportedIRType(
                codeGenContext,
                "helper function parameter",
                helper->getParamType(parameterIndex));
        }
    }
    return SLANG_OK;
}

// Visits the exact direct-call graph and records each reachable function once in preorder.
SlangResult _visitNVVMFunction(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    IRFunc* entryPoint,
    IRFunc* function,
    List<IRFunc*>& functions,
    HashSet<IRFunc*>& functionSet,
    HashSet<IRFunc*>& activeFunctions,
    HashSet<IRFunc*>& completedFunctions)
{
    if (completedFunctions.contains(function))
        return SLANG_OK;
    if (!activeFunctions.add(function))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("recursive function call"));
    if (functionSet.add(function))
        functions.add(function);

    for (auto block : function->getBlocks())
    {
        for (auto inst : block->getOrdinaryInsts())
        {
            auto call = as<IRCall>(inst);
            if (!call)
                continue;
            if (!call->getOperandCount())
                return _diagnoseUnsupportedIR(codeGenContext, toSlice("call"));

            auto helper = as<IRFunc>(call->getOperand(0));
            SLANG_RETURN_ON_FAIL(
                _validateNVVMHelperTarget(codeGenContext, linkedIR, entryPoint, helper));
            if (activeFunctions.contains(helper))
                return _diagnoseUnsupportedIR(codeGenContext, toSlice("recursive function call"));
            SLANG_RETURN_ON_FAIL(_visitNVVMFunction(
                codeGenContext,
                linkedIR,
                entryPoint,
                helper,
                functions,
                functionSet,
                activeFunctions,
                completedFunctions));
        }
    }

    activeFunctions.remove(function);
    completedFunctions.add(function);
    return SLANG_OK;
}

// Collects the finite direct-call closure rooted at the sole selected entry point.
SlangResult _collectNVVMFunctions(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    IRFunc* entryPoint,
    List<IRFunc*>& functions,
    HashSet<IRFunc*>& functionSet)
{
    HashSet<IRFunc*> activeFunctions;
    HashSet<IRFunc*> completedFunctions;
    return _visitNVVMFunction(
        codeGenContext,
        linkedIR,
        entryPoint,
        entryPoint,
        functions,
        functionSet,
        activeFunctions,
        completedFunctions);
}

// Checks that function values remain direct callees rather than becoming first-class data.
SlangResult _validateNVVMFunctionUses(
    CodeGenContext* codeGenContext,
    const List<IRFunc*>& functions)
{
    for (auto function : functions)
    {
        for (auto use = function->firstUse; use; use = use->nextUse)
        {
            auto call = as<IRCall>(use->getUser());
            if (!call || use != call->getCalleeUse())
                return _diagnoseUnsupportedIR(codeGenContext, toSlice("function value use"));
        }
    }
    return SLANG_OK;
}

// Returns the physical symbol for one canonical module-scope groupshared producer. User globals
// normally carry a mangled name. A synthesized atomic global may be anonymous, so derive a stable
// private name from its order among the exact shared producers admitted by this backend.
String _getNVVMSharedGlobalName(IRModule* module, IRGlobalVar* target)
{
    if (!module || !target)
        return String();
    const UnownedStringSlice mangledName = getMangledName(target);
    if (mangledName.getLength())
        return String(mangledName);

    Index sharedIndex = 0;
    for (auto globalInst : module->getGlobalInsts())
    {
        NVVMSharedGlobal sharedGlobal;
        if (!getNVVMSupportedSharedGlobal(globalInst, &sharedGlobal))
            continue;
        auto globalVar = sharedGlobal.globalVar;
        if (globalVar == target)
        {
            StringBuilder name;
            name << "__slang_nvvm_shared_" << sharedIndex;
            return name.produceString();
        }
        ++sharedIndex;
    }
    return String();
}

// Chooses a distinct physical symbol for every emitted function before provider discovery.
//
// Linked user functions already carry an entry-point, CUDA export, or mangled name. Generated
// legalization helpers are intentionally anonymous, so give only those helpers a deterministic
// private name derived from their position in the reachable call closure.
SlangResult _collectNVVMFunctionNames(
    CodeGenContext* codeGenContext,
    IRModule* module,
    IRFunc* entryPoint,
    const List<IRFunc*>& functions,
    List<String>& outFunctionNames)
{
    outFunctionNames.clear();
    HashSet<String> names;
    for (auto function : functions)
    {
        String name = _getNVVMFunctionName(function, entryPoint);
        if (name.getLength() && !names.add(String(name)))
        {
            StringBuilder construct;
            construct << "duplicate function name: " << name;
            return _diagnoseUnsupportedIR(codeGenContext, construct.getUnownedSlice());
        }
    }
    for (auto globalInst : module->getGlobalInsts())
    {
        NVVMConventionalGlobalParams globalParams;
        if (_getNVVMConventionalGlobalParams(globalInst, globalParams))
        {
            if (!names.add(String("SLANG_globalParams")))
                return _diagnoseUnsupportedIR(codeGenContext, toSlice("global storage name"));
            continue;
        }
        NVVMSharedGlobal sharedGlobal;
        if (!getNVVMSupportedSharedGlobal(globalInst, &sharedGlobal))
            continue;
        auto globalVar = sharedGlobal.globalVar;
        const String name = _getNVVMSharedGlobalName(module, globalVar);
        if (!name.getLength() || !names.add(name))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("global storage name"));
    }

    Index anonymousIndex = 0;
    for (auto function : functions)
    {
        String canonicalName = _getNVVMFunctionName(function, entryPoint);
        if (canonicalName.getLength())
        {
            outFunctionNames.add(String(canonicalName));
            continue;
        }

        String generatedName;
        do
        {
            StringBuilder nameBuilder;
            nameBuilder << "__slang_nvvm_internal_" << anonymousIndex++;
            generatedName = nameBuilder.produceString();
        } while (!names.add(generatedName));
        outFunctionNames.add(_Move(generatedName));
    }
    return SLANG_OK;
}

// Plans one local allocation from its canonical producer and retains the layout proof. Consider
// `struct R { uint16_t tag; vector<BFloat16, 3> value; }; R local;`: the record is semantic IR,
// but its local role requires component-array field storage. A successful value-type lookup
// cannot establish that role, so admission and CUDA layout are checked before recording it.
SlangResult _planNVVMLocalStorage(
    CodeGenContext* codeGenContext,
    IRInst* inst,
    NVVMPlannedLocalStorage& outStorage)
{
    outStorage = {};
    outStorage.source = inst;
    if (auto pointer = asNVVMHitObjectPointerType(inst->getDataType()))
    {
        outStorage.valueType = pointer->getValueType();
        outStorage.valueUse = NVVMTypeUse::Storage;
        outStorage.alignment = 1;
        return SLANG_OK;
    }

    IRStructType* physicalStorageType = nullptr;
    if (asNVVMSupportedLocalPhysicalStoragePointerType(inst->getDataType(), &physicalStorageType))
    {
        IRSizeAndAlignment physicalLayout;
        if (!_getNVVMAggregateStorageLayout(codeGenContext, physicalStorageType, physicalLayout) ||
            physicalLayout.size <= 0 || physicalLayout.alignment <= 0)
        {
            return _diagnoseUnsupportedIR(
                codeGenContext,
                toSlice("local physical parameter-group storage layout"));
        }
        outStorage.valueType = physicalStorageType;
        outStorage.valueUse = NVVMTypeUse::ParameterGroupStorage;
        SLANG_RELEASE_ASSERT(physicalLayout.alignment <= UINT32_MAX);
        outStorage.alignment = uint32_t(physicalLayout.alignment);
        return SLANG_OK;
    }

    if (asNVVMSupportedLocalCopyableValuePointerType(inst->getDataType(), &outStorage.valueType))
    {
        outStorage.alignment = _getNVVMExecutableValueAlignment(outStorage.valueType);
    }
    else if (
        asNVVMSupportedLocalHelperValuePointerType(inst->getDataType(), &outStorage.valueType) ||
        _getNVVMLocalSubstandardRecordArrayPointer(inst))
    {
        if (!outStorage.valueType)
            outStorage.valueType = cast<IRPtrTypeBase>(inst->getDataType())->getValueType();
        uint32_t count = 0;
        if (asNVVMBFloat16VectorType(outStorage.valueType, &count))
        {
            IRSizeAndAlignment cudaLayout;
            const uint32_t alignment = _getNVVMBFloat16VectorStorageAlignment(outStorage.valueType);
            if (SLANG_FAILED(getSizeAndAlignment(
                    codeGenContext->getTargetReq(),
                    IRTypeLayoutRules::getCUDA(),
                    outStorage.valueType,
                    &cudaLayout)) ||
                cudaLayout.size != count * 2 || cudaLayout.alignment != alignment)
            {
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    toSlice("local BF16 vector storage layout"));
            }
            outStorage.valueUse = NVVMTypeUse::Storage;
            outStorage.alignment = alignment;
        }
        else if (
            asNVVMSupportedLocalSubstandardRecordType(outStorage.valueType) ||
            asNVVMSupportedLocalSubstandardRecordArrayType(outStorage.valueType))
        {
            if (!_hasNVVMCompatibleAggregateStorageLayout(
                    codeGenContext,
                    outStorage.valueType,
                    nullptr,
                    false,
                    true))
            {
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    toSlice("local substandard record storage layout"));
            }
            IRSizeAndAlignment physicalLayout;
            SLANG_RELEASE_ASSERT(_getNVVMAggregateStorageLayout(
                codeGenContext,
                outStorage.valueType,
                physicalLayout,
                nullptr,
                false,
                true));
            SLANG_RELEASE_ASSERT(
                physicalLayout.alignment > 0 && physicalLayout.alignment <= UINT32_MAX);
            outStorage.valueUse = NVVMTypeUse::Storage;
            outStorage.alignment = uint32_t(physicalLayout.alignment);
        }
        else
        {
            if (!_hasNVVMCompatibleHelperValueLayout(codeGenContext, outStorage.valueType))
            {
                return _diagnoseUnsupportedIR(codeGenContext, toSlice("local helper-value layout"));
            }
            outStorage.alignment = _getNVVMExecutableValueAlignment(outStorage.valueType);
        }
    }
    else
    {
        IRType* valueType = nullptr;
        if (!asNVVMSupportedLocalResourceValuePointerType(inst->getDataType(), &valueType))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("var"));
        if (!_hasNVVMCompatibleHelperValueLayout(codeGenContext, valueType))
        {
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("local resource-struct layout"));
        }
        outStorage.valueType = valueType;
        outStorage.alignment = _getNVVMExecutableValueAlignment(valueType);
    }
    SLANG_RELEASE_ASSERT(outStorage.alignment);
    return SLANG_OK;
}

// Chooses a BF16 memory conversion only after the address producer has proved the local role.
// BF2 is already a native vector in both roles; BF3/BF4 transport the same bits in lane arrays.
NVVMPlannedStorageConversion _planNVVMBFloat16StorageConversion(
    IRVectorType* type,
    NVVMTypeUse resultUse)
{
    uint32_t count = 0;
    SLANG_RELEASE_ASSERT(asNVVMBFloat16VectorType(type, &count));
    NVVMPlannedStorageConversion conversion;
    if (count > 2)
    {
        conversion.kind = NVVMStorageConversionKind::BFloat16Vector;
        conversion.type = type;
        conversion.laneCount = count;
        conversion.resultUse = resultUse;
    }
    return conversion;
}

// Reuses selected address provenance when planning memory operations. Consider a borrowed
// Payload whose float3 field is read-only: the selection permits a read but carries no compact
// parameter-group role. The same semantic field in a ParameterBlock has that independent role.
// Canonical IR still owns pointee/address-space identity; this is a view of checked address facts.
struct NVVMMemoryAddress
{
    IRInst* root = nullptr;
    IRType* structuredStorageType = nullptr;
    IRVectorType* localBFloat16Vector = nullptr;
    bool hasUnloweredCompactVector = false;
    bool isConventionalGlobal = false;
    bool isParameterGroupStorage = false;
};

NVVMMemoryAddress _getNVVMMemoryAddress(const NVVMEmissionPlan& plan, IRInst* pointer)
{
    NVVMMemoryAddress address;
    address.root = plan.addresses.getRoot(pointer);
    IRType* localValueType = nullptr;
    if (asNVVMSupportedLocalHelperValuePointerType(pointer->getDataType(), &localValueType))
        address.localBFloat16Vector = asNVVMBFloat16VectorType(localValueType);
    bool hasCompactStorage = false;
    if (pointer->getOp() == kIROp_FieldAddress)
    {
        const auto field = plan.addresses.findFieldAddress(pointer);
        SLANG_RELEASE_ASSERT(field);
        address.isConventionalGlobal = field->selection.isConventionalGlobal;
        if (!address.localBFloat16Vector && field->selection.isLocalSubstandardRecordStorage)
            address.localBFloat16Vector =
                asNVVMBFloat16VectorType(field->selection.field->getFieldType());
        hasCompactStorage =
            (field->selection.isParameterGroupStorage || field->selection.isConventionalGlobal) &&
            !field->selection.isMutable;
    }
    else if (pointer->getOp() == kIROp_GetElementPtr)
    {
        const auto element = plan.addresses.findElementAddress(pointer);
        SLANG_RELEASE_ASSERT(element);
        hasCompactStorage = element->kind == NVVMElementAddressKind::Sequential &&
                            element->isParameterGroupStorage && element->isReadOnly &&
                            asNVVMSupportedAggregateStorageArrayType(element->aggregateType);
    }
    address.isParameterGroupStorage = hasCompactStorage;
    if (hasCompactStorage)
    {
        auto pointerType = cast<IRPtrTypeBase>(pointer->getDataType());
        address.hasUnloweredCompactVector =
            asNVVMSupportedCompactParameterGroupVectorType(pointerType->getValueType()) != nullptr;
    }
    address.structuredStorageType =
        _getNVVMStructuredBufferStoragePointerValueType(plan.addresses, pointer);
    return address;
}

// Address-only pointers originate at a current-function formal, a proved parameter-group load,
// or a checked offset. Internal formals are conditional roots: every direct call validates its
// actual producer before accepting an exact type match, before any provider mutation.
IRInst* _getNVVMLayoutPointerRoot(
    const NVVMEmissionPlan& plan,
    IRFunc* entryPoint,
    IRFunc* function,
    IRInst* value)
{
    if (!value || !asNVVMSupportedLayoutTransportPointerType(value->getDataType()))
        return nullptr;
    if (auto parameter = as<IRParam>(value))
    {
        if (parameter->getParent() != function->getFirstBlock())
            return nullptr;
        if (function != entryPoint &&
            (!function->isDefinition() || function->findDecoration<IREntryPointDecoration>() ||
             function->findDecoration<IRCudaKernelDecoration>() ||
             function->findDecorationImpl(kIROp_CudaDeviceExportDecoration)))
            return nullptr;
        return parameter;
    }
    if (const auto load = _findPlannedNVVMOperation(plan.loads, value))
        return load->isLayoutPointerRoot && value->getParent()->getParent() == function ? value
                                                                                        : nullptr;
    if (auto offset = plan.layoutPointerOffsets.tryGetValue(value))
        return offset->root->getParent()->getParent() == function ? offset->root : nullptr;
    return nullptr;
}

// Selects scalar lanes using the same layout rules that own record stride and field offsets.
// For example, CDataLayout stores Bool in one byte, while Scalar/Std430 store four bytes;
// Float3 always has three four-byte payload lanes even when its field alignment is sixteen.
bool _getNVVMLayoutStorage(
    CodeGenContext* context,
    IRPtrTypeBase* pointerType,
    NVVMLayoutStorage& storage)
{
    storage = {};
    IRType* type = pointerType->getValueType();
    IRType* scalar = type;
    uint32_t lanes = 1;
    if (auto vector = as<IRVectorType>(type))
    {
        if (!asNVVMSupported32BitNumericVectorType(type, &lanes))
            return false;
        scalar = vector->getElementType();
    }
    if (!isNVVMBoolType(scalar) && !isNVVMFloat32Type(scalar) &&
        !(isNVVMSupportedIntegerScalarType(scalar) &&
          (_getNVVMExecutableValueAlignment(scalar) == 4 ||
           _getNVVMExecutableValueAlignment(scalar) == 8)))
        return false;
    auto rules = getTypeLayoutRuleForBuffer(context->getTargetProgram(), pointerType);
    IRSizeAndAlignment layout;
    if (SLANG_FAILED(getSizeAndAlignment(context->getTargetReq(), rules, scalar, &layout)) ||
        layout.size <= 0 || layout.size > 8 || layout.alignment <= 0 ||
        layout.alignment > layout.size ||
        (isNVVMBoolType(scalar) && layout.size != 1 && layout.size != 4))
        return false;
    storage.valueType = type;
    storage.scalarType = scalar;
    storage.scalarSize = uint32_t(layout.size);
    storage.alignment = uint32_t(layout.alignment);
    storage.laneCount = lanes;
    return true;
}

// Proves one canonical field from an admitted root or an already checked parent selection.
// Retain actual field keys and shared-rule offsets; LLVM's native struct layout is not the ABI.
bool _planNVVMLayoutField(
    CodeGenContext* context,
    const NVVMEmissionPlan& plan,
    IRFunc* entryPoint,
    IRFunc* function,
    IRFieldAddress* field,
    NVVMPlannedFieldAddress& selected)
{
    auto baseType = as<IRPtrTypeBase>(field->getBase()->getDataType());
    auto resultType = as<IRPtrTypeBase>(field->getDataType());
    auto record = baseType ? as<IRStructType>(baseType->getValueType()) : nullptr;
    const auto parent = plan.addresses.findFieldAddress(field->getBase());
    IRInst* root = parent && parent->isLayoutStorage
                       ? parent->root
                       : _getNVVMLayoutPointerRoot(plan, entryPoint, function, field->getBase());
    if (!root || !record || !resultType || resultType->getOp() != kIROp_PtrType ||
        resultType->getOperandCount() != 4 ||
        resultType->getAddressSpace() != baseType->getAddressSpace() ||
        resultType->getAccessQualifier() != baseType->getAccessQualifier() ||
        resultType->getDataLayout() != baseType->getDataLayout() ||
        !_findNVVMStructField(
            record,
            field->getField(),
            selected.selection.field,
            selected.selection.fieldIndex) ||
        !isTypeEqual(selected.selection.field->getFieldType(), resultType->getValueType()))
        return false;
    IRIntegerValue offset = 0;
    auto rules = getTypeLayoutRuleForBuffer(context->getTargetProgram(), baseType);
    if (SLANG_FAILED(
            getOffset(context->getTargetReq(), rules, selected.selection.field, &offset)) ||
        offset < 0)
        return false;
    if (!asNVVMSupportedCopyableStructType(resultType->getValueType()) &&
        !_getNVVMLayoutStorage(context, resultType, selected.layoutStorage))
        return false;
    selected.source = field;
    selected.base = field->getBase();
    selected.root = root;
    selected.isLayoutStorage = true;
    selected.byteOffset = uint64_t(offset);
    selected.selection.isMutable = baseType->getAccessQualifier() == AccessQualifier::ReadWrite;
    return true;
}

// Registers Bool conversion at the selected physical width before capability preflight.
void _requireNVVMLayoutBoolConversion(
    NVVMOperationRequirements& requirements,
    const NVVMLayoutStorage& storage,
    bool load,
    NVVMValueRecipeStep& step)
{
    if (!isNVVMBoolType(storage.scalarType))
        return;
    const SlangNVVMValueTypeDesc physical = {
        SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
        storage.scalarSize * 8,
        1};
    step.operation = load ? SLANG_NVVM_VALUE_OP_NOT_EQUAL : SLANG_NVVM_VALUE_OP_INTEGER_CONVERT;
    step.resultType = load ? NVVMSemantics::kBool : physical;
    step.operandTypes[0] = load ? physical : NVVMSemantics::kBool;
    step.operandTypes[1] = physical;
    step.operandCount = load ? 2 : 1;
    step.diagnosticName = "layout field Boolean conversion";
    _requireValueOperation(requirements.valueOperations, step.getDesc(), step.diagnosticName);
}

// Resolves a physical space from already checked producers. A default `Ptr<int>` kernel
// parameter is global, but the same type in a helper can carry a local address. Preserve that
// distinction; field/element records and planned pointer loads already own their source roles.
bool _getNVVMScopedPointerSpace(
    const NVVMEmissionPlan& plan,
    IRFunc* entryPoint,
    IRInst* pointer,
    SlangNVVMAddressSpace& outSpace,
    bool* outIsKnownLocalStorage = nullptr)
{
    if (outIsKnownLocalStorage)
        *outIsKnownLocalStorage = false;
    auto element = plan.addresses.findElementAddress(pointer);
    if (plan.addresses.findRootBuffer(pointer) ||
        (element && element->kind == NVVMElementAddressKind::RawBuffer))
    {
        outSpace = SLANG_NVVM_ADDRESS_SPACE_GLOBAL;
        return true;
    }
    IRInst* root = plan.addresses.getRoot(pointer);
    if (const auto space = plan.scopedPointerSpaces.tryGetValue(root))
    {
        outSpace = space->addressSpace;
        if (outIsKnownLocalStorage)
            *outIsKnownLocalStorage = space->isKnownLocalStorage;
        return true;
    }
    if (root->getOp() == kIROp_Var)
    {
        outSpace = SLANG_NVVM_ADDRESS_SPACE_GENERIC;
        if (outIsKnownLocalStorage)
            *outIsKnownLocalStorage = true;
        return true;
    }
    if (getNVVMSupportedSharedGlobal(root) ||
        asNVVMSupportedSharedHelperPointerType(root->getDataType()))
    {
        outSpace = SLANG_NVVM_ADDRESS_SPACE_SHARED;
        return true;
    }
    const bool isEntryParameter =
        as<IRParam>(root) && root->getParent() == entryPoint->getFirstBlock() &&
        asNVVMSupportedDeviceCopyableValuePointerType(root->getDataType());
    const auto load = _findPlannedNVVMOperation(plan.loads, root);
    if (isEntryParameter || (load && load->isGlobalUserPointer))
    {
        outSpace = SLANG_NVVM_ADDRESS_SPACE_GLOBAL;
        return true;
    }
    return false;
}

// Consumes the canonical memory attributes after ordinary pointer availability and permission
// checks. A coherent access has one exact scalar descriptor, not an ordinary-load fallback.
SlangResult _planNVVMScopedMemory(
    CodeGenContext* codeGenContext,
    const NVVMEmissionPlan& plan,
    IRFunc* entryPoint,
    IRInst* inst,
    IRInst* pointer,
    IRType* valueType,
    bool& outIsScoped,
    SlangNVVMMemoryOperationDesc& outOperation)
{
    outIsScoped = false;
    outOperation = {};
    auto scopeAttr = inst->findAttr<IRMemoryScopeAttr>();
    if (!scopeAttr)
        return SLANG_OK;
    // Source capability inference may warn and upgrade a requirement without changing the
    // selected downstream architecture. Scoped PTX accesses require the actual target to be SM70.
    if (!codeGenContext->getTargetCaps().implies(CapabilityAtom::_cuda_sm_7_0))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("scoped memory requires SM 7.0"));
    for (auto attr : inst->getAllAttrs())
    {
        if (!as<IRMemoryScopeAttr>(attr) && !as<IRAlignedAttr>(attr))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("scoped memory attributes"));
    }
    auto alignedAttr = inst->findAttr<IRAlignedAttr>();
    auto scope =
        scopeAttr->getOperandCount() == 1 ? as<IRIntLit>(scopeAttr->getMemoryScope()) : nullptr;
    auto alignment = alignedAttr && alignedAttr->getOperandCount() == 1
                         ? as<IRIntLit>(alignedAttr->getOperand(0))
                         : nullptr;
    if (inst->getAllAttrs().getCount() != 2 || !scope || !alignment || alignment->getValue() < 0 ||
        alignment->getValue() > kNVVMUInt32Max ||
        !_getNVVMSemanticType(valueType, outOperation.valueType) ||
        !_getNVVMScopedPointerSpace(plan, entryPoint, pointer, outOperation.addressSpace))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("scoped memory access"));
    switch (MemoryScope(scope->getValue()))
    {
    case MemoryScope::Device:
        outOperation.scope = SLANG_NVVM_MEMORY_SCOPE_DEVICE;
        break;
    case MemoryScope::Workgroup:
        outOperation.scope = SLANG_NVVM_MEMORY_SCOPE_WORKGROUP;
        break;
    default:
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("scoped memory access"));
    }
    outOperation.operation =
        inst->getOp() == kIROp_Load ? SLANG_NVVM_MEMORY_OP_LOAD : SLANG_NVVM_MEMORY_OP_STORE;
    outOperation.alignment = uint32_t(alignment->getValue());
    if (!NVVMSemantics::isSupported(outOperation))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("scoped memory access"));
    outIsScoped = true;
    return SLANG_OK;
}

// Plans the complete ordinary-load decision before any provider mutation. Access flags and
// physical representation are independent: `read(__constref Payload p) { return p.value; }`
// keeps a native float3, while the same semantic field in a parameter group is compact storage.
SlangResult _planNVVMLoad(
    CodeGenContext* codeGenContext,
    NVVMOperationRequirements& requirements,
    IRLoad* load,
    NVVMPlannedLoad& outLoad)
{
    outLoad = {};
    outLoad.source = load;
    outLoad.pointer = load->getPtr();
    if (auto storage = _findNVVMLayoutStorage(requirements.emissionPlan.addresses, load->getPtr()))
    {
        SLANG_RELEASE_ASSERT(storage->valueType);
        outLoad.layoutStorage = *storage;
        outLoad.alignment = storage->alignment;
        _requireNVVMLayoutBoolConversion(
            requirements,
            *storage,
            true,
            outLoad.layoutBoolConversion);
        return SLANG_OK;
    }
    if (asNVVMSupportedLayoutTransportPointerType(load->getDataType()))
    {
        // Consider `cbuffer Globals { LayoutPtr<Record, ScalarDataLayout> p; }`.
        // The loaded cbuffer and its selected field already have checked records. Only this
        // direct immutable field path grants the loaded address a root; matching types alone do
        // not.
        const auto field = requirements.emissionPlan.addresses.findFieldAddress(load->getPtr());
        const auto groupLoad =
            field ? _findPlannedNVVMOperation(requirements.emissionPlan.loads, field->base)
                  : nullptr;
        const auto groupField =
            groupLoad ? requirements.emissionPlan.addresses.findFieldAddress(groupLoad->pointer)
                      : nullptr;
        NVVMConventionalGlobalParams globals;
        outLoad.isLayoutPointerRoot = field && field->selection.isParameterGroupStorage &&
                                      !field->selection.isMutable && groupLoad &&
                                      as<IRConstantBufferType>(groupLoad->source->getDataType()) &&
                                      groupField && groupField->selection.isConventionalGlobal &&
                                      _getNVVMConventionalGlobalParams(groupField->base, globals);
    }
    const auto address = _getNVVMMemoryAddress(requirements.emissionPlan, load->getPtr());
    // Consider a ConstantBuffer record containing Tensor<float> fields. Its device pointer
    // leaves are AS1 in memory, but a whole-record load becomes an ordinary AS0 helper value.
    // Decode from the checked CUDA offsets rather than leaking storage types into SSA.
    if (address.isParameterGroupStorage && isNVVMSupportedHelperValueType(load->getDataType()) &&
        !isNVVMSupportedCopyableValueType(load->getDataType()) &&
        (as<IRStructType>(load->getDataType()) || as<IRArrayType>(load->getDataType())))
    {
        if (!getNVVMCUDAValueLayout(codeGenContext, load->getDataType(), outLoad.cudaValueLayout))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("parameter-group value layout"));
        _requireNVVMCUDAValueDecodeOperations(
            requirements.valueOperations,
            outLoad.cudaValueLayout);
        outLoad.alignment = outLoad.cudaValueLayout.alignment;
        return SLANG_OK;
    }
    IRType* storageType = address.structuredStorageType;
    auto localBFloat16Vector = address.localBFloat16Vector;
    const uint32_t physicalAlignment =
        _getNVVMPhysicalAggregateStorageAlignment(codeGenContext, load->getDataType());
    const uint32_t valueAlignment = _getNVVMExecutableValueAlignment(load->getDataType());
    // Shared buffer-element lowering must express compact group loads as ordinary IR over
    // physical fields. Reject a raw float3/half3/half4 load here: its storage array cannot be
    // used as a native vector value, even when both roles have already been cached.
    if (address.hasUnloweredCompactVector)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("unlowered compact group load"));
    outLoad.alignment = physicalAlignment;
    if (localBFloat16Vector)
    {
        outLoad.alignment = _getNVVMBFloat16VectorStorageAlignment(localBFloat16Vector);
        outLoad.conversion =
            _planNVVMBFloat16StorageConversion(localBFloat16Vector, NVVMTypeUse::Value);
    }
    if (!outLoad.alignment)
        outLoad.alignment = outLoad.isLayoutPointerRoot ? kNVVMPointerAlignment : valueAlignment;
    if (storageType)
    {
        outLoad.alignment = _getNVVMStructuredBufferMemoryAlignment(codeGenContext, storageType);
        SLANG_RELEASE_ASSERT(outLoad.alignment);
        outLoad.conversion.kind = NVVMStorageConversionKind::StructuredBuffer;
        outLoad.conversion.type = storageType;
        outLoad.conversion.structuredRecipe =
            _planNVVMStructuredBufferStorageConversion(requirements, storageType, true);
    }
    NVVMRawBufferType rawBufferType;
    NVVMSurfaceType surfaceType;
    NVVMReadOnlyTextureType sampledTextureType;
    if (getNVVMSupportedRawBufferType(load->getDataType(), rawBufferType) ||
        getNVVMSupportedSurfaceType(load->getDataType(), surfaceType) ||
        getNVVMSupportedReadOnlyTextureType(load->getDataType(), sampledTextureType) ||
        asNVVMSupportedSamplerValueType(load->getDataType()) ||
        asNVVMSupportedParameterGroupType(load->getDataType()))
    {
        outLoad.alignment = kNVVMPointerAlignment;
    }
    if (!storageType && !localBFloat16Vector && !physicalAlignment && !valueAlignment &&
        !asNVVMSupportedParameterGroupType(load->getDataType()) && !outLoad.isLayoutPointerRoot)
    {
        return _diagnoseUnsupportedIRType(codeGenContext, "load result type", load->getDataType());
    }
    SLANG_RELEASE_ASSERT(outLoad.alignment);
    IRType* parameterGroupElementType = nullptr;
    const bool isParameterGroupPointer = _getNVVMParameterGroupPointer(
        requirements.emissionPlan.addresses,
        load->getPtr(),
        parameterGroupElementType);
    outLoad.flags = outLoad.isLayoutPointerRoot || isParameterGroupPointer ||
                            isPointerToImmutableLocation(address.root)
                        ? SLANG_NVVM_LOAD_FLAG_INVARIANT
                        : SLANG_NVVM_LOAD_FLAG_NONE;
    if (isNVVMHitObjectStorageType(load->getDataType()))
        outLoad.flags = SLANG_NVVM_LOAD_FLAG_NONE;
    // Consider `ParameterBlock<Tensor> tensor`, where Tensor contains a device pointer.
    // Loading that field has the same AS1 representation as a pointer in collected globals.
    // Retain its provenance so passing it to a helper or storing it in a local descriptor uses
    // the existing AS1-to-AS0 conversion instead of leaking a storage pointer into a value slot.
    outLoad.isGlobalUserPointer =
        asNVVMSupportedDeviceCopyableValuePointerType(load->getDataType()) &&
        (address.isConventionalGlobal || address.isParameterGroupStorage);
    return SLANG_OK;
}

// Retains the exact store ABI and storage conversion selected from the admitted address root.
// A device pointer saved in local helper storage uses its helper representation; the source
// pointer's integer width alone cannot establish that provenance.
void _planNVVMStore(
    CodeGenContext* codeGenContext,
    NVVMOperationRequirements& requirements,
    IRStore* store,
    NVVMPlannedStore& outStore)
{
    outStore = {};
    outStore.source = store;
    outStore.pointer = store->getPtr();
    outStore.value = store->getVal();
    if (auto storage = _findNVVMLayoutStorage(requirements.emissionPlan.addresses, store->getPtr()))
    {
        SLANG_RELEASE_ASSERT(storage->valueType);
        outStore.layoutStorage = *storage;
        outStore.alignment = storage->alignment;
        _requireNVVMLayoutBoolConversion(
            requirements,
            *storage,
            false,
            outStore.layoutBoolConversion);
        return;
    }
    const auto address = _getNVVMMemoryAddress(requirements.emissionPlan, store->getPtr());
    IRType* storageType = address.structuredStorageType;
    IRInst* rootAddress = address.root;
    outStore.usesHelperPointerValue =
        asNVVMSupportedDeviceCopyableValuePointerType(store->getVal()->getDataType()) &&
        rootAddress && asNVVMSupportedLocalHelperValuePointerType(rootAddress->getDataType());
    auto localBFloat16Vector = address.localBFloat16Vector;
    if (localBFloat16Vector)
    {
        outStore.alignment = _getNVVMBFloat16VectorStorageAlignment(localBFloat16Vector);
        outStore.conversion =
            _planNVVMBFloat16StorageConversion(localBFloat16Vector, NVVMTypeUse::Storage);
    }
    else
    {
        outStore.alignment = _getNVVMPhysicalAggregateStorageAlignment(
            codeGenContext,
            store->getVal()->getDataType());
    }
    if (!outStore.alignment)
        outStore.alignment = _getNVVMExecutableValueAlignment(store->getVal()->getDataType());
    if (storageType)
    {
        outStore.alignment = _getNVVMStructuredBufferMemoryAlignment(codeGenContext, storageType);
        SLANG_RELEASE_ASSERT(outStore.alignment);
        outStore.conversion.kind = NVVMStorageConversionKind::StructuredBuffer;
        outStore.conversion.type = storageType;
        outStore.conversion.structuredRecipe =
            _planNVVMStructuredBufferStorageConversion(requirements, storageType, false);
    }
}

// Checks one function body using the same block and SSA order that emission will use.
SlangResult _validateNVVMFunction(
    CodeGenContext* codeGenContext,
    IRFunc* entryPoint,
    IRFunc* function,
    const HashSet<IRFunc*>& functionSet,
    NVVMOperationRequirements& requirements)
{
    const bool isEntryPoint = function == entryPoint;
    const auto selectedStage =
        entryPoint->findDecoration<IREntryPointDecoration>()->getProfile().getStage();
    const bool usesKernelParameters = isEntryPoint && selectedStage != Stage::Callable;
    const bool permitsHitObject = selectedStage == Stage::RayGeneration ||
                                  selectedStage == Stage::Miss ||
                                  selectedStage == Stage::ClosestHit;
    for (UInt i = 0; i < function->getParamCount(); ++i)
        if (asNVVMHitObjectPointerType(function->getParamType(i)) &&
            (!permitsHitObject || function->findDecorationImpl(kIROp_CudaDeviceExportDecoration)))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("HitObject reference stage"));
    if (!isEntryPoint)
    {
        if (hasNVVMHalfHelperABITransport(function->getResultType()))
            _requireNVVMHalfHelperABIOperations(
                requirements.valueOperations,
                function->getResultType());
        for (UInt parameterIndex = 0; parameterIndex < function->getParamCount(); ++parameterIndex)
        {
            IRType* parameterType = function->getParamType(parameterIndex);
            if (hasNVVMHalfHelperABITransport(parameterType))
                _requireNVVMHalfHelperABIOperations(requirements.valueOperations, parameterType);
        }
    }
    IRBlock* entryBlock = function->getFirstBlock();
    if (!entryBlock)
        return _diagnoseUnsupportedIR(
            codeGenContext,
            isEntryPoint ? toSlice("entry block") : toSlice("helper entry block"));

    HashSet<IRBlock*> functionBlocks;
    for (auto block : function->getBlocks())
        functionBlocks.add(block);
    RefPtr<IRDominatorTree> dominatorTree = computeDominatorTree(function);
    List<IRBlock*> bodyOrder = _getNVVMBodyOrder(function, dominatorTree);
    for (auto block : bodyOrder)
    {
        if (!functionBlocks.contains(block))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch target"));
    }

    HashSet<IRInst*> availableValues;
    UInt actualParamCount = 0;
    for (auto param : function->getParams())
    {
        const bool isSupportedType =
            usesKernelParameters ? isNVVMSupportedParameterType(param->getDataType())
                                 : _isSupportedNVVMHelperParameterType(param->getDataType());
        if (actualParamCount >= function->getParamCount() || !isSupportedType ||
            !isTypeEqual(param->getDataType(), function->getParamType(actualParamCount)))
        {
            return _diagnoseUnsupportedIR(
                codeGenContext,
                usesKernelParameters ? toSlice("entry-point parameter")
                                     : toSlice("helper function parameter"));
        }
        // Existing bare copyable pointers retain their AS1 execution role, including scoped
        // global memory. Newly admitted helper pointees need the executable AS0 decoder.
        if (usesKernelParameters &&
            (isNVVMSupportedCopyableValueType(param->getDataType()) ||
             asNVVMSupportedHelperStructType(param->getDataType()) ||
             asNVVMSupportedHelperArrayType(param->getDataType()) ||
             (asNVVMSupportedDeviceHelperValuePointerType(param->getDataType()) &&
              !asNVVMSupportedDeviceCopyableValuePointerType(param->getDataType()))))
        {
            NVVMCUDAValueLayout layout;
            if (!getNVVMCUDAValueLayout(codeGenContext, param->getDataType(), layout))
                return _diagnoseUnsupportedIR(codeGenContext, toSlice("numeric entry layout"));
            requirements.emissionPlan.entryValueParameters[param] = layout;
            _requireNVVMCUDAValueDecodeOperations(requirements.valueOperations, layout);
        }
        NVVMRawBufferType rawBufferType;
        if (usesKernelParameters &&
            getNVVMSupportedRawBufferType(param->getDataType(), rawBufferType) &&
            !_hasNVVMCompatibleRawBufferElementLayout(codeGenContext, param->getDataType()))
        {
            return _diagnoseUnsupportedIR(
                codeGenContext,
                toSlice("structured-buffer element layout"));
        }
        if (usesKernelParameters && isNVVMSupportedResourceAggregateType(param->getDataType()) &&
            !requirements.emissionPlan.entryValueParameters.containsKey(param))
        {
            uint32_t alignment = 0;
            if (!_getNVVMByValueParameterAlignment(codeGenContext, param->getDataType(), alignment))
            {
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    toSlice("entry-point parameter layout"));
            }
            if (!_hasNVVMCompatibleHelperValueLayout(codeGenContext, param->getDataType()))
            {
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    toSlice("entry-point parameter layout"));
            }
        }
        IRType* parameterGroupElementType = nullptr;
        if (usesKernelParameters &&
            asNVVMSupportedParameterGroupType(param->getDataType(), &parameterGroupElementType) &&
            !_hasNVVMCompatibleAggregateStorageLayout(
                codeGenContext,
                parameterGroupElementType,
                nullptr,
                true))
        {
            return _diagnoseUnsupportedIR(
                codeGenContext,
                toSlice("entry-point parameter-group layout"));
        }
        availableValues.add(param);
        ++actualParamCount;
    }
    if (actualParamCount != function->getParamCount())
    {
        return _diagnoseUnsupportedIR(
            codeGenContext,
            isEntryPoint ? toSlice("entry-point parameter count")
                         : toSlice("helper parameter count"));
    }
    // Register every accepted block parameter before checking uses because emission creates all
    // phi placeholders before any body. Ordinary values join this set in the second pass, in the
    // same order in which their LLVM instructions will be emitted.
    for (auto block : function->getBlocks())
    {
        if (block != entryBlock)
        {
            for (auto param : block->getParams())
            {
                // A conditional resource selection produces the same canonical block parameter as
                // any scalar join. For example, choosing between two elements of
                // `Texture2D textures[2]` passes one texture handle to the merge block. Admit the
                // complete established executable-value algebra here; generic LLVM phi emission
                // already preserves each selected provider representation.
                if (!_getNVVMExecutableValueAlignment(param->getDataType()) &&
                    !asNVVMBFloat16VectorType(param->getDataType()))
                {
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("basic-block parameter"));
                }
                availableValues.add(param);
            }
        }

        IRTerminatorInst* terminator = block->getTerminator();
        if (!terminator)
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("missing terminator"));

        for (auto inst : block->getOrdinaryInsts())
        {
            switch (inst->getOp())
            {
            case kIROp_Var:
                {
                    if (asNVVMHitObjectPointerType(inst->getDataType()) && !permitsHitObject)
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("HitObject storage stage"));
                    NVVMPlannedLocalStorage storage;
                    SLANG_RETURN_ON_FAIL(_planNVVMLocalStorage(codeGenContext, inst, storage));
                    requirements.emissionPlan.localStorage.add(storage);
                }
                break;

            case kIROp_LoadFromUninitializedMemory:
            case kIROp_GetStringHash:
            case kIROp_DebugNoScope:
                {
                    NVVMPlannedEphemeralValue value;
                    if (!_resolveNVVMEphemeralValue(inst, value))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            UnownedStringSlice(getIROpInfo(inst->getOp()).name));
                    }
                    requirements.emissionPlan.ephemeralValues.add(value);
                }
                break;

            case kIROp_OptixCallShader:
                {
                    const bool empty = inst->getDataType()->getOp() == kIROp_VoidType;
                    if ((selectedStage != Stage::RayGeneration && selectedStage != Stage::Miss &&
                         selectedStage != Stage::ClosestHit && selectedStage != Stage::Callable) ||
                        inst->getOperandCount() != (empty ? 1u : 2u) ||
                        inst->getOperand(0)->getDataType()->getOp() != kIROp_UIntType ||
                        (!empty &&
                         (!isNVVMSupportedCopyableValueType(inst->getDataType()) ||
                          !isTypeEqual(inst->getDataType(), inst->getOperand(1)->getDataType()))))
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("OptiX callable"));
                    requirements.emissionPlan.callables.add(
                        {inst, inst->getOperand(0), empty ? nullptr : inst->getOperand(1)});
                }
                break;

            case kIROp_AllocateOpaqueHandle:
            case kIROp_OptixHitObjectMakeNop:
            case kIROp_OptixHitObjectMakeMiss:
            case kIROp_OptixCurrentHitQuery:
            case kIROp_OptixHitObjectQuery:
            case kIROp_OptixHitObjectSetSbt:
            case kIROp_OptixHitObjectLoadSbt:
            case kIROp_OptixHitObjectReorder:
            case kIROp_OptixReorderHint:
            case kIROp_OptixHitObjectPayload:
                {
                    NVVMPlannedHitObjectOperation operation;
                    if (!_planNVVMHitObject(
                            inst,
                            entryPoint->findDecoration<IREntryPointDecoration>()
                                ->getProfile()
                                .getStage(),
                            operation))
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("OptiX HitObject operation"));
                    requirements.emissionPlan.hitObjectOperations.add(_Move(operation));
                }
                break;

            case kIROp_OptixCurrentTransformRow:
            case kIROp_OptixInstanceTransformRow:
                {
                    NVVMPlannedInstanceTransform transform;
                    if (!_planNVVMInstanceTransform(
                            inst,
                            entryPoint->findDecoration<IREntryPointDecoration>()
                                ->getProfile()
                                .getStage(),
                            transform))
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("OptiX transform row"));
                    requirements.emissionPlan.instanceTransforms.add(transform);
                }
                break;

            case kIROp_OptixTraceRayPayload:
                {
                    NVVMPlannedTraceRay trace;
                    if (!_planNVVMTraceRay(
                            inst,
                            entryPoint->findDecoration<IREntryPointDecoration>()
                                ->getProfile()
                                .getStage(),
                            trace))
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("OptiX register trace"));
                    requirements.emissionPlan.traceRays.add(_Move(trace));
                }
                break;

            case kIROp_GetOptiXPayloadRegister:
            case kIROp_SetOptiXPayloadRegister:
            case kIROp_GetOptiXHitAttribute:
                {
                    NVVMPlannedNamedIntrinsic primitive;
                    if (!_planNVVMOptixRegister(
                            inst,
                            entryPoint->findDecoration<IREntryPointDecoration>()
                                ->getProfile()
                                .getStage(),
                            primitive))
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("OptiX register primitive"));
                    requirements.emissionPlan.namedIntrinsics.add(_Move(primitive));
                }
                break;

            case kIROp_GetOptiXSbtDataPtr:
                {
                    IRType* storageType = nullptr;
                    if (!_isNVVMOptixStage(entryPoint->findDecoration<IREntryPointDecoration>()
                                               ->getProfile()
                                               .getStage()) ||
                        inst->getOperandCount() || !as<IRConstantBufferType>(inst->getDataType()) ||
                        !asNVVMSupportedParameterGroupType(inst->getDataType(), &storageType) ||
                        !_hasNVVMCompatibleAggregateStorageLayout(
                            codeGenContext,
                            storageType,
                            nullptr,
                            true))
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("OptiX SBT pointer"));
                    NVVMPlannedNamedIntrinsic primitive;
                    primitive.source = inst;
                    primitive.name = "_optix_get_sbt_data_ptr_64";
                    primitive.resultType = NVVMSemantics::kUnsignedI64;
                    requirements.emissionPlan.namedIntrinsics.add(_Move(primitive));
                }
                break;

            case kIROp_Load:
                break;

            case kIROp_Store:
                if ((inst->findAttr<IRMemoryScopeAttr>()
                         ? inst->getOperandCount() - inst->getAllAttrs().getCount()
                         : inst->getOperandCount()) != 2 ||
                    !inst->getOperand(0))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("store"));
                break;

            case kIROp_SwizzledStore:
                {
                    NVVMVectorSwizzledStore store;
                    if (!_getNVVMVectorSwizzledStore(inst, store))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("RWStructuredBuffer numeric vector swizzled store"));
                    }
                }
                break;

            case kIROp_NVVMSurfaceLoad:
            case kIROp_NVVMSurfaceStore:
                {
                    NVVMPlannedSurfaceOperation surfaceOperation;
                    if (!_resolveNVVMPhysicalSurfaceOperation(inst, surfaceOperation))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("physical surface operation"));
                    }
                    _requireSurfaceOperation(
                        requirements.surfaceOperations,
                        inst,
                        surfaceOperation.desc,
                        surfaceOperation.desc.operation == SLANG_NVVM_SURFACE_OP_LOAD
                            ? "physical surface load"
                            : "physical surface store");
                    requirements.emissionPlan.surfaceOperations.add(surfaceOperation);
                }
                break;

            case kIROp_MakeUInt64:
                {
                    NVVMPlannedUInt64WordConstruction construction;
                    if (!_resolveNVVMUInt64WordConstruction(inst, construction))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("canonical UInt64 word construction"));
                    }
                    _requireNVVMUInt64WordConstructionOperations(
                        requirements.valueOperations,
                        construction);
                    requirements.emissionPlan.uint64WordConstructions.add(construction);
                }
                break;

            case kIROp_DefaultConstruct:
                {
                    NVVMPlannedDefaultResourceValue defaultValue;
                    if (!_resolveNVVMDefaultResourceValue(inst, defaultValue))
                    {
                        return _diagnoseUnsupportedIRType(
                            codeGenContext,
                            "default construct type",
                            inst->getDataType());
                    }
                    requirements.emissionPlan.defaultResourceValues.add(defaultValue);
                }
                break;

            case kIROp_Add:
            case kIROp_Sub:
            case kIROp_Mul:
            case kIROp_Fma:
            case kIROp_Div:
            case kIROp_IRem:
            case kIROp_Lsh:
            case kIROp_Rsh:
            case kIROp_BitAnd:
            case kIROp_BitOr:
            case kIROp_BitXor:
            case kIROp_BitNot:
            case kIROp_And:
            case kIROp_Or:
            case kIROp_Not:
            case kIROp_Neg:
            case kIROp_IntCast:
            case kIROp_CastIntToFloat:
            case kIROp_CastFloatToInt:
            case kIROp_FloatCast:
            case kIROp_Select:
                {
                    NVVMPlannedNumericTruthiness truthiness;
                    if (_resolveNVVMNumericTruthiness(inst, truthiness))
                    {
                        _requireNVVMNumericTruthinessOperations(
                            requirements.valueOperations,
                            truthiness);
                        requirements.emissionPlan.numericTruthinessOperations.add(truthiness);
                        break;
                    }
                    NVVMResolvedValueOperation operation;
                    if (!_resolveNVVMValueOperation(inst, operation))
                        return inst->getOperandCount() == 2
                                   ? _diagnoseUnsupportedIRTypeRelation(
                                         codeGenContext,
                                         getIROpInfo(inst->getOp()).name,
                                         inst->getOperand(0)->getDataType(),
                                         inst->getOperand(1)->getDataType())
                                   : _diagnoseUnsupportedIR(
                                         codeGenContext,
                                         UnownedStringSlice(getIROpInfo(inst->getOp()).name));
                    _planNVVMValueOperation(requirements, inst, operation);
                }
                break;

            case kIROp_FRem:
                {
                    NVVMPlannedFloatingRemainder operation;
                    if (!_resolveNVVMFloatingRemainderOperation(inst, operation))
                    {
                        return _diagnoseUnsupportedIRTypeRelation(
                            codeGenContext,
                            getIROpInfo(inst->getOp()).name,
                            inst->getOperand(0)->getDataType(),
                            inst->getOperand(1)->getDataType());
                    }
                    _requireValueOperation(
                        requirements.valueOperations,
                        operation.scalarStep.getDesc(),
                        operation.scalarStep.diagnosticName);
                    requirements.requiresCUDADeviceLibrary = true;
                    requirements.emissionPlan.floatingRemainderOperations.add(operation);
                }
                break;

            case kIROp_CombinedTextureSamplerGetSampler:
            case kIROp_CastDescriptorHandleToResource:
            case kIROp_CastResourceToDescriptorHandle:
            case kIROp_CastUInt64ToDescriptorHandle:
            case kIROp_CastDescriptorHandleToUInt64:
                {
                    IRInst* value = nullptr;
                    if (!_getNVVMDescriptorHandleConversion(inst, value))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            UnownedStringSlice(getIROpInfo(inst->getOp()).name));
                    }
                }
                break;

            case kIROp_PtrCast:
                {
                    // __getAddress retains its declared pointer value type. This is an
                    // exact-pointee qualification conversion, never a layout reinterpretation.
                    if (inst->getOperandCount() != 1)
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("pointer qualification arity"));
                    auto result = asNVVMSupportedDeviceHelperValuePointerType(inst->getDataType());
                    if (!result)
                        result = asNVVMSupportedSharedHelperPointerType(inst->getDataType());
                    if (!result)
                        result = asNVVMSupportedLocalCopyableValuePointerType(inst->getDataType());
                    auto source = as<IRPtrTypeBase>(inst->getOperand(0)->getDataType());
                    if (!result || !source ||
                        !isTypeEqual(result->getValueType(), source->getValueType()))
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("pointer qualification"));
                }
                break;

            case kIROp_CastIntToPtr:
            case kIROp_BitCast:
                {
                    NVVMPlannedResourceBitCast resourceBitCast;
                    if (_resolveNVVMResourceBitCast(inst, resourceBitCast))
                    {
                        _requireNVVMResourceBitCastOperations(
                            requirements.valueOperations,
                            resourceBitCast);
                        requirements.emissionPlan.resourceBitCasts.add(resourceBitCast);
                        break;
                    }
                    NVVMPointerConversion pointerCast;
                    if (_getNVVMPointerConversion(inst, pointerCast))
                        break;
                    NVVMResolvedValueOperation operation;
                    if (!_resolveNVVMValueOperation(inst, operation))
                        return _diagnoseUnsupportedIRTypeRelation(
                            codeGenContext,
                            "bitCast type",
                            inst->getOperand(0)->getDataType(),
                            inst->getDataType());
                    _planNVVMValueOperation(requirements, inst, operation);
                }
                break;

            case kIROp_AtomicLoad:
            case kIROp_AtomicStore:
            case kIROp_AtomicExchange:
            case kIROp_AtomicCompareExchange:
            case kIROp_AtomicAdd:
            case kIROp_AtomicSub:
            case kIROp_AtomicAnd:
            case kIROp_AtomicOr:
            case kIROp_AtomicXor:
            case kIROp_AtomicMin:
            case kIROp_AtomicMax:
            case kIROp_AtomicInc:
            case kIROp_AtomicDec:
                {
                    NVVMPlannedAtomicOperation operation;
                    if (!_resolveNVVMAtomicOperation(inst, operation))
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            UnownedStringSlice(getIROpInfo(inst->getOp()).name));
                    requirements.emissionPlan.atomicOperations.add(operation);
                }
                break;

            case kIROp_Less:
            case kIROp_Eql:
            case kIROp_Neq:
            case kIROp_Greater:
            case kIROp_Leq:
            case kIROp_Geq:
                {
                    NVVMResolvedValueOperation operation;
                    if (!_resolveNVVMValueOperation(inst, operation))
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            UnownedStringSlice(getIROpInfo(inst->getOp()).name));
                    _planNVVMValueOperation(requirements, inst, operation);
                }
                break;

            case kIROp_Call:
                {
                    auto call = as<IRCall>(inst);
                    auto callee =
                        call && call->getOperandCount() ? as<IRFunc>(call->getOperand(0)) : nullptr;
                    if (!callee || !_isSupportedNVVMHelperResultType(inst->getDataType()))
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("value call"));
                }
                break;

            case kIROp_MakeVector:
            case kIROp_MakeVectorFromScalar:
            case kIROp_MakeArray:
            case kIROp_MakeArrayFromElement:
            case kIROp_MakeStruct:
            case kIROp_Swizzle:
            case kIROp_SwizzleSet:
            case kIROp_GetElement:
                {
                    NVVMSequentialElement element;
                    NVVMVectorConstruction construction;
                    NVVMAggregateElement aggregateElement;
                    NVVMAggregateConstruction aggregateConstruction;
                    if (!_getNVVMSequentialElement(inst, element) &&
                        !_getNVVMVectorConstruction(inst, construction) &&
                        !_getNVVMAggregateElement(inst, aggregateElement) &&
                        !_getNVVMAggregateConstruction(inst, aggregateConstruction))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            UnownedStringSlice(getIROpInfo(inst->getOp()).name));
                    }
                    if (aggregateConstruction.resultUse == NVVMTypeUse::Storage)
                    {
                        auto arrayType = cast<IRArrayType>(aggregateConstruction.resultType);
                        NVVMPlannedAggregateStorageConstruction storage;
                        storage.source = inst;
                        storage.elementRecipe = _planNVVMStructuredBufferStorageConversion(
                            requirements,
                            arrayType->getElementType(),
                            false);
                        requirements.emissionPlan.aggregateStorageConstructions.add(storage);
                    }
                }
                break;

            case kIROp_Sample:
            case kIROp_SampleLevel:
            case kIROp_TextureFetch:
            case kIROp_TextureGather:
            case kIROp_TextureQuerySize:
            case kIROp_TextureQueryLayerCount:
                {
                    NVVMTextureOperationRequirement operation;
                    if (!_resolveNVVMTextureOperation(inst, operation))
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            UnownedStringSlice(getIROpInfo(inst->getOp()).name));
                    requirements.textureOperations.add(operation);
                }
                break;

            case kIROp_GenericAsm:
                {
                    auto genericAsm = as<IRGenericAsm>(inst);
                    if (isEntryPoint || genericAsm != terminator ||
                        genericAsm->findDecoration<IRNVVMSemanticDecoration>())
                    {
                        return _diagnoseUnsupportedGenericAsm(codeGenContext, genericAsm, function);
                    }
                    NVVMPlannedNamedIntrinsic namedIntrinsic;
                    if (_getNVVMNamedIntrinsicDesc(genericAsm, function, namedIntrinsic))
                    {
                        if (namedIntrinsic.name.startsWith(toSlice("_optix_")) &&
                            !_isNVVMOptixPrimitiveStage(
                                namedIntrinsic.name.getUnownedSlice(),
                                entryPoint->findDecoration<IREntryPointDecoration>()
                                    ->getProfile()
                                    .getStage()))
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("OptiX primitive stage"));
                        requirements.requiresCUDADeviceLibrary |=
                            namedIntrinsic.isDeviceLibraryFunction;
                        requirements.emissionPlan.namedIntrinsics.add(_Move(namedIntrinsic));
                        break;
                    }
                    return _diagnoseUnsupportedGenericAsm(codeGenContext, genericAsm, function);
                }
                break;

            case kIROp_WaveGetConvergedMask:
            case kIROp_WaveMaskBallot:
            case kIROp_WaveMaskMatch:
                {
                    NVVMResolvedValueOperation operation;
                    if (!_resolveNVVMValueOperation(inst, operation) || !operation.staticEntry)
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            UnownedStringSlice(getIROpInfo(inst->getOp()).name));
                    }
                    _planNVVMValueOperation(requirements, inst, operation);
                }
                break;

            case kIROp_CastPtrToInt:
                if (inst->getOperandCount() != 1 || !_isNVVMAddressIntegerType(inst->getDataType()))
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("pointer address conversion"));
                break;

            case kIROp_GetOffsetPtr:
                if (inst->getOperandCount() != 2 ||
                    (!asNVVMSupportedDeviceNumericPointerType(inst->getDataType()) &&
                     !asNVVMSupportedDeviceHelperValuePointerType(inst->getDataType()) &&
                     !asNVVMSupportedLayoutTransportPointerType(inst->getDataType()) &&
                     !asNVVMSupportedSharedHelperPointerType(inst->getDataType()) &&
                     !asNVVMSupportedSharedElementPointerType(inst->getDataType())))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("selected pointer offset"));
                }
                break;

            case kIROp_GetElementPtr:
                if (inst->getOperandCount() != 2 || !as<IRPtrTypeBase>(inst->getDataType()))
                    return _diagnoseUnsupportedIRType(
                        codeGenContext,
                        "sequential element pointer",
                        inst->getDataType());
                break;

            case kIROp_GetStructuredBufferPtr:
            case kIROp_GetUntypedBufferPtr:
                break;

            case kIROp_RWStructuredBufferGetElementPtr:
                if (inst->getOperandCount() != 2 ||
                    !asNVVMSupportedRWStructuredBufferElementPointerType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("raw RWStructuredBuffer numeric element pointer"));
                }
                break;

            case kIROp_StructuredBufferLoad:
            case kIROp_RWStructuredBufferLoad:
                {
                    NVVMPlannedStructuredLoad load;
                    if (!_getNVVMStructuredBufferLoad(inst, load))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("raw structured-buffer value load"));
                    }
                    load.source = inst;
                    load.alignment =
                        _getNVVMStructuredBufferMemoryAlignment(codeGenContext, load.resultType);
                    SLANG_RELEASE_ASSERT(load.alignment);
                    if (isNVVMSupportedStructuredBufferStorageType(load.resultType))
                    {
                        load.conversion.kind = NVVMStorageConversionKind::StructuredBuffer;
                        load.conversion.structuredRecipe =
                            _planNVVMStructuredBufferStorageConversion(
                                requirements,
                                load.resultType,
                                true);
                    }
                    requirements.emissionPlan.structuredLoads.add(_Move(load));
                }
                break;

            case kIROp_StructuredBufferGetDimensions:
                {
                    NVVMStructuredBufferDimensions dimensions;
                    if (!_getNVVMStructuredBufferDimensions(codeGenContext, inst, dimensions))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("structured-buffer dimensions"));
                    }
                    _requireValueOperation(
                        requirements.valueOperations,
                        kNVVMRawBufferCountConversion,
                        "structured-buffer count conversion");
                }
                break;

            case kIROp_ByteAddressBufferLoad:
            case kIROp_ByteAddressBufferStore:
                {
                    NVVMByteAddressAccess access;
                    if (!_getNVVMByteAddressAccess(inst, access))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("core byte-address buffer access"));
                    }
                }
                break;

            case kIROp_GetEquivalentStructuredBuffer:
                {
                    NVVMEquivalentStructuredBuffer conversion;
                    IRSizeAndAlignment elementLayout;
                    if (!_getNVVMEquivalentStructuredBuffer(inst, conversion) ||
                        !_getNVVMStructuredBufferStorageLayout(
                            codeGenContext,
                            conversion.resultType.structuredElementType,
                            elementLayout) ||
                        elementLayout.size <= 0 || elementLayout.size > kNVVMUInt32Max)
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("equivalent structured-buffer view"));
                    }
                    requirements.emissionPlan.equivalentStructuredBuffers[inst] = {
                        conversion.buffer,
                        conversion.resultType.structuredElementType,
                        uint32_t(elementLayout.size)};
                    _requireValueOperation(
                        requirements.valueOperations,
                        kNVVMByteViewCountDivision,
                        "byte-view element count");
                }
                break;

            case kIROp_FieldExtract:
                {
                    NVVMStructFieldSelection field;
                    if (!_getNVVMStructFieldValue(as<IRFieldExtract>(inst), field))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("scalar struct field value"));
                    }
                }
                break;

            case kIROp_FieldAddress:
                break;

            case kIROp_Return:
            case kIROp_Unreachable:
                break;

            case kIROp_UnconditionalBranch:
            case kIROp_Loop:
            case kIROp_IfElse:
            case kIROp_Switch:
                break;

            default:
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    UnownedStringSlice(getIROpInfo(inst->getOp()).name));
            }
        }
    }

    bool hasHelperReturn = false;
    // Reachable reverse postorder puts every dominating ordinary producer before its consumer
    // without making physical sibling order part of legality. Unreachable blocks retain physical
    // order, and phi definitions are already available in every block.
    for (auto block : bodyOrder)
    {
        IRTerminatorInst* terminator = block->getTerminator();
        SLANG_ASSERT(terminator);

        for (auto inst : block->getOrdinaryInsts())
        {
            switch (inst->getOp())
            {
            case kIROp_OptixCallShader:
                {
                    const auto call =
                        _findPlannedNVVMOperation(requirements.emissionPlan.callables, inst);
                    SLANG_RELEASE_ASSERT(call);
                    SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                        codeGenContext,
                        call->index,
                        inst,
                        availableValues,
                        dominatorTree));
                    if (call->payload)
                    {
                        SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                            codeGenContext,
                            call->payload,
                            inst,
                            availableValues,
                            dominatorTree));
                        availableValues.add(inst);
                    }
                }
                break;

            case kIROp_AllocateOpaqueHandle:
            case kIROp_OptixHitObjectMakeNop:
            case kIROp_OptixHitObjectMakeMiss:
            case kIROp_OptixCurrentHitQuery:
            case kIROp_OptixHitObjectQuery:
            case kIROp_OptixHitObjectSetSbt:
            case kIROp_OptixHitObjectLoadSbt:
            case kIROp_OptixHitObjectReorder:
            case kIROp_OptixReorderHint:
            case kIROp_OptixHitObjectPayload:
                {
                    const auto operation = _findPlannedNVVMOperation(
                        requirements.emissionPlan.hitObjectOperations,
                        inst);
                    SLANG_RELEASE_ASSERT(operation);
                    for (auto value : operation->operands)
                    {
                        if (asNVVMHitObjectPointerType(value->getDataType()) &&
                            !_getNVVMHitObjectPointer(requirements.emissionPlan.addresses, value))
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("HitObject reference producer"));
                        SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                            codeGenContext,
                            value,
                            inst,
                            availableValues,
                            dominatorTree));
                    }
                    if (inst->getDataType()->getOp() != kIROp_VoidType)
                        availableValues.add(inst);
                }
                break;

            case kIROp_OptixCurrentTransformRow:
            case kIROp_OptixInstanceTransformRow:
                {
                    const auto transform = _findPlannedNVVMOperation(
                        requirements.emissionPlan.instanceTransforms,
                        inst);
                    SLANG_RELEASE_ASSERT(transform);
                    if (transform->handle)
                        SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                            codeGenContext,
                            transform->handle,
                            inst,
                            availableValues,
                            dominatorTree));
                    availableValues.add(inst);
                }
                break;

            case kIROp_OptixTraceRayPayload:
            case kIROp_GetOptiXPayloadRegister:
            case kIROp_SetOptiXPayloadRegister:
            case kIROp_GetOptiXHitAttribute:
                {
                    const auto trace =
                        _findPlannedNVVMOperation(requirements.emissionPlan.traceRays, inst);
                    const auto named =
                        _findPlannedNVVMOperation(requirements.emissionPlan.namedIntrinsics, inst);
                    SLANG_RELEASE_ASSERT(trace || named);
                    const auto& operands = trace ? trace->operands : named->operandValues;
                    for (auto value : operands)
                        SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                            codeGenContext,
                            value,
                            inst,
                            availableValues,
                            dominatorTree));
                    if (inst->getDataType()->getOp() != kIROp_VoidType)
                        availableValues.add(inst);
                }
                break;

            case kIROp_GetOptiXSbtDataPtr:
            case kIROp_Var:
                availableValues.add(inst);
                break;

            case kIROp_LoadFromUninitializedMemory:
            case kIROp_GetStringHash:
            case kIROp_DebugNoScope:
                {
                    const auto value =
                        _findPlannedNVVMOperation(requirements.emissionPlan.ephemeralValues, inst);
                    SLANG_RELEASE_ASSERT(value);
                    if (value->kind != NVVMPlannedEphemeralValueKind::IgnoredDebugNoScope)
                        availableValues.add(inst);
                }
                break;

            case kIROp_Load:
                {
                    auto load = cast<IRLoad>(inst);
                    SLANG_RETURN_ON_FAIL(_validatePointerValue(
                        codeGenContext,
                        requirements,
                        load->getPtr(),
                        load,
                        availableValues,
                        dominatorTree,
                        false,
                        load->getDataType()));
                    IRType* storageType = _getNVVMStructuredBufferStoragePointerValueType(
                        requirements.emissionPlan.addresses,
                        inst->getOperand(0));
                    if (storageType && !isTypeEqual(storageType, inst->getDataType()))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("structured-buffer load type"));
                    }
                    NVVMPlannedLoad plannedLoad;
                    SLANG_RETURN_ON_FAIL(
                        _planNVVMLoad(codeGenContext, requirements, load, plannedLoad));
                    SLANG_RETURN_ON_FAIL(_planNVVMScopedMemory(
                        codeGenContext,
                        requirements.emissionPlan,
                        entryPoint,
                        load,
                        load->getPtr(),
                        load->getDataType(),
                        plannedLoad.isScoped,
                        plannedLoad.memoryOperation));
                    if (plannedLoad.isScoped)
                        plannedLoad.flags = SLANG_NVVM_LOAD_FLAG_NONE;
                    requirements.emissionPlan.loads.add(plannedLoad);
                    availableValues.add(load);
                }
                break;

            case kIROp_Store:
                {
                    auto store = cast<IRStore>(inst);
                    SLANG_RETURN_ON_FAIL(_validatePointerValue(
                        codeGenContext,
                        requirements,
                        store->getPtr(),
                        store,
                        availableValues,
                        dominatorTree,
                        true,
                        store->getVal()->getDataType()));
                    SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                        codeGenContext,
                        store->getVal(),
                        store,
                        availableValues,
                        dominatorTree));
                    if (IRType* storageType = _getNVVMStructuredBufferStoragePointerValueType(
                            requirements.emissionPlan.addresses,
                            inst->getOperand(0)))
                    {
                        if (!isTypeEqual(storageType, inst->getOperand(1)->getDataType()))
                        {
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("structured-buffer store type"));
                        }
                    }
                    if (isPointerToImmutableLocation(
                            requirements.emissionPlan.addresses.getRoot(inst->getOperand(0))))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("store to immutable location"));
                    }
                    NVVMPlannedStore plannedStore;
                    _planNVVMStore(codeGenContext, requirements, store, plannedStore);
                    SLANG_RETURN_ON_FAIL(_planNVVMScopedMemory(
                        codeGenContext,
                        requirements.emissionPlan,
                        entryPoint,
                        store,
                        store->getPtr(),
                        store->getVal()->getDataType(),
                        plannedStore.isScoped,
                        plannedStore.memoryOperation));
                    requirements.emissionPlan.stores.add(plannedStore);
                }
                break;

            case kIROp_SwizzledStore:
                {
                    NVVMVectorSwizzledStore store;
                    SLANG_RELEASE_ASSERT(_getNVVMVectorSwizzledStore(inst, store));
                    SLANG_RETURN_ON_FAIL(_validatePointerValue(
                        codeGenContext,
                        requirements,
                        store.destination,
                        inst,
                        availableValues,
                        dominatorTree,
                        true,
                        store.destinationType));
                    SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                        codeGenContext,
                        store.source,
                        inst,
                        availableValues,
                        dominatorTree));
                }
                break;

            case kIROp_NVVMSurfaceLoad:
            case kIROp_NVVMSurfaceStore:
                {
                    const auto surfaceOperation = _findPlannedNVVMOperation(
                        requirements.emissionPlan.surfaceOperations,
                        inst);
                    SLANG_RELEASE_ASSERT(surfaceOperation);
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        surfaceOperation->surface,
                        inst,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                        codeGenContext,
                        surfaceOperation->coordinate,
                        inst,
                        availableValues,
                        dominatorTree));
                    if (surfaceOperation->value)
                    {
                        SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                            codeGenContext,
                            surfaceOperation->value,
                            inst,
                            availableValues,
                            dominatorTree));
                    }
                    else
                    {
                        availableValues.add(inst);
                    }
                }
                break;

            case kIROp_MakeUInt64:
                {
                    const auto construction = _findPlannedNVVMOperation(
                        requirements.emissionPlan.uint64WordConstructions,
                        inst);
                    SLANG_RELEASE_ASSERT(construction);
                    SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                        codeGenContext,
                        construction->lowWord,
                        inst,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                        codeGenContext,
                        construction->highWord,
                        inst,
                        availableValues,
                        dominatorTree));
                    availableValues.add(inst);
                }
                break;

            case kIROp_DefaultConstruct:
                {
                    SLANG_RELEASE_ASSERT(_findPlannedNVVMOperation(
                        requirements.emissionPlan.defaultResourceValues,
                        inst));
                    availableValues.add(inst);
                }
                break;

            case kIROp_Add:
            case kIROp_Sub:
            case kIROp_Mul:
            case kIROp_Fma:
            case kIROp_Div:
            case kIROp_IRem:
            case kIROp_FRem:
            case kIROp_Lsh:
            case kIROp_Rsh:
            case kIROp_BitAnd:
            case kIROp_BitOr:
            case kIROp_BitXor:
            case kIROp_BitNot:
            case kIROp_And:
            case kIROp_Or:
            case kIROp_Not:
            case kIROp_Neg:
            case kIROp_Less:
            case kIROp_Eql:
            case kIROp_Neq:
            case kIROp_Greater:
            case kIROp_Leq:
            case kIROp_Geq:
            case kIROp_IntCast:
            case kIROp_CastIntToFloat:
            case kIROp_CastFloatToInt:
            case kIROp_FloatCast:
            case kIROp_Select:
                {
                    if (inst->getOp() == kIROp_FRem)
                    {
                        SLANG_RELEASE_ASSERT(_findPlannedNVVMOperation(
                            requirements.emissionPlan.floatingRemainderOperations,
                            inst));
                    }
                    else
                    {
                        if (!_findPlannedNVVMOperation(
                                requirements.emissionPlan.numericTruthinessOperations,
                                inst))
                        {
                            SLANG_RELEASE_ASSERT(
                                _findPlannedNVVMValueOperation(requirements, inst));
                        }
                    }
                    for (UInt operandIndex = 0; operandIndex < inst->getOperandCount();
                         ++operandIndex)
                    {
                        SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                            codeGenContext,
                            inst->getOperand(operandIndex),
                            inst,
                            availableValues,
                            dominatorTree));
                    }
                    availableValues.add(inst);
                }
                break;

            case kIROp_CombinedTextureSamplerGetSampler:
            case kIROp_CastDescriptorHandleToResource:
            case kIROp_CastResourceToDescriptorHandle:
            case kIROp_CastUInt64ToDescriptorHandle:
            case kIROp_CastDescriptorHandleToUInt64:
                {
                    IRInst* value = nullptr;
                    SLANG_RELEASE_ASSERT(_getNVVMDescriptorHandleConversion(inst, value));
                    SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                        codeGenContext,
                        value,
                        inst,
                        availableValues,
                        dominatorTree));
                    availableValues.add(inst);
                }
                break;

            case kIROp_CastPtrToInt:
                {
                    auto value = inst->getOperand(0);
                    if (!_getNVVMLayoutPointerRoot(
                            requirements.emissionPlan,
                            entryPoint,
                            function,
                            value))
                    {
                        auto pointer =
                            asNVVMSupportedDeviceCopyableValuePointerType(value->getDataType());
                        if (!pointer)
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("pointer address producer"));
                        SLANG_RETURN_ON_FAIL(_validatePointerValue(
                            codeGenContext,
                            requirements,
                            value,
                            inst,
                            availableValues,
                            dominatorTree,
                            false,
                            pointer->getValueType()));
                    }
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        value,
                        inst,
                        availableValues,
                        dominatorTree));
                    requirements.emissionPlan.pointerToIntegerValues[inst] = value;
                    availableValues.add(inst);
                }
                break;

            case kIROp_PtrCast:
                {
                    auto value = inst->getOperand(0);
                    const bool resultIsShared =
                        asNVVMSupportedSharedHelperPointerType(inst->getDataType()) != nullptr;
                    const bool sourceIsShared =
                        _asNVVMSupportedSharedPointerValue(value) != nullptr;
                    if (resultIsShared != sourceIsShared || !_isSupportedNVVMHelperArgument(
                                                                codeGenContext,
                                                                requirements.emissionPlan.addresses,
                                                                value,
                                                                inst->getDataType()))
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("pointer qualification access"));
                    auto result = cast<IRPtrTypeBase>(inst->getDataType());
                    SLANG_RETURN_ON_FAIL(_validatePointerValue(
                        codeGenContext,
                        requirements,
                        value,
                        inst,
                        availableValues,
                        dominatorTree,
                        result->getAccessQualifier() == AccessQualifier::ReadWrite,
                        result->getValueType()));
                    requirements.emissionPlan.pointerQualificationValues[inst] = value;
                    NVVMPlannedPointerSpace space;
                    if (_getNVVMScopedPointerSpace(
                            requirements.emissionPlan,
                            entryPoint,
                            value,
                            space.addressSpace,
                            &space.isKnownLocalStorage))
                    {
                        // Qualification changes the physical pointer to AS0, except for shared
                        // pointers. Preserve allocation provenance independently of that change.
                        if (!resultIsShared)
                            space.addressSpace = SLANG_NVVM_ADDRESS_SPACE_GENERIC;
                        requirements.emissionPlan.scopedPointerSpaces[inst] = space;
                    }
                    availableValues.add(inst);
                }
                break;

            case kIROp_CastIntToPtr:
            case kIROp_BitCast:
                {
                    const auto resourceBitCast =
                        _findPlannedNVVMOperation(requirements.emissionPlan.resourceBitCasts, inst);
                    if (resourceBitCast)
                    {
                        SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                            codeGenContext,
                            resourceBitCast->value,
                            inst,
                            availableValues,
                            dominatorTree));
                        availableValues.add(inst);
                        break;
                    }
                    NVVMPointerConversion pointerCast;
                    if (_getNVVMPointerConversion(inst, pointerCast))
                    {
                        if (pointerCast.resultIsPointer)
                        {
                            SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                                codeGenContext,
                                pointerCast.value,
                                inst,
                                availableValues,
                                dominatorTree));
                        }
                        else
                        {
                            SLANG_RETURN_ON_FAIL(_validatePointerValue(
                                codeGenContext,
                                requirements,
                                pointerCast.value,
                                inst,
                                availableValues,
                                dominatorTree,
                                false,
                                pointerCast.pointerType->getValueType()));
                        }
                        availableValues.add(inst);
                        break;
                    }

                    SLANG_RELEASE_ASSERT(_findPlannedNVVMValueOperation(requirements, inst));
                    SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                        codeGenContext,
                        inst->getOperand(0),
                        inst,
                        availableValues,
                        dominatorTree));
                    availableValues.add(inst);
                }
                break;

            case kIROp_AtomicLoad:
            case kIROp_AtomicStore:
            case kIROp_AtomicExchange:
            case kIROp_AtomicCompareExchange:
            case kIROp_AtomicAdd:
            case kIROp_AtomicSub:
            case kIROp_AtomicAnd:
            case kIROp_AtomicOr:
            case kIROp_AtomicXor:
            case kIROp_AtomicMin:
            case kIROp_AtomicMax:
            case kIROp_AtomicInc:
            case kIROp_AtomicDec:
                {
                    const auto operation =
                        _findPlannedNVVMOperation(requirements.emissionPlan.atomicOperations, inst);
                    SLANG_RELEASE_ASSERT(operation);
                    SLANG_RETURN_ON_FAIL(_validatePointerValue(
                        codeGenContext,
                        requirements,
                        operation->pointer,
                        inst,
                        availableValues,
                        dominatorTree,
                        true,
                        cast<IRPtrTypeBase>(operation->pointer->getDataType())->getValueType()));
                    // Type admission precedes address planning. Complete the descriptor only after
                    // pointer availability/access checks, using the same physical-space facts as
                    // coherent loads/stores. A helper pointer can carry either global or shared
                    // memory.
                    SlangNVVMAddressSpace space = SLANG_NVVM_ADDRESS_SPACE_GENERIC;
                    bool isKnownLocalStorage = false;
                    _getNVVMScopedPointerSpace(
                        requirements.emissionPlan,
                        entryPoint,
                        operation->pointer,
                        space,
                        &isKnownLocalStorage);
                    operation->desc.addressSpace = space;
                    if (isKnownLocalStorage || !NVVMSemantics::isSupported(operation->desc))
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("atomic pointer address space"));
                    _requireNVVMAtomicOperations(requirements, *operation);
                    for (uint32_t i = 0; i < operation->valueCount; ++i)
                    {
                        SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                            codeGenContext,
                            operation->values[i],
                            inst,
                            availableValues,
                            dominatorTree));
                    }
                    if (inst->getOp() != kIROp_AtomicStore)
                        availableValues.add(inst);
                }
                break;

            case kIROp_Call:
                {
                    auto call = cast<IRCall>(inst);
                    auto callee = as<IRFunc>(call->getOperand(0));
                    if (!callee || callee == entryPoint || !functionSet.contains(callee) ||
                        !isTypeEqual(call->getDataType(), callee->getResultType()) ||
                        call->getArgCount() != callee->getParamCount())
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("direct scalar call"));
                    }
                    for (UInt argumentIndex = 0; argumentIndex < call->getArgCount();
                         ++argumentIndex)
                    {
                        IRInst* argument = call->getArg(argumentIndex);
                        auto parameterType = callee->getParamType(argumentIndex);
                        if (_findNVVMLayoutStorage(requirements.emissionPlan.addresses, argument))
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("layout field call argument"));
                        const bool isLayoutPointer =
                            asNVVMSupportedLayoutTransportPointerType(parameterType) != nullptr;
                        // A same-typed global or block parameter is not an admitted address root.
                        // Check every actual, including later calls to an already accepted helper.
                        if (isLayoutPointer &&
                            (!argument || argument->getDataType() != parameterType ||
                             !_getNVVMLayoutPointerRoot(
                                 requirements.emissionPlan,
                                 entryPoint,
                                 function,
                                 argument)))
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("layout pointer call argument producer"));
                        if (!_isSupportedNVVMHelperArgument(
                                codeGenContext,
                                requirements.emissionPlan.addresses,
                                argument,
                                callee->getParamType(argumentIndex)))
                        {
                            return argument ? _diagnoseUnsupportedIRTypeRelation(
                                                  codeGenContext,
                                                  "call argument type",
                                                  argument->getDataType(),
                                                  callee->getParamType(argumentIndex))
                                            : _diagnoseUnsupportedIR(
                                                  codeGenContext,
                                                  toSlice("call argument type"));
                        }
                        if (isLayoutPointer)
                        {
                            SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                                codeGenContext,
                                argument,
                                call,
                                availableValues,
                                dominatorTree));
                        }
                        else if (
                            _getNVVMLocalSubstandardRecordArrayPointer(argument) ||
                            _getNVVMLocalValueReference(
                                requirements.emissionPlan.addresses,
                                argument) ||
                            asNVVMSupportedLocalCopyableValuePointerType(argument->getDataType()) ||
                            asNVVMSupportedLocalHelperValuePointerType(argument->getDataType()) ||
                            asNVVMSupportedDeviceHelperValuePointerType(argument->getDataType()) ||
                            asNVVMSupportedRWStructuredBufferElementPointerType(
                                argument->getDataType()) ||
                            asNVVMSupportedHelperReferencePointerType(argument->getDataType()) ||
                            asNVVMSupportedPhysicalStorageReferencePointerType(
                                argument->getDataType()) ||
                            asNVVMSupportedLocalPhysicalStoragePointerType(
                                argument->getDataType()) ||
                            asNVVMSupportedSharedHelperPointerType(argument->getDataType()) ||
                            asNVVMSupportedSharedElementPointerType(argument->getDataType()) ||
                            asNVVMSupportedDerivedCopyableValuePointerType(argument->getDataType()))
                        {
                            SLANG_RETURN_ON_FAIL(_validatePointerValue(
                                codeGenContext,
                                requirements,
                                argument,
                                call,
                                availableValues,
                                dominatorTree,
                                false,
                                cast<IRPtrTypeBase>(argument->getDataType())->getValueType()));
                        }
                        else
                        {
                            SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                                codeGenContext,
                                argument,
                                call,
                                availableValues,
                                dominatorTree));
                        }
                    }
                    if (!as<IRVoidType>(call->getDataType()))
                        availableValues.add(call);
                }
                break;

            case kIROp_MakeVector:
            case kIROp_MakeVectorFromScalar:
            case kIROp_MakeArray:
            case kIROp_MakeArrayFromElement:
            case kIROp_MakeStruct:
            case kIROp_Swizzle:
            case kIROp_SwizzleSet:
            case kIROp_GetElement:
                {
                    NVVMSequentialElement element;
                    NVVMVectorConstruction construction;
                    NVVMAggregateElement aggregateElement;
                    NVVMAggregateConstruction aggregateConstruction;
                    if (_getNVVMSequentialElement(inst, element))
                    {
                        SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                            codeGenContext,
                            element.base,
                            inst,
                            availableValues,
                            dominatorTree));
                    }
                    else if (_getNVVMVectorConstruction(inst, construction))
                    {
                        for (uint32_t i = 0; i < construction.elementCount; ++i)
                        {
                            const NVVMVectorConstructElement& source = construction.elements[i];
                            if (source.value)
                            {
                                SLANG_RETURN_ON_FAIL(_validateScalarValue(
                                    codeGenContext,
                                    source.value,
                                    inst,
                                    availableValues,
                                    dominatorTree));
                            }
                            else
                            {
                                SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                                    codeGenContext,
                                    source.extractedBase,
                                    inst,
                                    availableValues,
                                    dominatorTree));
                            }
                        }
                    }
                    else if (_getNVVMAggregateElement(inst, aggregateElement))
                    {
                        SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                            codeGenContext,
                            aggregateElement.base,
                            inst,
                            availableValues,
                            dominatorTree));
                    }
                    else
                    {
                        SLANG_RELEASE_ASSERT(
                            _getNVVMAggregateConstruction(inst, aggregateConstruction));
                        for (uint32_t i = 0; i < aggregateConstruction.elementCount; ++i)
                        {
                            SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                                codeGenContext,
                                inst->getOperand(
                                    aggregateConstruction.repeatsSingleElement ? 0 : i),
                                inst,
                                availableValues,
                                dominatorTree));
                        }
                    }
                    availableValues.add(inst);
                }
                break;

            case kIROp_GenericAsm:
                {
                    const auto named =
                        _findPlannedNVVMOperation(requirements.emissionPlan.namedIntrinsics, inst);
                    SLANG_RELEASE_ASSERT(named);
                    for (Index i = 0; i < named->operandValues.getCount(); ++i)
                    {
                        IRInst* value = named->operandValues[i];
                        if (named->operands[i].kind ==
                            SLANG_NVVM_NAMED_INTRINSIC_OPERAND_OUT_POINTER)
                        {
                            SLANG_RETURN_ON_FAIL(_validatePointerValue(
                                codeGenContext,
                                requirements,
                                value,
                                inst,
                                availableValues,
                                dominatorTree,
                                true,
                                cast<IRPtrTypeBase>(value->getDataType())->getValueType()));
                        }
                        else
                        {
                            SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                                codeGenContext,
                                value,
                                inst,
                                availableValues,
                                dominatorTree));
                        }
                    }
                }
                SLANG_ASSERT(inst == terminator);
                hasHelperReturn = true;
                break;

            case kIROp_Sample:
            case kIROp_SampleLevel:
            case kIROp_TextureFetch:
            case kIROp_TextureGather:
            case kIROp_TextureQuerySize:
            case kIROp_TextureQueryLayerCount:
                for (UInt i = 0; i < inst->getOperandCount(); ++i)
                {
                    SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                        codeGenContext,
                        inst->getOperand(i),
                        inst,
                        availableValues,
                        dominatorTree));
                }
                availableValues.add(inst);
                break;

            case kIROp_WaveGetConvergedMask:
                availableValues.add(inst);
                break;

            case kIROp_WaveMaskBallot:
                SLANG_RETURN_ON_FAIL(_validateWaveMaskValue(
                    codeGenContext,
                    inst->getOperand(0),
                    inst,
                    availableValues,
                    dominatorTree));
                SLANG_RETURN_ON_FAIL(_validateBooleanValue(
                    codeGenContext,
                    inst->getOperand(1),
                    inst,
                    availableValues,
                    dominatorTree));
                availableValues.add(inst);
                break;

            case kIROp_WaveMaskMatch:
                SLANG_RETURN_ON_FAIL(_validateWaveMaskValue(
                    codeGenContext,
                    inst->getOperand(0),
                    inst,
                    availableValues,
                    dominatorTree));
                SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                    codeGenContext,
                    inst->getOperand(1),
                    inst,
                    availableValues,
                    dominatorTree));
                availableValues.add(inst);
                break;

            case kIROp_GetOffsetPtr:
                {
                    IRInst* basePointer = inst->getOperand(0);
                    IRInst* elementOffset = inst->getOperand(1);
                    if (auto layoutPointer =
                            asNVVMSupportedLayoutTransportPointerType(basePointer->getDataType()))
                    {
                        auto root = _getNVVMLayoutPointerRoot(
                            requirements.emissionPlan,
                            entryPoint,
                            function,
                            basePointer);
                        if (!root || inst->getDataType() != layoutPointer)
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("layout pointer offset producer"));
                        SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                            codeGenContext,
                            basePointer,
                            inst,
                            availableValues,
                            dominatorTree));
                        SLANG_RETURN_ON_FAIL(_validateI32Value(
                            codeGenContext,
                            elementOffset,
                            inst,
                            availableValues,
                            dominatorTree));
                        IRSizeAndAlignment layout;
                        auto rules = getTypeLayoutRuleForBuffer(
                            codeGenContext->getTargetProgram(),
                            layoutPointer);
                        // Every signed 32-bit index times this stride fits in signed 64 bits.
                        // Check the alignment addition before asking the layout for its stride.
                        if (SLANG_FAILED(getSizeAndAlignment(
                                codeGenContext->getTargetReq(),
                                rules,
                                layoutPointer->getValueType(),
                                &layout)) ||
                            layout.size <= 0 || layout.alignment <= 0 ||
                            layout.size > INT64_MAX - (layout.alignment - 1) ||
                            layout.getStride() <= 0 ||
                            uint64_t(layout.getStride()) > (uint64_t(1) << 32) - 1)
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("layout pointer stride"));
                        NVVMPlannedLayoutPointerOffset selected;
                        selected.base = basePointer;
                        selected.index = elementOffset;
                        selected.root = root;
                        selected.resultType = layoutPointer;
                        selected.stride = uint64_t(layout.getStride());
                        const SlangNVVMValueTypeDesc wide = {
                            SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
                            64,
                            1};
                        const SlangNVVMValueTypeDesc products[] = {wide, wide};
                        _setNVVMValueRecipeStep(
                            selected.widenIndex,
                            SLANG_NVVM_VALUE_OP_INTEGER_CONVERT,
                            wide,
                            &NVVMSemantics::kSignedI32,
                            1,
                            "layout pointer index extension");
                        _setNVVMValueRecipeStep(
                            selected.scaleIndex,
                            SLANG_NVVM_VALUE_OP_MULTIPLY,
                            wide,
                            products,
                            2,
                            "layout pointer byte offset");
                        _requireValueOperation(
                            requirements.valueOperations,
                            selected.widenIndex.getDesc(),
                            selected.widenIndex.diagnosticName);
                        _requireValueOperation(
                            requirements.valueOperations,
                            selected.scaleIndex.getDesc(),
                            selected.scaleIndex.diagnosticName);
                        requirements.emissionPlan.layoutPointerOffsets[inst] = selected;
                        availableValues.add(inst);
                        break;
                    }
                    auto basePointerType =
                        basePointer
                            ? asNVVMSupportedDeviceNumericPointerType(basePointer->getDataType())
                            : nullptr;
                    if (!basePointerType && basePointer)
                        basePointerType =
                            asNVVMSupportedDeviceHelperValuePointerType(basePointer->getDataType());
                    if (basePointerType && !_hasNVVMCompatibleHelperValueLayout(
                                               codeGenContext,
                                               basePointerType->getValueType()))
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("user pointer element layout"));
                    if (!basePointerType && basePointer)
                        basePointerType =
                            asNVVMSupportedSharedHelperPointerType(basePointer->getDataType());
                    if (!basePointerType && basePointer)
                        basePointerType =
                            asNVVMSupportedSharedElementPointerType(basePointer->getDataType());
                    if (!basePointerType ||
                        !isTypeEqual(inst->getDataType(), basePointer->getDataType()))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("pointer offset result type"));
                    }
                    SLANG_RETURN_ON_FAIL(_validatePointerValue(
                        codeGenContext,
                        requirements,
                        basePointer,
                        inst,
                        availableValues,
                        dominatorTree,
                        false,
                        basePointerType->getValueType()));
                    SLANG_RETURN_ON_FAIL(_validateInteger32Value(
                        codeGenContext,
                        elementOffset,
                        inst,
                        availableValues,
                        dominatorTree));
                    NVVMPlannedPointerSpace space;
                    if (_getNVVMScopedPointerSpace(
                            requirements.emissionPlan,
                            entryPoint,
                            basePointer,
                            space.addressSpace,
                            &space.isKnownLocalStorage))
                        requirements.emissionPlan.scopedPointerSpaces[inst] = space;
                    availableValues.add(inst);
                }
                break;

            case kIROp_GetElementPtr:
                {
                    NVVMPlannedElementAddress selected;
                    selected.source = inst;
                    selected.base = inst->getOperand(0);
                    selected.index = inst->getOperand(1);
                    selected.resultType = cast<IRPtrTypeBase>(inst->getDataType());
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        selected.base,
                        inst,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateInteger32Value(
                        codeGenContext,
                        selected.index,
                        inst,
                        availableValues,
                        dominatorTree));
                    if (auto storage = _findNVVMLayoutStorage(
                            requirements.emissionPlan.addresses,
                            selected.base))
                    {
                        auto baseType = as<IRPtrTypeBase>(selected.base->getDataType());
                        auto vector =
                            storage->valueType ? as<IRVectorType>(storage->valueType) : nullptr;
                        auto index = as<IRIntLit>(selected.index);
                        if (!vector || !index || index->getValue() < 0 ||
                            uint64_t(index->getValue()) >= storage->laneCount ||
                            selected.resultType->getOp() != kIROp_PtrType ||
                            selected.resultType->getOperandCount() != 4 ||
                            selected.resultType->getValueType() != vector->getElementType() ||
                            selected.resultType->getAddressSpace() != baseType->getAddressSpace() ||
                            selected.resultType->getAccessQualifier() !=
                                baseType->getAccessQualifier() ||
                            selected.resultType->getDataLayout() != baseType->getDataLayout() ||
                            !_getNVVMLayoutStorage(
                                codeGenContext,
                                selected.resultType,
                                selected.layoutStorage))
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("layout vector component address"));
                        selected.kind = NVVMElementAddressKind::Sequential;
                        selected.isLayoutStorage = true;
                        selected.byteOffset = uint64_t(index->getValue()) * storage->scalarSize;
                        selected.root = requirements.emissionPlan.addresses.getRoot(selected.base);
                        selected.aggregateType = vector;
                        selected.isReadOnly =
                            baseType->getAccessQualifier() != AccessQualifier::ReadWrite;
                        requirements.emissionPlan.addresses.addElementAddress(selected);
                        availableValues.add(inst);
                        break;
                    }
                    NVVMRawBufferElementPointer raw;
                    NVVMSequentialElementPointer sequential;
                    const bool isRaw = _getNVVMRawBufferElementPointer(
                        requirements.emissionPlan.addresses,
                        inst,
                        raw);
                    const bool isSequential = !isRaw && _getNVVMSequentialElementPointer(
                                                            requirements.emissionPlan.addresses,
                                                            inst,
                                                            sequential);
                    const bool hasDirectResultType =
                        asNVVMSupportedDevicePointerType(inst->getDataType()) ||
                        asNVVMSupportedSharedElementPointerType(inst->getDataType());
                    if (!hasDirectResultType && !isRaw && !isSequential)
                        return _diagnoseUnsupportedIRType(
                            codeGenContext,
                            "sequential element pointer",
                            inst->getDataType());
                    selected.kind = isRaw          ? NVVMElementAddressKind::RawBuffer
                                    : isSequential ? NVVMElementAddressKind::Sequential
                                                   : NVVMElementAddressKind::DeviceArray;
                    selected.aggregateType = sequential.aggregateType;
                    selected.isReadOnly = sequential.isImmutable;
                    selected.isParameterGroupStorage = sequential.isParameterGroupStorage;
                    selected.isLocalSubstandardRecordStorage =
                        sequential.isLocalSubstandardRecordStorage;
                    selected.propagatesGlobalUserPointer =
                        asNVVMSupportedDeviceCopyableValuePointerType(inst->getDataType()) !=
                        nullptr;
                    selected.root = requirements.emissionPlan.addresses.getRoot(selected.base);
                    if (auto buffer =
                            requirements.emissionPlan.addresses.findRootBuffer(selected.base))
                        selected.isReadOnly |= buffer->access == NVVMBufferAccess::ReadOnly;
                    selected.diagnosticName = getNVVMSupportedSharedGlobal(selected.base)
                                                  ? "shared aggregate element pointer"
                                              : isRaw        ? "raw buffer scalar element pointer"
                                              : isSequential ? "numeric sequential element pointer"
                                                             : "device i32 array element pointer";
                    requirements.emissionPlan.addresses.addElementAddress(selected);
                    const auto address =
                        requirements.emissionPlan.addresses.findElementAddress(inst);
                    IRInst* basePointer = address->base;
                    IRInst* elementIndex = address->index;
                    if (address->kind == NVVMElementAddressKind::RawBuffer)
                    {
                        SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                            codeGenContext,
                            basePointer,
                            inst,
                            availableValues,
                            dominatorTree));
                        SLANG_RETURN_ON_FAIL(_validateInteger32Value(
                            codeGenContext,
                            elementIndex,
                            inst,
                            availableValues,
                            dominatorTree));
                        availableValues.add(inst);
                        break;
                    }
                    if (address->kind == NVVMElementAddressKind::Sequential)
                    {
                        if (address->isReadOnly)
                        {
                            SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                                codeGenContext,
                                basePointer,
                                inst,
                                availableValues,
                                dominatorTree));
                        }
                        else
                        {
                            SLANG_RETURN_ON_FAIL(_validatePointerValue(
                                codeGenContext,
                                requirements,
                                basePointer,
                                inst,
                                availableValues,
                                dominatorTree,
                                false,
                                address->aggregateType));
                        }
                        SLANG_RETURN_ON_FAIL(_validateInteger32Value(
                            codeGenContext,
                            elementIndex,
                            inst,
                            availableValues,
                            dominatorTree));
                        availableValues.add(inst);
                        break;
                    }
                    IRArrayType* arrayType = nullptr;
                    auto basePointerType = basePointer ? asNVVMSupportedDeviceArrayPointerType(
                                                             basePointer->getDataType(),
                                                             &arrayType)
                                                       : nullptr;
                    auto resultPointerType = asNVVMSupportedDevicePointerType(inst->getDataType());
                    const bool isDeviceArrayElement =
                        basePointerType && resultPointerType && arrayType &&
                        basePointerType->getAddressSpace() ==
                            resultPointerType->getAddressSpace() &&
                        basePointerType->getAccessQualifier() ==
                            resultPointerType->getAccessQualifier() &&
                        isTypeEqual(arrayType->getElementType(), resultPointerType->getValueType());
                    if (!isDeviceArrayElement)
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("array element pointer relation"));
                    }
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        basePointer,
                        inst,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateInteger32Value(
                        codeGenContext,
                        elementIndex,
                        inst,
                        availableValues,
                        dominatorTree));
                    availableValues.add(inst);
                }
                break;

            case kIROp_GetStructuredBufferPtr:
            case kIROp_GetUntypedBufferPtr:
                {
                    NVVMRawBufferDataPointer dataPointer;
                    if (!_getNVVMRawBufferDataPointer(inst, dataPointer))
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("raw buffer data pointer"));
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        dataPointer.buffer,
                        inst,
                        availableValues,
                        dominatorTree));
                    requirements.emissionPlan.addresses.addDataPointer(dataPointer);
                    availableValues.add(inst);
                }
                break;

            case kIROp_FieldAddress:
                {
                    auto field = cast<IRFieldAddress>(inst);
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        field->getBase(),
                        inst,
                        availableValues,
                        dominatorTree));
                    NVVMPlannedFieldAddress address;
                    const auto parent =
                        requirements.emissionPlan.addresses.findFieldAddress(field->getBase());
                    // A derived ScalarLayout spelling does not create a new external-layout
                    // root. An existing typed parent owns its child's representation.
                    const auto parentElement =
                        requirements.emissionPlan.addresses.findElementAddress(field->getBase());
                    if ((!parent && !parentElement &&
                         asNVVMSupportedLayoutTransportPointerType(
                             field->getBase()->getDataType())) ||
                        (parent && parent->isLayoutStorage))
                    {
                        if (!_planNVVMLayoutField(
                                codeGenContext,
                                requirements.emissionPlan,
                                entryPoint,
                                function,
                                field,
                                address))
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("layout field address"));
                        requirements.emissionPlan.addresses.addFieldAddress(address);
                        availableValues.add(inst);
                        break;
                    }
                    if (!_getNVVMStructFieldAddress(
                            requirements.emissionPlan.addresses,
                            field,
                            address.selection))
                        return _diagnoseUnsupportedIRType(
                            codeGenContext,
                            "struct field address result",
                            inst->getDataType());
                    address.source = inst;
                    address.base = field->getBase();
                    address.root = requirements.emissionPlan.addresses.getRoot(address.base);
                    if (auto buffer =
                            requirements.emissionPlan.addresses.findRootBuffer(address.base))
                        address.selection.isMutable &=
                            buffer->access == NVVMBufferAccess::ReadWrite;
                    requirements.emissionPlan.addresses.addFieldAddress(address);
                    availableValues.add(inst);
                }
                break;

            case kIROp_FieldExtract:
                {
                    auto fieldExtract = cast<IRFieldExtract>(inst);
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        fieldExtract->getBase(),
                        inst,
                        availableValues,
                        dominatorTree));
                }
                availableValues.add(inst);
                break;

            case kIROp_StructuredBufferLoad:
            case kIROp_RWStructuredBufferLoad:
                {
                    const auto plannedLoad =
                        _findPlannedNVVMOperation(requirements.emissionPlan.structuredLoads, inst);
                    SLANG_RELEASE_ASSERT(plannedLoad);
                    const auto& load = *plannedLoad;
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        load.buffer,
                        inst,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateInteger32Value(
                        codeGenContext,
                        load.elementIndex,
                        inst,
                        availableValues,
                        dominatorTree));
                    availableValues.add(inst);
                }
                break;

            case kIROp_StructuredBufferGetDimensions:
                {
                    NVVMStructuredBufferDimensions dimensions;
                    if (!_getNVVMStructuredBufferDimensions(codeGenContext, inst, dimensions))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("structured-buffer dimensions relation"));
                    }
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        dimensions.buffer,
                        inst,
                        availableValues,
                        dominatorTree));
                    availableValues.add(inst);
                }
                break;

            case kIROp_ByteAddressBufferLoad:
            case kIROp_ByteAddressBufferStore:
                {
                    NVVMByteAddressAccess access;
                    SLANG_RELEASE_ASSERT(_getNVVMByteAddressAccess(inst, access));
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        access.buffer,
                        inst,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateUnsignedI32Value(
                        codeGenContext,
                        access.byteOffset,
                        inst,
                        availableValues,
                        dominatorTree,
                        toSlice("byte-address offset")));
                    if (access.isStore)
                    {
                        SLANG_RETURN_ON_FAIL(_validateByteAddressValue(
                            codeGenContext,
                            access.value,
                            inst,
                            availableValues,
                            dominatorTree));
                    }
                    else
                    {
                        availableValues.add(inst);
                    }
                }
                break;

            case kIROp_GetEquivalentStructuredBuffer:
                {
                    NVVMEquivalentStructuredBuffer conversion;
                    SLANG_RELEASE_ASSERT(_getNVVMEquivalentStructuredBuffer(inst, conversion));
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        conversion.buffer,
                        inst,
                        availableValues,
                        dominatorTree));
                    availableValues.add(inst);
                }
                break;

            case kIROp_RWStructuredBufferGetElementPtr:
                {
                    NVVMStructuredBufferElementPointer elementPointer;
                    if (!_getNVVMStructuredBufferElementPointer(inst, elementPointer))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("raw RWStructuredBuffer numeric relation"));
                    }
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        elementPointer.buffer,
                        inst,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateInteger32Value(
                        codeGenContext,
                        elementPointer.elementIndex,
                        inst,
                        availableValues,
                        dominatorTree));
                    requirements.emissionPlan.addresses.addStructuredElement(elementPointer);
                    availableValues.add(inst);
                }
                break;

            case kIROp_Return:
                {
                    auto returnInst = cast<IRReturn>(inst);
                    if (returnInst != terminator || !returnInst->getVal())
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("return value"));
                    if (isEntryPoint)
                    {
                        if (returnInst->getVal()->getOp() != kIROp_VoidLit)
                            return _diagnoseUnsupportedIR(codeGenContext, toSlice("return value"));
                    }
                    else
                    {
                        if (!isTypeEqual(
                                returnInst->getVal()->getDataType(),
                                function->getResultType()))
                        {
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("helper return type"));
                        }
                        if (as<IRVoidType>(function->getResultType()))
                        {
                            if (returnInst->getVal()->getOp() != kIROp_VoidLit)
                            {
                                return _diagnoseUnsupportedIR(
                                    codeGenContext,
                                    toSlice("void helper return"));
                            }
                        }
                        else
                        {
                            if (asNVVMSupportedLocalCopyableValuePointerType(
                                    function->getResultType()) ||
                                asNVVMSupportedLocalHelperValuePointerType(
                                    function->getResultType()) ||
                                asNVVMSupportedDeviceHelperValuePointerType(
                                    function->getResultType()))
                            {
                                SLANG_RETURN_ON_FAIL(_validatePointerValue(
                                    codeGenContext,
                                    requirements,
                                    returnInst->getVal(),
                                    returnInst,
                                    availableValues,
                                    dominatorTree,
                                    false,
                                    cast<IRPtrTypeBase>(function->getResultType())
                                        ->getValueType()));
                            }
                            else if (
                                as<IRStructType>(function->getResultType()) ||
                                as<IRArrayType>(function->getResultType()))
                            {
                                SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                                    codeGenContext,
                                    returnInst->getVal(),
                                    returnInst,
                                    availableValues,
                                    dominatorTree));
                            }
                            else
                            {
                                SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                                    codeGenContext,
                                    returnInst->getVal(),
                                    returnInst,
                                    availableValues,
                                    dominatorTree));
                            }
                        }
                        hasHelperReturn = true;
                    }
                }
                break;

            case kIROp_Unreachable:
                if (inst != terminator)
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("unreachable position"));
                break;

            case kIROp_UnconditionalBranch:
                {
                    auto branch = cast<IRUnconditionalBranch>(inst);
                    if (branch != terminator)
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch position"));
                    SLANG_RETURN_ON_FAIL(_validateBranchArguments(
                        codeGenContext,
                        branch,
                        entryBlock,
                        functionBlocks,
                        availableValues,
                        dominatorTree));
                }
                break;

            case kIROp_Loop:
                {
                    auto loop = cast<IRLoop>(inst);
                    if (loop != terminator)
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("loop position"));
                    SLANG_RETURN_ON_FAIL(_validateBranchArguments(
                        codeGenContext,
                        loop,
                        entryBlock,
                        functionBlocks,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        loop->getBreakBlock(),
                        functionBlocks));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        loop->getContinueBlock(),
                        functionBlocks));
                }
                break;

            case kIROp_IfElse:
                {
                    auto ifElse = cast<IRIfElse>(inst);
                    if (ifElse != terminator)
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("conditional branch position"));
                    }
                    if (!ifElse->getCondition() ||
                        !isNVVMBoolType(ifElse->getCondition()->getDataType()))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("conditional branch condition"));
                    }
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        ifElse->getCondition(),
                        ifElse,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        ifElse->getTrueBlock(),
                        functionBlocks));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        ifElse->getFalseBlock(),
                        functionBlocks));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        ifElse->getAfterBlock(),
                        functionBlocks));
                    if (ifElse->getTrueBlock()->getFirstParam() ||
                        ifElse->getFalseBlock()->getFirstParam())
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("conditional branch target parameter"));
                    }
                }
                break;

            case kIROp_Switch:
                {
                    auto switchInst = cast<IRSwitch>(inst);
                    if (switchInst != terminator)
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("switch position"));
                    if (!switchInst->getCondition() ||
                        !isNVVMSupportedIntegerScalarType(
                            switchInst->getCondition()->getDataType()))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("integer switch condition"));
                    }
                    SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                        codeGenContext,
                        switchInst->getCondition(),
                        switchInst,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        switchInst->getBreakLabel(),
                        functionBlocks));

                    IRBlock* defaultBlock = switchInst->getDefaultLabel();
                    if (!defaultBlock)
                        defaultBlock = switchInst->getBreakLabel();
                    SLANG_RETURN_ON_FAIL(
                        _validateBlockTarget(codeGenContext, defaultBlock, functionBlocks));
                    if (defaultBlock->getFirstParam())
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("switch default target parameter"));
                    }

                    for (UInt caseIndex = 0; caseIndex < switchInst->getCaseCount(); ++caseIndex)
                    {
                        auto caseValue = _asExecutableSelectedIntegerConstant(
                            switchInst->getCaseValue(caseIndex));
                        IRBlock* caseBlock = switchInst->getCaseLabel(caseIndex);
                        if (!caseValue || !isTypeEqual(
                                              caseValue->getDataType(),
                                              switchInst->getCondition()->getDataType()))
                        {
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("integer switch case value"));
                        }
                        SLANG_RETURN_ON_FAIL(
                            _validateBlockTarget(codeGenContext, caseBlock, functionBlocks));
                        if (caseBlock->getFirstParam())
                        {
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("switch case target parameter"));
                        }
                        for (UInt previousIndex = 0; previousIndex < caseIndex; ++previousIndex)
                        {
                            auto previousValue =
                                cast<IRIntLit>(switchInst->getCaseValue(previousIndex));
                            if (previousValue->getValue() == caseValue->getValue())
                            {
                                return _diagnoseUnsupportedIR(
                                    codeGenContext,
                                    toSlice("duplicate integer switch case"));
                            }
                        }
                    }
                }
                break;

            default:
                SLANG_UNEXPECTED("NVVM validation reached an unclassified instruction");
            }
        }
    }

    if (!isEntryPoint && !hasHelperReturn)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("helper return"));

    // Every non-entry phi needs at least one actual CFG predecessor. Structural `IRLoop`
    // break/continue and `IRIfElse::afterBlock` operands are deliberately absent from this list.
    for (auto block : function->getBlocks())
    {
        if (block == entryBlock || !block->getFirstParam())
            continue;

        auto predecessors = block->getPredecessors();
        if (predecessors.isEmpty())
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("basic-block predecessor"));
        for (auto predecessor : predecessors)
        {
            auto branch = as<IRUnconditionalBranch>(predecessor->getTerminator());
            if (!branch || branch->getTargetBlock() != block)
            {
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    toSlice("parameterized predecessor edge"));
            }
        }
    }
    return SLANG_OK;
}

using NVVMValueMap = Dictionary<IRInst*, SlangNVVMValueHandle>;
using NVVMGlobalUserPointerSet = HashSet<IRInst*>;

// Materializes the one concrete value selected for a canonical
// `LoadFromUninitializedMemory`. The IR contract permits this choice, and storing the completed
// handle in the SSA value map makes every use of the instruction observe the same value.
SlangResult _emitNVVMChosenUndefinedValue(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    IRType* type,
    NVVMTypeLoweringContext& typeContext,
    SlangNVVMValueHandle& outValue)
{
    outValue = nullptr;
    SLANG_RELEASE_ASSERT(
        isNVVMSupportedCopyableValueType(type) || asNVVMSupportedSamplerValueType(type));

    // Consider `SamplerState sampler; texture.SampleLevel(sampler, uv, 0);` on CUDA.
    // SSA construction produces an undefined sampler, but the texture object already owns its
    // sampling state. SampleLevel validates the sampler argument without passing it to the
    // provider. Select zero for its existing i64 placeholder, just as for numeric undefined
    // values; this does not create a texture handle or admit undefined resource aggregates.
    if (isNVVMSupportedIntegerScalarType(type) || isNVVMBoolType(type) ||
        asNVVMSupportedSamplerValueType(type))
    {
        SlangNVVMTypeHandle loweredType = nullptr;
        SLANG_RETURN_ON_FAIL(typeContext.lowerType(type, NVVMTypeUse::Value, loweredType));
        return _requireBuilderOperation(
            codeGenContext,
            "chosen undefined integer value",
            builder.getIntegerConstant(module, loweredType, 0, outValue));
    }

    uint32_t floatingPointBitWidth = 0;
    if (isNVVMSupportedFloatingPointScalarType(type, &floatingPointBitWidth))
    {
        SlangNVVMTypeHandle loweredType = nullptr;
        SLANG_RETURN_ON_FAIL(typeContext.lowerType(type, NVVMTypeUse::Value, loweredType));
        return _requireBuilderOperation(
            codeGenContext,
            "chosen undefined floating-point value",
            builder
                .getFloatingPointConstant(module, loweredType, floatingPointBitWidth, 0, outValue));
    }

    uint32_t elementCount = 0;
    IRType* repeatedElementType = nullptr;
    bool isVector = false;
    if (auto vectorType = asNVVMSupportedValueVectorType(type, &elementCount))
    {
        repeatedElementType = vectorType->getElementType();
        isVector = true;
    }
    else if (auto arrayType = asNVVMSupportedCopyableArrayType(type, &elementCount))
    {
        repeatedElementType = arrayType->getElementType();
    }

    List<SlangNVVMValueHandle> elements;
    if (repeatedElementType)
    {
        SlangNVVMValueHandle element = nullptr;
        SLANG_RETURN_ON_FAIL(_emitNVVMChosenUndefinedValue(
            codeGenContext,
            builder,
            module,
            repeatedElementType,
            typeContext,
            element));
        for (uint32_t index = 0; index < elementCount; ++index)
            elements.add(element);
    }
    else
    {
        auto structType = asNVVMSupportedCopyableStructType(type);
        SLANG_RELEASE_ASSERT(structType);
        for (auto field : structType->getFields())
        {
            SlangNVVMValueHandle element = nullptr;
            SLANG_RETURN_ON_FAIL(_emitNVVMChosenUndefinedValue(
                codeGenContext,
                builder,
                module,
                field->getFieldType(),
                typeContext,
                element));
            elements.add(element);
        }
    }

    SlangNVVMTypeHandle loweredType = nullptr;
    SLANG_RETURN_ON_FAIL(typeContext.lowerType(type, NVVMTypeUse::Value, loweredType));
    return _requireBuilderOperation(
        codeGenContext,
        isVector ? "chosen undefined vector value" : "chosen undefined aggregate value",
        isVector ? builder.emitVectorConstruct(
                       module,
                       loweredType,
                       elements.getBuffer(),
                       size_t(elements.getCount()),
                       outValue)
                 : builder.emitAggregateConstruct(
                       module,
                       loweredType,
                       elements.getBuffer(),
                       size_t(elements.getCount()),
                       outValue));
}

// Materializes one preflighted resource placeholder retained by optional-none lowering. The
// provider representation is the same exact value type used for non-default resource values.
SlangResult _emitNVVMDefaultResourceValue(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const NVVMPlannedDefaultResourceValue& defaultValue,
    NVVMTypeLoweringContext& typeContext,
    SlangNVVMValueHandle& outValue)
{
    outValue = nullptr;
    SLANG_RELEASE_ASSERT(defaultValue.resultType);

    if (defaultValue.kind == NVVMPlannedDefaultResourceValueKind::DescriptorHandle)
    {
        SlangNVVMTypeHandle handleType = nullptr;
        SLANG_RETURN_ON_FAIL(
            typeContext.lowerType(defaultValue.resultType, NVVMTypeUse::Value, handleType));
        return _requireBuilderOperation(
            codeGenContext,
            "default descriptor handle",
            builder.getIntegerConstant(module, handleType, 0, outValue));
    }

    SLANG_RELEASE_ASSERT(
        defaultValue.kind == NVVMPlannedDefaultResourceValueKind::RawStructuredBuffer &&
        defaultValue.structuredElementType);
    const NVVMTypeUse elementUse =
        isNVVMSupportedStructuredBufferStorageType(defaultValue.structuredElementType)
            ? NVVMTypeUse::StructuredBufferStorage
            : NVVMTypeUse::Value;
    SlangNVVMTypeHandle elementType = nullptr;
    SLANG_RETURN_ON_FAIL(
        typeContext.lowerType(defaultValue.structuredElementType, elementUse, elementType));
    SlangNVVMTypeHandle dataPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "default raw-buffer data-pointer type",
        builder.getPointerType(
            module,
            elementType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            dataPointerType)));
    SlangNVVMTypeHandle countType = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "default raw-buffer count type",
        builder.getIntegerType(module, 64, countType)));
    SlangNVVMValueHandle zero = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "default raw-buffer zero",
        builder.getIntegerConstant(module, countType, 0, zero)));
    SlangNVVMValueHandle nullDataPointer = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "default raw-buffer null data pointer",
        builder.emitBitCast(module, dataPointerType, zero, nullDataPointer)));
    SlangNVVMTypeHandle rawBufferType = nullptr;
    SLANG_RETURN_ON_FAIL(
        typeContext.lowerType(defaultValue.resultType, NVVMTypeUse::Value, rawBufferType));
    const SlangNVVMValueHandle elements[] = {nullDataPointer, zero};
    return _requireBuilderOperation(
        codeGenContext,
        "default raw-buffer view",
        builder.emitAggregateConstruct(
            module,
            rawBufferType,
            elements,
            SLANG_COUNT_OF(elements),
            outValue));
}

// Returns an already-lowered SSA value or materializes an exact preflighted scalar literal.
SlangResult _getLoweredNVVMValue(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    IRInst* irValue,
    NVVMValueMap& valueMap,
    NVVMTypeLoweringContext& typeContext,
    SlangNVVMValueHandle& outValue)
{
    outValue = nullptr;
    if (auto mappedValue = valueMap.tryGetValue(irValue))
    {
        outValue = *mappedValue;
        return SLANG_OK;
    }

    // Module aggregate constants are immutable value trees, not storage. Materialize each tree at
    // its use so every instruction remains owned by the current function. The shared value map may
    // still cache provider scalar constants, but caching an aggregate instruction there could make
    // a later function refer to SSA emitted in an earlier function.
    if (_isNVVMSupportedModuleConstantValue(irValue))
    {
        NVVMVectorConstruction vectorConstruction;
        if (_getNVVMVectorConstruction(irValue, vectorConstruction))
        {
            SlangNVVMValueHandle loweredElements[4] = {};
            for (uint32_t i = 0; i < vectorConstruction.elementCount; ++i)
            {
                SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                    codeGenContext,
                    builder,
                    module,
                    vectorConstruction.elements[i].value,
                    valueMap,
                    typeContext,
                    loweredElements[i]));
            }
            SlangNVVMTypeHandle loweredType = nullptr;
            SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                vectorConstruction.resultType,
                NVVMTypeUse::Value,
                loweredType));
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "module constant vector construction",
                builder.emitVectorConstruct(
                    module,
                    loweredType,
                    loweredElements,
                    vectorConstruction.elementCount,
                    outValue)));
            return SLANG_OK;
        }

        NVVMAggregateConstruction aggregateConstruction;
        if (_getNVVMAggregateConstruction(irValue, aggregateConstruction))
        {
            SLANG_RELEASE_ASSERT(aggregateConstruction.resultUse == NVVMTypeUse::Value);
            List<SlangNVVMValueHandle> loweredElements;
            for (uint32_t i = 0; i < aggregateConstruction.elementCount; ++i)
            {
                SlangNVVMValueHandle loweredElement = nullptr;
                SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                    codeGenContext,
                    builder,
                    module,
                    irValue->getOperand(aggregateConstruction.repeatsSingleElement ? 0 : i),
                    valueMap,
                    typeContext,
                    loweredElement));
                loweredElements.add(loweredElement);
            }
            SlangNVVMTypeHandle loweredType = nullptr;
            SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                aggregateConstruction.resultType,
                NVVMTypeUse::Value,
                loweredType));
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "module constant aggregate construction",
                builder.emitAggregateConstruct(
                    module,
                    loweredType,
                    loweredElements.getBuffer(),
                    size_t(loweredElements.getCount()),
                    outValue)));
            return SLANG_OK;
        }
    }

    if (auto intLit = _asExecutableSelectedIntegerConstant(irValue))
    {
        SlangNVVMTypeHandle integerType = nullptr;
        IRIntegerValue integerValue = intLit->getValue();
        uint32_t bitWidth = 0;
        bool isSigned = false;
        SLANG_RELEASE_ASSERT(
            isNVVMSupportedIntegerScalarType(intLit->getDataType(), &bitWidth, &isSigned));
        if (!isSigned && bitWidth < 64 && integerValue >= (IRIntegerValue(1) << (bitWidth - 1)))
            integerValue -= IRIntegerValue(1) << bitWidth;
        SLANG_RETURN_ON_FAIL(
            typeContext.lowerType(intLit->getDataType(), NVVMTypeUse::Value, integerType));
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "selected integer constant",
            builder.getIntegerConstant(module, integerType, int64_t(integerValue), outValue)));
        valueMap[irValue] = outValue;
        return SLANG_OK;
    }

    if (auto boolLit = _asExecutableBoolConstant(irValue))
    {
        SlangNVVMTypeHandle boolType = nullptr;
        SLANG_RETURN_ON_FAIL(
            typeContext.lowerType(boolLit->getDataType(), NVVMTypeUse::Value, boolType));
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "Boolean constant",
            builder.getIntegerConstant(module, boolType, boolLit->getValue() ? 1 : 0, outValue)));
        valueMap[irValue] = outValue;
        return SLANG_OK;
    }

    if (auto nullPointer = _asExecutableNullDevicePointer(irValue))
    {
        SlangNVVMTypeHandle int64Type = nullptr;
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "null UserPointer integer type",
            builder.getIntegerType(module, 64, int64Type)));
        SlangNVVMValueHandle zero = nullptr;
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "null UserPointer bit pattern",
            builder.getIntegerConstant(module, int64Type, 0, zero)));
        SlangNVVMTypeHandle pointerType = nullptr;
        SLANG_RETURN_ON_FAIL(
            typeContext.lowerType(nullPointer->getDataType(), NVVMTypeUse::Value, pointerType));
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "null UserPointer materialization",
            builder.emitBitCast(module, pointerType, zero, outValue)));
        valueMap[irValue] = outValue;
        return SLANG_OK;
    }

    auto floatLit = _asExecutableFloatingPointConstant(irValue);
    SLANG_RELEASE_ASSERT(floatLit);
    SlangNVVMTypeHandle floatingPointType = nullptr;
    SLANG_RETURN_ON_FAIL(
        typeContext.lowerType(floatLit->getDataType(), NVVMTypeUse::Value, floatingPointType));
    // IRBuilder::getFloatValue already rounded finite FP8 literals with the shared format
    // helpers. Recover those checked encodings exactly; no runtime narrowing or saturation
    // policy participates here. Nonfinite FP8 literals are rejected by operand preflight.
    if (isNVVMFloat8Type(floatLit->getDataType()))
    {
        const float value = float(floatLit->getValue());
        SLANG_RELEASE_ASSERT(!Math::IsNaN(value) && !Math::IsInf(value));
        const uint8_t bits = floatLit->getDataType()->getOp() == kIROp_FloatE4M3Type
                                 ? FloatToFloatE4M3(value)
                                 : FloatToFloatE5M2(value);
        // The integer-constant API takes a signed value in the destination width. For
        // example, negative zero's byte 0x80 must cross that API as -128, preserving its bits.
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "canonical finite FP8 constant bits",
            builder
                .getIntegerConstant(module, floatingPointType, bitCast<int8_t>(bits), outValue)));
        valueMap[irValue] = outValue;
        return SLANG_OK;
    }
    // IRBuilder::getFloatValue already rounded this canonical BF16 literal. Recover its
    // checked bits with the same core helper; dynamic Float32 narrowing has a separate
    // target operation and must not impose a new NaN policy on an existing literal.
    if (isNVVMBFloat16Type(floatLit->getDataType()))
    {
        // The builder takes a signed in-width value. For example, -1.25's bits 0xbfa0
        // must cross the i16 API as -16480 so the sign bit is preserved.
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "canonical BF16 constant bits",
            builder.getIntegerConstant(
                module,
                floatingPointType,
                bitCast<int16_t>(FloatToBFloat16(float(floatLit->getValue()))),
                outValue)));
        valueMap[irValue] = outValue;
        return SLANG_OK;
    }
    uint32_t bitWidth = 0;
    SLANG_RELEASE_ASSERT(
        isNVVMSupportedFloatingPointScalarType(floatLit->getDataType(), &bitWidth));
    const uint64_t bitPattern = bitWidth == 16 ? uint64_t(FloatToHalf(float(floatLit->getValue())))
                                : bitWidth == 32
                                    ? uint64_t(uint32_t(FloatAsInt(float(floatLit->getValue()))))
                                    : uint64_t(DoubleAsInt64(floatLit->getValue()));
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        bitWidth == 16   ? "float16 constant"
        : bitWidth == 32 ? "float32 constant"
                         : "float64 constant",
        builder
            .getFloatingPointConstant(module, floatingPointType, bitWidth, bitPattern, outValue)));
    valueMap[irValue] = outValue;
    return SLANG_OK;
}

// Returns the physical helper representation of one selected value. Most values already have one
// representation. An exact UserPointer is provenance-sensitive: kernel parameters and pointers
// loaded from conventional globals or parameter-group storage stay AS1 for ordinary memory
// operations, then widen to AS0 when a helper value boundary must also admit local addresses.
SlangResult _getLoweredNVVMHelperValue(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    IRInst* irValue,
    NVVMValueMap& valueMap,
    const NVVMGlobalUserPointerSet& globalUserPointers,
    NVVMValueMap& helperValueMap,
    NVVMTypeLoweringContext& typeContext,
    SlangNVVMValueHandle& outValue)
{
    if (!asNVVMSupportedDeviceCopyableValuePointerType(irValue->getDataType()) ||
        !globalUserPointers.contains(irValue))
    {
        return _getLoweredNVVMValue(
            codeGenContext,
            builder,
            module,
            irValue,
            valueMap,
            typeContext,
            outValue);
    }

    if (auto mappedValue = helperValueMap.tryGetValue(irValue))
    {
        outValue = *mappedValue;
        return SLANG_OK;
    }

    SlangNVVMValueHandle globalPointer = nullptr;
    SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
        codeGenContext,
        builder,
        module,
        irValue,
        valueMap,
        typeContext,
        globalPointer));
    SLANG_RETURN_ON_FAIL(_emitNVVMExecutableUserPointer(
        codeGenContext,
        builder,
        module,
        irValue->getDataType(),
        globalPointer,
        typeContext,
        outValue));
    helperValueMap[irValue] = outValue;
    return SLANG_OK;
}

SlangResult _emitNVVMSequentialElementExtract(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle aggregate,
    uint32_t index,
    SlangNVVMValueHandle& outElement)
{
    SlangNVVMTypeHandle indexType = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "structured-buffer conversion index type",
        builder.getIntegerType(module, 32, indexType)));
    SlangNVVMValueHandle indexValue = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "structured-buffer conversion index",
        builder.getIntegerConstant(module, indexType, index, indexValue)));
    return _requireBuilderOperation(
        codeGenContext,
        "structured-buffer vector lane extraction",
        builder.emitSequentialElementExtract(module, aggregate, indexValue, outElement));
}

// Emits CUDA floating remainder for the exact scalar/vector shape proven by preflight. Keeping
// the scalar libdevice call in the existing generic operation interface makes the target-specific
// semantic choice explicit without teaching the LLVM provider about Slang matrix legalization.
SlangResult _emitNVVMFloatingRemainderOperation(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const NVVMPlannedFloatingRemainder& operation,
    NVVMValueMap& valueMap,
    NVVMTypeLoweringContext& typeContext,
    SlangNVVMValueHandle& outValue)
{
    SlangNVVMValueHandle operands[2] = {};
    for (uint32_t i = 0; i < SLANG_COUNT_OF(operands); ++i)
    {
        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
            codeGenContext,
            builder,
            module,
            operation.operands[i],
            valueMap,
            typeContext,
            operands[i]));
    }

    const SlangNVVMValueOperationDesc scalarDesc = operation.scalarStep.getDesc();
    if (isTypeEqual(operation.resultType, operation.scalarType))
    {
        return _requireBuilderOperation(
            codeGenContext,
            operation.scalarStep.diagnosticName,
            builder.emitValueOperation(
                module,
                scalarDesc,
                operands,
                SLANG_COUNT_OF(operands),
                outValue));
    }

    SLANG_RELEASE_ASSERT(
        asNVVMSupportedValueVectorType(operation.resultType) && operation.laneCount > 0);
    List<SlangNVVMValueHandle> results;
    for (uint32_t lane = 0; lane < operation.laneCount; ++lane)
    {
        SlangNVVMValueHandle laneOperands[2] = {};
        for (uint32_t operandIndex = 0; operandIndex < SLANG_COUNT_OF(operands); ++operandIndex)
        {
            if (operation.operandIsVector[operandIndex])
            {
                SLANG_RETURN_ON_FAIL(_emitNVVMSequentialElementExtract(
                    codeGenContext,
                    builder,
                    module,
                    operands[operandIndex],
                    lane,
                    laneOperands[operandIndex]));
            }
            else
            {
                laneOperands[operandIndex] = operands[operandIndex];
            }
        }

        SlangNVVMValueHandle result = nullptr;
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            operation.scalarStep.diagnosticName,
            builder.emitValueOperation(
                module,
                scalarDesc,
                laneOperands,
                SLANG_COUNT_OF(laneOperands),
                result)));
        results.add(result);
    }

    SlangNVVMTypeHandle resultType = nullptr;
    SLANG_RETURN_ON_FAIL(
        typeContext.lowerType(operation.resultType, NVVMTypeUse::Value, resultType));
    return _requireBuilderOperation(
        codeGenContext,
        "CUDA floating-point remainder vector construction",
        builder.emitVectorConstruct(
            module,
            resultType,
            results.getBuffer(),
            size_t(results.getCount()),
            outValue));
}

// Converts whole local BF16 vectors without interpreting the lane bits. Consider this example:
//
//     void replace(inout vector<BFloat16, 3> x, vector<BFloat16, 3> y) { x = y; }
//
// The canonical IR keeps both values as Vec(BFloat16Type,3). The helper parameter points to
// [3 x i16] storage, while y is a <3 x i16> register value. Stores extract the vector lanes into
// the array; loads perform the inverse. BF2 already has the same physical type in both roles.
SlangResult _emitNVVMBFloat16LocalStorageConversion(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    NVVMTypeLoweringContext& typeContext,
    const NVVMPlannedStorageConversion& conversion,
    bool storageToValue,
    SlangNVVMValueHandle input,
    SlangNVVMValueHandle& outValue)
{
    const uint32_t count = conversion.laneCount;
    SLANG_RELEASE_ASSERT(
        conversion.kind == NVVMStorageConversionKind::BFloat16Vector && count >= 3 && count <= 4);
    SlangNVVMValueHandle elements[4] = {};
    for (uint32_t i = 0; i < count; ++i)
    {
        if (storageToValue)
        {
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "local BF16 component extraction",
                builder.emitAggregateElementExtract(module, input, i, elements[i])));
        }
        else
        {
            SLANG_RETURN_ON_FAIL(_emitNVVMSequentialElementExtract(
                codeGenContext,
                builder,
                module,
                input,
                i,
                elements[i]));
        }
    }
    SlangNVVMTypeHandle targetType = nullptr;
    SLANG_RETURN_ON_FAIL(typeContext.lowerType(conversion.type, conversion.resultUse, targetType));
    return _requireBuilderOperation(
        codeGenContext,
        "local BF16 storage conversion",
        storageToValue
            ? builder.emitVectorConstruct(module, targetType, elements, count, outValue)
            : builder.emitAggregateConstruct(module, targetType, elements, count, outValue));
}

// Executes the preflight recipe without repeating type classification or field traversal.
SlangResult _emitNVVMStructuredBufferStorageConversion(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    NVVMTypeLoweringContext& typeContext,
    const List<NVVMStructuredConversionRecipe>& recipes,
    Index recipeIndex,
    SlangNVVMValueHandle input,
    SlangNVVMValueHandle& outValue)
{
    SLANG_RELEASE_ASSERT(recipeIndex >= 0 && recipeIndex < recipes.getCount());
    const auto& recipe = recipes[recipeIndex];
    switch (recipe.kind)
    {
    case NVVMStructuredConversionKind::Identity:
        outValue = input;
        return SLANG_OK;
    case NVVMStructuredConversionKind::Boolean:
        if (recipe.storageToValue)
        {
            SlangNVVMTypeHandle storageType = nullptr;
            SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                recipe.type,
                NVVMTypeUse::StructuredBufferStorage,
                storageType));
            SlangNVVMValueHandle zero = nullptr;
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "structured-buffer Boolean zero",
                builder.getIntegerConstant(module, storageType, 0, zero)));
            const SlangNVVMValueHandle operands[] = {input, zero};
            return _requireBuilderOperation(
                codeGenContext,
                "structured-buffer Boolean load conversion",
                builder.emitValueOperation(
                    module,
                    kNVVMStructuredBoolLoadOperation,
                    operands,
                    SLANG_COUNT_OF(operands),
                    outValue));
        }
        return _requireBuilderOperation(
            codeGenContext,
            "structured-buffer Boolean store conversion",
            builder.emitValueOperation(
                module,
                kNVVMStructuredBoolStoreOperation,
                &input,
                1,
                outValue));
    case NVVMStructuredConversionKind::Elements:
        break;
    default:
        SLANG_UNEXPECTED("unknown structured storage conversion");
    }
    List<SlangNVVMValueHandle> elements;
    for (Index i = 0; i < recipe.children.getCount(); ++i)
    {
        SlangNVVMValueHandle element = nullptr;
        if (recipe.extractAggregate)
        {
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "structured-buffer aggregate element extraction",
                builder.emitAggregateElementExtract(module, input, uint32_t(i), element)));
        }
        else
        {
            SLANG_RETURN_ON_FAIL(_emitNVVMSequentialElementExtract(
                codeGenContext,
                builder,
                module,
                input,
                uint32_t(i),
                element));
        }
        SlangNVVMValueHandle converted = nullptr;
        SLANG_RETURN_ON_FAIL(_emitNVVMStructuredBufferStorageConversion(
            codeGenContext,
            builder,
            module,
            typeContext,
            recipes,
            recipe.children[i],
            element,
            converted));
        elements.add(converted);
    }
    SlangNVVMTypeHandle targetType = nullptr;
    SLANG_RETURN_ON_FAIL(typeContext.lowerType(
        recipe.type,
        recipe.storageToValue ? NVVMTypeUse::Value : NVVMTypeUse::StructuredBufferStorage,
        targetType));
    return _requireBuilderOperation(
        codeGenContext,
        "structured-buffer planned construction",
        recipe.constructAggregate ? builder.emitAggregateConstruct(
                                        module,
                                        targetType,
                                        elements.getBuffer(),
                                        size_t(elements.getCount()),
                                        outValue)
                                  : builder.emitVectorConstruct(
                                        module,
                                        targetType,
                                        elements.getBuffer(),
                                        size_t(elements.getCount()),
                                        outValue));
}

SlangResult _getNVVMRecipeIntegerConstant(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    uint32_t bitWidth,
    int64_t value,
    SlangNVVMValueHandle& outValue)
{
    SlangNVVMTypeHandle integerType = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "scalar intrinsic recipe integer type",
        builder.getIntegerType(module, bitWidth, integerType)));
    return _requireBuilderOperation(
        codeGenContext,
        "scalar intrinsic recipe integer constant",
        builder.getIntegerConstant(module, integerType, value, outValue));
}

// Executes a retained byte displacement without interpreting the logical record layout.
SlangResult _emitNVVMLayoutByteAddress(
    CodeGenContext* context,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle base,
    uint64_t offset,
    SlangNVVMTypeHandle pointee,
    SlangNVVMValueHandle& result)
{
    SlangNVVMValueHandle byteOffset = nullptr;
    SLANG_RETURN_ON_FAIL(
        _getNVVMRecipeIntegerConstant(context, builder, module, 64, int64_t(offset), byteOffset));
    if (!pointee)
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            context,
            "layout address byte type",
            builder.getIntegerType(module, 8, pointee)));
    }
    return _requireBuilderOperation(
        context,
        "layout field byte address",
        builder.emitByteOffsetPointer(module, base, byteOffset, pointee, result));
}

// Decodes only logical lanes from the checked CUDA entry carrier. Padding (notably Half3's
// fourth storage lane) never becomes a semantic value, and helpers see canonical SSA values.
SlangResult _emitNVVMCUDAValueDecode(
    CodeGenContext* context,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    NVVMTypeLoweringContext& types,
    const NVVMCUDAValueLayout& layout,
    SlangNVVMValueHandle parameter,
    SlangNVVMValueHandle& outValue)
{
    if (layout.isUserPointer)
    {
        SlangNVVMTypeHandle executableType = nullptr;
        SLANG_RETURN_ON_FAIL(types.lowerType(layout.type, NVVMTypeUse::Value, executableType));
        return _requireBuilderOperation(
            context,
            "entry UserPointer decode",
            builder.emitPointerAddressSpaceCast(module, executableType, parameter, outValue));
    }
    if (layout.isDescriptor)
    {
        outValue = parameter;
        return SLANG_OK;
    }
    if (layout.isAggregate())
    {
        List<SlangNVVMValueHandle> elements;
        const uint32_t count =
            layout.elementCount ? layout.elementCount : uint32_t(layout.children.getCount());
        for (uint32_t i = 0; i < count; ++i)
        {
            const auto& child = layout.children[layout.elementCount ? 0 : i];
            const uint32_t offset =
                layout.elementCount ? i * layout.elementStride : layout.fieldOffsets[i];
            SlangNVVMTypeHandle childParameterType = nullptr, childStorageType = nullptr;
            SLANG_RETURN_ON_FAIL(
                types.lowerCUDAValueType(child, childParameterType, childStorageType));
            SlangNVVMValueHandle address = nullptr;
            SLANG_RETURN_ON_FAIL(_emitNVVMLayoutByteAddress(
                context,
                builder,
                module,
                parameter,
                offset,
                childStorageType,
                address));
            SlangNVVMValueHandle input = address;
            if (!child.isByValue())
                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                    context,
                    "numeric entry scalar load",
                    builder.emitLoad(
                        module,
                        address,
                        child.alignment,
                        SLANG_NVVM_LOAD_FLAG_NONE,
                        input)));
            SlangNVVMValueHandle value = nullptr;
            SLANG_RETURN_ON_FAIL(
                _emitNVVMCUDAValueDecode(context, builder, module, types, child, input, value));
            elements.add(value);
        }
        SlangNVVMTypeHandle valueType = nullptr;
        SLANG_RETURN_ON_FAIL(types.lowerType(layout.type, NVVMTypeUse::Value, valueType));
        return _requireBuilderOperation(
            context,
            "numeric entry aggregate decode",
            builder.emitAggregateConstruct(
                module,
                valueType,
                elements.getBuffer(),
                elements.getCount(),
                outValue));
    }
    SlangNVVMValueHandle storage = parameter;
    if (layout.isVector)
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            context,
            "numeric entry storage load",
            builder.emitLoad(
                module,
                parameter,
                layout.alignment,
                SLANG_NVVM_LOAD_FLAG_NONE,
                storage)));
    }
    SlangNVVMValueHandle lanes[4] = {};
    for (uint32_t i = 0; i < layout.laneCount; ++i)
    {
        SlangNVVMValueHandle lane = storage;
        if (layout.isVector)
        {
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                context,
                "numeric entry lane extraction",
                builder.emitAggregateElementExtract(module, storage, i, lane)));
        }
        if (layout.isBoolean)
        {
            SlangNVVMTypeHandle byteType = nullptr;
            SlangNVVMValueHandle zero = nullptr;
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                context,
                "numeric entry Boolean byte",
                builder.getIntegerType(module, 8, byteType)));
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                context,
                "numeric entry Boolean zero",
                builder.getIntegerConstant(module, byteType, 0, zero)));
            const SlangNVVMValueHandle operands[] = {lane, zero};
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                context,
                "numeric entry Boolean decode",
                builder.emitValueOperation(
                    module,
                    kNVVMStructuredBoolLoadOperation,
                    operands,
                    2,
                    lanes[i])));
        }
        else if (layout.isHalf)
        {
            SLANG_RETURN_ON_FAIL(_emitNVVMHalfHelperABIReinterpretation(
                context,
                builder,
                module,
                types,
                layout.scalarType,
                false,
                lane,
                lanes[i]));
        }
        else
            lanes[i] = lane;
    }
    outValue = lanes[0];
    if (layout.isVector)
    {
        SlangNVVMTypeHandle vectorType = nullptr;
        SLANG_RETURN_ON_FAIL(types.lowerType(layout.type, NVVMTypeUse::Value, vectorType));
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            context,
            "numeric entry vector decode",
            builder.emitVectorConstruct(module, vectorType, lanes, layout.laneCount, outValue)));
    }
    return SLANG_OK;
}

// Executes a checked memory conversion without rediscovering its address role or layout.
SlangResult _emitNVVMPlannedStorageConversion(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    NVVMTypeLoweringContext& typeContext,
    const List<NVVMStructuredConversionRecipe>& recipes,
    const NVVMPlannedStorageConversion& conversion,
    bool storageToValue,
    SlangNVVMValueHandle input,
    SlangNVVMValueHandle& outValue)
{
    switch (conversion.kind)
    {
    case NVVMStorageConversionKind::Identity:
        outValue = input;
        return SLANG_OK;
    case NVVMStorageConversionKind::StructuredBuffer:
        return _emitNVVMStructuredBufferStorageConversion(
            codeGenContext,
            builder,
            module,
            typeContext,
            recipes,
            conversion.structuredRecipe,
            input,
            outValue);
    case NVVMStorageConversionKind::BFloat16Vector:
        return _emitNVVMBFloat16LocalStorageConversion(
            codeGenContext,
            builder,
            module,
            typeContext,
            conversion,
            storageToValue,
            input,
            outValue);
    default:
        SLANG_UNEXPECTED("unknown planned storage conversion");
    }
}

SlangResult _emitNVVMValueRecipeStep(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const NVVMValueRecipeStep& step,
    const SlangNVVMValueHandle* operands,
    uint32_t operandCount,
    SlangNVVMValueHandle& outValue)
{
    SLANG_ASSERT(operandCount == step.operandCount);
    return _requireBuilderOperation(
        codeGenContext,
        step.diagnosticName,
        builder.emitValueOperation(module, step.getDesc(), operands, operandCount, outValue));
}

// Loads or stores the selected scalar payload lanes. Three-lane vectors never access a fourth
// LLVM padding lane, and Bool conversion uses the selected one- or four-byte integer storage.
SlangResult _emitNVVMLayoutMemory(
    CodeGenContext* context,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    NVVMTypeLoweringContext& types,
    const NVVMLayoutStorage& storage,
    const NVVMValueRecipeStep& boolConversion,
    bool load,
    SlangNVVMValueHandle pointer,
    SlangNVVMValueHandle input,
    SlangNVVMValueHandle& result)
{
    SlangNVVMTypeHandle scalarType = nullptr;
    if (boolConversion.operation)
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            context,
            "layout Boolean storage type",
            builder.getIntegerType(module, storage.scalarSize * 8, scalarType)));
    }
    else
    {
        SLANG_RETURN_ON_FAIL(types.lowerType(storage.scalarType, NVVMTypeUse::Value, scalarType));
    }
    SlangNVVMValueHandle lanes[4] = {};
    SLANG_RELEASE_ASSERT(storage.laneCount >= 1 && storage.laneCount <= SLANG_COUNT_OF(lanes));
    for (uint32_t i = 0; i < storage.laneCount; ++i)
    {
        SlangNVVMValueHandle lanePointer = nullptr;
        SLANG_RETURN_ON_FAIL(_emitNVVMLayoutByteAddress(
            context,
            builder,
            module,
            pointer,
            uint64_t(i) * storage.scalarSize,
            scalarType,
            lanePointer));
        if (load)
        {
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                context,
                "layout field load",
                builder.emitLoad(
                    module,
                    lanePointer,
                    storage.alignment,
                    SLANG_NVVM_LOAD_FLAG_NONE,
                    lanes[i])));
            if (boolConversion.operation)
            {
                SlangNVVMValueHandle zero = nullptr, converted = nullptr;
                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                    context,
                    "layout Boolean zero",
                    builder.getIntegerConstant(module, scalarType, 0, zero)));
                const SlangNVVMValueHandle operands[] = {lanes[i], zero};
                SLANG_RETURN_ON_FAIL(_emitNVVMValueRecipeStep(
                    context,
                    builder,
                    module,
                    boolConversion,
                    operands,
                    2,
                    converted));
                lanes[i] = converted;
            }
        }
        else
        {
            SlangNVVMValueHandle lane = input;
            if (as<IRVectorType>(storage.valueType))
            {
                SLANG_RETURN_ON_FAIL(
                    _emitNVVMSequentialElementExtract(context, builder, module, input, i, lane));
            }
            if (boolConversion.operation)
            {
                SlangNVVMValueHandle converted = nullptr;
                SLANG_RETURN_ON_FAIL(_emitNVVMValueRecipeStep(
                    context,
                    builder,
                    module,
                    boolConversion,
                    &lane,
                    1,
                    converted));
                lane = converted;
            }
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                context,
                "layout field store",
                builder.emitStore(module, lane, lanePointer, storage.alignment)));
        }
    }
    if (load)
    {
        result = lanes[0];
        if (as<IRVectorType>(storage.valueType))
        {
            SlangNVVMTypeHandle vectorType = nullptr;
            SLANG_RETURN_ON_FAIL(
                types.lowerType(storage.valueType, NVVMTypeUse::Value, vectorType));
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                context,
                "layout vector reconstruction",
                builder.emitVectorConstruct(module, vectorType, lanes, storage.laneCount, result)));
        }
    }
    return SLANG_OK;
}

// Emits the CUDA prelude's four full-mask indexed reads. Every surviving source quad must be
// complete, and named non-exited lanes must execute matching shuffle sequences. In particular,
// this is not a vote over only the callers in the current branch, and no hardware mask is sampled.

SlangResult _emitNVVMUInt64WordConstruction(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const NVVMPlannedUInt64WordConstruction& construction,
    NVVMValueMap& valueMap,
    NVVMTypeLoweringContext& typeContext,
    SlangNVVMValueHandle& outValue)
{
    SlangNVVMValueHandle words32[2] = {};
    IRInst* semanticWords[] = {construction.lowWord, construction.highWord};
    for (uint32_t i = 0; i < SLANG_COUNT_OF(semanticWords); ++i)
    {
        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
            codeGenContext,
            builder,
            module,
            semanticWords[i],
            valueMap,
            typeContext,
            words32[i]));
    }

    SlangNVVMValueHandle words64[2] = {};
    for (uint32_t i = 0; i < SLANG_COUNT_OF(words32); ++i)
    {
        SLANG_RETURN_ON_FAIL(_emitNVVMValueRecipeStep(
            codeGenContext,
            builder,
            module,
            construction.wordConversion,
            &words32[i],
            1,
            words64[i]));
    }

    SlangNVVMValueHandle shiftAmount = nullptr;
    SLANG_RETURN_ON_FAIL(
        _getNVVMRecipeIntegerConstant(codeGenContext, builder, module, 64, 32, shiftAmount));
    const SlangNVVMValueHandle shiftOperands[] = {words64[1], shiftAmount};
    SlangNVVMValueHandle shiftedHighWord = nullptr;
    SLANG_RETURN_ON_FAIL(_emitNVVMValueRecipeStep(
        codeGenContext,
        builder,
        module,
        construction.highWordShift,
        shiftOperands,
        SLANG_COUNT_OF(shiftOperands),
        shiftedHighWord));

    const SlangNVVMValueHandle combineOperands[] = {words64[0], shiftedHighWord};
    return _emitNVVMValueRecipeStep(
        codeGenContext,
        builder,
        module,
        construction.combine,
        combineOperands,
        SLANG_COUNT_OF(combineOperands),
        outValue);
}

SlangResult _emitNVVMResourceBitCast(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const NVVMPlannedResourceBitCast& bitCast,
    NVVMValueMap& valueMap,
    NVVMTypeLoweringContext& typeContext,
    SlangNVVMValueHandle& outValue)
{
    outValue = nullptr;
    SlangNVVMValueHandle input = nullptr;
    SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
        codeGenContext,
        builder,
        module,
        bitCast.value,
        valueMap,
        typeContext,
        input));

    if (bitCast.kind == NVVMPlannedResourceBitCastKind::OpaqueHandle64)
    {
        return _emitNVVMValueRecipeStep(
            codeGenContext,
            builder,
            module,
            bitCast.steps[0],
            &input,
            1,
            outValue);
    }

    SLANG_RELEASE_ASSERT(bitCast.kind == NVVMPlannedResourceBitCastKind::RawBuffer);

    SlangNVVMTypeHandle uintType = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "descriptor payload word type",
        builder.getIntegerType(module, 32, uintType)));
    SlangNVVMTypeHandle uint2Type = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "descriptor pointer payload type",
        builder.getVectorType(module, uintType, 2, uint2Type)));

    SlangNVVMValueHandle indices[4] = {};
    for (uint32_t i = 0; i < SLANG_COUNT_OF(indices); ++i)
    {
        SLANG_RETURN_ON_FAIL(
            _getNVVMRecipeIntegerConstant(codeGenContext, builder, module, 32, i, indices[i]));
    }

    if (!bitCast.resultIsResourceValue)
    {
        SlangNVVMValueHandle dataPointer = nullptr;
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "descriptor raw-buffer data extraction",
            builder.emitAggregateElementExtract(module, input, 0, dataPointer)));
        SlangNVVMValueHandle count = nullptr;
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "descriptor raw-buffer count extraction",
            builder.emitAggregateElementExtract(module, input, 1, count)));

        SlangNVVMValueHandle pointerWords = nullptr;
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "descriptor data-pointer bit transport",
            builder.emitBitCast(module, uint2Type, dataPointer, pointerWords)));
        SlangNVVMValueHandle words[4] = {};
        for (uint32_t i = 0; i < 2; ++i)
        {
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "descriptor data-pointer word extraction",
                builder.emitSequentialElementExtract(module, pointerWords, indices[i], words[i])));
        }

        SLANG_RETURN_ON_FAIL(_emitNVVMValueRecipeStep(
            codeGenContext,
            builder,
            module,
            bitCast.steps[0],
            &count,
            1,
            words[2]));
        SlangNVVMValueHandle shiftAmount = nullptr;
        SLANG_RETURN_ON_FAIL(
            _getNVVMRecipeIntegerConstant(codeGenContext, builder, module, 64, 32, shiftAmount));
        const SlangNVVMValueHandle shiftOperands[] = {count, shiftAmount};
        SlangNVVMValueHandle shiftedCount = nullptr;
        SLANG_RETURN_ON_FAIL(_emitNVVMValueRecipeStep(
            codeGenContext,
            builder,
            module,
            bitCast.steps[1],
            shiftOperands,
            SLANG_COUNT_OF(shiftOperands),
            shiftedCount));
        SLANG_RETURN_ON_FAIL(_emitNVVMValueRecipeStep(
            codeGenContext,
            builder,
            module,
            bitCast.steps[0],
            &shiftedCount,
            1,
            words[3]));

        SlangNVVMTypeHandle payloadType = nullptr;
        SLANG_RETURN_ON_FAIL(
            typeContext.lowerType(bitCast.payloadType, NVVMTypeUse::Value, payloadType));
        return _requireBuilderOperation(
            codeGenContext,
            "descriptor AnyValue payload construction",
            builder
                .emitVectorConstruct(module, payloadType, words, SLANG_COUNT_OF(words), outValue));
    }

    SlangNVVMValueHandle words[4] = {};
    for (uint32_t i = 0; i < SLANG_COUNT_OF(words); ++i)
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "descriptor AnyValue payload word extraction",
            builder.emitSequentialElementExtract(module, input, indices[i], words[i])));
    }
    SlangNVVMValueHandle pointerWords = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "descriptor pointer payload construction",
        builder.emitVectorConstruct(module, uint2Type, words, 2, pointerWords)));

    SlangNVVMTypeHandle elementType = nullptr;
    if (bitCast.rawBufferIsByteAddress)
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "descriptor byte-address-buffer element type",
            builder.getIntegerType(module, 32, elementType)));
    }
    else
    {
        const NVVMTypeUse elementUse = bitCast.rawBufferElementUsesStructuredStorage
                                           ? NVVMTypeUse::StructuredBufferStorage
                                           : NVVMTypeUse::Value;
        SLANG_RETURN_ON_FAIL(
            typeContext.lowerType(bitCast.rawBufferElementType, elementUse, elementType));
    }
    SlangNVVMTypeHandle dataPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "descriptor raw-buffer data-pointer type",
        builder.getPointerType(
            module,
            elementType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            dataPointerType)));
    SlangNVVMValueHandle dataPointer = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "descriptor data-pointer reconstruction",
        builder.emitBitCast(module, dataPointerType, pointerWords, dataPointer)));

    SlangNVVMValueHandle countWords64[2] = {};
    for (uint32_t i = 0; i < 2; ++i)
    {
        SLANG_RETURN_ON_FAIL(_emitNVVMValueRecipeStep(
            codeGenContext,
            builder,
            module,
            bitCast.steps[0],
            &words[i + 2],
            1,
            countWords64[i]));
    }
    SlangNVVMValueHandle shiftAmount = nullptr;
    SLANG_RETURN_ON_FAIL(
        _getNVVMRecipeIntegerConstant(codeGenContext, builder, module, 64, 32, shiftAmount));
    const SlangNVVMValueHandle shiftOperands[] = {countWords64[1], shiftAmount};
    SlangNVVMValueHandle shiftedHighWord = nullptr;
    SLANG_RETURN_ON_FAIL(_emitNVVMValueRecipeStep(
        codeGenContext,
        builder,
        module,
        bitCast.steps[1],
        shiftOperands,
        SLANG_COUNT_OF(shiftOperands),
        shiftedHighWord));
    const SlangNVVMValueHandle combineOperands[] = {countWords64[0], shiftedHighWord};
    SlangNVVMValueHandle count = nullptr;
    SLANG_RETURN_ON_FAIL(_emitNVVMValueRecipeStep(
        codeGenContext,
        builder,
        module,
        bitCast.steps[2],
        combineOperands,
        SLANG_COUNT_OF(combineOperands),
        count));

    SlangNVVMTypeHandle resourceValueType = nullptr;
    SLANG_RETURN_ON_FAIL(
        typeContext.lowerType(bitCast.resourceValueType, NVVMTypeUse::Value, resourceValueType));
    const SlangNVVMValueHandle handleElements[] = {dataPointer, count};
    return _requireBuilderOperation(
        codeGenContext,
        "descriptor raw-buffer view reconstruction",
        builder.emitAggregateConstruct(
            module,
            resourceValueType,
            handleElements,
            SLANG_COUNT_OF(handleElements),
            outValue));
}


// Materializes one already-typed scalar value across the exact integer scalar/vector shape owned
// by the canonical instruction. Signedness does not change the physical provider type.
SlangResult _emitNVVMIntegerSplat(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    IRType* dataIRType,
    const SlangNVVMValueTypeDesc& dataType,
    SlangNVVMValueHandle scalarValue,
    const char* diagnosticName,
    NVVMTypeLoweringContext& typeContext,
    SlangNVVMValueHandle& outValue)
{
    SLANG_RELEASE_ASSERT(
        dataIRType && NVVMSemantics::isSelectedIntegerValue(dataType) && scalarValue);
    if (dataType.laneCount == 1)
    {
        outValue = scalarValue;
        return SLANG_OK;
    }

    SlangNVVMTypeHandle vectorType = nullptr;
    SLANG_RETURN_ON_FAIL(typeContext.lowerType(dataIRType, NVVMTypeUse::Value, vectorType));
    SlangNVVMValueHandle elements[4] = {};
    SLANG_RELEASE_ASSERT(dataType.laneCount <= SLANG_COUNT_OF(elements));
    for (uint32_t lane = 0; lane < dataType.laneCount; ++lane)
        elements[lane] = scalarValue;
    return _requireBuilderOperation(
        codeGenContext,
        diagnosticName,
        builder.emitVectorConstruct(module, vectorType, elements, dataType.laneCount, outValue));
}

SlangResult _emitNVVMIntegerSplatConstant(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    IRType* dataIRType,
    const SlangNVVMValueTypeDesc& dataType,
    int64_t value,
    const char* diagnosticName,
    NVVMTypeLoweringContext& typeContext,
    SlangNVVMValueHandle& outValue)
{
    SlangNVVMValueHandle scalarValue = nullptr;
    SLANG_RETURN_ON_FAIL(_getNVVMRecipeIntegerConstant(
        codeGenContext,
        builder,
        module,
        dataType.bitWidth,
        value,
        scalarValue));
    return _emitNVVMIntegerSplat(
        codeGenContext,
        builder,
        module,
        dataIRType,
        dataType,
        scalarValue,
        diagnosticName,
        typeContext,
        outValue);
}

SlangResult _emitNVVMNumericTruthiness(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const NVVMPlannedNumericTruthiness& operation,
    NVVMValueMap& valueMap,
    NVVMTypeLoweringContext& typeContext,
    SlangNVVMValueHandle& outValue)
{
    SlangNVVMValueHandle value = nullptr;
    SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
        codeGenContext,
        builder,
        module,
        operation.value,
        valueMap,
        typeContext,
        value));
    SlangNVVMValueHandle zero = nullptr;
    if (NVVMSemantics::isSelectedIntegerValue(operation.valueType))
    {
        SLANG_RETURN_ON_FAIL(_emitNVVMIntegerSplatConstant(
            codeGenContext,
            builder,
            module,
            as<IRType>(operation.value->getDataType()),
            operation.valueType,
            0,
            "integer truthiness zero",
            typeContext,
            zero));
    }
    else
    {
        SLANG_RELEASE_ASSERT(
            NVVMSemantics::isSelectedFloatValue(operation.valueType) &&
            operation.valueType.laneCount == 1);
        SlangNVVMTypeHandle floatingType = nullptr;
        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
            as<IRType>(operation.value->getDataType()),
            NVVMTypeUse::Value,
            floatingType));
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "floating-point truthiness zero",
            builder.getFloatingPointConstant(
                module,
                floatingType,
                operation.valueType.bitWidth,
                0,
                zero)));
    }
    const SlangNVVMValueHandle operands[] = {value, zero};
    SLANG_RETURN_ON_FAIL(_emitNVVMValueRecipeStep(
        codeGenContext,
        builder,
        module,
        operation.comparison,
        operands,
        SLANG_COUNT_OF(operands),
        outValue));
    SLANG_RELEASE_ASSERT(outValue);
    return SLANG_OK;
}


} // namespace

SlangResult validateNVVMSupportedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    NVVMOperationRequirements& outRequirements)
{
    outRequirements = {};
    auto& options = codeGenContext->getTargetProgram()->getOptionSet();
    if (!options.hasValidOptixVersion())
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("OptiX target version"));
    outRequirements.optixVersion = uint32_t(options.getOptixVersion());
    if (!linkedIR.module || linkedIR.entryPoints.getCount() != 1)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point count"));

    IRFunc* entryPoint = linkedIR.entryPoints[0];
    if (!entryPoint || entryPoint->getParent() != linkedIR.module->getModuleInst() ||
        !entryPoint->isDefinition())
    {
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point definition"));
    }

    auto entryPointDecoration = entryPoint->findDecoration<IREntryPointDecoration>();
    if (!entryPointDecoration)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point decoration"));
    const auto stage = entryPointDecoration->getProfile().getStage();
    if (stage != Stage::Compute && !_isNVVMOptixStage(stage))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point stage"));
    if (_isNVVMOptixStage(stage) && stage != Stage::Callable && entryPoint->getParamCount())
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("OptiX entry parameters"));
    if (stage == Stage::Callable)
    {
        // Callable payloads are canonical mutable references. Empty-type legalization removes
        // both the formal and caller's payload, yielding the same zero-argument ABI.
        if (entryPoint->getParamCount() > 1)
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("callable parameter count"));
        if (entryPoint->getParamCount())
        {
            auto type = entryPoint->getParamType(0);
            auto pointer = as<IRPtrTypeBase>(type);
            if (!pointer ||
                (type->getOp() != kIROp_BorrowInOutParamType &&
                 type->getOp() != kIROp_OutParamType) ||
                !isNVVMSupportedCopyableValueType(pointer->getValueType()) ||
                !_isSupportedNVVMHelperParameterType(type))
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    toSlice("callable payload parameter"));
        }
    }
    if (!entryPointDecoration->getName()->getStringSlice().getLength())
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point name"));
    if (!as<IRVoidType>(entryPoint->getResultType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point result type"));

    List<IRFunc*>& functions = outRequirements.emissionPlan.functions;
    HashSet<IRFunc*> functionSet;
    SLANG_RETURN_ON_FAIL(
        _collectNVVMFunctions(codeGenContext, linkedIR, entryPoint, functions, functionSet));
    SLANG_RETURN_ON_FAIL(_collectNVVMFunctionNames(
        codeGenContext,
        linkedIR.module,
        entryPoint,
        functions,
        outRequirements.emissionPlan.functionNames));
    SLANG_RETURN_ON_FAIL(_validateNVVMFunctionUses(codeGenContext, functions));

    for (auto function : functions)
    {
        SLANG_RETURN_ON_FAIL(_validateNVVMFunction(
            codeGenContext,
            entryPoint,
            function,
            functionSet,
            outRequirements));
    }

    if (outRequirements.optixVersion < 90000 &&
        outRequirements.emissionPlan.hitObjectOperations.getCount())
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("OptiX 8 HitObject operations"));

    NVVMConventionalGlobalParams conventionalGlobalParams;
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        NVVMConventionalGlobalParams globalParams;
        if (_getNVVMConventionalGlobalParams(globalInst, globalParams))
            conventionalGlobalParams = globalParams;
    }
    HashSet<IRInst*> selectedReachableStructTypes;
    if (conventionalGlobalParams.elementType)
    {
        for (auto field : conventionalGlobalParams.elementType->getFields())
        {
            IRType* fieldType = field->getFieldType();
            if (!isNVVMSupportedConventionalGlobalFieldType(field))
            {
                return _diagnoseUnsupportedIRType(
                    codeGenContext,
                    "conventional global field",
                    fieldType);
            }
            auto resourceStruct = asNVVMSupportedResourceStructType(fieldType);
            auto storageArray = asNVVMSupportedAggregateStorageArrayType(fieldType);
            auto storageStruct = asNVVMSupportedAggregateStorageStructType(fieldType);
            // Resource-value records already have a layout proof below using their lowered
            // fields. For example, AppendStructuredBuffer becomes an elements/counter record,
            // while its binding key retains the source StructuredBufferTypeLayout. That source
            // layout must not be interpreted as the carrier's StructTypeLayout. Storage-only
            // records and arrays still require the distinct aggregate-storage proof.
            if (storageArray || (storageStruct && !resourceStruct))
            {
                auto fieldVarLayout = findVarLayout(field->getKey());
                if (!_hasNVVMCompatibleAggregateStorageLayout(
                        codeGenContext,
                        fieldType,
                        fieldVarLayout ? fieldVarLayout->getTypeLayout() : nullptr))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("aggregate storage layout"));
                }
                if (auto elementStruct = storageStruct
                                             ? storageStruct
                                             : as<IRStructType>(storageArray->getElementType()))
                {
                    _addNVVMReachableStructTypes(elementStruct, selectedReachableStructTypes);
                }
            }
            IRType* parameterGroupElementType = nullptr;
            if (asNVVMSupportedParameterGroupType(fieldType, &parameterGroupElementType) &&
                !_hasNVVMCompatibleAggregateStorageLayout(
                    codeGenContext,
                    parameterGroupElementType,
                    nullptr,
                    true))
            {
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    toSlice("parameter-group storage layout"));
            }
            if (auto parameterGroupStruct = as<IRStructType>(parameterGroupElementType))
            {
                _addNVVMReachableStructTypes(
                    parameterGroupStruct,
                    selectedReachableStructTypes,
                    true);
            }
            if (resourceStruct)
            {
                if (!_hasNVVMCompatibleStructLayout(codeGenContext, resourceStruct))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("conventional resource-struct layout"));
                }
                _addNVVMReachableStructTypes(resourceStruct, selectedReachableStructTypes);
            }
            NVVMRawBufferType rawBufferType;
            if (getNVVMSupportedRawBufferType(fieldType, rawBufferType) &&
                !_hasNVVMCompatibleRawBufferElementLayout(codeGenContext, fieldType))
            {
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    toSlice("structured-buffer element layout"));
            }
            _addNVVMReachableStructTypes(fieldType, selectedReachableStructTypes);
        }
    }

    for (auto function : functions)
    {
        IRType* resultType = function->getResultType();
        _addNVVMReachableStructTypes(resultType, selectedReachableStructTypes);
        for (auto parameter : function->getParams())
        {
            IRType* parameterType = parameter->getDataType();
            _addNVVMReachableStructTypes(parameterType, selectedReachableStructTypes);
            if (auto reference = _getNVVMLocalSubstandardRecordArrayPointer(parameter))
            {
                _addNVVMReachableStructTypes(
                    reference->getValueType(),
                    selectedReachableStructTypes);
            }
            NVVMRawBufferType rawBufferType;
            if (getNVVMSupportedRawBufferType(parameter->getDataType(), rawBufferType) &&
                !_hasNVVMCompatibleRawBufferElementLayout(codeGenContext, parameter->getDataType()))
            {
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    toSlice("structured-buffer element layout"));
            }
            IRType* pointerValueType = nullptr;
            if (asNVVMSupportedLocalResourceValuePointerType(
                    parameter->getDataType(),
                    &pointerValueType))
            {
                _addNVVMReachableStructTypes(pointerValueType, selectedReachableStructTypes);
            }
            IRType* copyablePointerValueType = nullptr;
            if (asNVVMSupportedLocalCopyableValuePointerType(
                    parameter->getDataType(),
                    &copyablePointerValueType))
            {
                _addNVVMReachableStructTypes(
                    copyablePointerValueType,
                    selectedReachableStructTypes);
            }
            IRType* helperPointerValueType = nullptr;
            if (asNVVMSupportedLocalHelperValuePointerType(
                    parameter->getDataType(),
                    &helperPointerValueType))
            {
                _addNVVMReachableStructTypes(helperPointerValueType, selectedReachableStructTypes);
            }
        }
        for (auto block : function->getBlocks())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                _addNVVMReachableStructTypes(inst->getDataType(), selectedReachableStructTypes);
                if (auto pointer = _getNVVMLocalSubstandardRecordArrayPointer(inst))
                    _addNVVMReachableStructTypes(
                        pointer->getValueType(),
                        selectedReachableStructTypes);
                IRType* localValueType = nullptr;
                if (inst->getOp() == kIROp_Var && asNVVMSupportedLocalCopyableValuePointerType(
                                                      inst->getDataType(),
                                                      &localValueType))
                {
                    _addNVVMReachableStructTypes(localValueType, selectedReachableStructTypes);
                }
                IRType* localResourceValueType = nullptr;
                if (inst->getOp() == kIROp_Var && asNVVMSupportedLocalResourceValuePointerType(
                                                      inst->getDataType(),
                                                      &localResourceValueType))
                {
                    _addNVVMReachableStructTypes(
                        localResourceValueType,
                        selectedReachableStructTypes);
                }
                IRType* localHelperValueType = nullptr;
                if (inst->getOp() == kIROp_Var && asNVVMSupportedLocalHelperValuePointerType(
                                                      inst->getDataType(),
                                                      &localHelperValueType))
                {
                    _addNVVMReachableStructTypes(
                        localHelperValueType,
                        selectedReachableStructTypes);
                }
            }
        }
    }
    // Packing a constant payload can eliminate every record-valued instruction while the
    // trace still retains its original type for admission. This checked metadata operand is
    // a type dependency just like a function signature; it does not require a storage witness.
    for (const auto& trace : outRequirements.emissionPlan.traceRays)
        _addNVVMReachableStructTypes(
            cast<IRType>(trace.source->getOperand(0)),
            selectedReachableStructTypes);

    // Validated HitObject operations also retain empty records for their zero-word payloads.
    // This is a metadata dependency, not admission of an empty ordinary executable value.
    for (const auto& operation : outRequirements.emissionPlan.hitObjectOperations)
        if (operation.payloadType)
            _addNVVMReachableStructTypes(operation.payloadType, selectedReachableStructTypes, true);

    // A group-shared global is also a canonical type root. Its pointer spelling does not make the
    // pointee reachable through ordinary SSA-type traversal, so collect the finite storage value
    // directly from the producer before auditing retained module-scope type declarations.
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        NVVMSharedGlobal sharedGlobal;
        if (getNVVMSupportedSharedGlobal(globalInst, &sharedGlobal))
            _addNVVMReachableStructTypes(sharedGlobal.storageType, selectedReachableStructTypes);
    }
    // Linking can retain module-scope types, layouts, capabilities, and constants needed to spell
    // the reachable functions. IRStructKey is also layout-only identity retained for raw CUDA
    // parameter layouts. A selected struct used by a reachable signature, local, or raw structured
    // buffer is its canonical value type, not an unrelated dropped global. Reject every other
    // semantic global so this emitter cannot silently drop a function, parameter, initializer, or
    // storage object.
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        if (auto globalFunction = as<IRFunc>(globalInst))
        {
            if (functionSet.contains(globalFunction))
                continue;
            return _diagnoseUnsupportedIR(
                codeGenContext,
                UnownedStringSlice(getIROpInfo(globalInst->getOp()).name));
        }
        if (as<IRGlobalVar>(globalInst))
        {
            NVVMSharedGlobal sharedGlobal;
            if (getNVVMSupportedSharedGlobal(globalInst, &sharedGlobal))
            {
                if (!_hasNVVMCompatibleHelperValueLayout(
                        codeGenContext,
                        sharedGlobal.alignmentType))
                {
                    return _diagnoseUnsupportedIRType(
                        codeGenContext,
                        "shared global storage layout",
                        sharedGlobal.storageType);
                }
                continue;
            }
            return _diagnoseUnsupportedIR(
                codeGenContext,
                UnownedStringSlice(getIROpInfo(globalInst->getOp()).name));
        }
        NVVMConventionalGlobalParams globalParams;
        if (_getNVVMConventionalGlobalParams(globalInst, globalParams))
            continue;
        // Hashed string literals are module reflection metadata. Like the C-family emitters, direct
        // NVVM preserves them in the linked module but emits no executable or storage declaration.
        if (_isNVVMSupportedModuleConstantValue(globalInst) ||
            as<IRGlobalHashedStringLiterals>(globalInst) || as<IRDecoration>(globalInst) ||
            as<IRConstant>(globalInst) || as<IRStructKey>(globalInst) ||
            getIROpInfo(globalInst->getOp()).isHoistable())
        {
            continue;
        }
        if (selectedReachableStructTypes.contains(globalInst))
            continue;
        // An exported link-time type can survive empty-type legalization solely because of
        // keepAlive. With no IR uses it needs no executable representation or provider declaration.
        if (as<IRStructType>(globalInst) && !globalInst->hasUses())
            continue;
        if (_isNVVMConventionalGlobalStorageType(conventionalGlobalParams, globalInst))
            continue;
        return _diagnoseUnsupportedIR(
            codeGenContext,
            UnownedStringSlice(getIROpInfo(globalInst->getOp()).name));
    }

    return SLANG_OK;
}
SlangResult emitNVVMIRFromLinkedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    const NVVMIRBuilder& builder,
    const NVVMOperationRequirements& requirements,
    ComPtr<IArtifact>& outArtifact,
    ISlangBlob* deviceLibraryContents)
{
    outArtifact.setNull();
    SLANG_RELEASE_ASSERT(linkedIR.entryPoints.getCount() == 1);
    if (!builder.supportsOptixVersion(requirements.optixVersion))
        return _requireBuilderOperation(
            codeGenContext,
            "OptiX target version",
            SLANG_E_NOT_AVAILABLE);


    ScopedNVVMDeviceLibrary libraryScope;
    libraryScope.builder = &builder;
    for (const auto& planned : requirements.emissionPlan.namedIntrinsics)
    {
        if (!planned.isDeviceLibraryFunction)
            continue;
        String diagnostics;
        SlangResult result =
            builder.loadDeviceLibrary(deviceLibraryContents, libraryScope.library, diagnostics);
        if (SLANG_FAILED(result))
        {
            if (diagnostics.getLength())
                codeGenContext->getSink()->diagnoseRaw(
                    Severity::Error,
                    diagnostics.getUnownedSlice());
            return _requireBuilderOperation(
                codeGenContext,
                "selected device-library parsing",
                result);
        }
        break;
    }

    // Capability queries are pure. Complete this exact typed preflight before module creation so
    // an unsupported overload cannot leave partial provider state behind.
    for (const auto& planned : requirements.emissionPlan.namedIntrinsics)
    {
        const auto intrinsic = planned.getDesc();
        if (!(planned.isDeviceLibraryFunction
                  ? builder.supportsDeviceLibraryFunction(libraryScope.library, intrinsic)
                  : builder.supportsNamedIntrinsic(intrinsic)))
        {
            String name(UnownedStringSlice(intrinsic.name, intrinsic.nameSize));
            return _requireBuilderOperation(
                codeGenContext,
                name.getBuffer(),
                SLANG_E_NOT_AVAILABLE);
        }
    }
    // The optional provider owns private storage layout. Retain its checked alignment locally;
    // selecting a provider must not mutate the already validated logical address plan.
    uint32_t hitObjectStorageAlignment = 0;
    bool requiresHitObjectStorage = requirements.emissionPlan.hitObjectOperations.getCount() != 0;
    for (const auto& storage : requirements.emissionPlan.localStorage)
        requiresHitObjectStorage |= isNVVMHitObjectStorageType(storage.valueType);
    for (auto function : requirements.emissionPlan.functions)
        for (UInt i = 0; i < function->getParamCount(); ++i)
            requiresHitObjectStorage |=
                asNVVMHitObjectPointerType(function->getParamType(i)) != nullptr;
    if (requiresHitObjectStorage)
    {
        uint32_t storageSize = 0, storageAlignment = 0;
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "HitObject storage ABI",
            builder.getHitObjectStorageLayout(storageSize, storageAlignment)));
        if (!storageSize || !storageAlignment || (storageAlignment & (storageAlignment - 1)) ||
            storageSize % storageAlignment)
            return _requireBuilderOperation(
                codeGenContext,
                "HitObject storage ABI",
                SLANG_E_INVALID_ARG);
        hitObjectStorageAlignment = storageAlignment;
        for (const auto& operation : requirements.emissionPlan.hitObjectOperations)
            if (!builder.supportsHitObjectOperation(operation.desc))
                return _requireBuilderOperation(
                    codeGenContext,
                    "OptiX HitObject operation",
                    SLANG_E_NOT_AVAILABLE);
    }
    if (requirements.emissionPlan.callables.getCount() && !builder.supportsCallable())
        return _requireBuilderOperation(codeGenContext, "OptiX callable", SLANG_E_NOT_AVAILABLE);
    for (const auto& transform : requirements.emissionPlan.instanceTransforms)
    {
        if (!(transform.handle ? builder.supportsInstanceTransform(transform.desc)
                               : builder.supportsCurrentTransform(transform.desc)))
            return _requireBuilderOperation(
                codeGenContext,
                "OptiX transform row",
                SLANG_E_NOT_AVAILABLE);
    }
    for (const auto& trace : requirements.emissionPlan.traceRays)
    {
        if (!builder.supportsTraceRay(trace.desc))
            return _requireBuilderOperation(
                codeGenContext,
                "OptiX register trace",
                SLANG_E_NOT_AVAILABLE);
    }
    for (const auto& requirement : requirements.valueOperations)
    {
        if (!builder.supportsValueOperation(requirement.getDesc()))
        {
            return _requireBuilderOperation(
                codeGenContext,
                requirement.diagnosticName,
                SLANG_E_NOT_AVAILABLE);
        }
    }
    for (const auto& requirement : requirements.atomicOperations)
    {
        if (!builder.supportsAtomicOperation(requirement.desc))
        {
            return _requireBuilderOperation(
                codeGenContext,
                requirement.diagnosticName,
                SLANG_E_NOT_AVAILABLE);
        }
    }
    for (const auto& load : requirements.emissionPlan.loads)
    {
        if (load.isScoped && !builder.supportsMemoryOperation(load.memoryOperation))
            return _requireBuilderOperation(
                codeGenContext,
                "scoped memory load",
                SLANG_E_NOT_AVAILABLE);
    }
    for (const auto& store : requirements.emissionPlan.stores)
    {
        if (store.isScoped && !builder.supportsMemoryOperation(store.memoryOperation))
            return _requireBuilderOperation(
                codeGenContext,
                "scoped memory store",
                SLANG_E_NOT_AVAILABLE);
    }
    for (const auto& requirement : requirements.surfaceOperations)
    {
        if (!builder.supportsSurfaceOperation(requirement.desc))
        {
            return _requireBuilderOperation(
                codeGenContext,
                requirement.diagnosticName,
                SLANG_E_NOT_AVAILABLE);
        }
    }
    for (const auto& requirement : requirements.textureOperations)
    {
        for (uint32_t i = 0; i < requirement.operationCount; ++i)
        {
            if (!builder.supportsTextureOperation(requirement.operations[i]))
            {
                return _requireBuilderOperation(
                    codeGenContext,
                    requirement.diagnosticName,
                    SLANG_E_NOT_AVAILABLE);
            }
        }
    }

    IRFunc* entryPoint = linkedIR.entryPoints[0];
    auto entryPointDecoration = entryPoint->findDecoration<IREntryPointDecoration>();
    SLANG_RELEASE_ASSERT(entryPointDecoration);

    const List<IRFunc*>& functions = requirements.emissionPlan.functions;
    const List<String>& functionNames = requirements.emissionPlan.functionNames;
    SLANG_RELEASE_ASSERT(functions.getCount() && functions[0] == entryPoint);
    SLANG_RELEASE_ASSERT(functionNames.getCount() == functions.getCount());

    NVVMEmissionPlanIndex planIndex;
    planIndex.initialize(requirements.emissionPlan);

    ScopedNVVMModule moduleScope;
    moduleScope.builder = &builder;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "module creation",
        builder.createModule(toSlice("slang-direct-nvvm"), moduleScope.module)));

    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "OptiX target configuration",
        builder.setOptixVersion(moduleScope.module, requirements.optixVersion)));
    NVVMTypeLoweringContext typeContext(codeGenContext, builder, moduleScope.module);
    Dictionary<IRFunc*, SlangNVVMValueHandle> functionMap;
    NVVMValueMap valueMap;
    NVVMValueMap helperValueMap;
    NVVMValueMap entryAggregatePointerMap;
    NVVMGlobalUserPointerSet globalUserPointers;
    Dictionary<IRBlock*, SlangNVVMBlockHandle> blockMap;

    // The canonical global owns storage class, value type, extent, and name. Lower those facts once
    // before any function declaration; ordinary body uses then resolve through the shared value
    // map.
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        NVVMConventionalGlobalParams globalParams;
        if (_getNVVMConventionalGlobalParams(globalInst, globalParams))
        {
            SlangNVVMTypeHandle loweredStructType = nullptr;
            SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                globalParams.elementType,
                NVVMTypeUse::Storage,
                loweredStructType));
            SlangNVVMValueHandle loweredStorage = nullptr;
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "conventional global parameter storage declaration",
                builder.declareGlobalStorage(
                    moduleScope.module,
                    loweredStructType,
                    SLANG_NVVM_LINKAGE_EXTERNAL,
                    SLANG_NVVM_ADDRESS_SPACE_CONSTANT,
                    kNVVMPointerAlignment,
                    toSlice("SLANG_globalParams"),
                    loweredStorage)));
            valueMap[globalParams.globalParam] = loweredStorage;
            continue;
        }

        NVVMSharedGlobal sharedGlobal;
        if (!getNVVMSupportedSharedGlobal(globalInst, &sharedGlobal))
            continue;
        auto globalVar = sharedGlobal.globalVar;
        IRType* sharedStorageType = sharedGlobal.storageType;

        SlangNVVMTypeHandle loweredStorageType = nullptr;
        SLANG_RETURN_ON_FAIL(
            typeContext.lowerType(sharedStorageType, NVVMTypeUse::Value, loweredStorageType));
        SlangNVVMValueHandle loweredStorage = nullptr;
        const String storageName = _getNVVMSharedGlobalName(linkedIR.module, globalVar);
        SLANG_RELEASE_ASSERT(storageName.getLength());
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "shared global storage declaration",
            builder.declareGlobalStorage(
                moduleScope.module,
                loweredStorageType,
                SLANG_NVVM_LINKAGE_INTERNAL,
                SLANG_NVVM_ADDRESS_SPACE_SHARED,
                _getNVVMExecutableValueAlignment(sharedGlobal.alignmentType),
                storageName.getUnownedSlice(),
                loweredStorage)));
        valueMap[globalVar] = loweredStorage;
    }

    // Every function is declared before any body is emitted. A call can therefore target a helper
    // that appears later in linked-IR order without turning physical order into a legality rule.
    for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
    {
        IRFunc* function = functions[functionIndex];
        const bool isEntryPoint = function == entryPoint;
        const bool usesKernelParameters =
            isEntryPoint &&
            entryPoint->findDecoration<IREntryPointDecoration>()->getProfile().getStage() !=
                Stage::Callable;
        SlangNVVMTypeHandle resultType = nullptr;
        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
            function->getResultType(),
            isEntryPoint ? NVVMTypeUse::EntryPointResult : NVVMTypeUse::HelperResult,
            resultType));

        List<SlangNVVMTypeHandle> parameterTypes;
        for (auto param : function->getParams())
        {
            SlangNVVMTypeHandle parameterType = nullptr;
            if (auto numeric = requirements.emissionPlan.entryValueParameters.tryGetValue(param))
            {
                SlangNVVMTypeHandle storageType = nullptr;
                SLANG_RETURN_ON_FAIL(
                    typeContext.lowerCUDAValueType(*numeric, parameterType, storageType));
            }
            else
            {
                SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                    param->getDataType(),
                    usesKernelParameters ? NVVMTypeUse::EntryPointParameter
                                         : NVVMTypeUse::HelperParameter,
                    parameterType));
            }
            parameterTypes.add(parameterType);
        }

        SlangNVVMTypeHandle functionType = nullptr;
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "function type",
            builder.getFunctionType(
                moduleScope.module,
                resultType,
                parameterTypes.getCount() ? parameterTypes.getBuffer() : nullptr,
                size_t(parameterTypes.getCount()),
                functionType)));

        SlangNVVMValueHandle loweredFunction = nullptr;
        const bool isExported =
            isEntryPoint || function->findDecorationImpl(kIROp_CudaDeviceExportDecoration);
        const SlangNVVMLinkage linkage =
            isExported ? SLANG_NVVM_LINKAGE_EXTERNAL : SLANG_NVVM_LINKAGE_INTERNAL;
        SlangNVVMFunctionFlags flags = SLANG_NVVM_FUNCTION_FLAG_NONE;
        if (!isEntryPoint && function->findDecoration<IRNoInlineDecoration>())
            flags |= SLANG_NVVM_FUNCTION_FLAG_NO_INLINE;
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "function declaration",
            builder.declareFunction(
                moduleScope.module,
                functionType,
                linkage,
                flags,
                functionNames[functionIndex].getUnownedSlice(),
                loweredFunction)));
        if (usesKernelParameters)
        {
            size_t parameterIndex = 0;
            for (auto parameter : function->getParams())
            {
                auto numeric =
                    requirements.emissionPlan.entryValueParameters.tryGetValue(parameter);
                if (numeric && numeric->isByValue())
                {
                    SlangNVVMTypeHandle parameterType = nullptr;
                    SlangNVVMTypeHandle storageType = nullptr;
                    SLANG_RETURN_ON_FAIL(
                        typeContext.lowerCUDAValueType(*numeric, parameterType, storageType));
                    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                        codeGenContext,
                        "numeric entry byval attributes",
                        builder.setFunctionParameterAttributes(
                            moduleScope.module,
                            loweredFunction,
                            parameterIndex,
                            SLANG_NVVM_PARAMETER_FLAG_BY_VALUE,
                            storageType,
                            numeric->alignment)));
                }
                else if (isNVVMSupportedResourceAggregateType(parameter->getDataType()))
                {
                    SlangNVVMTypeHandle aggregateType = nullptr;
                    SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                        parameter->getDataType(),
                        NVVMTypeUse::Value,
                        aggregateType));
                    uint32_t alignment = 0;
                    SLANG_RELEASE_ASSERT(_getNVVMByValueParameterAlignment(
                        codeGenContext,
                        parameter->getDataType(),
                        alignment));
                    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                        codeGenContext,
                        "by-value aggregate parameter attributes",
                        builder.setFunctionParameterAttributes(
                            moduleScope.module,
                            loweredFunction,
                            parameterIndex,
                            SLANG_NVVM_PARAMETER_FLAG_BY_VALUE,
                            aggregateType,
                            alignment)));
                }
                ++parameterIndex;
            }
        }
        functionMap[function] = loweredFunction;
    }

    for (auto function : functions)
    {
        size_t parameterIndex = 0;
        for (auto param : function->getParams())
        {
            SlangNVVMValueHandle parameter = nullptr;
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "function parameter",
                builder.getFunctionParameter(
                    moduleScope.module,
                    functionMap.getValue(function),
                    parameterIndex,
                    parameter)));
            valueMap[param] = parameter;
            if (function == entryPoint &&
                isNVVMSupportedResourceAggregateType(param->getDataType()) &&
                !requirements.emissionPlan.entryValueParameters.containsKey(param))
            {
                entryAggregatePointerMap[param] = parameter;
            }
            if (function == entryPoint &&
                !requirements.emissionPlan.entryValueParameters.containsKey(param) &&
                asNVVMSupportedDeviceCopyableValuePointerType(param->getDataType()))
            {
                globalUserPointers.add(param);
            }
            ++parameterIndex;
        }
    }

    for (auto function : functions)
    {
        // LLVM branches can refer to blocks declared later, so create this function's complete CFG
        // before emitting any body instruction.
        Index blockIndex = 0;
        for (auto block : function->getBlocks())
        {
            StringBuilder nameBuilder;
            if (blockIndex == 0)
                nameBuilder << "entry";
            else
                nameBuilder << "block" << blockIndex;
            String blockName = nameBuilder.produceString();

            SlangNVVMBlockHandle loweredBlock = nullptr;
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "basic-block creation",
                builder.createBlock(
                    moduleScope.module,
                    functionMap.getValue(function),
                    blockName.getUnownedSlice(),
                    loweredBlock)));
            blockMap[block] = loweredBlock;
            ++blockIndex;
        }

        IRBlock* entryBlock = function->getFirstBlock();
        bool hasHalfParameter = false;
        const bool hasEntryValueParameters =
            function == entryPoint &&
            requirements.emissionPlan.entryValueParameters.getCount() != 0;
        bool hasEntryAggregateValueParameter = false;
        for (auto param : function->getParams())
        {
            hasHalfParameter =
                hasHalfParameter ||
                (function != entryPoint && hasNVVMHalfHelperABITransport(param->getDataType()));
            hasEntryAggregateValueParameter =
                hasEntryAggregateValueParameter ||
                (function == entryPoint &&
                 isNVVMSupportedResourceAggregateType(param->getDataType()) &&
                 !requirements.emissionPlan.entryValueParameters.containsKey(param));
        }
        if (hasHalfParameter || hasEntryAggregateValueParameter || hasEntryValueParameters)
        {
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                hasHalfParameter ? "Half helper ABI entry-block selection"
                                 : "aggregate entry-parameter block selection",
                builder.setInsertBlock(moduleScope.module, blockMap.getValue(entryBlock))));
        }
        if (hasEntryValueParameters)
        {
            for (auto param : function->getParams())
            {
                auto numeric = requirements.emissionPlan.entryValueParameters.tryGetValue(param);
                if (!numeric)
                    continue;
                SlangNVVMValueHandle decoded = nullptr;
                SLANG_RETURN_ON_FAIL(_emitNVVMCUDAValueDecode(
                    codeGenContext,
                    builder,
                    moduleScope.module,
                    typeContext,
                    *numeric,
                    valueMap.getValue(param),
                    decoded));
                valueMap[param] = decoded;
            }
        }
        if (hasHalfParameter)
        {
            for (auto param : function->getParams())
            {
                if (!hasNVVMHalfHelperABITransport(param->getDataType()))
                    continue;
                SlangNVVMValueHandle loweredValue = nullptr;
                SLANG_RETURN_ON_FAIL(_emitNVVMHalfHelperABIReinterpretation(
                    codeGenContext,
                    builder,
                    moduleScope.module,
                    typeContext,
                    param->getDataType(),
                    false,
                    valueMap.getValue(param),
                    loweredValue));
                valueMap[param] = loweredValue;
            }
        }
        if (hasEntryAggregateValueParameter)
        {
            // Consider `kernel(uniform Params params) { helper(params); }`, where `Params`
            // contains a resource. NVPTX exposes `params` as the physical `byval` pointer created
            // above, but `helper` takes the ordinary first-class struct. Load that semantic value
            // once while retaining the pointer in `entryAggregatePointerMap` for direct field
            // addressing.
            for (auto param : function->getParams())
            {
                if (!isNVVMSupportedResourceAggregateType(param->getDataType()) ||
                    requirements.emissionPlan.entryValueParameters.containsKey(param))
                    continue;

                SlangNVVMValueHandle loweredPointer = valueMap.getValue(param);
                uint32_t alignment = 0;
                SLANG_RELEASE_ASSERT(_getNVVMByValueParameterAlignment(
                    codeGenContext,
                    param->getDataType(),
                    alignment));
                SlangNVVMValueHandle loweredValue = nullptr;
                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                    codeGenContext,
                    "by-value aggregate parameter load",
                    builder.emitLoad(
                        moduleScope.module,
                        loweredPointer,
                        alignment,
                        SLANG_NVVM_LOAD_FLAG_INVARIANT,
                        loweredValue)));
                valueMap[param] = loweredValue;
            }
        }
        // Consider the loop header header(i, sum). Its phis must exist before the compare and body
        // use them, while their backedge values are not emitted until later blocks. Create every
        // phi placeholder now; incoming pairs are attached after all bodies and terminators exist.
        for (auto block : function->getBlocks())
        {
            if (block == entryBlock)
                continue;

            for (auto param : block->getParams())
            {
                SlangNVVMTypeHandle parameterType = nullptr;
                SLANG_RETURN_ON_FAIL(
                    typeContext.lowerType(param->getDataType(), NVVMTypeUse::Value, parameterType));
                SlangNVVMValueHandle loweredPhi = nullptr;
                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                    codeGenContext,
                    "value phi",
                    builder.emitPhi(
                        moduleScope.module,
                        blockMap.getValue(block),
                        parameterType,
                        loweredPhi)));
                valueMap[param] = loweredPhi;
            }
        }

        RefPtr<IRDominatorTree> dominatorTree = computeDominatorTree(function);
        List<IRBlock*> bodyOrder = _getNVVMBodyOrder(function, dominatorTree);
        for (auto block : bodyOrder)
        {
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "insertion-block selection",
                builder.setInsertBlock(moduleScope.module, blockMap.getValue(block))));

            for (auto inst : block->getOrdinaryInsts())
            {
                switch (inst->getOp())
                {
                case kIROp_Var:
                    {
                        const auto storage = planIndex.findLocalStorage(inst);
                        SLANG_RELEASE_ASSERT(storage);
                        SlangNVVMTypeHandle loweredValueType = nullptr;
                        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                            storage->valueType,
                            storage->valueUse,
                            loweredValueType));
                        SlangNVVMValueHandle loweredStorage = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            storage->valueUse == NVVMTypeUse::ParameterGroupStorage
                                ? "local physical parameter-group storage"
                                : "local copyable storage",
                            builder.emitLocalStorage(
                                moduleScope.module,
                                loweredValueType,
                                isNVVMHitObjectStorageType(storage->valueType)
                                    ? hitObjectStorageAlignment
                                    : storage->alignment,
                                toSlice("slangLocal"),
                                loweredStorage)));
                        valueMap[inst] = loweredStorage;
                    }
                    break;

                case kIROp_OptixCallShader:
                    {
                        const auto call = planIndex.findCallable(inst);
                        SLANG_RELEASE_ASSERT(call);
                        SlangNVVMValueHandle index = nullptr, payload = nullptr, result = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            call->index,
                            valueMap,
                            typeContext,
                            index));
                        if (call->payload)
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                call->payload,
                                valueMap,
                                typeContext,
                                payload));
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "OptiX callable",
                            builder.emitCallable(moduleScope.module, index, payload, result)));
                        if (result)
                            valueMap[inst] = result;
                    }
                    break;

                case kIROp_AllocateOpaqueHandle:
                case kIROp_OptixHitObjectMakeNop:
                case kIROp_OptixHitObjectMakeMiss:
                case kIROp_OptixCurrentHitQuery:
                case kIROp_OptixHitObjectQuery:
                case kIROp_OptixHitObjectSetSbt:
                case kIROp_OptixHitObjectLoadSbt:
                case kIROp_OptixHitObjectReorder:
                case kIROp_OptixReorderHint:
                case kIROp_OptixHitObjectPayload:
                    {
                        const auto operation = planIndex.findHitObjectOperation(inst);
                        SLANG_RELEASE_ASSERT(operation);
                        List<SlangNVVMValueHandle> operands;
                        for (auto operand : operation->operands)
                        {
                            SlangNVVMValueHandle value = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                operand,
                                valueMap,
                                typeContext,
                                value));
                            operands.add(value);
                        }
                        SlangNVVMValueHandle value = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "OptiX HitObject operation",
                            builder.emitHitObjectOperation(
                                moduleScope.module,
                                operation->desc,
                                operands.getBuffer(),
                                operands.getCount(),
                                value)));
                        if (value)
                            valueMap[inst] = value;
                    }
                    break;

                case kIROp_OptixCurrentTransformRow:
                case kIROp_OptixInstanceTransformRow:
                    {
                        const auto transform = planIndex.findInstanceTransform(inst);
                        SLANG_RELEASE_ASSERT(transform);
                        SlangNVVMValueHandle handle = nullptr, value = nullptr;
                        if (transform->handle)
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                transform->handle,
                                valueMap,
                                typeContext,
                                handle));
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "OptiX transform row",
                            transform->handle ? builder.emitInstanceTransform(
                                                    moduleScope.module,
                                                    transform->desc,
                                                    &handle,
                                                    1,
                                                    value)
                                              : builder.emitCurrentTransform(
                                                    moduleScope.module,
                                                    transform->desc,
                                                    nullptr,
                                                    0,
                                                    value)));
                        valueMap[inst] = value;
                    }
                    break;

                case kIROp_OptixTraceRayPayload:
                case kIROp_GetOptiXPayloadRegister:
                case kIROp_SetOptiXPayloadRegister:
                case kIROp_GetOptiXHitAttribute:
                    {
                        const auto trace = planIndex.findTraceRay(inst);
                        const auto primitive = planIndex.findNamedIntrinsic(inst);
                        SLANG_RELEASE_ASSERT(trace || primitive);
                        List<SlangNVVMValueHandle> operands;
                        for (auto operand : trace ? trace->operands : primitive->operandValues)
                        {
                            SlangNVVMValueHandle value = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                operand,
                                valueMap,
                                typeContext,
                                value));
                            operands.add(value);
                        }
                        SlangNVVMValueHandle value = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "OptiX register operation",
                            trace ? builder.emitTraceRay(
                                        moduleScope.module,
                                        trace->desc,
                                        operands.getBuffer(),
                                        size_t(operands.getCount()),
                                        value)
                                  : builder.emitNamedIntrinsic(
                                        moduleScope.module,
                                        primitive->getDesc(),
                                        operands.getBuffer(),
                                        size_t(operands.getCount()),
                                        value)));
                        if (inst->getDataType()->getOp() != kIROp_VoidType)
                            valueMap[inst] = value;
                    }
                    break;

                case kIROp_GetOptiXSbtDataPtr:
                    {
                        const auto primitive = planIndex.findNamedIntrinsic(inst);
                        SLANG_RELEASE_ASSERT(primitive);
                        SlangNVVMValueHandle bits = nullptr, pointer = nullptr;
                        SlangNVVMTypeHandle type = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "OptiX SBT address",
                            builder.emitNamedIntrinsic(
                                moduleScope.module,
                                primitive->getDesc(),
                                nullptr,
                                0,
                                bits)));
                        SLANG_RETURN_ON_FAIL(
                            typeContext.lowerType(inst->getDataType(), NVVMTypeUse::Value, type));
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "OptiX SBT pointer",
                            builder.emitBitCast(moduleScope.module, type, bits, pointer)));
                        valueMap[inst] = pointer;
                    }
                    break;

                case kIROp_LoadFromUninitializedMemory:
                case kIROp_GetStringHash:
                case kIROp_DebugNoScope:
                    {
                        const auto value = planIndex.findEphemeralValue(inst);
                        SLANG_RELEASE_ASSERT(value);
                        if (value->kind == NVVMPlannedEphemeralValueKind::IgnoredDebugNoScope)
                            break;

                        SlangNVVMValueHandle loweredValue = nullptr;
                        if (value->kind == NVVMPlannedEphemeralValueKind::ChosenUndefined)
                        {
                            SLANG_RETURN_ON_FAIL(_emitNVVMChosenUndefinedValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                value->valueType,
                                typeContext,
                                loweredValue));
                        }
                        else
                        {
                            SLANG_RELEASE_ASSERT(
                                value->kind == NVVMPlannedEphemeralValueKind::StableStringHash);
                            SlangNVVMTypeHandle loweredType = nullptr;
                            SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                                value->valueType,
                                NVVMTypeUse::Value,
                                loweredType));
                            const UnownedStringSlice string =
                                value->stringLiteral->getStringSlice();
                            const uint32_t hashBits =
                                getStableHashCode32(string.begin(), string.getLength()).hash;
                            const int64_t hash = hashBits >= (uint64_t(1) << 31)
                                                     ? int64_t(hashBits) - (int64_t(1) << 32)
                                                     : int64_t(hashBits);
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "stable literal string hash",
                                builder.getIntegerConstant(
                                    moduleScope.module,
                                    loweredType,
                                    hash,
                                    loweredValue)));
                        }
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_Load:
                    {
                        const auto load = planIndex.findLoad(inst);
                        SLANG_RELEASE_ASSERT(load);
                        SlangNVVMValueHandle loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            load->pointer,
                            valueMap,
                            typeContext,
                            loweredPointer));
                        if (load->cudaValueLayout.type)
                        {
                            SlangNVVMValueHandle result = nullptr;
                            SLANG_RETURN_ON_FAIL(_emitNVVMCUDAValueDecode(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                typeContext,
                                load->cudaValueLayout,
                                loweredPointer,
                                result));
                            valueMap[inst] = result;
                            break;
                        }
                        if (load->layoutStorage.valueType)
                        {
                            SlangNVVMValueHandle result = nullptr;
                            SLANG_RETURN_ON_FAIL(_emitNVVMLayoutMemory(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                typeContext,
                                load->layoutStorage,
                                load->layoutBoolConversion,
                                true,
                                loweredPointer,
                                nullptr,
                                result));
                            valueMap[inst] = result;
                            break;
                        }
                        SlangNVVMValueHandle loweredValue = nullptr;
                        if (load->isScoped)
                        {
                            const SlangNVVMValueHandle operands[] = {loweredPointer};
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "scoped memory load",
                                builder.emitMemoryOperation(
                                    moduleScope.module,
                                    load->memoryOperation,
                                    operands,
                                    1,
                                    loweredValue)));
                        }
                        else
                        {
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "value load",
                                builder.emitLoad(
                                    moduleScope.module,
                                    loweredPointer,
                                    load->alignment,
                                    load->flags,
                                    loweredValue)));
                        }
                        SlangNVVMValueHandle semanticValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_emitNVVMPlannedStorageConversion(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            typeContext,
                            requirements.emissionPlan.structuredConversions,
                            load->conversion,
                            true,
                            loweredValue,
                            semanticValue));
                        if (load->isGlobalUserPointer)
                            globalUserPointers.add(inst);
                        valueMap[inst] = semanticValue;
                    }
                    break;

                case kIROp_Store:
                    {
                        const auto store = planIndex.findStore(inst);
                        SLANG_RELEASE_ASSERT(store);
                        SlangNVVMValueHandle loweredValue = nullptr;
                        if (store->usesHelperPointerValue)
                        {
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMHelperValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                store->value,
                                valueMap,
                                globalUserPointers,
                                helperValueMap,
                                typeContext,
                                loweredValue));
                        }
                        else
                        {
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                store->value,
                                valueMap,
                                typeContext,
                                loweredValue));
                        }
                        if (store->layoutStorage.valueType)
                        {
                            SlangNVVMValueHandle pointer = nullptr, unused = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                store->pointer,
                                valueMap,
                                typeContext,
                                pointer));
                            SLANG_RETURN_ON_FAIL(_emitNVVMLayoutMemory(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                typeContext,
                                store->layoutStorage,
                                store->layoutBoolConversion,
                                false,
                                pointer,
                                loweredValue,
                                unused));
                            break;
                        }
                        SlangNVVMValueHandle storageValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_emitNVVMPlannedStorageConversion(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            typeContext,
                            requirements.emissionPlan.structuredConversions,
                            store->conversion,
                            false,
                            loweredValue,
                            storageValue));
                        SlangNVVMValueHandle loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            store->pointer,
                            valueMap,
                            typeContext,
                            loweredPointer));
                        if (store->isScoped)
                        {
                            const SlangNVVMValueHandle operands[] = {loweredPointer, storageValue};
                            SlangNVVMValueHandle unused = nullptr;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "scoped memory store",
                                builder.emitMemoryOperation(
                                    moduleScope.module,
                                    store->memoryOperation,
                                    operands,
                                    2,
                                    unused)));
                        }
                        else
                        {
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "value store",
                                builder.emitStore(
                                    moduleScope.module,
                                    storageValue,
                                    loweredPointer,
                                    store->alignment)));
                        }
                    }
                    break;

                case kIROp_SwizzledStore:
                    {
                        NVVMVectorSwizzledStore store;
                        SLANG_RELEASE_ASSERT(_getNVVMVectorSwizzledStore(inst, store));
                        SlangNVVMValueHandle loweredDestination = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            store.destination,
                            valueMap,
                            typeContext,
                            loweredDestination));
                        SlangNVVMValueHandle loweredSource = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            store.source,
                            valueMap,
                            typeContext,
                            loweredSource));
                        SlangNVVMTypeHandle loweredElementType = nullptr;
                        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                            store.elementType,
                            NVVMTypeUse::Value,
                            loweredElementType));
                        SlangNVVMTypeHandle loweredIndexType = nullptr;
                        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                            cast<IRSwizzledStore>(inst)->getElementIndex(0)->getDataType(),
                            NVVMTypeUse::Value,
                            loweredIndexType));

                        for (uint32_t sourceIndex = 0; sourceIndex < store.sourceElementCount;
                             ++sourceIndex)
                        {
                            SlangNVVMValueHandle loweredSourceElement = loweredSource;
                            if (store.sourceElementCount > 1)
                            {
                                SlangNVVMValueHandle loweredSourceIndex = nullptr;
                                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                    codeGenContext,
                                    "numeric vector swizzled-store source index",
                                    builder.getIntegerConstant(
                                        moduleScope.module,
                                        loweredIndexType,
                                        sourceIndex,
                                        loweredSourceIndex)));
                                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                    codeGenContext,
                                    "numeric vector swizzled-store extraction",
                                    builder.emitSequentialElementExtract(
                                        moduleScope.module,
                                        loweredSource,
                                        loweredSourceIndex,
                                        loweredSourceElement)));
                            }

                            SlangNVVMValueHandle loweredByteOffset = nullptr;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "numeric vector swizzled-store byte offset",
                                builder.getIntegerConstant(
                                    moduleScope.module,
                                    loweredIndexType,
                                    int64_t(store.destinationIndices[sourceIndex] * 4),
                                    loweredByteOffset)));
                            SlangNVVMValueHandle loweredElementPointer = nullptr;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "numeric vector swizzled-store element pointer",
                                builder.emitByteOffsetPointer(
                                    moduleScope.module,
                                    loweredDestination,
                                    loweredByteOffset,
                                    loweredElementType,
                                    loweredElementPointer)));
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "numeric vector swizzled-store element",
                                builder.emitStore(
                                    moduleScope.module,
                                    loweredSourceElement,
                                    loweredElementPointer,
                                    kNVVMScalar32Alignment)));
                        }
                    }
                    break;

                case kIROp_NVVMSurfaceLoad:
                case kIROp_NVVMSurfaceStore:
                    {
                        const auto operation = planIndex.findSurfaceOperation(inst);
                        SLANG_RELEASE_ASSERT(operation);

                        IRInst* semanticOperands[] = {
                            operation->surface,
                            operation->coordinate,
                            operation->value,
                        };
                        const size_t operandCount = operation->value ? 3 : 2;
                        SlangNVVMValueHandle loweredOperands[3] = {};
                        for (size_t operandIndex = 0; operandIndex < operandCount; ++operandIndex)
                        {
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                semanticOperands[operandIndex],
                                valueMap,
                                typeContext,
                                loweredOperands[operandIndex]));
                        }
                        SlangNVVMValueHandle loweredResult = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            operation->diagnosticName,
                            builder.emitSurfaceOperation(
                                moduleScope.module,
                                operation->desc,
                                loweredOperands,
                                operandCount,
                                loweredResult)));
                        if (!operation->value)
                            valueMap[inst] = loweredResult;
                    }
                    break;

                case kIROp_MakeUInt64:
                    {
                        const auto construction = planIndex.findUInt64WordConstruction(inst);
                        SLANG_RELEASE_ASSERT(construction);
                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_emitNVVMUInt64WordConstruction(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            *construction,
                            valueMap,
                            typeContext,
                            loweredValue));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_DefaultConstruct:
                    {
                        const auto defaultValue = planIndex.findDefaultResourceValue(inst);
                        SLANG_RELEASE_ASSERT(defaultValue);
                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_emitNVVMDefaultResourceValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            *defaultValue,
                            typeContext,
                            loweredValue));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_Add:
                case kIROp_Sub:
                case kIROp_Mul:
                case kIROp_Fma:
                case kIROp_Div:
                case kIROp_IRem:
                case kIROp_Lsh:
                case kIROp_Rsh:
                case kIROp_BitAnd:
                case kIROp_BitOr:
                case kIROp_BitXor:
                case kIROp_BitNot:
                case kIROp_And:
                case kIROp_Or:
                case kIROp_Not:
                case kIROp_Neg:
                case kIROp_Less:
                case kIROp_Eql:
                case kIROp_Neq:
                case kIROp_Greater:
                case kIROp_Leq:
                case kIROp_Geq:
                case kIROp_IntCast:
                case kIROp_CastIntToFloat:
                case kIROp_CastFloatToInt:
                case kIROp_FloatCast:
                case kIROp_Select:
                case kIROp_WaveGetConvergedMask:
                case kIROp_WaveMaskBallot:
                case kIROp_WaveMaskMatch:
                    {
                        const auto truthiness = planIndex.findNumericTruthiness(inst);
                        if (truthiness)
                        {
                            SlangNVVMValueHandle loweredValue = nullptr;
                            SLANG_RETURN_ON_FAIL(_emitNVVMNumericTruthiness(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                *truthiness,
                                valueMap,
                                typeContext,
                                loweredValue));
                            valueMap[inst] = loweredValue;
                            break;
                        }
                        const auto plannedOperation = planIndex.findValueOperation(inst);
                        SLANG_RELEASE_ASSERT(plannedOperation);
                        const NVVMValueOperationRequirement& operation =
                            plannedOperation->operation;
                        SlangNVVMValueHandle loweredOperands[3] = {};
                        for (UInt operandIndex = 0; operandIndex < inst->getOperandCount();
                             ++operandIndex)
                        {
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                inst->getOperand(operandIndex),
                                valueMap,
                                typeContext,
                                loweredOperands[operandIndex]));
                        }

                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            operation.diagnosticName,
                            builder.emitValueOperation(
                                moduleScope.module,
                                operation.getDesc(),
                                inst->getOperandCount() ? loweredOperands : nullptr,
                                inst->getOperandCount(),
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_FRem:
                    {
                        const auto operation = planIndex.findFloatingRemainder(inst);
                        SLANG_RELEASE_ASSERT(operation);
                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_emitNVVMFloatingRemainderOperation(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            *operation,
                            valueMap,
                            typeContext,
                            loweredValue));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_CombinedTextureSamplerGetSampler:
                case kIROp_CastDescriptorHandleToResource:
                case kIROp_CastResourceToDescriptorHandle:
                case kIROp_CastUInt64ToDescriptorHandle:
                case kIROp_CastDescriptorHandleToUInt64:
                    {
                        IRInst* value = nullptr;
                        SLANG_RELEASE_ASSERT(_getNVVMDescriptorHandleConversion(inst, value));
                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            value,
                            valueMap,
                            typeContext,
                            loweredValue));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_CastPtrToInt:
                    {
                        auto value =
                            requirements.emissionPlan.pointerToIntegerValues.tryGetValue(inst);
                        SLANG_RELEASE_ASSERT(value);
                        SlangNVVMValueHandle loweredPointer = nullptr, loweredResult = nullptr;
                        SlangNVVMTypeHandle integerType = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            *value,
                            valueMap,
                            typeContext,
                            loweredPointer));
                        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                            inst->getDataType(),
                            NVVMTypeUse::Value,
                            integerType));
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "pointer address conversion",
                            builder.emitBitCast(
                                moduleScope.module,
                                integerType,
                                loweredPointer,
                                loweredResult)));
                        valueMap[inst] = loweredResult;
                    }
                    break;

                case kIROp_PtrCast:
                    {
                        auto source =
                            requirements.emissionPlan.pointerQualificationValues.tryGetValue(inst);
                        SLANG_RELEASE_ASSERT(source);

                        SlangNVVMValueHandle value = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMHelperValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            *source,
                            valueMap,
                            globalUserPointers,
                            helperValueMap,
                            typeContext,
                            value));
                        // Preflight proved identical pointees, access and physical address space.
                        // Qualification preserves the selected generic or shared LLVM pointer.
                        valueMap[inst] = value;
                    }
                    break;

                case kIROp_CastIntToPtr:
                case kIROp_BitCast:
                    {
                        const auto resourceBitCast = planIndex.findResourceBitCast(inst);
                        if (resourceBitCast)
                        {
                            SlangNVVMValueHandle loweredValue = nullptr;
                            SLANG_RETURN_ON_FAIL(_emitNVVMResourceBitCast(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                *resourceBitCast,
                                valueMap,
                                typeContext,
                                loweredValue));
                            valueMap[inst] = loweredValue;
                            break;
                        }
                        NVVMPointerConversion pointerCast;
                        if (!_getNVVMPointerConversion(inst, pointerCast))
                        {
                            const auto plannedOperation = planIndex.findValueOperation(inst);
                            SLANG_RELEASE_ASSERT(plannedOperation);
                            const NVVMValueOperationRequirement& operation =
                                plannedOperation->operation;
                            SlangNVVMValueHandle loweredOperand = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                inst->getOperand(0),
                                valueMap,
                                typeContext,
                                loweredOperand));
                            SlangNVVMValueHandle loweredValue = nullptr;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                operation.diagnosticName,
                                builder.emitValueOperation(
                                    moduleScope.module,
                                    operation.getDesc(),
                                    &loweredOperand,
                                    1,
                                    loweredValue)));
                            valueMap[inst] = loweredValue;
                            break;
                        }

                        SlangNVVMValueHandle loweredOperand = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            pointerCast.value,
                            valueMap,
                            typeContext,
                            loweredOperand));
                        SlangNVVMTypeHandle loweredResultType = nullptr;
                        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                            inst->getDataType(),
                            NVVMTypeUse::Value,
                            loweredResultType));
                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "pointer bit-pattern transport",
                            builder.emitBitCast(
                                moduleScope.module,
                                loweredResultType,
                                loweredOperand,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_AtomicLoad:
                case kIROp_AtomicStore:
                case kIROp_AtomicExchange:
                case kIROp_AtomicCompareExchange:
                case kIROp_AtomicAdd:
                case kIROp_AtomicSub:
                case kIROp_AtomicAnd:
                case kIROp_AtomicOr:
                case kIROp_AtomicXor:
                case kIROp_AtomicMin:
                case kIROp_AtomicMax:
                case kIROp_AtomicInc:
                case kIROp_AtomicDec:
                    {
                        const auto operation = planIndex.findAtomicOperation(inst);
                        SLANG_RELEASE_ASSERT(operation);
                        SlangNVVMValueHandle loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            operation->pointer,
                            valueMap,
                            typeContext,
                            loweredPointer));
                        SlangNVVMValueHandle loweredOperands[3] = {loweredPointer};
                        size_t loweredOperandCount = 1;
                        if (operation->hasImplicitValue)
                        {
                            auto pointerType =
                                cast<IRPtrTypeBase>(operation->pointer->getDataType());
                            SlangNVVMTypeHandle loweredValueType = nullptr;
                            SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                                pointerType->getValueType(),
                                NVVMTypeUse::Value,
                                loweredValueType));
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "atomic implicit value",
                                builder.getIntegerConstant(
                                    moduleScope.module,
                                    loweredValueType,
                                    operation->implicitValue,
                                    loweredOperands[loweredOperandCount])));
                            ++loweredOperandCount;
                        }
                        else
                        {
                            for (uint32_t i = 0; i < operation->valueCount; ++i)
                            {
                                SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                    codeGenContext,
                                    builder,
                                    moduleScope.module,
                                    operation->values[i],
                                    valueMap,
                                    typeContext,
                                    loweredOperands[loweredOperandCount]));
                                ++loweredOperandCount;
                            }
                        }
                        if (operation->negatesValue)
                        {
                            SlangNVVMValueHandle negatedValue = nullptr;
                            SLANG_RETURN_ON_FAIL(_emitNVVMValueRecipeStep(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                operation->valueNegation,
                                &loweredOperands[1],
                                1,
                                negatedValue));
                            loweredOperands[1] = negatedValue;
                        }
                        SlangNVVMValueHandle loweredResult = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            operation->diagnosticName,
                            builder.emitAtomicOperation(
                                moduleScope.module,
                                operation->desc,
                                loweredOperands,
                                loweredOperandCount,
                                loweredResult)));
                        if (inst->getOp() != kIROp_AtomicStore)
                            valueMap[inst] = loweredResult;
                    }
                    break;


                case kIROp_Call:
                    {
                        auto call = cast<IRCall>(inst);
                        auto callee = cast<IRFunc>(call->getOperand(0));
                        List<SlangNVVMValueHandle> loweredArguments;
                        for (UInt argumentIndex = 0; argumentIndex < call->getArgCount();
                             ++argumentIndex)
                        {
                            SlangNVVMValueHandle loweredArgument = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMHelperValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                call->getArg(argumentIndex),
                                valueMap,
                                globalUserPointers,
                                helperValueMap,
                                typeContext,
                                loweredArgument));
                            const bool hasGlobalPhysicalStorageArgument =
                                asNVVMSupportedPhysicalStorageReferencePointerType(
                                    callee->getParamType(argumentIndex),
                                    nullptr) &&
                                asNVVMSupportedParameterGroupType(
                                    call->getArg(argumentIndex)->getDataType(),
                                    nullptr);
                            if ((asNVVMSupportedHelperReferencePointerType(
                                     callee->getParamType(argumentIndex),
                                     nullptr) &&
                                 _isNVVMGlobalHelperReferenceArgument(
                                     requirements.emissionPlan.addresses,
                                     call->getArg(argumentIndex))) ||
                                hasGlobalPhysicalStorageArgument)
                            {
                                SlangNVVMTypeHandle loweredReferenceType = nullptr;
                                SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                                    callee->getParamType(argumentIndex),
                                    NVVMTypeUse::HelperParameter,
                                    loweredReferenceType));
                                SlangNVVMValueHandle genericArgument = nullptr;
                                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                    codeGenContext,
                                    "global-to-generic helper reference conversion",
                                    builder.emitPointerAddressSpaceCast(
                                        moduleScope.module,
                                        loweredReferenceType,
                                        loweredArgument,
                                        genericArgument)));
                                loweredArgument = genericArgument;
                            }
                            if (hasNVVMHalfHelperABITransport(callee->getParamType(argumentIndex)))
                            {
                                SlangNVVMValueHandle physicalArgument = nullptr;
                                SLANG_RETURN_ON_FAIL(_emitNVVMHalfHelperABIReinterpretation(
                                    codeGenContext,
                                    builder,
                                    moduleScope.module,
                                    typeContext,
                                    callee->getParamType(argumentIndex),
                                    true,
                                    loweredArgument,
                                    physicalArgument));
                                loweredArgument = physicalArgument;
                            }
                            loweredArguments.add(loweredArgument);
                        }

                        SlangNVVMValueHandle physicalValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "value call",
                            builder.emitCall(
                                moduleScope.module,
                                functionMap.getValue(callee),
                                loweredArguments.getCount() ? loweredArguments.getBuffer()
                                                            : nullptr,
                                size_t(loweredArguments.getCount()),
                                physicalValue)));
                        SlangNVVMValueHandle loweredValue = physicalValue;
                        if (hasNVVMHalfHelperABITransport(call->getDataType()))
                        {
                            SLANG_RETURN_ON_FAIL(_emitNVVMHalfHelperABIReinterpretation(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                typeContext,
                                call->getDataType(),
                                false,
                                physicalValue,
                                loweredValue));
                        }
                        valueMap[call] = loweredValue;
                    }
                    break;

                case kIROp_MakeVector:
                case kIROp_MakeVectorFromScalar:
                case kIROp_MakeArray:
                case kIROp_MakeArrayFromElement:
                case kIROp_MakeStruct:
                case kIROp_Swizzle:
                case kIROp_SwizzleSet:
                case kIROp_GetElement:
                    {
                        NVVMAggregateElement aggregateElement;
                        if (_getNVVMAggregateElement(inst, aggregateElement))
                        {
                            SlangNVVMValueHandle loweredBase = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                aggregateElement.base,
                                valueMap,
                                typeContext,
                                loweredBase));
                            SlangNVVMValueHandle loweredValue = nullptr;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "fixed aggregate element extraction",
                                builder.emitAggregateElementExtract(
                                    moduleScope.module,
                                    loweredBase,
                                    aggregateElement.index,
                                    loweredValue)));
                            valueMap[inst] = loweredValue;
                            break;
                        }

                        NVVMAggregateConstruction aggregateConstruction;
                        if (_getNVVMAggregateConstruction(inst, aggregateConstruction))
                        {
                            List<SlangNVVMValueHandle> loweredElements;
                            for (uint32_t i = 0; i < aggregateConstruction.elementCount; ++i)
                            {
                                SlangNVVMValueHandle loweredElement = nullptr;
                                SLANG_RETURN_ON_FAIL(_getLoweredNVVMHelperValue(
                                    codeGenContext,
                                    builder,
                                    moduleScope.module,
                                    inst->getOperand(
                                        aggregateConstruction.repeatsSingleElement ? 0 : i),
                                    valueMap,
                                    globalUserPointers,
                                    helperValueMap,
                                    typeContext,
                                    loweredElement));
                                if (aggregateConstruction.resultUse == NVVMTypeUse::Storage)
                                {
                                    const auto storage =
                                        planIndex.findAggregateStorageConstruction(inst);
                                    SLANG_RELEASE_ASSERT(storage);
                                    SlangNVVMValueHandle storageElement = nullptr;
                                    SLANG_RETURN_ON_FAIL(_emitNVVMStructuredBufferStorageConversion(
                                        codeGenContext,
                                        builder,
                                        moduleScope.module,
                                        typeContext,
                                        requirements.emissionPlan.structuredConversions,
                                        storage->elementRecipe,
                                        loweredElement,
                                        storageElement));
                                    loweredElement = storageElement;
                                }
                                loweredElements.add(loweredElement);
                            }
                            SlangNVVMTypeHandle loweredResultType = nullptr;
                            SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                                aggregateConstruction.resultType,
                                aggregateConstruction.resultUse,
                                loweredResultType));
                            SlangNVVMValueHandle loweredValue = nullptr;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "fixed aggregate construction",
                                builder.emitAggregateConstruct(
                                    moduleScope.module,
                                    loweredResultType,
                                    loweredElements.getBuffer(),
                                    size_t(loweredElements.getCount()),
                                    loweredValue)));
                            valueMap[inst] = loweredValue;
                            break;
                        }

                        NVVMSequentialElement element;
                        NVVMVectorConstruction construction;
                        if (_getNVVMSequentialElement(inst, element))
                        {
                            SlangNVVMValueHandle loweredBase = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                element.base,
                                valueMap,
                                typeContext,
                                loweredBase));
                            SlangNVVMValueHandle loweredIndex = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                element.index,
                                valueMap,
                                typeContext,
                                loweredIndex));
                            SlangNVVMValueHandle loweredValue = nullptr;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "sequential value element extraction",
                                builder.emitSequentialElementExtract(
                                    moduleScope.module,
                                    loweredBase,
                                    loweredIndex,
                                    loweredValue)));
                            valueMap[inst] = loweredValue;
                            break;
                        }

                        SLANG_RELEASE_ASSERT(_getNVVMVectorConstruction(inst, construction));
                        SlangNVVMValueHandle loweredElements[4] = {};
                        for (uint32_t i = 0; i < construction.elementCount; ++i)
                        {
                            const NVVMVectorConstructElement& source = construction.elements[i];
                            if (source.value)
                            {
                                SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                    codeGenContext,
                                    builder,
                                    moduleScope.module,
                                    source.value,
                                    valueMap,
                                    typeContext,
                                    loweredElements[i]));
                            }
                            else
                            {
                                SlangNVVMValueHandle loweredBase = nullptr;
                                SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                    codeGenContext,
                                    builder,
                                    moduleScope.module,
                                    source.extractedBase,
                                    valueMap,
                                    typeContext,
                                    loweredBase));
                                SlangNVVMTypeHandle loweredIndexType = nullptr;
                                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                    codeGenContext,
                                    "vector extraction index type",
                                    builder
                                        .getIntegerType(moduleScope.module, 32, loweredIndexType)));
                                SlangNVVMValueHandle loweredIndex = nullptr;
                                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                    codeGenContext,
                                    "vector extraction index",
                                    builder.getIntegerConstant(
                                        moduleScope.module,
                                        loweredIndexType,
                                        source.extractedIndex,
                                        loweredIndex)));
                                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                    codeGenContext,
                                    "numeric vector swizzle extraction",
                                    builder.emitSequentialElementExtract(
                                        moduleScope.module,
                                        loweredBase,
                                        loweredIndex,
                                        loweredElements[i])));
                            }
                        }
                        SlangNVVMTypeHandle loweredResultType = nullptr;
                        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                            construction.resultType,
                            NVVMTypeUse::Value,
                            loweredResultType));
                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "value vector construction",
                            builder.emitVectorConstruct(
                                moduleScope.module,
                                loweredResultType,
                                loweredElements,
                                construction.elementCount,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_Sample:
                case kIROp_SampleLevel:
                case kIROp_TextureFetch:
                case kIROp_TextureGather:
                case kIROp_TextureQuerySize:
                case kIROp_TextureQueryLayerCount:
                    {
                        const auto* operation =
                            _findTextureOperationRequirement(requirements.textureOperations, inst);
                        SLANG_RELEASE_ASSERT(operation);
                        IRInst* operands[] = {
                            operation->texture,
                            operation->coordinate,
                            operation->level};
                        const UInt operandCount = operation->level        ? 3
                                                  : operation->coordinate ? 2
                                                                          : 1;
                        SlangNVVMValueHandle loweredOperands[3] = {};
                        for (UInt i = 0; i < operandCount; ++i)
                        {
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                operands[i],
                                valueMap,
                                typeContext,
                                loweredOperands[i]));
                        }
                        SlangNVVMValueHandle values[3] = {};
                        for (UInt i = 0; i < operation->operationCount; ++i)
                        {
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                operation->diagnosticName,
                                builder.emitTextureOperation(
                                    moduleScope.module,
                                    operation->operations[i],
                                    loweredOperands,
                                    operandCount,
                                    values[i])));
                        }
                        SlangNVVMValueHandle value = values[0];
                        if (operation->operationCount > 1)
                        {
                            SlangNVVMTypeHandle resultType = nullptr;
                            SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                                inst->getDataType(),
                                NVVMTypeUse::Value,
                                resultType));
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "texture spatial dimensions",
                                builder.emitVectorConstruct(
                                    moduleScope.module,
                                    resultType,
                                    values,
                                    operation->operationCount,
                                    value)));
                        }
                        valueMap[inst] = value;
                    }
                    break;

                case kIROp_GenericAsm:
                    {
                        auto genericAsm = as<IRGenericAsm>(inst);
                        if (const auto namedIntrinsic = planIndex.findNamedIntrinsic(inst))
                        {
                            List<SlangNVVMValueHandle> operands;
                            for (IRInst* operand : namedIntrinsic->operandValues)
                            {
                                SlangNVVMValueHandle lowered = nullptr;
                                SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                    codeGenContext,
                                    builder,
                                    moduleScope.module,
                                    operand,
                                    valueMap,
                                    typeContext,
                                    lowered));
                                operands.add(lowered);
                            }
                            SlangNVVMValueHandle value = nullptr;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                namedIntrinsic->isDeviceLibraryFunction
                                    ? "named device-library function"
                                    : "named LLVM intrinsic",
                                namedIntrinsic->isDeviceLibraryFunction
                                    ? builder.emitDeviceLibraryFunction(
                                          libraryScope.library,
                                          moduleScope.module,
                                          namedIntrinsic->getDesc(),
                                          operands.getBuffer(),
                                          size_t(operands.getCount()),
                                          value)
                                    : builder.emitNamedIntrinsic(
                                          moduleScope.module,
                                          namedIntrinsic->getDesc(),
                                          operands.getBuffer(),
                                          size_t(operands.getCount()),
                                          value)));
                            if (namedIntrinsic->resultType.kind == SLANG_NVVM_VALUE_TYPE_VOID)
                            {
                                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                    codeGenContext,
                                    "named LLVM intrinsic void return",
                                    builder.emitReturnVoid(moduleScope.module)));
                            }
                            else
                            {
                                SLANG_RETURN_ON_FAIL(_emitNVVMFunctionValueReturn(
                                    codeGenContext,
                                    builder,
                                    moduleScope.module,
                                    typeContext,
                                    function,
                                    "named LLVM intrinsic return",
                                    value));
                            }
                            break;
                        }

                        return _diagnoseUnsupportedGenericAsm(codeGenContext, genericAsm, function);
                    }
                    break;


                case kIROp_GetOffsetPtr:
                    {
                        SlangNVVMValueHandle loweredBasePointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            typeContext,
                            loweredBasePointer));
                        SlangNVVMValueHandle loweredElementOffset = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredElementOffset));
                        if (auto selected =
                                requirements.emissionPlan.layoutPointerOffsets.tryGetValue(inst))
                        {
                            SlangNVVMValueHandle widened = nullptr, stride = nullptr,
                                                 byteOffset = nullptr, result = nullptr;
                            SLANG_RETURN_ON_FAIL(_emitNVVMValueRecipeStep(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                selected->widenIndex,
                                &loweredElementOffset,
                                1,
                                widened));
                            SLANG_RETURN_ON_FAIL(_getNVVMRecipeIntegerConstant(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                64,
                                int64_t(selected->stride),
                                stride));
                            SlangNVVMValueHandle products[] = {widened, stride};
                            SLANG_RETURN_ON_FAIL(_emitNVVMValueRecipeStep(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                selected->scaleIndex,
                                products,
                                2,
                                byteOffset));
                            SlangNVVMTypeHandle byteType = nullptr;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "layout pointer byte type",
                                builder.getIntegerType(moduleScope.module, 8, byteType)));
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "layout pointer offset",
                                builder.emitByteOffsetPointer(
                                    moduleScope.module,
                                    loweredBasePointer,
                                    byteOffset,
                                    byteType,
                                    result)));
                            valueMap[inst] = result;
                            break;
                        }
                        SlangNVVMValueHandle loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "selected pointer offset",
                            builder.emitPointerOffset(
                                moduleScope.module,
                                loweredBasePointer,
                                loweredElementOffset,
                                loweredPointer)));
                        valueMap[inst] = loweredPointer;
                        if (asNVVMSupportedDeviceCopyableValuePointerType(inst->getDataType()) &&
                            globalUserPointers.contains(inst->getOperand(0)))
                        {
                            globalUserPointers.add(inst);
                        }
                    }
                    break;

                case kIROp_GetElementPtr:
                    {
                        const auto address = planIndex.findElementAddress(inst);
                        SLANG_RELEASE_ASSERT(
                            address && address->kind != NVVMElementAddressKind::Pending);
                        SlangNVVMValueHandle loweredBasePointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            address->base,
                            valueMap,
                            typeContext,
                            loweredBasePointer));
                        if (address->isLayoutStorage)
                        {
                            SlangNVVMValueHandle result = nullptr;
                            SLANG_RETURN_ON_FAIL(_emitNVVMLayoutByteAddress(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                loweredBasePointer,
                                address->byteOffset,
                                nullptr,
                                result));
                            valueMap[inst] = result;
                            break;
                        }
                        SlangNVVMValueHandle loweredElementIndex = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            address->index,
                            valueMap,
                            typeContext,
                            loweredElementIndex));
                        SlangNVVMValueHandle loweredPointer = nullptr;
                        const SlangResult pointerResult =
                            address->kind == NVVMElementAddressKind::RawBuffer
                                ? builder.emitPointerOffset(
                                      moduleScope.module,
                                      loweredBasePointer,
                                      loweredElementIndex,
                                      loweredPointer)
                                : builder.emitSequentialElementPointer(
                                      moduleScope.module,
                                      loweredBasePointer,
                                      loweredElementIndex,
                                      loweredPointer);
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            address->diagnosticName,
                            pointerResult));
                        valueMap[inst] = loweredPointer;
                        if (address->propagatesGlobalUserPointer &&
                            globalUserPointers.contains(address->base))
                        {
                            globalUserPointers.add(inst);
                        }
                    }
                    break;

                case kIROp_GetStructuredBufferPtr:
                case kIROp_GetUntypedBufferPtr:
                    {
                        const auto plannedData =
                            requirements.emissionPlan.addresses.findDataPointer(inst);
                        SLANG_RELEASE_ASSERT(plannedData);
                        const auto& dataPointer = *plannedData;
                        SlangNVVMValueHandle loweredBuffer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            dataPointer.buffer,
                            valueMap,
                            typeContext,
                            loweredBuffer));
                        SlangNVVMValueHandle loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw buffer data pointer",
                            builder.emitAggregateElementExtract(
                                moduleScope.module,
                                loweredBuffer,
                                0,
                                loweredPointer)));
                        valueMap[inst] = loweredPointer;
                    }
                    break;

                case kIROp_FieldAddress:
                    {
                        const auto address = planIndex.findFieldAddress(inst);
                        SLANG_RELEASE_ASSERT(address);
                        SlangNVVMValueHandle loweredBasePointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            address->base,
                            valueMap,
                            typeContext,
                            loweredBasePointer));
                        if (address->isLayoutStorage)
                        {
                            SlangNVVMValueHandle result = nullptr;
                            SLANG_RETURN_ON_FAIL(_emitNVVMLayoutByteAddress(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                loweredBasePointer,
                                address->byteOffset,
                                nullptr,
                                result));
                            valueMap[inst] = result;
                            break;
                        }
                        SlangNVVMValueHandle loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            address->selection.isMutable ? "mutable struct field address"
                                                         : "immutable struct field address",
                            builder.emitStructFieldPointer(
                                moduleScope.module,
                                loweredBasePointer,
                                address->selection.fieldIndex,
                                loweredPointer)));
                        valueMap[inst] = loweredPointer;
                    }
                    break;

                case kIROp_FieldExtract:
                    {
                        auto fieldExtract = cast<IRFieldExtract>(inst);
                        NVVMStructFieldSelection resolvedField;
                        SLANG_RELEASE_ASSERT(_getNVVMStructFieldValue(fieldExtract, resolvedField));
                        SlangNVVMValueHandle loweredValue = nullptr;
                        // Only entries retained in the physical pointer map use direct field
                        // addresses. Numeric entries have already been decoded to semantic values;
                        // merge-block parameters likewise never use the launch-storage path.
                        const bool isPointerBackedEntryParameter =
                            entryAggregatePointerMap.containsKey(fieldExtract->getBase());
                        if (isPointerBackedEntryParameter)
                        {
                            SlangNVVMValueHandle loweredBase =
                                entryAggregatePointerMap.getValue(fieldExtract->getBase());
                            SlangNVVMValueHandle loweredFieldPointer = nullptr;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "by-value aggregate field pointer",
                                builder.emitStructFieldPointer(
                                    moduleScope.module,
                                    loweredBase,
                                    resolvedField.fieldIndex,
                                    loweredFieldPointer)));
                            const uint32_t alignment =
                                _getNVVMExecutableValueAlignment(fieldExtract->getDataType());
                            SLANG_RELEASE_ASSERT(alignment);
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "by-value aggregate field load",
                                builder.emitLoad(
                                    moduleScope.module,
                                    loweredFieldPointer,
                                    alignment,
                                    SLANG_NVVM_LOAD_FLAG_INVARIANT,
                                    loweredValue)));
                        }
                        else
                        {
                            SlangNVVMValueHandle loweredBase = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                fieldExtract->getBase(),
                                valueMap,
                                typeContext,
                                loweredBase));
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "first-class aggregate field extraction",
                                builder.emitAggregateElementExtract(
                                    moduleScope.module,
                                    loweredBase,
                                    resolvedField.fieldIndex,
                                    loweredValue)));
                        }
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_StructuredBufferLoad:
                case kIROp_RWStructuredBufferLoad:
                    {
                        const auto plannedLoad = planIndex.findStructuredLoad(inst);
                        SLANG_RELEASE_ASSERT(plannedLoad);
                        const auto& load = *plannedLoad;
                        SlangNVVMValueHandle loweredBuffer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            load.buffer,
                            valueMap,
                            typeContext,
                            loweredBuffer));
                        SlangNVVMValueHandle loweredDataPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw StructuredBuffer data pointer",
                            builder.emitAggregateElementExtract(
                                moduleScope.module,
                                loweredBuffer,
                                0,
                                loweredDataPointer)));
                        SlangNVVMValueHandle loweredElementIndex = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            load.elementIndex,
                            valueMap,
                            typeContext,
                            loweredElementIndex));
                        SlangNVVMValueHandle loweredElementPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw StructuredBuffer numeric element pointer",
                            builder.emitPointerOffset(
                                moduleScope.module,
                                loweredDataPointer,
                                loweredElementIndex,
                                loweredElementPointer)));

                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw structured-buffer value load",
                            builder.emitLoad(
                                moduleScope.module,
                                loweredElementPointer,
                                load.alignment,
                                load.flags,
                                loweredValue)));
                        if (load.conversion.kind == NVVMStorageConversionKind::StructuredBuffer)
                        {
                            SlangNVVMValueHandle semanticValue = nullptr;
                            SLANG_RETURN_ON_FAIL(_emitNVVMStructuredBufferStorageConversion(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                typeContext,
                                requirements.emissionPlan.structuredConversions,
                                load.conversion.structuredRecipe,
                                loweredValue,
                                semanticValue));
                            loweredValue = semanticValue;
                        }
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_StructuredBufferGetDimensions:
                    {
                        NVVMStructuredBufferDimensions dimensions;
                        SLANG_RELEASE_ASSERT(
                            _getNVVMStructuredBufferDimensions(codeGenContext, inst, dimensions));
                        SlangNVVMValueHandle loweredBuffer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            dimensions.buffer,
                            valueMap,
                            typeContext,
                            loweredBuffer));
                        SlangNVVMValueHandle loweredCount64 = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw structured-buffer element count",
                            builder.emitAggregateElementExtract(
                                moduleScope.module,
                                loweredBuffer,
                                1,
                                loweredCount64)));
                        SlangNVVMValueHandle loweredCount = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "structured-buffer count conversion",
                            builder.emitValueOperation(
                                moduleScope.module,
                                kNVVMRawBufferCountConversion,
                                &loweredCount64,
                                1,
                                loweredCount)));
                        SlangNVVMTypeHandle loweredElementType = nullptr;
                        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                            dimensions.resultType->getElementType(),
                            NVVMTypeUse::Value,
                            loweredElementType));
                        SlangNVVMValueHandle loweredStride = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "structured-buffer element stride",
                            builder.getIntegerConstant(
                                moduleScope.module,
                                loweredElementType,
                                dimensions.elementStride,
                                loweredStride)));
                        const SlangNVVMValueHandle elements[] = {loweredCount, loweredStride};
                        SlangNVVMTypeHandle loweredResultType = nullptr;
                        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                            dimensions.resultType,
                            NVVMTypeUse::Value,
                            loweredResultType));
                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "structured-buffer dimensions",
                            builder.emitVectorConstruct(
                                moduleScope.module,
                                loweredResultType,
                                elements,
                                SLANG_COUNT_OF(elements),
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_ByteAddressBufferLoad:
                case kIROp_ByteAddressBufferStore:
                    {
                        NVVMByteAddressAccess access;
                        SLANG_RELEASE_ASSERT(_getNVVMByteAddressAccess(inst, access));

                        SlangNVVMValueHandle loweredBuffer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            access.buffer,
                            valueMap,
                            typeContext,
                            loweredBuffer));
                        SlangNVVMValueHandle loweredDataPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw byte-address buffer data pointer",
                            builder.emitAggregateElementExtract(
                                moduleScope.module,
                                loweredBuffer,
                                0,
                                loweredDataPointer)));

                        SlangNVVMValueHandle loweredByteOffset = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            access.byteOffset,
                            valueMap,
                            typeContext,
                            loweredByteOffset));
                        SlangNVVMTypeHandle loweredValueType = nullptr;
                        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                            access.valueType,
                            NVVMTypeUse::Value,
                            loweredValueType));
                        SlangNVVMValueHandle loweredValuePointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw byte-address buffer byte offset",
                            builder.emitByteOffsetPointer(
                                moduleScope.module,
                                loweredDataPointer,
                                loweredByteOffset,
                                loweredValueType,
                                loweredValuePointer)));

                        if (access.isStore)
                        {
                            SlangNVVMValueHandle loweredValue = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                access.value,
                                valueMap,
                                typeContext,
                                loweredValue));
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "raw byte-address buffer store",
                                builder.emitStore(
                                    moduleScope.module,
                                    loweredValue,
                                    loweredValuePointer,
                                    access.alignment)));
                        }
                        else
                        {
                            SlangNVVMValueHandle loweredValue = nullptr;
                            const SlangNVVMLoadFlags flags =
                                access.bufferType.access == NVVMBufferAccess::ReadOnly
                                    ? SLANG_NVVM_LOAD_FLAG_INVARIANT
                                    : SLANG_NVVM_LOAD_FLAG_NONE;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "raw byte-address buffer load",
                                builder.emitLoad(
                                    moduleScope.module,
                                    loweredValuePointer,
                                    access.alignment,
                                    flags,
                                    loweredValue)));
                            valueMap[inst] = loweredValue;
                        }
                    }
                    break;

                case kIROp_GetEquivalentStructuredBuffer:
                    {
                        const auto selected =
                            requirements.emissionPlan.equivalentStructuredBuffers.tryGetValue(inst);
                        SLANG_RELEASE_ASSERT(selected);
                        const auto& conversion = *selected;
                        SlangNVVMValueHandle loweredBuffer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            conversion.buffer,
                            valueMap,
                            typeContext,
                            loweredBuffer));
                        SlangNVVMValueHandle loweredDataPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw byte-address buffer data pointer",
                            builder.emitAggregateElementExtract(
                                moduleScope.module,
                                loweredBuffer,
                                0,
                                loweredDataPointer)));
                        SlangNVVMValueHandle loweredCount = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw byte-address buffer byte extent",
                            builder.emitAggregateElementExtract(
                                moduleScope.module,
                                loweredBuffer,
                                1,
                                loweredCount)));

                        SlangNVVMTypeHandle countType = nullptr;
                        SlangNVVMValueHandle stride = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "byte-view extent type",
                            builder.getIntegerType(moduleScope.module, 64, countType)));
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "typed-view element stride",
                            builder.getIntegerConstant(
                                moduleScope.module,
                                countType,
                                conversion.elementStride,
                                stride)));
                        const SlangNVVMValueHandle countOperands[] = {loweredCount, stride};
                        SlangNVVMValueHandle elementCount = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "byte-view element count",
                            builder.emitValueOperation(
                                moduleScope.module,
                                kNVVMByteViewCountDivision,
                                countOperands,
                                SLANG_COUNT_OF(countOperands),
                                elementCount)));

                        SlangNVVMTypeHandle loweredIndexType = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw buffer reinterpretation index type",
                            builder.getIntegerType(moduleScope.module, 32, loweredIndexType)));
                        SlangNVVMValueHandle loweredZero = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw buffer reinterpretation zero offset",
                            builder.getIntegerConstant(
                                moduleScope.module,
                                loweredIndexType,
                                0,
                                loweredZero)));
                        SlangNVVMTypeHandle loweredElementType = nullptr;
                        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                            conversion.elementType,
                            NVVMTypeUse::Value,
                            loweredElementType));
                        SlangNVVMValueHandle loweredTypedPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw buffer reinterpretation pointer",
                            builder.emitByteOffsetPointer(
                                moduleScope.module,
                                loweredDataPointer,
                                loweredZero,
                                loweredElementType,
                                loweredTypedPointer)));
                        SlangNVVMTypeHandle loweredResultType = nullptr;
                        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                            inst->getDataType(),
                            NVVMTypeUse::Value,
                            loweredResultType));
                        const SlangNVVMValueHandle loweredElements[] = {
                            loweredTypedPointer,
                            elementCount,
                        };
                        SlangNVVMValueHandle loweredResult = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw buffer reinterpreted view",
                            builder.emitAggregateConstruct(
                                moduleScope.module,
                                loweredResultType,
                                loweredElements,
                                SLANG_COUNT_OF(loweredElements),
                                loweredResult)));
                        valueMap[inst] = loweredResult;
                    }
                    break;

                case kIROp_RWStructuredBufferGetElementPtr:
                    {
                        const auto element =
                            requirements.emissionPlan.addresses.findStructuredElement(inst);
                        SLANG_RELEASE_ASSERT(element);
                        SlangNVVMValueHandle loweredBuffer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            element->buffer,
                            valueMap,
                            typeContext,
                            loweredBuffer));
                        SlangNVVMValueHandle loweredElementIndex = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            element->elementIndex,
                            valueMap,
                            typeContext,
                            loweredElementIndex));
                        SlangNVVMValueHandle loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw RWStructuredBuffer data pointer",
                            builder.emitAggregateElementExtract(
                                moduleScope.module,
                                loweredBuffer,
                                0,
                                loweredPointer)));
                        SlangNVVMValueHandle loweredElementPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw RWStructuredBuffer numeric element pointer",
                            builder.emitPointerOffset(
                                moduleScope.module,
                                loweredPointer,
                                loweredElementIndex,
                                loweredElementPointer)));
                        valueMap[inst] = loweredElementPointer;
                    }
                    break;

                case kIROp_Return:
                    if (function == entryPoint || as<IRVoidType>(function->getResultType()))
                    {
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "void return",
                            builder.emitReturnVoid(moduleScope.module)));
                    }
                    else
                    {
                        auto returnInst = cast<IRReturn>(inst);
                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMHelperValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            returnInst->getVal(),
                            valueMap,
                            globalUserPointers,
                            helperValueMap,
                            typeContext,
                            loweredValue));
                        SLANG_RETURN_ON_FAIL(_emitNVVMFunctionValueReturn(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            typeContext,
                            function,
                            _usesGenericNVVMFunctions(function) ? "generic value return"
                                                                : "signed i32 return",
                            loweredValue));
                    }
                    break;

                case kIROp_Unreachable:
                    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                        codeGenContext,
                        "unreachable terminator",
                        builder.emitUnreachable(moduleScope.module)));
                    break;

                case kIROp_UnconditionalBranch:
                case kIROp_Loop:
                    {
                        auto branch = cast<IRUnconditionalBranch>(inst);
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            inst->getOp() == kIROp_Loop ? "loop entry branch"
                                                        : "unconditional branch",
                            builder.emitBranch(
                                moduleScope.module,
                                blockMap.getValue(branch->getTargetBlock()))));
                    }
                    break;

                case kIROp_IfElse:
                    {
                        auto ifElse = cast<IRIfElse>(inst);
                        SlangNVVMValueHandle loweredCondition = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            ifElse->getCondition(),
                            valueMap,
                            typeContext,
                            loweredCondition));
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "conditional branch",
                            builder.emitConditionalBranch(
                                moduleScope.module,
                                loweredCondition,
                                blockMap.getValue(ifElse->getTrueBlock()),
                                blockMap.getValue(ifElse->getFalseBlock()))));
                    }
                    break;

                case kIROp_Switch:
                    {
                        auto switchInst = cast<IRSwitch>(inst);
                        SlangNVVMValueHandle loweredCondition = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            switchInst->getCondition(),
                            valueMap,
                            typeContext,
                            loweredCondition));

                        List<SlangNVVMValueHandle> loweredCaseValues;
                        List<SlangNVVMBlockHandle> loweredCaseBlocks;
                        for (UInt caseIndex = 0; caseIndex < switchInst->getCaseCount();
                             ++caseIndex)
                        {
                            SlangNVVMValueHandle loweredCaseValue = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                switchInst->getCaseValue(caseIndex),
                                valueMap,
                                typeContext,
                                loweredCaseValue));
                            loweredCaseValues.add(loweredCaseValue);
                            loweredCaseBlocks.add(
                                blockMap.getValue(switchInst->getCaseLabel(caseIndex)));
                        }
                        IRBlock* defaultBlock = switchInst->getDefaultLabel();
                        if (!defaultBlock)
                            defaultBlock = switchInst->getBreakLabel();
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "integer switch",
                            builder.emitSwitch(
                                moduleScope.module,
                                loweredCondition,
                                loweredCaseValues.getBuffer(),
                                loweredCaseBlocks.getBuffer(),
                                size_t(loweredCaseValues.getCount()),
                                blockMap.getValue(defaultBlock))));
                    }
                    break;

                default:
                    SLANG_UNEXPECTED("NVVM emission received IR that was not preflighted");
                }
            }
        }

        // Slang block parameters are the phi source of truth: argument N on each actual predecessor
        // edge feeds parameter N. At this point even loop backedge instructions exist, so every
        // pair can be attached without reconstructing a local variable or searching an operand
        // graph.
        for (auto block : function->getBlocks())
        {
            if (block == entryBlock || !block->getFirstParam())
                continue;

            for (auto predecessor : block->getPredecessors())
            {
                auto branch = as<IRUnconditionalBranch>(predecessor->getTerminator());
                SLANG_RELEASE_ASSERT(branch && branch->getTargetBlock() == block);

                UInt phiParameterIndex = 0;
                for (auto param : block->getParams())
                {
                    SlangNVVMValueHandle loweredArgument = nullptr;
                    SLANG_RETURN_ON_FAIL(_getLoweredNVVMHelperValue(
                        codeGenContext,
                        builder,
                        moduleScope.module,
                        branch->getArg(phiParameterIndex),
                        valueMap,
                        globalUserPointers,
                        helperValueMap,
                        typeContext,
                        loweredArgument));
                    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                        codeGenContext,
                        "value phi incoming value",
                        builder.addPhiIncoming(
                            moduleScope.module,
                            valueMap.getValue(param),
                            loweredArgument,
                            blockMap.getValue(predecessor))));
                    ++phiParameterIndex;
                }
            }
        }
    }

    if (entryPoint->findDecoration<IREntryPointDecoration>()->getProfile().getStage() !=
        Stage::Callable)
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "kernel annotation",
            builder.markFunctionAsKernel(moduleScope.module, functionMap.getValue(entryPoint))));

    ComPtr<ISlangBlob> serializedIR;
    String verifierDiagnostics;
    SlangResult serializationResult = builder.serializeModule(
        moduleScope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
        serializedIR,
        verifierDiagnostics);
    if (SLANG_FAILED(serializationResult))
    {
        _requireBuilderOperation(
            codeGenContext,
            "verified NVVM IR 2.0 assembly serialization",
            serializationResult);
        if (verifierDiagnostics.getLength())
        {
            codeGenContext->getSink()->diagnoseRaw(
                Severity::Note,
                verifierDiagnostics.getUnownedSlice());
        }
        return serializationResult;
    }
    if (verifierDiagnostics.getLength())
    {
        codeGenContext->getSink()->diagnoseRaw(
            Severity::Note,
            verifierDiagnostics.getUnownedSlice());
    }
    if (!serializedIR || !serializedIR->getBufferSize())
    {
        return _requireBuilderOperation(
            codeGenContext,
            "verified NVVM IR 2.0 assembly serialization",
            SLANG_FAIL);
    }

    auto artifact = ArtifactUtil::createArtifact(
        ArtifactDesc::make(ArtifactKind::Assembly, ArtifactPayload::LLVMIR, ArtifactStyle::Kernel));
    artifact->addRepresentationUnknown(serializedIR);
    ArtifactUtil::addAssociated(artifact, linkedIR.metadata);
    outArtifact = artifact;
    return SLANG_OK;
}

} // namespace Slang
