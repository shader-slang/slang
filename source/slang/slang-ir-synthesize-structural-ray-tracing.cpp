#include "slang-ir-synthesize-structural-ray-tracing.h"

#include "slang-ir-clone.h"
#include "slang-ir-dominators.h"
#include "slang-ir-inline.h"
#include "slang-ir-insts.h"
#include "slang-ir-structural-ray-tracing.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"
#include "slang-structural-ray-tracing.h"

namespace Slang
{

static void _collectProgramOperations(IRInst* parent, List<IRInst*>& operations);

static Stage _getStructuralRayTracingNativeStage(StructuralRayTracingStageKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingStageKind::ClosestHit:
        return Stage::ClosestHit;
    case StructuralRayTracingStageKind::AnyHit:
        return Stage::AnyHit;
    case StructuralRayTracingStageKind::Intersection:
        return Stage::Intersection;
    case StructuralRayTracingStageKind::Miss:
        return Stage::Miss;
    case StructuralRayTracingStageKind::Callable:
        return Stage::Callable;
    default:
        return Stage::Unknown;
    }
}

static IRFunc* _getStructuralRayTracingStageFunc(IRInst* value)
{
    return as<IRFunc>(value);
}

static void _addEmptyStructuralRayTracingEntryPointLayout(
    IRBuilder& builder,
    IRFunc* func,
    Stage stage)
{
    IRStructTypeLayout::Builder paramsTypeLayoutBuilder(&builder);
    IRVarLayout::Builder paramsLayoutBuilder(&builder, paramsTypeLayoutBuilder.build());
    paramsLayoutBuilder.setStage(stage);

    IRTypeLayout::Builder resultTypeLayoutBuilder(&builder);
    IRVarLayout::Builder resultLayoutBuilder(&builder, resultTypeLayoutBuilder.build());
    resultLayoutBuilder.setStage(stage);

    auto entryPointLayout =
        builder.getEntryPointLayout(paramsLayoutBuilder.build(), resultLayoutBuilder.build());
    builder.addLayoutDecoration(func, entryPointLayout);
}

static void _addStructuralRayTracingEntryPointInfo(
    IRBuilder& builder,
    IRFunc* adapter,
    StructuralRayTracingStageKind stageKind,
    IRFunc* invoke,
    IRType* stageType,
    IRStringLit* stageSourceTypeName,
    IRStringLit* stageTypeIdentity,
    IRType* contextType,
    IRType* payloadType,
    IRType* payloadSemanticType,
    IRType* recordType,
    IRType* hitAttributesType,
    StructuralRayTracingHitAttributesKind hitAttributesKind,
    IRType* callableDataType,
    IRIntegerValue payloadLocation)
{
    addStructuralRayTracingEntryPointInfo(
        builder,
        adapter,
        {
            .stageKind = stageKind,
            .invoke = invoke,
            .stageType = stageType,
            .stageSourceTypeName = stageSourceTypeName,
            .stageTypeIdentity = stageTypeIdentity,
            .contextType = contextType,
            .payloadType = payloadType,
            .payloadSemanticType = payloadSemanticType,
            .recordType = recordType,
            .hitAttributesType = hitAttributesType,
            .callableDataType = callableDataType,
            .hitAttributesKind = hitAttributesKind,
            .payloadLocation = payloadLocation,
        });
}

struct StructuralRayTracingGeneratedEntryPoint
{
    StructuralRayTracingStageKind stageKind;
    IRStringLit* stageSourceTypeName;
    IRStringLit* stageTypeIdentity;
    String physicalName;
    IRFunc* adapter;
};

static IRFunc* _findGeneratedStructuralRayTracingEntryPoint(
    const List<StructuralRayTracingGeneratedEntryPoint>& generated,
    StructuralRayTracingStageKind stageKind,
    IRStringLit* stageTypeIdentity,
    UnownedStringSlice physicalName)
{
    // A schema may list the same stage in many records, and those exact requests should share one
    // native adapter. A renamed selected entry point is a different request even when the schema
    // also references that same semantic stage: the selected adapter keeps the client-provided
    // symbol while the schema materializes the default symbol advertised by reflection.
    for (auto& item : generated)
    {
        if (item.stageKind == stageKind &&
            item.stageTypeIdentity->getStringSlice() == stageTypeIdentity->getStringSlice() &&
            item.physicalName.getUnownedSlice() == physicalName)
        {
            return item.adapter;
        }
    }
    return nullptr;
}

static const StructuralRayTracingGeneratedEntryPoint* _findStructuralRayTracingPhysicalNameOwner(
    const List<StructuralRayTracingGeneratedEntryPoint>& generated,
    UnownedStringSlice physicalName)
{
    for (auto& item : generated)
    {
        if (item.physicalName.getUnownedSlice() == physicalName)
            return &item;
    }
    return nullptr;
}

static bool _validateStructuralRayTracingPhysicalNameOwner(
    const List<StructuralRayTracingGeneratedEntryPoint>& generated,
    StructuralRayTracingStageKind stageKind,
    IRStringLit* stageSourceTypeName,
    IRStringLit* stageTypeIdentity,
    UnownedStringSlice physicalName,
    SourceLoc location,
    DiagnosticSink* sink)
{
    auto owner = _findStructuralRayTracingPhysicalNameOwner(generated, physicalName);
    if (!owner || (owner->stageKind == stageKind && owner->stageTypeIdentity->getStringSlice() ==
                                                        stageTypeIdentity->getStringSlice()))
    {
        return true;
    }

    // Preparation has no diagnostic sink, so retain both requests and let the post-specialization
    // synthesis boundary report the conflict. That later boundary is also where schema-generated
    // and explicitly selected adapters first coexist.
    if (!sink)
        return true;

    sink->diagnose(Diagnostics::StructuralRayTracingEntryPointNameCollision{
        .physicalName = String(physicalName),
        .firstStage = String(owner->stageSourceTypeName->getStringSlice()),
        .secondStage = String(stageSourceTypeName->getStringSlice()),
        .location = location});
    return false;
}

static String _getRequestedStructuralRayTracingEntryPointName(
    StructuralRayTracingStageKind stageKind,
    IRFunc* invoke,
    IRStringLit* stageSourceTypeName)
{
    SLANG_RELEASE_ASSERT(invoke && stageSourceTypeName);
    auto result = getStructuralRayTracingEntryPointName(stageSourceTypeName->getStringSlice());
    // The selected source function carries the component API's rename until the preparation pass
    // replaces it. Schema-only functions have no entry-point decoration and therefore retain the
    // deterministic default name.
    if (auto entryPoint = invoke->findDecoration<IREntryPointDecoration>())
    {
        auto selectedStage = entryPoint->getProfile().getStage();
        SLANG_RELEASE_ASSERT(selectedStage == _getStructuralRayTracingNativeStage(stageKind));
        result = entryPoint->getName()->getStringSlice();
    }
    return result;
}

static IRFunc* _generateStructuralRayTracingEntryPoint(
    IRModule* module,
    List<IRFunc*>& ioEntryPoints,
    List<StructuralRayTracingGeneratedEntryPoint>& generated,
    StructuralRayTracingStageKind stageKind,
    IRType* stageType,
    IRStringLit* stageSourceTypeName,
    IRStringLit* stageTypeIdentity,
    bool isPresent,
    IRInst* invokeValue,
    IRType* contextType,
    IRType* payloadType = nullptr,
    IRType* recordType = nullptr,
    IRType* hitAttributesType = nullptr,
    StructuralRayTracingHitAttributesKind hitAttributesKind =
        StructuralRayTracingHitAttributesKind::None,
    IRType* callableDataType = nullptr,
    IRType* payloadSemanticType = nullptr,
    IRIntegerValue payloadLocation = -1,
    DiagnosticSink* sink = nullptr,
    IRInst* diagnosticOwner = nullptr)
{
    SLANG_RELEASE_ASSERT(stageType && stageSourceTypeName && stageTypeIdentity && invokeValue);
    if (!isPresent)
    {
        SLANG_RELEASE_ASSERT(
            as<IRVoidType>(stageType) && stageSourceTypeName->getStringSlice().getLength() == 0 &&
            stageTypeIdentity->getStringSlice().getLength() == 0 && as<IRVoidLit>(invokeValue));
        return nullptr;
    }

    auto invoke = _getStructuralRayTracingStageFunc(invokeValue);
    SLANG_RELEASE_ASSERT(
        invoke && !as<IRVoidType>(stageType) &&
        stageSourceTypeName->getStringSlice().getLength() != 0 &&
        stageTypeIdentity->getStringSlice().getLength() != 0);
    SLANG_RELEASE_ASSERT(
        !payloadType || as<IRVoidType>(payloadType) ||
        (payloadSemanticType && payloadLocation >= 0));

    auto name =
        _getRequestedStructuralRayTracingEntryPointName(stageKind, invoke, stageSourceTypeName);
    if (auto existing = _findGeneratedStructuralRayTracingEntryPoint(
            generated,
            stageKind,
            stageTypeIdentity,
            name.getUnownedSlice()))
    {
        auto info = existing->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
        SLANG_RELEASE_ASSERT(
            !info || payloadLocation < 0 || info->getPayloadLocation()->getValue() < 0 ||
            info->getPayloadLocation()->getValue() == payloadLocation);
        return existing;
    }

    auto diagnosticLocation = diagnosticOwner && diagnosticOwner->sourceLoc.isValid()
                                  ? diagnosticOwner->sourceLoc
                                  : invoke->sourceLoc;
    if (!_validateStructuralRayTracingPhysicalNameOwner(
            generated,
            stageKind,
            stageSourceTypeName,
            stageTypeIdentity,
            name.getUnownedSlice(),
            diagnosticLocation,
            sink))
    {
        return nullptr;
    }

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());
    auto adapter = builder.createFunc();
    adapter->setFullType(builder.getFuncType(List<IRType*>(), builder.getVoidType()));

    auto stage = _getStructuralRayTracingNativeStage(stageKind);
    builder.addNameHintDecoration(adapter, name.getUnownedSlice());
    builder.addEntryPointDecoration(
        adapter,
        Profile(stage),
        name.getUnownedSlice(),
        toSlice("structural-ray-tracing"));
    builder.addKeepAliveDecoration(adapter);
    _addEmptyStructuralRayTracingEntryPointLayout(builder, adapter, stage);
    _addStructuralRayTracingEntryPointInfo(
        builder,
        adapter,
        stageKind,
        invoke,
        stageType,
        stageSourceTypeName,
        stageTypeIdentity,
        contextType,
        payloadType,
        payloadSemanticType,
        recordType,
        hitAttributesType,
        hitAttributesKind,
        callableDataType,
        payloadLocation);

    builder.setInsertInto(adapter);
    builder.emitBlock();
    List<IRInst*> arguments;
    for (UInt i = 0; i < invoke->getParamCount(); ++i)
        arguments.add(builder.emitDefaultConstruct(invoke->getParamType(i)));
    builder
        .emitCallInst(invoke->getResultType(), invoke, arguments.getCount(), arguments.getBuffer());
    builder.emitReturn();

    generated.add({stageKind, stageSourceTypeName, stageTypeIdentity, name, adapter});
    ioEntryPoints.add(adapter);
    return adapter;
}

static bool _validateStructuralRayTracingEntryTraceContext(
    IRInst* operation,
    IRType* schema,
    IRType* expectedTraceContext,
    IRType* entry,
    IRType* actualTraceContext,
    DiagnosticSink* sink)
{
    if (actualTraceContext == expectedTraceContext)
        return true;

    sink->diagnose(Diagnostics::StructuralRayTracingEntryTraceContextMismatch{
        .entry = entry,
        .actualType = actualTraceContext,
        .schema = schema,
        .expectedType = expectedTraceContext,
        .location = operation->sourceLoc});
    return false;
}

static bool _insertStructuralRayTracingEntry(
    IRInst* operation,
    IRType* schema,
    IRType* entry,
    const char* section,
    HashSet<IRType*>& entries,
    DiagnosticSink* sink)
{
    if (entries.add(entry))
        return true;

    sink->diagnose(Diagnostics::DuplicateStructuralRayTracingEntry{
        .section = section,
        .entry = entry,
        .schema = schema,
        .location = operation->sourceLoc});
    return false;
}

// Return whether `type` has a fixed-size, ordinary value representation that every structural
// ray-tracing target can copy through its native payload or shader-record ABI.
//
// Consider this example:
//
//     struct MaterialRecord { Texture2D texture; }
//     struct HitContext : IHitContext { typealias Record = MaterialRecord; ... }
//
// AST-to-IR lowering puts the concrete `%MaterialRecord` type directly on each
// `structuralRayTracingHitGroupInfo` decoration. Generic specialization runs before this file
// validates those decorations, so a closed schema reaches this helper as basic, aggregate, enum,
// or disallowed opaque IR types rather than as source declarations that need to be rediscovered.
// Keeping this check on those canonical decoration operands gives descriptor lowering and native
// entry-point synthesis one target-independent contract.
static bool _isStructuralRayTracingPlainDataTypeImpl(
    IRType* type,
    HashSet<IRType*>& activeStructTypes)
{
    type = as<IRType>(unwrapAttributedType(type));
    if (!type)
        return false;

    // A non-copyable declaration cannot become portable merely because each of its fields could
    // otherwise be copied. `getResolvedInstForDecorations` follows the defining generic, when
    // present, solely to find that declaration decoration; field recursion still uses the
    // specialized concrete type below.
    if (getResolvedInstForDecorations(type)->findDecoration<IRNonCopyableTypeDecoration>())
        return false;

    if (auto basicType = as<IRBasicType>(type))
        return basicType->getBaseType() != BaseType::Void;
    if (isPackedFloatType(type))
        return true;

    // Plain ABI data must have a concrete size at this boundary. A specialized vector, matrix, or
    // array therefore carries literal dimensions; accepting a symbolic generic operand here would
    // advertise a fixed record or payload size that target layout cannot actually compute.
    switch (type->getOp())
    {
    case kIROp_VectorType:
        {
            auto vectorType = cast<IRVectorType>(type);
            return as<IRIntLit>(vectorType->getElementCount()) &&
                   _isStructuralRayTracingPlainDataTypeImpl(
                       vectorType->getElementType(),
                       activeStructTypes);
        }

    case kIROp_MatrixType:
        {
            auto matrixType = cast<IRMatrixType>(type);
            return as<IRIntLit>(matrixType->getRowCount()) &&
                   as<IRIntLit>(matrixType->getColumnCount()) &&
                   _isStructuralRayTracingPlainDataTypeImpl(
                       matrixType->getElementType(),
                       activeStructTypes);
        }

    case kIROp_EnumType:
        return _isStructuralRayTracingPlainDataTypeImpl(
            cast<IREnumType>(type)->getTagType(),
            activeStructTypes);

    case kIROp_ArrayType:
        {
            auto arrayType = cast<IRArrayType>(type);
            return as<IRIntLit>(arrayType->getElementCount()) &&
                   _isStructuralRayTracingPlainDataTypeImpl(
                       arrayType->getElementType(),
                       activeStructTypes);
        }

    case kIROp_StructType:
        {
            // A by-value cycle has no finite representation. Slang normally rejects one before IR
            // generation, but treating it as non-plain here keeps this ABI boundary total over IR.
            if (!activeStructTypes.add(type))
                return false;

            bool result = true;
            for (auto field : cast<IRStructType>(type)->getFields())
            {
                if (!_isStructuralRayTracingPlainDataTypeImpl(
                        field->getFieldType(),
                        activeStructTypes))
                {
                    result = false;
                    break;
                }
            }
            activeStructTypes.remove(type);
            return result;
        }

    default:
        // This closed whitelist intentionally excludes unsized arrays, pointer/ref-like types,
        // atomics, interfaces/existentials, resources, and all other opaque handles.
        return false;
    }
}

static bool _isStructuralRayTracingPlainDataType(IRType* type)
{
    HashSet<IRType*> activeStructTypes;
    return _isStructuralRayTracingPlainDataTypeImpl(type, activeStructTypes);
}

enum class StructuralRayTracingDataRole
{
    Payload,
    Record,
    Callable,
    IntersectionAttribute,
};

using StructuralRayTracingDataDiagnosticKey = KeyValuePair<IRType*, UInt>;

static UnownedStringSlice _getStructuralRayTracingDataRoleName(StructuralRayTracingDataRole role)
{
    switch (role)
    {
    case StructuralRayTracingDataRole::Payload:
        return toSlice("payload");
    case StructuralRayTracingDataRole::Record:
        return toSlice("record");
    case StructuralRayTracingDataRole::Callable:
        return toSlice("callable");
    case StructuralRayTracingDataRole::IntersectionAttribute:
        return toSlice("intersection-attribute");
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing data role");
    }
}

static bool _validateStructuralRayTracingDataType(
    IRInst* owner,
    IRType* type,
    StructuralRayTracingDataRole role,
    HashSet<StructuralRayTracingDataDiagnosticKey>& diagnosedTypes,
    DiagnosticSink* sink)
{
    auto canonicalType = as<IRType>(unwrapAttributedType(type));
    SLANG_RELEASE_ASSERT(canonicalType);
    if (as<IRVoidType>(canonicalType))
    {
        if (role == StructuralRayTracingDataRole::Record)
            return true;

        // `void` is deliberately valid for a record that carries no application data, but every
        // transported ABI role needs a concrete type. Include the role in the deduplication key:
        // one schema can independently misuse `void` as both CallableData and custom attributes,
        // and reporting only the first role would hide the second contract violation.
        StructuralRayTracingDataDiagnosticKey key(canonicalType, UInt(role));
        if (diagnosedTypes.add(key))
        {
            auto location =
                owner->sourceLoc.isValid() ? owner->sourceLoc : canonicalType->sourceLoc;
            sink->diagnose(Diagnostics::StructuralRayTracingVoidAbiData{
                .role = _getStructuralRayTracingDataRoleName(role),
                .location = location});
        }
        return false;
    }

    if (_isStructuralRayTracingPlainDataType(canonicalType))
        return true;

    // A schema commonly repeats one payload or record across several stages. Diagnose the type
    // once at the owning trace/call operation instead of producing one follow-on error per entry.
    StructuralRayTracingDataDiagnosticKey key(canonicalType, UInt(role));
    if (diagnosedTypes.add(key))
    {
        auto location = owner->sourceLoc.isValid() ? owner->sourceLoc : canonicalType->sourceLoc;
        sink->diagnose(Diagnostics::StructuralRayTracingRecordNotPlainData{
            .type = canonicalType,
            .role = _getStructuralRayTracingDataRoleName(role),
            .location = location});
    }
    return false;
}

static IRInst* _findReachableStructuralRayTracingPayloadAccess(
    IRFunc* func,
    HashSet<IRFunc*>& visitedFunctions)
{
    if (!func || !visitedFunctions.add(func))
        return nullptr;

    // Only visit executable CFG blocks. This excludes operations in unreachable blocks while
    // still following ordinary helper calls from the concrete, post-specialization stage body.
    // `getResolvedInstForDecorations` handles a remaining specialized generic callee by resolving
    // it to the function body selected by the call.
    for (auto block : getReversePostorder(func))
    {
        for (auto inst : block->getChildren())
        {
            if (inst->getOp() == kIROp_StructuralRayTracingGetPayload)
                return inst;

            if (auto call = as<IRCall>(inst))
            {
                auto callee = as<IRFunc>(getResolvedInstForDecorations(call->getCallee()));
                if (auto access =
                        _findReachableStructuralRayTracingPayloadAccess(callee, visitedFunctions))
                {
                    return access;
                }
            }
        }
    }
    return nullptr;
}

static bool _validateStructuralRayTracingEmptyPayloadAccess(
    IRInst* invokeValue,
    IRType* payloadType,
    HashSet<IRInst*>& diagnosedAccesses,
    DiagnosticSink* sink)
{
    if (!isSemanticallyEmptyStructuralRayTracingPayloadType(payloadType))
        return true;

    auto invoke = as<IRFunc>(getResolvedInstForDecorations(invokeValue));
    if (!invoke)
        return true;
    HashSet<IRFunc*> visitedFunctions;
    auto access = _findReachableStructuralRayTracingPayloadAccess(invoke, visitedFunctions);
    if (!access)
        return true;

    if (diagnosedAccesses.add(access))
    {
        sink->diagnose(Diagnostics::StructuralRayTracingEmptyPayloadValueIr{
            .payloadType = payloadType,
            .location = access->sourceLoc});
    }
    return false;
}

bool validateStructuralRayTracingEntryPoint(IRFunc* entryPoint, DiagnosticSink* sink)
{
    auto info = entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
    if (!info)
        return true;

    HashSet<StructuralRayTracingDataDiagnosticKey> diagnosedTypes;
    HashSet<IRInst*> diagnosedEmptyPayloadAccesses;
    bool isValid = true;
    auto stageKind = StructuralRayTracingStageKind(info->getStageKind()->getValue());
    if (stageKind == StructuralRayTracingStageKind::ClosestHit ||
        stageKind == StructuralRayTracingStageKind::AnyHit ||
        stageKind == StructuralRayTracingStageKind::Miss)
    {
        isValid &= _validateStructuralRayTracingDataType(
            entryPoint,
            info->getPayloadType(),
            StructuralRayTracingDataRole::Payload,
            diagnosedTypes,
            sink);
        isValid &= _validateStructuralRayTracingEmptyPayloadAccess(
            info->getInvoke(),
            info->getPayloadType(),
            diagnosedEmptyPayloadAccesses,
            sink);
    }
    else if (stageKind == StructuralRayTracingStageKind::Callable)
    {
        isValid &= _validateStructuralRayTracingDataType(
            entryPoint,
            info->getCallableDataType(),
            StructuralRayTracingDataRole::Callable,
            diagnosedTypes,
            sink);
    }
    if ((stageKind == StructuralRayTracingStageKind::ClosestHit ||
         stageKind == StructuralRayTracingStageKind::AnyHit ||
         stageKind == StructuralRayTracingStageKind::Intersection) &&
        StructuralRayTracingHitAttributesKind(info->getHitAttributesKind()->getValue()) ==
            StructuralRayTracingHitAttributesKind::Custom)
    {
        isValid &= _validateStructuralRayTracingDataType(
            entryPoint,
            info->getHitAttributesType(),
            StructuralRayTracingDataRole::IntersectionAttribute,
            diagnosedTypes,
            sink);
    }
    isValid &= _validateStructuralRayTracingDataType(
        entryPoint,
        info->getRecordType(),
        StructuralRayTracingDataRole::Record,
        diagnosedTypes,
        sink);
    return isValid;
}

bool validateStructuralRayTracingSchemaOperation(IRInst* operation, DiagnosticSink* sink)
{
    auto traceOperation = as<IRStructuralRayTracingTrace>(operation);
    auto callOperation = as<IRStructuralRayTracingCallShader>(operation);
    SLANG_RELEASE_ASSERT(traceOperation || callOperation);

    IRType* schema = as<IRType>(
        traceOperation ? traceOperation->getProgramLayout() : callOperation->getProgramLayout());
    IRType* expectedTraceContext = as<IRType>(
        traceOperation ? traceOperation->getTraceContext() : callOperation->getTraceContext());
    SLANG_RELEASE_ASSERT(schema && expectedTraceContext);
    bool isValid = true;
    bool isPayloadServed = !traceOperation;
    HashSet<IRType*> hitGroups;
    HashSet<IRType*> missShaders;
    HashSet<IRType*> callableShaders;
    HashSet<StructuralRayTracingDataDiagnosticKey> diagnosedDataTypes;
    HashSet<IRInst*> diagnosedEmptyPayloadAccesses;
    IRType* schemaCallableDataType = nullptr;
    IRStructuralRayTracingCallableShaderInfoDecoration* firstCallableEntry = nullptr;

    if (traceOperation)
    {
        auto methodKind =
            StructuralRayTracingTraceMethodKind(traceOperation->getTraceMethodKind()->getValue());
        SLANG_RELEASE_ASSERT(methodKind != StructuralRayTracingTraceMethodKind::None);
        if (methodKind == StructuralRayTracingTraceMethodKind::ExplicitPayload &&
            isSemanticallyEmptyStructuralRayTracingPayloadType(traceOperation->getPayloadType()))
        {
            sink->diagnose(Diagnostics::StructuralRayTracingEmptyPayloadValueIr{
                .payloadType = traceOperation->getPayloadType(),
                .location = operation->sourceLoc});
            isValid = false;
        }
    }

    for (auto decoration : operation->getDecorations())
    {
        if (auto entry = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
        {
            auto closestHit = getStructuralRayTracingHitGroupStageInvoke(
                entry,
                StructuralRayTracingStageKind::ClosestHit);
            auto anyHit = getStructuralRayTracingHitGroupStageInvoke(
                entry,
                StructuralRayTracingStageKind::AnyHit);
            // Resolve the intersection stage here even though payload-access validation does not
            // consume it. This schema boundary validates all three presence records before any
            // target-specific adapter interprets the group.
            getStructuralRayTracingHitGroupStageInvoke(
                entry,
                StructuralRayTracingStageKind::Intersection);
            isValid &= _validateStructuralRayTracingDataType(
                operation,
                entry->getPayloadType(),
                StructuralRayTracingDataRole::Payload,
                diagnosedDataTypes,
                sink);
            isValid &= _validateStructuralRayTracingDataType(
                operation,
                entry->getRecordType(),
                StructuralRayTracingDataRole::Record,
                diagnosedDataTypes,
                sink);
            if (StructuralRayTracingHitAttributesKind(entry->getHitAttributesKind()->getValue()) ==
                StructuralRayTracingHitAttributesKind::Custom)
            {
                isValid &= _validateStructuralRayTracingDataType(
                    operation,
                    cast<IRType>(entry->getHitAttributesType()),
                    StructuralRayTracingDataRole::IntersectionAttribute,
                    diagnosedDataTypes,
                    sink);
            }
            isValid &= _validateStructuralRayTracingEmptyPayloadAccess(
                closestHit,
                entry->getPayloadType(),
                diagnosedEmptyPayloadAccesses,
                sink);
            isValid &= _validateStructuralRayTracingEmptyPayloadAccess(
                anyHit,
                entry->getPayloadType(),
                diagnosedEmptyPayloadAccesses,
                sink);
            isValid &= _validateStructuralRayTracingEntryTraceContext(
                operation,
                schema,
                expectedTraceContext,
                entry->getGroupType(),
                entry->getTraceContextType(),
                sink);
            isValid &= _insertStructuralRayTracingEntry(
                operation,
                schema,
                entry->getGroupType(),
                "hit-group",
                hitGroups,
                sink);
            // Two distinct payload declarations may have the same native layout. Serving a trace
            // is a source contract, so compare the specialization-aware semantic type operand,
            // not the ABI payload type that later legalization is free to rewrite.
            if (traceOperation &&
                entry->getPayloadSemanticType() == traceOperation->getPayloadSemanticType())
                isPayloadServed = true;
        }
        else if (auto entry = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration))
        {
            isValid &= _validateStructuralRayTracingDataType(
                operation,
                entry->getPayloadType(),
                StructuralRayTracingDataRole::Payload,
                diagnosedDataTypes,
                sink);
            isValid &= _validateStructuralRayTracingDataType(
                operation,
                entry->getRecordType(),
                StructuralRayTracingDataRole::Record,
                diagnosedDataTypes,
                sink);
            isValid &= _validateStructuralRayTracingEmptyPayloadAccess(
                entry->getMiss(),
                entry->getPayloadType(),
                diagnosedEmptyPayloadAccesses,
                sink);
            isValid &= _validateStructuralRayTracingEntryTraceContext(
                operation,
                schema,
                expectedTraceContext,
                entry->getMissType(),
                entry->getTraceContextType(),
                sink);
            isValid &= _insertStructuralRayTracingEntry(
                operation,
                schema,
                entry->getMissType(),
                "miss-shader",
                missShaders,
                sink);
            if (traceOperation &&
                entry->getPayloadSemanticType() == traceOperation->getPayloadSemanticType())
                isPayloadServed = true;
        }
        else if (auto entry = as<IRStructuralRayTracingCallableShaderInfoDecoration>(decoration))
        {
            // Metal exposes one schema-wide callable visible-function table, so every callable
            // entry must share one ABI. Validate that invariant from the complete linked schema,
            // even when the operation that activated the schema is a trace rather than a callable
            // dispatch. Function index zero is the declaration-order source of truth.
            if (!firstCallableEntry || entry->getFunctionIndex()->getValue() <
                                           firstCallableEntry->getFunctionIndex()->getValue())
            {
                firstCallableEntry = entry;
                schemaCallableDataType = as<IRType>(entry->getCallableDataType());
            }
            isValid &= _validateStructuralRayTracingDataType(
                operation,
                entry->getRecordType(),
                StructuralRayTracingDataRole::Record,
                diagnosedDataTypes,
                sink);
            isValid &= _validateStructuralRayTracingDataType(
                operation,
                cast<IRType>(entry->getCallableDataType()),
                StructuralRayTracingDataRole::Callable,
                diagnosedDataTypes,
                sink);
            isValid &= _validateStructuralRayTracingEntryTraceContext(
                operation,
                schema,
                expectedTraceContext,
                entry->getCallableType(),
                entry->getTraceContextType(),
                sink);
            isValid &= _insertStructuralRayTracingEntry(
                operation,
                schema,
                entry->getCallableType(),
                "callable-shader",
                callableShaders,
                sink);
        }
    }

    if (firstCallableEntry)
    {
        for (auto decoration : operation->getDecorations())
        {
            auto entry = as<IRStructuralRayTracingCallableShaderInfoDecoration>(decoration);
            if (!entry || entry->getCallableDataType() == schemaCallableDataType)
                continue;
            sink->diagnose(Diagnostics::StructuralRayTracingCallableDataMismatch{
                .shader = entry->getCallableType(),
                .actualType = entry->getCallableDataType(),
                .expectedType = schemaCallableDataType,
                .location = operation->sourceLoc});
            isValid = false;
        }
    }
    if (callOperation)
    {
        if (!firstCallableEntry)
        {
            sink->diagnose(Diagnostics::StructuralRayTracingCallWithoutShaders{
                .location = operation->sourceLoc});
            isValid = false;
        }
        else if (callOperation->getCallableDataType() != schemaCallableDataType)
        {
            sink->diagnose(Diagnostics::StructuralRayTracingCallableDataMismatch{
                .shader = firstCallableEntry->getCallableType(),
                .actualType = firstCallableEntry->getCallableDataType(),
                .expectedType = callOperation->getCallableDataType(),
                .location = operation->sourceLoc});
            isValid = false;
        }
    }

    if (!isPayloadServed)
    {
        sink->diagnose(Diagnostics::StructuralRayTracingPayloadNotServed{
            .schema = schema,
            .payloadType = traceOperation->getPayloadType(),
            .location = operation->sourceLoc});
        isValid = false;
    }
    return isValid;
}

static IRIntegerValue _materializeStructuralRayTracingPayloadLocation(
    IRModule* module,
    IRInst* schemaOperation,
    IRType* payloadType,
    IRType* payloadSemanticType,
    IRIntegerValue location)
{
    auto moduleInst = module->getModuleInst();
    // Manifest construction assigns the location before any per-entry clone. Materialization only
    // publishes that already-decided integer beside the local cloned type; it never attempts to
    // recover semantic identity across a later specialization boundary.
    SLANG_RELEASE_ASSERT(location >= 0);

    IRBuilder builder(module);
    addStructuralRayTracingProgramPayloadLocation(
        builder,
        moduleInst,
        payloadType,
        payloadSemanticType,
        location);
    if (schemaOperation)
    {
        addStructuralRayTracingProgramPayloadLocation(
            builder,
            schemaOperation,
            payloadType,
            payloadSemanticType,
            location);
    }
    return location;
}

static void _materializeStructuralRayTracingPayloadLocations(
    IRModule* module,
    const List<IRInst*>& programOperations)
{
    for (auto inst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(inst);
        auto info =
            func ? func->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>() : nullptr;
        if (!info || as<IRVoidType>(info->getPayloadType()))
            continue;
        auto location = _materializeStructuralRayTracingPayloadLocation(
            module,
            nullptr,
            info->getPayloadType(),
            info->getPayloadSemanticType(),
            info->getPayloadLocation()->getValue());
        SLANG_RELEASE_ASSERT(info->getPayloadLocation()->getValue() == location);
    }

    for (auto operation : programOperations)
    {
        if (auto trace = as<IRStructuralRayTracingTrace>(operation))
        {
            _materializeStructuralRayTracingPayloadLocation(
                module,
                operation,
                trace->getPayloadType(),
                trace->getPayloadSemanticType(),
                trace->getPayloadLocation()->getValue());
        }
        for (auto decoration : operation->getDecorations())
        {
            if (auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
            {
                if (as<IRVoidType>(group->getPayloadType()))
                    continue;
                _materializeStructuralRayTracingPayloadLocation(
                    module,
                    operation,
                    group->getPayloadType(),
                    group->getPayloadSemanticType(),
                    group->getPayloadLocation()->getValue());
            }
            else if (auto entry = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration))
            {
                _materializeStructuralRayTracingPayloadLocation(
                    module,
                    operation,
                    entry->getPayloadType(),
                    entry->getPayloadSemanticType(),
                    entry->getPayloadLocation()->getValue());
            }
        }
    }
}

void preparePortableStructuralRayTracingEntryPoints(IRModule* module, List<IRFunc*>& ioEntryPoints)
{
    List<StructuralRayTracingGeneratedEntryPoint> generated;
    List<IRFunc*> structuralEntryPoints;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(inst))
        {
            if (func->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
                structuralEntryPoints.add(func);
        }
    }

    List<IRFunc*> selectedEntryPoints = ioEntryPoints;
    ioEntryPoints.clear();
    for (auto entryPoint : selectedEntryPoints)
    {
        if (!entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
            ioEntryPoints.add(entryPoint);
    }

    for (auto entryPoint : structuralEntryPoints)
    {
        auto info = entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
        auto stageKind = StructuralRayTracingStageKind(info->getStageKind()->getValue());
        _generateStructuralRayTracingEntryPoint(
            module,
            ioEntryPoints,
            generated,
            stageKind,
            info->getStageType(),
            info->getStageSourceTypeName(),
            info->getStageTypeIdentity(),
            true,
            info->getInvoke(),
            info->getContextType(),
            info->getPayloadType(),
            info->getRecordType(),
            info->getHitAttributesType(),
            StructuralRayTracingHitAttributesKind(info->getHitAttributesKind()->getValue()),
            info->getCallableDataType(),
            info->getPayloadSemanticType(),
            info->getPayloadLocation()->getValue());
        if (auto entryPointDecoration = entryPoint->findDecoration<IREntryPointDecoration>())
            entryPointDecoration->removeAndDeallocate();
        info->removeAndDeallocate();
    }
}

void synthesizePortableStructuralRayTracingEntryPoints(
    IRModule* module,
    List<IRFunc*>& ioEntryPoints,
    DiagnosticSink* sink)
{
    List<IRInst*> programOperations;
    _collectProgramOperations(module->getModuleInst(), programOperations);
    _materializeStructuralRayTracingPayloadLocations(module, programOperations);
    List<StructuralRayTracingGeneratedEntryPoint> generated;

    for (auto entryPoint : ioEntryPoints)
    {
        if (auto info =
                entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
        {
            if (!validateStructuralRayTracingEntryPoint(entryPoint, sink))
                continue;
            auto entryPointDecoration = entryPoint->findDecoration<IREntryPointDecoration>();
            SLANG_RELEASE_ASSERT(entryPointDecoration);
            auto stageKind = StructuralRayTracingStageKind(info->getStageKind()->getValue());
            auto physicalName = entryPointDecoration->getName()->getStringSlice();
            if (_findGeneratedStructuralRayTracingEntryPoint(
                    generated,
                    stageKind,
                    info->getStageTypeIdentity(),
                    physicalName))
            {
                continue;
            }
            if (!_validateStructuralRayTracingPhysicalNameOwner(
                    generated,
                    stageKind,
                    info->getStageSourceTypeName(),
                    info->getStageTypeIdentity(),
                    physicalName,
                    info->getInvoke()->sourceLoc,
                    sink))
            {
                continue;
            }
            generated.add(
                {stageKind,
                 info->getStageSourceTypeName(),
                 info->getStageTypeIdentity(),
                 String(physicalName),
                 entryPoint});
        }
    }

    for (auto operation : programOperations)
    {
        // Schema validation is the boundary at which every linked structural entry is visible.
        // Do not synthesize target entry points from an invalid schema: doing so would produce
        // follow-on diagnostics from entry points whose contexts or payloads are already known to
        // be incompatible with this operation.
        if (!validateStructuralRayTracingSchemaOperation(operation, sink))
            continue;
        for (auto decoration : operation->getDecorations())
        {
            if (auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
            {
                auto hitAttributesKind = StructuralRayTracingHitAttributesKind(
                    group->getHitAttributesKind()->getValue());
                auto payloadLocation = group->getPayloadLocation()->getValue();
                SLANG_RELEASE_ASSERT(payloadLocation >= 0);
                if (hitAttributesKind == StructuralRayTracingHitAttributesKind::Curve)
                {
                    sink->diagnose(Diagnostics::StructuralRayTracingCurveRequiresMetal{
                        .location = operation->sourceLoc});
                    continue;
                }
                _generateStructuralRayTracingEntryPoint(
                    module,
                    ioEntryPoints,
                    generated,
                    StructuralRayTracingStageKind::ClosestHit,
                    group->getClosestHitType(),
                    group->getClosestHitSourceTypeName(),
                    group->getClosestHitTypeIdentity(),
                    group->getHasClosestHit()->getValue(),
                    group->getClosestHit(),
                    group->getContextType(),
                    group->getPayloadType(),
                    group->getRecordType(),
                    group->getHitAttributesType(),
                    hitAttributesKind,
                    nullptr,
                    group->getPayloadSemanticType(),
                    payloadLocation,
                    sink,
                    operation);
                _generateStructuralRayTracingEntryPoint(
                    module,
                    ioEntryPoints,
                    generated,
                    StructuralRayTracingStageKind::AnyHit,
                    group->getAnyHitType(),
                    group->getAnyHitSourceTypeName(),
                    group->getAnyHitTypeIdentity(),
                    group->getHasAnyHit()->getValue(),
                    group->getAnyHit(),
                    group->getContextType(),
                    group->getPayloadType(),
                    group->getRecordType(),
                    group->getHitAttributesType(),
                    hitAttributesKind,
                    nullptr,
                    group->getPayloadSemanticType(),
                    payloadLocation,
                    sink,
                    operation);
                _generateStructuralRayTracingEntryPoint(
                    module,
                    ioEntryPoints,
                    generated,
                    StructuralRayTracingStageKind::Intersection,
                    group->getIntersectionType(),
                    group->getIntersectionSourceTypeName(),
                    group->getIntersectionTypeIdentity(),
                    group->getHasIntersection()->getValue(),
                    group->getIntersection(),
                    group->getContextType(),
                    nullptr,
                    group->getRecordType(),
                    group->getHitAttributesType(),
                    hitAttributesKind,
                    nullptr,
                    nullptr,
                    -1,
                    sink,
                    operation);
            }
            else if (auto entry = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration))
            {
                auto payloadLocation = entry->getPayloadLocation()->getValue();
                SLANG_RELEASE_ASSERT(payloadLocation >= 0);
                _generateStructuralRayTracingEntryPoint(
                    module,
                    ioEntryPoints,
                    generated,
                    StructuralRayTracingStageKind::Miss,
                    entry->getMissType(),
                    entry->getMissSourceTypeName(),
                    entry->getMissTypeIdentity(),
                    true,
                    entry->getMiss(),
                    entry->getContextType(),
                    entry->getPayloadType(),
                    entry->getRecordType(),
                    nullptr,
                    StructuralRayTracingHitAttributesKind::None,
                    nullptr,
                    entry->getPayloadSemanticType(),
                    payloadLocation,
                    sink,
                    operation);
            }
            else if (
                auto entry = as<IRStructuralRayTracingCallableShaderInfoDecoration>(decoration))
            {
                _generateStructuralRayTracingEntryPoint(
                    module,
                    ioEntryPoints,
                    generated,
                    StructuralRayTracingStageKind::Callable,
                    entry->getCallableType(),
                    entry->getCallableSourceTypeName(),
                    entry->getCallableTypeIdentity(),
                    true,
                    entry->getCallable(),
                    entry->getContextType(),
                    nullptr,
                    entry->getRecordType(),
                    nullptr,
                    StructuralRayTracingHitAttributesKind::None,
                    entry->getCallableDataType(),
                    nullptr,
                    -1,
                    sink,
                    operation);
            }
        }
    }
}

struct StructuralRayTracingStageParameterThreader
{
    StructuralRayTracingStageParameterThreader(
        IRModule* module,
        IRType* parameterType,
        LayoutResourceKind resourceKind,
        const char* parameterName,
        const char* semanticName,
        bool isInput,
        bool isOutput,
        IRType* entryPointParameterType = nullptr,
        IRStructKey* entryPointValueKey = nullptr)
        : m_module(module)
        , m_parameterType(parameterType)
        , m_resourceKind(resourceKind)
        , m_parameterName(parameterName)
        , m_semanticName(semanticName)
        , m_isInput(isInput)
        , m_isOutput(isOutput)
        , m_entryPointParameterType(entryPointParameterType)
        , m_entryPointValueKey(entryPointValueKey)
    {
    }

    IRModule* m_module;
    IRType* m_parameterType;
    LayoutResourceKind m_resourceKind;
    const char* m_parameterName;
    const char* m_semanticName;
    bool m_isInput;
    bool m_isOutput;
    IRType* m_entryPointParameterType;
    IRStructKey* m_entryPointValueKey;
    Dictionary<IRFunc*, IRInst*> m_parameters;

    void registerParameter(IRFunc* func, IRInst* parameter) { m_parameters[func] = parameter; }

    IRFunc* findEnclosingFunc(IRInst* inst)
    {
        for (auto parent = inst; parent; parent = parent->getParent())
        {
            if (auto func = as<IRFunc>(parent))
                return func;
        }
        return nullptr;
    }

    IRInst* findOrCreateParameter(IRInst* inst)
    {
        auto func = findEnclosingFunc(inst);
        SLANG_ASSERT(func);
        return findOrCreateParameter(func);
    }

    IRInst* findOrCreateParameter(IRFunc* func)
    {
        if (auto found = m_parameters.tryGetValue(func))
            return *found;

        auto firstBlock = func->getFirstBlock();
        SLANG_ASSERT(firstBlock);

        IRBuilder builder(m_module);
        auto entryPointDecoration = func->findDecoration<IREntryPointDecoration>();
        auto physicalParameterType = entryPointDecoration && m_entryPointParameterType
                                         ? m_entryPointParameterType
                                         : m_parameterType;
        auto parameter = builder.createParam(physicalParameterType);
        builder.addNameHintDecoration(parameter, UnownedTerminatedStringSlice(m_parameterName));
        parameter->insertBefore(firstBlock->getFirstOrdinaryInst());

        IRInst* parameterValue = parameter;
        if (entryPointDecoration && m_entryPointValueKey)
        {
            builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());
            parameterValue =
                builder.emitFieldExtract(m_parameterType, parameter, m_entryPointValueKey);
        }
        m_parameters.add(func, parameterValue);

        if (entryPointDecoration)
        {
            if (m_isInput)
                builder.addSimpleDecoration<IRGlobalInputDecoration>(parameter);
            if (m_isOutput)
                builder.addSimpleDecoration<IRGlobalOutputDecoration>(parameter);
            if (m_semanticName)
            {
                builder.addSemanticDecoration(
                    parameter,
                    UnownedTerminatedStringSlice(m_semanticName));
            }

            IRTypeLayout::Builder typeLayoutBuilder(&builder);
            typeLayoutBuilder.addResourceUsage(m_resourceKind, LayoutSize(1));
            IRVarLayout::Builder varLayoutBuilder(&builder, typeLayoutBuilder.build());
            varLayoutBuilder.findOrAddResourceInfo(m_resourceKind);
            varLayoutBuilder.setStage(entryPointDecoration->getProfile().getStage());
            builder.addLayoutDecoration(parameter, varLayoutBuilder.build());
        }

        fixUpFuncType(func);

        // Register the parameter before rewriting callers so recursive call graphs terminate.
        List<IRCall*> callUses;
        for (auto use = func->firstUse; use; use = use->nextUse)
        {
            if (auto call = as<IRCall>(use->getUser()))
            {
                if (call->getCallee() == func)
                    callUses.add(call);
            }
        }

        for (auto call : callUses)
        {
            List<IRInst*> args;
            for (UInt i = 0; i < call->getArgCount(); ++i)
                args.add(call->getArg(i));
            args.add(findOrCreateParameter(call));

            builder.setInsertBefore(call);
            auto newCall = builder.emitCallInst(
                call->getDataType(),
                call->getCallee(),
                args.getCount(),
                args.getBuffer());
            call->replaceUsesWith(newCall);
            call->removeAndDeallocate();
        }

        return parameterValue;
    }

    void lower(IRInst* operation)
    {
        auto parameter = findOrCreateParameter(operation);
        operation->replaceUsesWith(parameter);
    }
};

static void _collectStageInputOperations(IRInst* parent, List<IRInst*>& operations)
{
    for (auto child = parent->getFirstChild(); child; child = child->getNextInst())
    {
        _collectStageInputOperations(child, operations);
        if (as<IRStructuralRayTracingStageInputOperation>(child))
            operations.add(child);
    }
}

static void _collectStructuralEntryPoints(IRModule* module, List<IRFunc*>& entryPoints)
{
    for (auto child = module->getModuleInst()->getFirstChild(); child; child = child->getNextInst())
    {
        if (auto func = as<IRFunc>(child))
        {
            if (func->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
                entryPoints.add(func);
        }
    }
}

void lowerMetalStructuralRayTracingStageInputOperations(
    IRModule* module,
    const Dictionary<IRFunc*, IRInst*>& entryPointPayloadValues)
{
    List<IRInst*> operations;
    _collectStageInputOperations(module->getModuleInst(), operations);
    List<IRFunc*> structuralEntryPoints;
    _collectStructuralEntryPoints(module, structuralEntryPoints);

    HashSet<IRType*> payloadTypes;
    for (auto entryPoint : structuralEntryPoints)
    {
        if (!entryPoint->findDecoration<IRMetalVisibleFunctionDecoration>() &&
            !entryPoint->findDecoration<IRMetalIntersectionFunctionDecoration>())
            continue;
        auto info = entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
        if (info && !as<IRVoidType>(info->getPayloadType()))
            payloadTypes.add(info->getPayloadType());
    }
    for (auto operation : operations)
    {
        if (operation->getOp() != kIROp_StructuralRayTracingGetPayload)
            continue;
        auto payloadPointerType = as<IRPtrTypeBase>(operation->getDataType());
        SLANG_ASSERT(payloadPointerType);
        payloadTypes.add(payloadPointerType->getValueType());
    }

    HashSet<IRInst*> loweredOperations;
    for (auto payloadType : payloadTypes)
    {
        IRBuilder builder(module);
        StructuralRayTracingStageParameterThreader threader(
            module,
            builder.getPtrType(payloadType, AddressSpace::ThreadLocal),
            LayoutResourceKind::RayPayload,
            "payload",
            nullptr,
            false,
            false);
        for (auto entryPoint : structuralEntryPoints)
        {
            auto info =
                entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
            if (!info || info->getPayloadType() != payloadType)
                continue;
            if (auto payloadValue = entryPointPayloadValues.tryGetValue(entryPoint))
                threader.registerParameter(entryPoint, *payloadValue);
        }
        for (auto operation : operations)
        {
            if (operation->getOp() != kIROp_StructuralRayTracingGetPayload)
                continue;
            auto payloadPtrType = as<IRPtrTypeBase>(operation->getDataType());
            SLANG_ASSERT(payloadPtrType);
            if (payloadPtrType->getValueType() == payloadType)
            {
                threader.lower(operation);
                loweredOperations.add(operation);
            }
        }
    }

    for (auto operation : loweredOperations)
        operation->removeAndDeallocate();
    loweredOperations.clear();

    // The generated Metal adapters have already replaced record reads with descriptor-buffer
    // loads. Thread any copy left in the exported source helper through an ordinary parameter so
    // the compiler-owned operation cannot reach general type legalization.
    HashSet<IRType*> remainingRecordTypes;
    operations.clear();
    _collectStageInputOperations(module->getModuleInst(), operations);
    for (auto operation : operations)
    {
        if (operation->getOp() == kIROp_StructuralRayTracingGetRecord)
            remainingRecordTypes.add(operation->getDataType());
    }

    for (auto recordType : remainingRecordTypes)
    {
        IRBuilder builder(module);
        StructuralRayTracingStageParameterThreader threader(
            module,
            builder.getPtrType(recordType, AddressSpace::ThreadLocal),
            LayoutResourceKind::ShaderRecord,
            "record",
            nullptr,
            false,
            false);
        for (auto operation : operations)
        {
            if (operation->getOp() == kIROp_StructuralRayTracingGetRecord &&
                operation->getDataType() == recordType)
            {
                builder.setInsertBefore(operation);
                auto record = builder.emitLoad(threader.findOrCreateParameter(operation));
                operation->replaceUsesWith(record);
                loweredOperations.add(operation);
            }
        }
    }

    for (auto operation : loweredOperations)
        operation->removeAndDeallocate();
    loweredOperations.clear();

    // Callable adapters bind the source data property to their thread-local visible-function
    // parameter. Thread any copy left in the exported source helper through the same ordinary
    // pointer type so the compiler-owned property operation cannot survive to legalization.
    HashSet<IRType*> remainingCallableDataTypes;
    operations.clear();
    _collectStageInputOperations(module->getModuleInst(), operations);
    for (auto operation : operations)
    {
        if (operation->getOp() != kIROp_StructuralRayTracingGetCallableData)
            continue;
        auto dataPointerType = as<IRPtrTypeBase>(operation->getDataType());
        SLANG_ASSERT(dataPointerType);
        remainingCallableDataTypes.add(dataPointerType->getValueType());
    }

    for (auto dataType : remainingCallableDataTypes)
    {
        IRBuilder builder(module);
        StructuralRayTracingStageParameterThreader threader(
            module,
            builder.getPtrType(dataType, AddressSpace::ThreadLocal),
            LayoutResourceKind::CallablePayload,
            "data",
            nullptr,
            true,
            true);
        for (auto operation : operations)
        {
            if (operation->getOp() != kIROp_StructuralRayTracingGetCallableData)
                continue;
            auto dataPointerType = cast<IRPtrTypeBase>(operation->getDataType());
            if (dataPointerType->getValueType() == dataType)
            {
                operation->replaceUsesWith(threader.findOrCreateParameter(operation));
                loweredOperations.add(operation);
            }
        }
    }

    for (auto operation : loweredOperations)
        operation->removeAndDeallocate();
    loweredOperations.clear();

    // Generated Metal adapters consume hit attributes from their native parameters or ray-data
    // state before reaching this point. A selected source-stage implementation remains exported as
    // an ordinary helper, though, so thread any attributes left in that helper graph through an
    // ordinary parameter. This keeps compiler-owned aggregate operations out of general type
    // legalization without assigning a native Metal stage ABI to the source helper itself.
    HashSet<IRType*> remainingHitAttributeTypes;
    operations.clear();
    _collectStageInputOperations(module->getModuleInst(), operations);
    for (auto operation : operations)
    {
        if (operation->getOp() == kIROp_StructuralRayTracingGetHitAttributes)
            remainingHitAttributeTypes.add(operation->getDataType());
    }

    for (auto attributeType : remainingHitAttributeTypes)
    {
        IRBuilder builder(module);
        StructuralRayTracingStageParameterThreader threader(
            module,
            builder.getPtrType(attributeType, AddressSpace::ThreadLocal),
            LayoutResourceKind::HitAttributes,
            "attributes",
            nullptr,
            false,
            false);
        for (auto operation : operations)
        {
            if (operation->getOp() == kIROp_StructuralRayTracingGetHitAttributes &&
                operation->getDataType() == attributeType)
            {
                builder.setInsertBefore(operation);
                auto attributes = builder.emitLoad(threader.findOrCreateParameter(operation));
                operation->replaceUsesWith(attributes);
                loweredOperations.add(operation);
            }
        }
    }

    for (auto operation : loweredOperations)
        operation->removeAndDeallocate();

    operations.clear();
    _collectStageInputOperations(module->getModuleInst(), operations);
    IRBuilder builder(module);
    for (auto operation : operations)
    {
        auto stageInputOperation = cast<IRStructuralRayTracingStageInputOperation>(operation);
        if (!stageInputOperation->hasFallback())
            continue;

        builder.setInsertBefore(operation);
        List<IRInst*> arguments;
        for (UInt i = 1; i < operation->getOperandCount(); ++i)
            arguments.add(operation->getOperand(i));
        auto call = builder.emitCallInst(
            operation->getDataType(),
            stageInputOperation->getFallback(),
            arguments.getCount(),
            arguments.getBuffer());
        operation->replaceUsesWith(call);
        operation->removeAndDeallocate();
    }
}

static StructuralRayTracingHitAttributesKind _getHitAttributesKind(
    IRStructuralRayTracingEntryPointInfoDecoration* info)
{
    return StructuralRayTracingHitAttributesKind(info->getHitAttributesKind()->getValue());
}

// Returns whether the native stage receives intersection attributes from traversal.
//
// An intersection shader produces custom attributes through ReportHit; it does not receive those
// attributes as an entry-point parameter. The accepted candidate is passed to closest-hit or
// any-hit instead, so only those two stages use the native SV_IntersectionAttributes input.
static bool _stageReceivesStructuralRayTracingHitAttributes(
    IRStructuralRayTracingEntryPointInfoDecoration* info)
{
    auto stageKind = StructuralRayTracingStageKind(info->getStageKind()->getValue());
    return stageKind == StructuralRayTracingStageKind::ClosestHit ||
           stageKind == StructuralRayTracingStageKind::AnyHit;
}

static void _addUniqueStructuralRayTracingPayloadType(
    List<IRType*>& payloadTypes,
    HashSet<IRType*>& seenPayloadTypes,
    IRType* payloadType)
{
    if (as<IRVoidType>(payloadType) || !seenPayloadTypes.add(payloadType))
        return;
    payloadTypes.add(payloadType);
}

void lowerPortableStructuralRayTracingStageInputOperations(IRModule* module)
{
    List<IRInst*> operations;
    _collectStageInputOperations(module->getModuleInst(), operations);
    List<IRFunc*> structuralEntryPoints;
    _collectStructuralEntryPoints(module, structuralEntryPoints);

    List<IRType*> loweredPayloadTypes;
    HashSet<IRType*> seenPayloadTypes;
    for (auto entryPoint : structuralEntryPoints)
    {
        auto info = entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
        _addUniqueStructuralRayTracingPayloadType(
            loweredPayloadTypes,
            seenPayloadTypes,
            info->getPayloadType());
    }
    for (auto operation : operations)
    {
        if (operation->getOp() != kIROp_StructuralRayTracingGetPayload)
            continue;

        auto payloadPtrType = as<IRPtrTypeBase>(operation->getDataType());
        SLANG_ASSERT(payloadPtrType);
        _addUniqueStructuralRayTracingPayloadType(
            loweredPayloadTypes,
            seenPayloadTypes,
            payloadPtrType->getValueType());
    }

    for (auto entryPoint : structuralEntryPoints)
    {
        auto info = entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
        auto payloadType = info->getPayloadType();
        if (as<IRVoidType>(payloadType))
            continue;

        auto location = info->getPayloadLocation()->getValue();
        SLANG_RELEASE_ASSERT(location >= 0);
        SLANG_RELEASE_ASSERT(
            findStructuralRayTracingProgramPayloadLocation(
                module->getModuleInst(),
                info->getPayloadSemanticType()) == location);
    }

    for (auto payloadType : loweredPayloadTypes)
    {
        IRBuilder builder(module);
        if (!payloadType->findDecoration<IRRayPayloadDecoration>())
            builder.addRayPayloadDecoration(payloadType);
        StructuralRayTracingStageParameterThreader threader(
            module,
            builder.getBorrowInOutParamType(payloadType),
            LayoutResourceKind::RayPayload,
            "payload",
            "SV_RayPayload",
            true,
            true);
        for (auto entryPoint : structuralEntryPoints)
        {
            auto info =
                entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
            if (info->getPayloadType() == payloadType)
            {
                auto parameter = threader.findOrCreateParameter(entryPoint);
                // Two semantic payloads can specialize to the same physical IR type. The helper
                // parameter may therefore be shared by type, but each native entry point must use
                // the location selected by its own semantic identity.
                builder.addVulkanRayPayloadInDecoration(
                    parameter,
                    info->getPayloadLocation()->getValue());
            }
        }
        for (auto candidate : operations)
        {
            if (candidate->getOp() != kIROp_StructuralRayTracingGetPayload)
                continue;
            auto candidatePtrType = as<IRPtrTypeBase>(candidate->getDataType());
            SLANG_ASSERT(candidatePtrType);
            if (candidatePtrType->getValueType() == payloadType)
                threader.lower(candidate);
        }
    }

    HashSet<IRType*> loweredRecordTypes;
    for (auto entryPoint : structuralEntryPoints)
    {
        auto info = entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
        auto recordType = info->getRecordType();
        if (!as<IRVoidType>(recordType))
            loweredRecordTypes.add(recordType);
    }
    for (auto operation : operations)
    {
        if (operation->getOp() == kIROp_StructuralRayTracingGetRecord)
            loweredRecordTypes.add(operation->getDataType());
    }

    for (auto recordType : loweredRecordTypes)
    {
        IRBuilder builder(module);
        StructuralRayTracingStageParameterThreader threader(
            module,
            recordType,
            LayoutResourceKind::ShaderRecord,
            "record",
            nullptr,
            false,
            false);
        for (auto entryPoint : structuralEntryPoints)
        {
            auto info =
                entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
            if (info->getRecordType() == recordType)
            {
                builder.setInsertBefore(entryPoint);
                auto recordBufferType = builder.getConstantBufferType(
                    recordType,
                    builder.getType(kIROp_DefaultBufferLayoutType));
                auto recordBuffer = builder.createGlobalParam(recordBufferType);
                builder.addNameHintDecoration(recordBuffer, UnownedTerminatedStringSlice("record"));
                builder.addEntryPointParamDecoration(recordBuffer, entryPoint);

                IRTypeLayout::Builder typeLayoutBuilder(&builder);
                typeLayoutBuilder.addResourceUsage(LayoutResourceKind::ShaderRecord, LayoutSize(1));
                IRVarLayout::Builder varLayoutBuilder(&builder, typeLayoutBuilder.build());
                varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::ShaderRecord);
                if (auto entryPointDecoration =
                        entryPoint->findDecoration<IREntryPointDecoration>())
                    varLayoutBuilder.setStage(entryPointDecoration->getProfile().getStage());
                builder.addLayoutDecoration(recordBuffer, varLayoutBuilder.build());

                builder.setInsertBefore(entryPoint->getFirstBlock()->getFirstOrdinaryInst());
                threader.registerParameter(entryPoint, builder.emitLoad(recordBuffer));
            }
        }
        for (auto candidate : operations)
        {
            if (candidate->getOp() == kIROp_StructuralRayTracingGetRecord &&
                candidate->getDataType() == recordType)
            {
                threader.lower(candidate);
            }
        }
    }

    HashSet<IRType*> loweredCallableDataTypes;
    for (auto entryPoint : structuralEntryPoints)
    {
        auto info = entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
        auto callableDataType = info->getCallableDataType();
        if (!as<IRVoidType>(callableDataType))
            loweredCallableDataTypes.add(callableDataType);
    }
    for (auto operation : operations)
    {
        if (operation->getOp() != kIROp_StructuralRayTracingGetCallableData)
            continue;

        auto callableDataPtrType = as<IRPtrTypeBase>(operation->getDataType());
        SLANG_ASSERT(callableDataPtrType);
        loweredCallableDataTypes.add(callableDataPtrType->getValueType());
    }

    for (auto callableDataType : loweredCallableDataTypes)
    {
        IRBuilder builder(module);
        StructuralRayTracingStageParameterThreader threader(
            module,
            builder.getBorrowInOutParamType(callableDataType),
            LayoutResourceKind::CallablePayload,
            "data",
            nullptr,
            true,
            true);
        for (auto entryPoint : structuralEntryPoints)
        {
            auto info =
                entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
            if (info->getCallableDataType() == callableDataType)
                threader.findOrCreateParameter(entryPoint);
        }
        for (auto candidate : operations)
        {
            if (candidate->getOp() != kIROp_StructuralRayTracingGetCallableData)
                continue;
            auto candidatePtrType = as<IRPtrTypeBase>(candidate->getDataType());
            SLANG_ASSERT(candidatePtrType);
            if (candidatePtrType->getValueType() == callableDataType)
                threader.lower(candidate);
        }
    }

    HashSet<IRType*> loweredHitAttributeTypes;
    for (auto entryPoint : structuralEntryPoints)
    {
        auto info = entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
        if (_getHitAttributesKind(info) == StructuralRayTracingHitAttributesKind::Custom)
            loweredHitAttributeTypes.add(info->getHitAttributesType());
    }
    for (auto operation : operations)
    {
        if (operation->getOp() != kIROp_StructuralRayTracingGetHitAttributes)
            continue;

        auto attributeType = operation->getDataType();
        loweredHitAttributeTypes.add(attributeType);
    }

    for (auto attributeType : loweredHitAttributeTypes)
    {
        StructuralRayTracingStageParameterThreader threader(
            module,
            attributeType,
            LayoutResourceKind::HitAttributes,
            "attributes",
            "SV_IntersectionAttributes",
            true,
            false);
        for (auto entryPoint : structuralEntryPoints)
        {
            auto info =
                entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
            if (_stageReceivesStructuralRayTracingHitAttributes(info) &&
                _getHitAttributesKind(info) == StructuralRayTracingHitAttributesKind::Custom &&
                info->getHitAttributesType() == attributeType)
            {
                threader.findOrCreateParameter(entryPoint);
            }
        }
        for (auto candidate : operations)
        {
            if (candidate->getOp() == kIROp_StructuralRayTracingGetHitAttributes &&
                candidate->getDataType() == attributeType)
            {
                threader.lower(candidate);
            }
        }
    }

    bool needsTriangleHitAttributes = false;
    for (auto entryPoint : structuralEntryPoints)
    {
        auto info = entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
        if (_getHitAttributesKind(info) == StructuralRayTracingHitAttributesKind::Triangle)
            needsTriangleHitAttributes = true;
    }
    for (auto operation : operations)
    {
        if (operation->getOp() == kIROp_StructuralRayTracingGetTriangleBarycentricCoord)
            needsTriangleHitAttributes = true;
    }

    if (needsTriangleHitAttributes)
    {
        IRBuilder builder(module);
        auto barycentricType = builder.getVectorType(builder.getFloatType(), 2);
        auto nativeAttributeType = builder.createStructType();
        builder.addNameHintDecoration(
            nativeAttributeType,
            UnownedTerminatedStringSlice("StructuralTriangleHitAttributes"));
        auto barycentricKey = builder.createStructKey();
        builder.addNameHintDecoration(barycentricKey, UnownedTerminatedStringSlice("barycentrics"));
        builder.createStructField(nativeAttributeType, barycentricKey, barycentricType);

        StructuralRayTracingStageParameterThreader threader(
            module,
            barycentricType,
            LayoutResourceKind::HitAttributes,
            "attributes",
            "SV_IntersectionAttributes",
            true,
            false,
            nativeAttributeType,
            barycentricKey);
        for (auto entryPoint : structuralEntryPoints)
        {
            auto info =
                entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
            if (_stageReceivesStructuralRayTracingHitAttributes(info) &&
                _getHitAttributesKind(info) == StructuralRayTracingHitAttributesKind::Triangle)
                threader.findOrCreateParameter(entryPoint);
        }
        for (auto candidate : operations)
        {
            if (candidate->getOp() == kIROp_StructuralRayTracingGetTriangleBarycentricCoord &&
                candidate->getDataType() == barycentricType)
            {
                threader.lower(candidate);
            }
        }
    }

    // The type-specific threaders above remove the operations they lower. Recollect the surviving
    // operations instead of walking the original list, which contains pointers to deallocated IR.
    operations.clear();
    _collectStageInputOperations(module->getModuleInst(), operations);

    IRBuilder builder(module);
    for (auto operation : operations)
    {
        auto stageInputOperation = cast<IRStructuralRayTracingStageInputOperation>(operation);
        if (stageInputOperation->hasFallback())
        {
            builder.setInsertBefore(operation);

            List<IRInst*> arguments;
            for (UInt i = 1; i < operation->getOperandCount(); ++i)
                arguments.add(operation->getOperand(i));

            auto call = builder.emitCallInst(
                operation->getDataType(),
                stageInputOperation->getFallback(),
                arguments.getCount(),
                arguments.getBuffer());
            operation->replaceUsesWith(call);
        }
        else
        {
            // Every used operation without a source fallback must have been consumed by a
            // type-specific lowering above. A surviving use indicates a missing ABI lowering and
            // must not be silently discarded.
            SLANG_RELEASE_ASSERT(!operation->hasUses());
        }
        operation->removeAndDeallocate();
    }

    for (auto entryPoint : structuralEntryPoints)
    {
        if (auto info =
                entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
        {
            info->removeAndDeallocate();
        }
    }
}

static void _collectProgramOperations(IRInst* parent, List<IRInst*>& operations)
{
    for (auto child = parent->getFirstChild(); child; child = child->getNextInst())
    {
        _collectProgramOperations(child, operations);
        if (child->getOp() == kIROp_StructuralRayTracingTrace ||
            child->getOp() == kIROp_StructuralRayTracingCallShader)
            operations.add(child);
    }
}

// Emit the standard-module fallback with the exact argument order retained by AST-to-IR lowering.
// The producer stores those arguments in `MakeValuePack`. `lowerTuples`, which intentionally runs
// before this target lowering, canonicalizes that container to `MakeStruct`; neither form denotes a
// source pack argument, so the container operands become the individual call arguments here.
static IRCall* _emitStructuralRayTracingFallbackCall(
    IRBuilder& builder,
    IRType* resultType,
    IRInst* fallback,
    IRInst* packedArguments)
{
    SLANG_RELEASE_ASSERT(as<IRMakeValuePack>(packedArguments) || as<IRMakeStruct>(packedArguments));
    List<IRInst*> arguments;
    for (UInt i = 0; i < packedArguments->getOperandCount(); ++i)
        arguments.add(packedArguments->getOperand(i));
    return builder.emitCallInst(resultType, fallback, arguments);
}

struct PortableStructuralRayTracingPayloadStorage
{
    IRType* payloadSemanticType;
    IRType* payloadType;
    IRIntegerValue location;
    IRGlobalVar* variable;
};

struct PortableStructuralRayTracingTraceAdapter
{
    IRFunc* fallback;
    IRType* payloadSemanticType;
    IRFunc* adapter;
};

struct PortableStructuralRayTracingTraceLoweringContext
{
    IRModule* module;
    TargetRequest* targetRequest;
    List<PortableStructuralRayTracingPayloadStorage> payloadStorage;
    List<PortableStructuralRayTracingTraceAdapter> adapters;

    PortableStructuralRayTracingTraceLoweringContext(IRModule* module, TargetRequest* targetRequest)
        : module(module), targetRequest(targetRequest)
    {
        SLANG_RELEASE_ASSERT(targetRequest);
    }

    /// Finds the compiler-owned Vulkan payload prototype referenced by `fallback`.
    ///
    /// The core `TraceRay` and `TraceMotionRay` implementations represent their outgoing Vulkan
    /// payload with exactly one global carrying `[__vulkanRayPayload(-1)]`; `-1` is the existing IR
    /// placeholder asking legalization to assign a location. Force-inlining those implementations
    /// into the structural fallback makes the reference explicit. Walking only the fallback's
    /// lexical instructions and inspecting their direct operands avoids the previous module-wide
    /// global/use-list search and makes this producer/consumer contract local and checkable.
    void findReferencedAutomaticVulkanPayloadImpl(IRInst* parent, IRGlobalVar*& ioResult)
    {
        for (auto inst = parent->getFirstChild(); inst; inst = inst->getNextInst())
        {
            for (UInt i = 0; i < inst->getOperandCount(); ++i)
            {
                auto variable = as<IRGlobalVar>(inst->getOperand(i));
                auto decoration =
                    variable ? variable->findDecoration<IRVulkanRayPayloadDecoration>() : nullptr;
                if (!decoration || cast<IRIntLit>(decoration->getOperand(0))->getValue() >= 0)
                {
                    continue;
                }
                SLANG_RELEASE_ASSERT(!ioResult || ioResult == variable);
                ioResult = variable;
            }
            findReferencedAutomaticVulkanPayloadImpl(inst, ioResult);
        }
    }

    IRGlobalVar* findReferencedAutomaticVulkanPayload(IRFunc* fallback)
    {
        IRGlobalVar* result = nullptr;
        findReferencedAutomaticVulkanPayloadImpl(fallback, result);
        return result;
    }

    IRGlobalVar* findOrCreatePayloadStorage(
        IRGlobalVar* prototype,
        IRType* payloadType,
        IRType* payloadSemanticType,
        IRIntegerValue location)
    {
        for (auto& item : payloadStorage)
        {
            if (item.payloadSemanticType != payloadSemanticType)
                continue;

            // Canonical aliases share an identity and therefore one native payload variable.
            // Different realized types or locations for that identity would mean compiler-owned
            // metadata became inconsistent after the linked-program assignment.
            SLANG_RELEASE_ASSERT(item.payloadType == payloadType && item.location == location);
            return item.variable;
        }

        IRBuilder builder(module);
        builder.setInsertInto(module->getModuleInst());
        IRCloneEnv cloneEnv;
        auto variable = as<IRGlobalVar>(cloneInst(&cloneEnv, &builder, prototype));
        SLANG_RELEASE_ASSERT(variable);
        removeLinkageDecorations(variable);

        auto decoration = variable->findDecoration<IRVulkanRayPayloadDecoration>();
        auto pointerType = as<IRPtrTypeBase>(variable->getDataType());
        SLANG_RELEASE_ASSERT(
            decoration && cast<IRIntLit>(decoration->getOperand(0))->getValue() < 0 &&
            pointerType && pointerType->getValueType() == payloadType);
        decoration->setOperand(0, builder.getIntValue(builder.getIntType(), location));

        payloadStorage.add({payloadSemanticType, payloadType, location, variable});
        return variable;
    }

    IRFunc* findAdapter(IRFunc* fallback, IRType* payloadSemanticType)
    {
        for (auto& item : adapters)
        {
            if (item.fallback == fallback && item.payloadSemanticType == payloadSemanticType)
            {
                return item.adapter;
            }
        }
        return nullptr;
    }

    IRFunc* getAdapter(IRStructuralRayTracingTrace* traceOperation)
    {
        auto fallback = as<IRFunc>(traceOperation->getFallback());
        SLANG_RELEASE_ASSERT(fallback);
        if (auto adapter = findAdapter(fallback, traceOperation->getPayloadSemanticType()))
            return adapter;

        // The portable standard-module fallback is `[ForceInline]` and calls the core `TraceRay`
        // helper, which owns Vulkan's outgoing payload variable. Inline that helper now so this
        // schema operation can bind the variable to its semantic payload identity before the
        // ordinary module-wide force-inlining pass erases the call boundary.
        performForceInlining(fallback);

        auto automaticPayloadVariable = findReferencedAutomaticVulkanPayload(fallback);
        if (!isKhronosTarget(targetRequest))
        {
            // HLSL and CUDA use their native TraceRay payload ABI and must not accidentally retain
            // the Vulkan-only prototype after target-switch specialization.
            SLANG_RELEASE_ASSERT(!automaticPayloadVariable);
            adapters.add({fallback, traceOperation->getPayloadSemanticType(), fallback});
            return fallback;
        }

        // Every Khronos structural trace is lowered through the core Vulkan payload placeholder.
        // Missing storage here is a broken standard-module/compiler contract, not a request for a
        // late best-effort location allocation.
        SLANG_RELEASE_ASSERT(automaticPayloadVariable);
        auto pointerType = as<IRPtrTypeBase>(automaticPayloadVariable->getDataType());
        SLANG_RELEASE_ASSERT(
            pointerType && pointerType->getValueType() == traceOperation->getPayloadType());

        auto location = findStructuralRayTracingProgramPayloadLocation(
            traceOperation,
            traceOperation->getPayloadSemanticType());
        SLANG_RELEASE_ASSERT(location >= 0);

        IRCloneEnv cloneEnv;
        auto variable = findOrCreatePayloadStorage(
            automaticPayloadVariable,
            traceOperation->getPayloadType(),
            traceOperation->getPayloadSemanticType(),
            location);
        cloneEnv.mapOldValToNew.add(automaticPayloadVariable, variable);

        IRBuilder builder(module);
        builder.setInsertInto(module->getModuleInst());
        auto adapter = as<IRFunc>(cloneInst(&cloneEnv, &builder, fallback));
        SLANG_RELEASE_ASSERT(adapter);
        removeLinkageDecorations(adapter);
        adapters.add({fallback, traceOperation->getPayloadSemanticType(), adapter});
        return adapter;
    }
};

void lowerPortableStructuralRayTracingOperations(IRModule* module, TargetRequest* targetRequest)
{
    List<IRInst*> operations;
    _collectProgramOperations(module->getModuleInst(), operations);

    PortableStructuralRayTracingTraceLoweringContext traceLoweringContext(module, targetRequest);
    IRBuilder builder(module);
    for (auto operation : operations)
    {
        builder.setInsertBefore(operation);
        IRInst* call = nullptr;
        if (auto traceOperation = as<IRStructuralRayTracingTrace>(operation))
        {
            call = _emitStructuralRayTracingFallbackCall(
                builder,
                traceOperation->getDataType(),
                traceLoweringContext.getAdapter(traceOperation),
                traceOperation->getFallbackArguments());
        }
        else
        {
            auto callOperation = cast<IRStructuralRayTracingCallShader>(operation);
            call = _emitStructuralRayTracingFallbackCall(
                builder,
                callOperation->getDataType(),
                callOperation->getFallback(),
                callOperation->getFallbackArguments());
        }
        operation->replaceUsesWith(call);
        operation->removeAndDeallocate();
    }
}

} // namespace Slang
