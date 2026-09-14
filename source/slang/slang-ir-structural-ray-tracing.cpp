#include "slang-ir-structural-ray-tracing.h"

#include "slang-diagnostics.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-optix-ray-tracing-abi.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-mangle.h"
#include "slang-module.h"
#include "slang-target.h"

namespace Slang
{

IROp getStructuralRayTracingStageInterfaceOp(StructuralRayTracingStageKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingStageKind::ClosestHit:
        return kIROp_ClosestHitStageInterface;
    case StructuralRayTracingStageKind::AnyHit:
        return kIROp_AnyHitStageInterface;
    case StructuralRayTracingStageKind::Intersection:
        return kIROp_IntersectionStageInterface;
    case StructuralRayTracingStageKind::Miss:
        return kIROp_MissStageInterface;
    case StructuralRayTracingStageKind::Callable:
        return kIROp_CallableStageInterface;
    default:
        return kIROp_Invalid;
    }
}

IROp getStructuralRayTracingStageInputOperationOp(StructuralRayTracingStageInputOperationKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingStageInputOperationKind::Payload:
        return kIROp_StructuralRayTracingGetPayload;
    case StructuralRayTracingStageInputOperationKind::CallableData:
        return kIROp_StructuralRayTracingGetCallableData;
    case StructuralRayTracingStageInputOperationKind::Record:
        return kIROp_StructuralRayTracingGetRecord;
    case StructuralRayTracingStageInputOperationKind::HitAttributes:
        return kIROp_StructuralRayTracingGetHitAttributes;
    case StructuralRayTracingStageInputOperationKind::TriangleBarycentricCoord:
        return kIROp_StructuralRayTracingGetTriangleBarycentricCoord;
    case StructuralRayTracingStageInputOperationKind::TriangleFrontFacing:
        return kIROp_StructuralRayTracingGetTriangleFrontFacing;
    case StructuralRayTracingStageInputOperationKind::CurveParameter:
        return kIROp_StructuralRayTracingGetCurveParameter;
    case StructuralRayTracingStageInputOperationKind::RayTMin:
        return kIROp_StructuralRayTracingGetRayTMin;
    case StructuralRayTracingStageInputOperationKind::RayTCurrent:
        return kIROp_StructuralRayTracingGetRayTCurrent;
    case StructuralRayTracingStageInputOperationKind::RayTime:
        return kIROp_StructuralRayTracingGetRayTime;
    case StructuralRayTracingStageInputOperationKind::RayFlags:
        return kIROp_StructuralRayTracingGetRayFlags;
    case StructuralRayTracingStageInputOperationKind::HitKind:
        return kIROp_StructuralRayTracingGetHitKind;
    case StructuralRayTracingStageInputOperationKind::WorldRayOrigin:
        return kIROp_StructuralRayTracingGetWorldRayOrigin;
    case StructuralRayTracingStageInputOperationKind::WorldRayDirection:
        return kIROp_StructuralRayTracingGetWorldRayDirection;
    case StructuralRayTracingStageInputOperationKind::ObjectSpaceRay:
        return kIROp_StructuralRayTracingGetObjectSpaceRay;
    case StructuralRayTracingStageInputOperationKind::PrimitiveIndex:
        return kIROp_StructuralRayTracingGetPrimitiveIndex;
    case StructuralRayTracingStageInputOperationKind::GeometryIndex:
        return kIROp_StructuralRayTracingGetGeometryIndex;
    case StructuralRayTracingStageInputOperationKind::InstanceIndex:
        return kIROp_StructuralRayTracingGetInstanceIndex;
    case StructuralRayTracingStageInputOperationKind::InstanceID:
        return kIROp_StructuralRayTracingGetInstanceID;
    case StructuralRayTracingStageInputOperationKind::ObjectToWorld:
        return kIROp_StructuralRayTracingGetObjectToWorld;
    case StructuralRayTracingStageInputOperationKind::WorldToObject:
        return kIROp_StructuralRayTracingGetWorldToObject;
    case StructuralRayTracingStageInputOperationKind::DispatchRaysIndex:
        return kIROp_StructuralRayTracingGetDispatchRaysIndex;
    case StructuralRayTracingStageInputOperationKind::DispatchRaysDimensions:
        return kIROp_StructuralRayTracingGetDispatchRaysDimensions;
    case StructuralRayTracingStageInputOperationKind::IgnoreHit:
        return kIROp_StructuralRayTracingIgnoreHit;
    case StructuralRayTracingStageInputOperationKind::AcceptHitAndEndSearch:
        return kIROp_StructuralRayTracingAcceptHitAndEndSearch;
    case StructuralRayTracingStageInputOperationKind::ReportHit:
        return kIROp_StructuralRayTracingReportHit;
    case StructuralRayTracingStageInputOperationKind::ReportHitWithKind:
        return kIROp_StructuralRayTracingReportHitWithKind;
    default:
        return kIROp_Invalid;
    }
}

bool isCompilerOwnedStructuralRayTracingIROp(IROp op)
{
    if ((op >= kIROp_FirstRaytracingStageInterface && op <= kIROp_LastRaytracingStageInterface) ||
        (op >= kIROp_FirstStructuralRayTracingStageInputOperation &&
         op <= kIROp_LastStructuralRayTracingStageInputOperation))
    {
        return true;
    }

    switch (op)
    {
    case kIROp_StructuralRayTracingProgramDescriptorType:
    case kIROp_StructuralRayTracingTrace:
    case kIROp_StructuralRayTracingCallShader:
    case kIROp_StructuralRayTracingProgramSchema:
    case kIROp_StructuralRayTracingLegacyAPIUseDecoration:
    case kIROp_MetalStructuralRayTracingTrace:
    case kIROp_MetalStructuralRayTracingCallShader:
    case kIROp_MetalStructuralRayTracingDispatchRaysIndex:
    case kIROp_MetalStructuralRayTracingDispatchRaysDimensions:
    case kIROp_StructuralRayTracingEntryPointInfoDecoration:
    case kIROp_StructuralRayTracingProgramPayloadLocationDecoration:
    case kIROp_StructuralRayTracingSemanticallyEmptyPayloadDecoration:
    case kIROp_StructuralRayTracingMetalPayloadMetadataDecoration:
    case kIROp_StructuralRayTracingOpenSectionDecoration:
    case kIROp_StructuralRayTracingDeferredEmptyPayloadDecoration:
    case kIROp_StructuralRayTracingVulkanPayloadStorageDecoration:
    case kIROp_StructuralRayTracingTaggedConformanceDecoration:
    case kIROp_StructuralRayTracingHitGroupInfoDecoration:
    case kIROp_StructuralRayTracingMissShaderInfoDecoration:
    case kIROp_StructuralRayTracingCallableShaderInfoDecoration:
    case kIROp_MetalIntersectionFunctionTable:
    case kIROp_MetalVisibleFunctionTable:
    case kIROp_MetalVisibleFunctionDecoration:
    case kIROp_MetalIntersectionFunctionDecoration:
        return true;
    default:
        return false;
    }
}

static void _collectStructuralRayTracingProgramDescriptorTypes(
    IRInst* root,
    List<IRStructuralRayTracingProgramDescriptorType*>& types)
{
    if (auto type = as<IRStructuralRayTracingProgramDescriptorType>(root))
        types.add(type);
    for (auto child : root->getChildren())
        _collectStructuralRayTracingProgramDescriptorTypes(child, types);
}

void lowerStructuralRayTracingProgramDescriptorTypes(
    IRModule* module,
    const Dictionary<IRType*, IRType*>& targetTypesBySchema)
{
    // Collect first because `replaceUsesWith` updates and deduplicates hoistable users. Walking
    // those users while mutating them can otherwise skip another descriptor nested in a function,
    // tuple, pointer, or specialized user struct type.
    List<IRStructuralRayTracingProgramDescriptorType*> descriptorTypes;
    _collectStructuralRayTracingProgramDescriptorTypes(module->getModuleInst(), descriptorTypes);
    for (auto descriptorType : descriptorTypes)
    {
        IRType* replacement = descriptorType->getStorageType();
        if (auto targetType = targetTypesBySchema.tryGetValue(descriptorType->getSchemaType()))
            replacement = *targetType;
        SLANG_RELEASE_ASSERT(replacement);
        descriptorType->replaceUsesWith(replacement);
        descriptorType->removeAndDeallocate();
    }

    descriptorTypes.clear();
    _collectStructuralRayTracingProgramDescriptorTypes(module->getModuleInst(), descriptorTypes);
    SLANG_RELEASE_ASSERT(descriptorTypes.getCount() == 0);
}

struct ReachableRayTracingAPIUses
{
    SourceLoc structuralLocation;
    SourceLoc legacyLocation;
};

// Adds the source stages that a structural dispatch operation may invoke at runtime.
//
// Consider this example:
//
//     struct Schema : rt::ITraceProgramSchema
//     {
//         typealias MissShaders = rt::MissShaderList<ImportedMiss>;
//         ...
//     }
//
//     tracer.trace(desc, scene, program, payload);
//
// The trace instruction has no ordinary IR call to `ImportedMiss::invoke`. Schema lowering records
// that relationship as compiler-owned metadata because the runtime SBT selector decides which
// listed stage runs. Target adapter synthesis later turns the selected metadata into an executable
// call. Mixed-API validation must follow that same producer-owned edge before synthesis; otherwise
// a legacy call hidden behind `ImportedMiss` is absent from the apparent call graph.
//
// A trace can select hit and miss entries only from its payload partition, while a callable
// operation can select every entry in the schema-wide callable table. Keeping those two rules here
// avoids treating unrelated schema entries as reachable merely because their metadata was linked.
static void _addStructuralRayTracingDispatchCallees(IRInst* operation, List<IRFunc*>& outCallees)
{
    if (auto trace = as<IRStructuralRayTracingTrace>(operation))
    {
        auto payloadSemanticType = trace->getPayloadSemanticType();
        SLANG_RELEASE_ASSERT(payloadSemanticType);
        for (auto decoration : trace->getDecorations())
        {
            if (auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
            {
                if (group->getPayloadSemanticType() != payloadSemanticType)
                    continue;

                static const StructuralRayTracingStageKind kHitGroupStageKinds[] = {
                    StructuralRayTracingStageKind::ClosestHit,
                    StructuralRayTracingStageKind::AnyHit,
                    StructuralRayTracingStageKind::Intersection,
                };
                for (auto stageKind : kHitGroupStageKinds)
                {
                    if (auto invoke = getStructuralRayTracingHitGroupStageInvoke(group, stageKind))
                        outCallees.add(invoke);
                }
            }
            else if (auto miss = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration))
            {
                if (miss->getPayloadSemanticType() != payloadSemanticType)
                    continue;

                auto invoke = as<IRFunc>(miss->getMiss());
                SLANG_RELEASE_ASSERT(invoke);
                outCallees.add(invoke);
            }
        }
        return;
    }

    auto callShader = as<IRStructuralRayTracingCallShader>(operation);
    SLANG_RELEASE_ASSERT(callShader);
    for (auto decoration : callShader->getDecorations())
    {
        auto callable = as<IRStructuralRayTracingCallableShaderInfoDecoration>(decoration);
        if (!callable)
            continue;

        auto invoke = as<IRFunc>(callable->getCallable());
        SLANG_RELEASE_ASSERT(invoke);
        outCallees.add(invoke);
    }
}

static void _collectReachableRayTracingAPIUses(
    IRFunc* function,
    HashSet<IRFunc*>& visitedFunctions,
    ReachableRayTracingAPIUses& uses)
{
    if (!function || !visitedFunctions.add(function))
        return;

    if (function->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
        uses.structuralLocation = function->sourceLoc;

    // Consider a separately compiled helper containing `TraceRay`, called by a ray-generation
    // entry point that also calls `RayTracer.trace`. Linking resolves the helper's call target but
    // does not restore its checked AST. Walking direct IR calls and the explicit structural
    // dispatch metadata from the selected entry point lets us observe both compiler-owned markers
    // without treating types or unrelated metadata as call-graph edges.
    List<IRFunc*> callees;
    for (auto block : function->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (inst->getOp() == kIROp_StructuralRayTracingTrace ||
                inst->getOp() == kIROp_StructuralRayTracingCallShader)
            {
                uses.structuralLocation = inst->sourceLoc;
                _addStructuralRayTracingDispatchCallees(inst, callees);
            }

            auto call = as<IRCall>(inst);
            if (!call)
                continue;
            if (call->findDecoration<IRStructuralRayTracingLegacyAPIUseDecoration>())
                uses.legacyLocation = call->sourceLoc;
            if (auto callee = as<IRFunc>(getResolvedInstForDecorations(call->getCallee())))
                callees.add(callee);
        }
    }

    for (auto callee : callees)
        _collectReachableRayTracingAPIUses(callee, visitedFunctions, uses);
}

void diagnoseMixedRayTracingAPIsInReachableIR(
    IRModule* module,
    List<IRFunc*> const& entryPoints,
    DiagnosticSink* sink)
{
    SLANG_RELEASE_ASSERT(module && sink);
    for (auto entryPoint : entryPoints)
    {
        HashSet<IRFunc*> visitedFunctions;
        ReachableRayTracingAPIUses uses;
        _collectReachableRayTracingAPIUses(entryPoint, visitedFunctions, uses);
        if (uses.structuralLocation.isValid() && uses.legacyLocation.isValid())
        {
            sink->diagnose(Diagnostics::MixedRayTracingApisInReachableCode{
                .legacyLocation = uses.legacyLocation,
                .structuralLocation = uses.structuralLocation});
            return;
        }
    }
}

static IRMakeValuePack* _getStructuralRayTracingProgramSchemaTypeIdentities(
    IRStructuralRayTracingProgramSchema* schema,
    StructuralRayTracingSectionKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingSectionKind::HitGroups:
        return schema->getHitGroupTypeIdentities();
    case StructuralRayTracingSectionKind::MissShaders:
        return schema->getMissShaderTypeIdentities();
    case StructuralRayTracingSectionKind::CallableShaders:
        return schema->getCallableShaderTypeIdentities();
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
    }
}

// Returns a replacement summary whose selected section includes `typeIdentity`.
//
// IR operands cannot grow in place. Completion therefore builds a new value pack and schema root,
// then moves the compiler-owned metadata from the old root. Callers must continue with the returned
// root because the old instruction is deallocated.
static IRStructuralRayTracingProgramSchema* _appendStructuralRayTracingProgramSchemaTypeIdentity(
    IRBuilder& builder,
    IRStructuralRayTracingProgramSchema* schema,
    StructuralRayTracingSectionKind kind,
    IRStringLit* typeIdentity)
{
    auto identities = _getStructuralRayTracingProgramSchemaTypeIdentities(schema, kind);
    SLANG_RELEASE_ASSERT(identities && typeIdentity);
    List<IRInst*> elements;
    for (UInt i = 0; i < identities->getOperandCount(); ++i)
        elements.add(identities->getOperand(i));
    elements.add(typeIdentity);

    IRBuilderInsertLocScope insertLocScope(&builder);
    builder.setInsertBefore(schema);
    auto extendedIdentities =
        cast<IRMakeValuePack>(builder.emitMakeValuePack(elements.getCount(), elements.getBuffer()));
    IRInst* operands[] = {
        schema->getSchemaType(),
        schema->getSchemaSourceTypeName(),
        schema->getSchemaTypeIdentity(),
        schema->getTraceContextType(),
        schema->getHitGroupSectionOpen(),
        schema->getMissShaderSectionOpen(),
        schema->getCallableShaderSectionOpen(),
        kind == StructuralRayTracingSectionKind::HitGroups ? extendedIdentities
                                                           : schema->getHitGroupTypeIdentities(),
        kind == StructuralRayTracingSectionKind::MissShaders
            ? extendedIdentities
            : schema->getMissShaderTypeIdentities(),
        kind == StructuralRayTracingSectionKind::CallableShaders
            ? extendedIdentities
            : schema->getCallableShaderTypeIdentities(),
    };
    auto extendedSchema = cast<IRStructuralRayTracingProgramSchema>(builder.emitIntrinsicInst(
        nullptr,
        kIROp_StructuralRayTracingProgramSchema,
        SLANG_COUNT_OF(operands),
        operands));
    extendedSchema->sourceLoc = schema->sourceLoc;

    // Rebuild through semantic accessors so the completion algorithm never knows physical operand
    // positions. Every child is a compiler-produced decoration; moving those same nodes preserves
    // the open-request pointers already collected for this completion pass.
    while (auto child = schema->getFirstDecorationOrChild())
    {
        SLANG_RELEASE_ASSERT(as<IRDecoration>(child));
        child->insertAtEnd(extendedSchema);
    }
    schema->replaceUsesWith(extendedSchema);
    schema->removeAndDeallocate();
    return extendedSchema;
}

IRFunc* getStructuralRayTracingHitGroupStageInvoke(
    IRStructuralRayTracingHitGroupInfoDecoration* group,
    StructuralRayTracingStageKind stageKind)
{
    SLANG_RELEASE_ASSERT(group);

    IRType* stageType = nullptr;
    IRStringLit* sourceTypeName = nullptr;
    IRStringLit* typeIdentity = nullptr;
    IRBoolLit* isPresent = nullptr;
    IRInst* invoke = nullptr;
    switch (stageKind)
    {
    case StructuralRayTracingStageKind::ClosestHit:
        stageType = group->getClosestHitType();
        sourceTypeName = group->getClosestHitSourceTypeName();
        typeIdentity = group->getClosestHitTypeIdentity();
        isPresent = group->getHasClosestHit();
        invoke = group->getClosestHit();
        break;
    case StructuralRayTracingStageKind::AnyHit:
        stageType = group->getAnyHitType();
        sourceTypeName = group->getAnyHitSourceTypeName();
        typeIdentity = group->getAnyHitTypeIdentity();
        isPresent = group->getHasAnyHit();
        invoke = group->getAnyHit();
        break;
    case StructuralRayTracingStageKind::Intersection:
        stageType = group->getIntersectionType();
        sourceTypeName = group->getIntersectionSourceTypeName();
        typeIdentity = group->getIntersectionTypeIdentity();
        isPresent = group->getHasIntersection();
        invoke = group->getIntersection();
        break;
    default:
        SLANG_UNEXPECTED("hit group does not contain the requested structural stage");
    }

    SLANG_RELEASE_ASSERT(stageType && sourceTypeName && typeIdentity && isPresent && invoke);
    if (!isPresent->getValue())
    {
        SLANG_RELEASE_ASSERT(
            as<IRVoidType>(stageType) && sourceTypeName->getStringSlice().getLength() == 0 &&
            typeIdentity->getStringSlice().getLength() == 0 && as<IRVoidLit>(invoke));
        return nullptr;
    }

    auto func = as<IRFunc>(invoke);
    SLANG_RELEASE_ASSERT(
        func && !as<IRVoidType>(stageType) && sourceTypeName->getStringSlice().getLength() != 0 &&
        typeIdentity->getStringSlice().getLength() != 0);
    return func;
}

bool isSemanticallyEmptyStructuralRayTracingPayloadType(IRType* type)
{
    auto resolvedType = type ? getResolvedInstForDecorations(unwrapAttributedType(type)) : nullptr;
    return resolvedType &&
           resolvedType->findDecoration<IRStructuralRayTracingSemanticallyEmptyPayloadDecoration>();
}

Result getStructuralRayTracingNativePayloadSize(
    TargetRequest* targetRequest,
    IRBuilder* builder,
    IRType* payloadType,
    IRType* payloadSemanticType,
    IRIntegerValue* outSize)
{
    SLANG_RELEASE_ASSERT(targetRequest && builder && payloadType && payloadSemanticType && outSize);
    *outSize = 0;
    if (isSemanticallyEmptyStructuralRayTracingPayloadType(payloadSemanticType))
        return SLANG_OK;

    if (isCUDATarget(targetRequest))
    {
        OptiXRayTracingPayloadABIInfo abiInfo;
        SLANG_RETURN_ON_FAIL(getOptiXRayTracingPayloadABIInfo(builder, payloadType, &abiInfo));
        *outSize = abiInfo.registerCount * kOptiXRayTracingRegisterSize;
        return SLANG_OK;
    }
    if (isD3DTarget(targetRequest) || isKhronosTarget(targetRequest))
    {
        auto layoutRules = isD3DTarget(targetRequest)
                               ? IRTypeLayoutRules::getD3DRayTracingInterface()
                               : IRTypeLayoutRules::getNatural();
        IRSizeAndAlignment layout;
        SLANG_RETURN_ON_FAIL(getSizeAndAlignment(targetRequest, layoutRules, payloadType, &layout));
        if (layout.size == IRSizeAndAlignment::kIndeterminateSize)
            return SLANG_FAIL;
        // Vulkan defines ray payload and hit-attribute block sizes as if every member used scalar
        // alignment. Slang's natural IR layout is exactly that rule; `getStride()` supplies the
        // final block tail padding. D3D uses the dedicated rule above because DXIL allocation also
        // pads nested aggregates before placing the following field.
        *outSize = layout.getStride();
    }
    return SLANG_OK;
}

Result getStructuralRayTracingNativeHitAttributeSize(
    TargetRequest* targetRequest,
    IRBuilder* builder,
    IRType* attributesType,
    IRIntegerValue* outSize)
{
    SLANG_RELEASE_ASSERT(targetRequest && builder && attributesType && outSize);
    *outSize = 0;
    if (isCUDATarget(targetRequest))
    {
        IRIntegerValue registerCount = 0;
        SLANG_RETURN_ON_FAIL(
            getOptiXRayTracingHitAttributeRegisterCount(builder, attributesType, &registerCount));
        *outSize = registerCount * kOptiXRayTracingRegisterSize;
        return SLANG_OK;
    }
    if (isD3DTarget(targetRequest) || isKhronosTarget(targetRequest))
    {
        auto layoutRules = isD3DTarget(targetRequest)
                               ? IRTypeLayoutRules::getD3DRayTracingInterface()
                               : IRTypeLayoutRules::getNatural();
        IRSizeAndAlignment layout;
        SLANG_RETURN_ON_FAIL(
            getSizeAndAlignment(targetRequest, layoutRules, attributesType, &layout));
        if (layout.size == IRSizeAndAlignment::kIndeterminateSize)
            return SLANG_FAIL;
        *outSize = layout.getStride();
    }
    return SLANG_OK;
}

// Collect every concrete Vulkan ray-payload location in the lexical IR subtree. Payload
// decorations may belong to globals, entry-point parameters, or declarations inside generics, so
// a module-scope-only scan would miss valid legacy locations. The negative value `-1` asks later
// legalization to choose a location and therefore does not reserve one here.
void collectUsedVulkanRayPayloadLocations(IRInst* root, HashSet<IRIntegerValue>& outLocations)
{
    for (auto inst = root->getFirstDecorationOrChild(); inst; inst = inst->getNextInst())
    {
        if (as<IRVulkanRayPayloadDecoration>(inst) || as<IRVulkanRayPayloadInDecoration>(inst))
        {
            auto location = cast<IRIntLit>(inst->getOperand(0))->getValue();
            if (location >= 0)
                outLocations.add(location);
        }
        collectUsedVulkanRayPayloadLocations(inst, outLocations);
    }
}

void addStructuralRayTracingProgramPayloadLocation(
    IRBuilder& builder,
    IRInst* owner,
    IRType* payloadType,
    IRType* payloadSemanticType,
    IRIntegerValue location)
{
    SLANG_RELEASE_ASSERT(owner && payloadType && payloadSemanticType && location >= 0);
    for (auto decoration : owner->getDecorations())
    {
        auto existing = as<IRStructuralRayTracingProgramPayloadLocationDecoration>(decoration);
        if (!existing || existing->getPayloadSemanticType() != payloadSemanticType)
        {
            continue;
        }
        // One owner represents one linked program (or one schema operation in that program), so a
        // semantic payload identity has exactly one realized type and location at this target
        // phase. Distinct identities may intentionally use different locations even when their
        // physical layouts are identical.
        SLANG_RELEASE_ASSERT(existing->getLocation()->getValue() == location);
        SLANG_RELEASE_ASSERT(existing->getPayloadType() == payloadType);
        return;
    }

    IRInst* operands[] = {
        payloadType,
        payloadSemanticType,
        builder.getIntValue(builder.getIntType(), location),
    };
    builder.addDecoration(
        owner,
        kIROp_StructuralRayTracingProgramPayloadLocationDecoration,
        operands,
        SLANG_COUNT_OF(operands));
}

IRIntegerValue findStructuralRayTracingProgramPayloadLocation(
    IRInst* owner,
    IRType* payloadSemanticType)
{
    SLANG_RELEASE_ASSERT(owner && payloadSemanticType);
    IRIntegerValue result = -1;
    for (auto decoration : owner->getDecorations())
    {
        auto assignment = as<IRStructuralRayTracingProgramPayloadLocationDecoration>(decoration);
        if (!assignment || assignment->getPayloadSemanticType() != payloadSemanticType)
        {
            continue;
        }
        auto location = assignment->getLocation()->getValue();
        SLANG_RELEASE_ASSERT(result < 0 || result == location);
        result = location;
    }
    return result;
}

void addStructuralRayTracingEntryPointInfo(
    IRBuilder& builder,
    IRFunc* func,
    const StructuralRayTracingEntryPointIRInfo& info)
{
    SLANG_RELEASE_ASSERT(
        info.invoke && info.stageType && info.stageSourceTypeName && info.stageTypeIdentity);
    auto voidType = builder.getVoidType();
    IRInst* operands[] = {
        builder.getIntValue(builder.getIntType(), IRIntegerValue(info.stageKind)),
        info.invoke,
        info.stageType,
        info.stageSourceTypeName,
        info.stageTypeIdentity,
        info.contextType ? info.contextType : voidType,
        info.payloadType ? info.payloadType : voidType,
        info.payloadSemanticType ? info.payloadSemanticType : builder.getVoidType(),
        info.recordType ? info.recordType : voidType,
        info.hitAttributesType ? info.hitAttributesType : voidType,
        info.callableDataType ? info.callableDataType : voidType,
        builder.getIntValue(builder.getIntType(), IRIntegerValue(info.hitAttributesKind)),
        builder.getIntValue(builder.getIntType(), info.payloadLocation),
    };
    builder.addDecoration(
        func,
        kIROp_StructuralRayTracingEntryPointInfoDecoration,
        operands,
        SLANG_COUNT_OF(operands));
}

static IRInterfaceType* _findInterfaceType(IRInst* inst)
{
    if (auto generic = as<IRGeneric>(inst))
        inst = findInnerMostGenericReturnVal(generic);
    return as<IRInterfaceType>(inst);
}

bool identifyStructuralRayTracingStageInterfaces(
    Module* module,
    const StructuralRayTracingDeclRegistry& registry,
    StructuralRayTracingStageKind* outMissingStage)
{
    auto irModule = module->getIRModule();
    auto astBuilder = module->getASTBuilder();
    SLANG_AST_BUILDER_RAII(astBuilder);

    for (int i = 0; i < int(StructuralRayTracingStageKind::Count); ++i)
    {
        auto kind = StructuralRayTracingStageKind(i);
        auto interfaceDecl = registry.getStageInterface(kind);
        auto mangledName = getMangledName(astBuilder, interfaceDecl);
        auto symbols = irModule->findSymbolByMangledName(ImmutableHashedString(mangledName));
        auto expectedOp = getStructuralRayTracingStageInterfaceOp(kind);
        bool found = false;

        for (auto symbol : symbols)
        {
            auto interfaceType = _findInterfaceType(symbol);
            if (!interfaceType)
                continue;
            if (interfaceType->getOp() != kIROp_InterfaceType &&
                interfaceType->getOp() != expectedOp)
            {
                continue;
            }

            // All stage-interface ops have the same storage and operand layout as
            // IRInterfaceType. The trusted-module load is the point where the ordinary
            // serialized interface receives its compiler-owned nominal identity.
            interfaceType->m_op = expectedOp;
            found = true;
        }

        if (!found)
        {
            if (outMissingStage)
                *outMissingStage = kind;
            return false;
        }
    }
    return true;
}

struct _StructuralRayTracingLinkedEntryCandidate
{
    IRDecoration* entryInfo = nullptr;
    IRStringLit* sourceTypeName = nullptr;
    IRStringLit* typeIdentity = nullptr;
};

static const char* _getStructuralRayTracingSectionDiagnosticName(
    StructuralRayTracingSectionKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingSectionKind::HitGroups:
        return "hit-group";
    case StructuralRayTracingSectionKind::MissShaders:
        return "miss-shader";
    case StructuralRayTracingSectionKind::CallableShaders:
        return "callable-shader";
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
    }
}

static const char* _getStructuralRayTracingSectionEntryInterfaceName(
    StructuralRayTracingSectionKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingSectionKind::HitGroups:
        return "IHitGroup";
    case StructuralRayTracingSectionKind::MissShaders:
        return "IMissShader";
    case StructuralRayTracingSectionKind::CallableShaders:
        return "ICallableShader";
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
    }
}

static void _collectStructuralRayTracingOpenSections(
    IRInst* root,
    List<IRStructuralRayTracingOpenSectionDecoration*>& outSections)
{
    for (auto inst = root->getFirstDecorationOrChild(); inst; inst = inst->getNextInst())
    {
        if (auto section = as<IRStructuralRayTracingOpenSectionDecoration>(inst))
            outSections.add(section);
        _collectStructuralRayTracingOpenSections(inst, outSections);
    }
}

static IRDecoration* _findStructuralRayTracingEntryInfo(
    IRWitnessTable* witnessTable,
    StructuralRayTracingSectionKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingSectionKind::HitGroups:
        return witnessTable->findDecoration<IRStructuralRayTracingHitGroupInfoDecoration>();
    case StructuralRayTracingSectionKind::MissShaders:
        return witnessTable->findDecoration<IRStructuralRayTracingMissShaderInfoDecoration>();
    case StructuralRayTracingSectionKind::CallableShaders:
        return witnessTable->findDecoration<IRStructuralRayTracingCallableShaderInfoDecoration>();
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
    }
}

static _StructuralRayTracingLinkedEntryCandidate _getStructuralRayTracingLinkedEntryCandidate(
    IRWitnessTable* witnessTable,
    StructuralRayTracingSectionKind kind)
{
    _StructuralRayTracingLinkedEntryCandidate result;
    result.entryInfo = _findStructuralRayTracingEntryInfo(witnessTable, kind);
    SLANG_RELEASE_ASSERT(result.entryInfo);
    IRType* entryType = nullptr;

    switch (kind)
    {
    case StructuralRayTracingSectionKind::HitGroups:
        {
            auto info = cast<IRStructuralRayTracingHitGroupInfoDecoration>(result.entryInfo);
            entryType = info->getGroupType();
            result.sourceTypeName = info->getGroupSourceTypeName();
            result.typeIdentity = info->getGroupTypeIdentity();
            SLANG_RELEASE_ASSERT(info->getIsLinked()->getValue());
            break;
        }
    case StructuralRayTracingSectionKind::MissShaders:
        {
            auto info = cast<IRStructuralRayTracingMissShaderInfoDecoration>(result.entryInfo);
            entryType = info->getMissType();
            result.sourceTypeName = info->getMissSourceTypeName();
            result.typeIdentity = info->getMissTypeIdentity();
            SLANG_RELEASE_ASSERT(info->getIsLinked()->getValue());
            break;
        }
    case StructuralRayTracingSectionKind::CallableShaders:
        {
            auto info = cast<IRStructuralRayTracingCallableShaderInfoDecoration>(result.entryInfo);
            entryType = info->getCallableType();
            result.sourceTypeName = info->getCallableSourceTypeName();
            result.typeIdentity = info->getCallableTypeIdentity();
            SLANG_RELEASE_ASSERT(info->getIsLinked()->getValue());
            break;
        }
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
    }
    SLANG_RELEASE_ASSERT(
        entryType && result.sourceTypeName && result.typeIdentity &&
        result.sourceTypeName->getStringSlice().getLength() != 0 &&
        result.typeIdentity->getStringSlice().getLength() != 0);
    return result;
}

static IRStructuralRayTracingHitGroupInfoDecoration* _appendStructuralRayTracingLinkedHitGroup(
    IRBuilder& builder,
    IRInst* operation,
    IRStructuralRayTracingHitGroupInfoDecoration* source,
    Index functionIndex)
{
    // Copy by semantic field, not physical operand position. The only rewritten facts are the
    // program-local dense index and linked origin; payload location is assigned in the following
    // whole-program pass together with listed entries.
    IRInst* operands[] = {
        source->getGroupType(),
        source->getGroupSourceTypeName(),
        source->getGroupTypeIdentity(),
        source->getGroupDeclLookupName(),
        builder.getIntValue(builder.getIntType(), functionIndex),
        source->getContextType(),
        source->getTraceContextType(),
        source->getPrimitiveType(),
        source->getPayloadType(),
        source->getPayloadSemanticType(),
        source->getRecordType(),
        source->getHitAttributesType(),
        source->getHitAttributesKind(),
        source->getClosestHitType(),
        source->getClosestHitSourceTypeName(),
        source->getClosestHitTypeIdentity(),
        source->getHasClosestHit(),
        source->getClosestHit(),
        source->getAnyHitType(),
        source->getAnyHitSourceTypeName(),
        source->getAnyHitTypeIdentity(),
        source->getHasAnyHit(),
        source->getAnyHit(),
        source->getIntersectionType(),
        source->getIntersectionSourceTypeName(),
        source->getIntersectionTypeIdentity(),
        source->getHasIntersection(),
        source->getIntersection(),
        builder.getBoolValue(true),
        source->getPayloadLocation(),
    };
    auto result = cast<IRStructuralRayTracingHitGroupInfoDecoration>(builder.addDecoration(
        operation,
        kIROp_StructuralRayTracingHitGroupInfoDecoration,
        operands,
        SLANG_COUNT_OF(operands)));
    result->sourceLoc = source->sourceLoc;
    return result;
}

static IRStructuralRayTracingMissShaderInfoDecoration* _appendStructuralRayTracingLinkedMissShader(
    IRBuilder& builder,
    IRInst* operation,
    IRStructuralRayTracingMissShaderInfoDecoration* source,
    Index functionIndex)
{
    IRInst* operands[] = {
        builder.getIntValue(builder.getIntType(), functionIndex),
        source->getContextType(),
        source->getTraceContextType(),
        source->getPayloadType(),
        source->getPayloadSemanticType(),
        source->getRecordType(),
        source->getMissType(),
        source->getMissSourceTypeName(),
        source->getMissTypeIdentity(),
        source->getMissDeclLookupName(),
        source->getMiss(),
        builder.getBoolValue(true),
        source->getPayloadLocation(),
    };
    auto result = cast<IRStructuralRayTracingMissShaderInfoDecoration>(builder.addDecoration(
        operation,
        kIROp_StructuralRayTracingMissShaderInfoDecoration,
        operands,
        SLANG_COUNT_OF(operands)));
    result->sourceLoc = source->sourceLoc;
    return result;
}

static IRStructuralRayTracingCallableShaderInfoDecoration*
_appendStructuralRayTracingLinkedCallableShader(
    IRBuilder& builder,
    IRInst* operation,
    IRStructuralRayTracingCallableShaderInfoDecoration* source,
    Index functionIndex)
{
    IRInst* operands[] = {
        builder.getIntValue(builder.getIntType(), functionIndex),
        source->getContextType(),
        source->getTraceContextType(),
        source->getCallableDataType(),
        source->getRecordType(),
        source->getCallableType(),
        source->getCallableSourceTypeName(),
        source->getCallableTypeIdentity(),
        source->getCallableDeclLookupName(),
        source->getCallable(),
        builder.getBoolValue(true),
    };
    auto result = cast<IRStructuralRayTracingCallableShaderInfoDecoration>(builder.addDecoration(
        operation,
        kIROp_StructuralRayTracingCallableShaderInfoDecoration,
        operands,
        SLANG_COUNT_OF(operands)));
    result->sourceLoc = source->sourceLoc;
    return result;
}

static void _collectStructuralRayTracingListedEntryState(
    IRInst* operation,
    StructuralRayTracingSectionKind kind,
    HashSet<UnownedStringSlice>& typeIdentities,
    Dictionary<IRType*, Index>& nextIndicesByPayload,
    Index& nextCallableIndex)
{
    for (auto decoration : operation->getDecorations())
    {
        switch (kind)
        {
        case StructuralRayTracingSectionKind::HitGroups:
            {
                auto info = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration);
                if (!info)
                    break;
                typeIdentities.add(info->getGroupTypeIdentity()->getStringSlice());
                auto& nextIndex =
                    nextIndicesByPayload.getOrAddValue(info->getPayloadSemanticType(), 0);
                nextIndex = Math::Max(nextIndex, Index(info->getFunctionIndex()->getValue() + 1));
                break;
            }
        case StructuralRayTracingSectionKind::MissShaders:
            {
                auto info = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration);
                if (!info)
                    break;
                typeIdentities.add(info->getMissTypeIdentity()->getStringSlice());
                auto& nextIndex =
                    nextIndicesByPayload.getOrAddValue(info->getPayloadSemanticType(), 0);
                nextIndex = Math::Max(nextIndex, Index(info->getFunctionIndex()->getValue() + 1));
                break;
            }
        case StructuralRayTracingSectionKind::CallableShaders:
            {
                auto info = as<IRStructuralRayTracingCallableShaderInfoDecoration>(decoration);
                if (!info)
                    break;
                typeIdentities.add(info->getCallableTypeIdentity()->getStringSlice());
                nextCallableIndex =
                    Math::Max(nextCallableIndex, Index(info->getFunctionIndex()->getValue() + 1));
                break;
            }
        default:
            SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
        }
    }
}

bool completeOpenStructuralRayTracingSchemas(IRModule* module, DiagnosticSink* sink)
{
    List<IRStructuralRayTracingOpenSectionDecoration*> openSections;
    _collectStructuralRayTracingOpenSections(module->getModuleInst(), openSections);
    if (openSections.getCount() == 0)
        return true;

    // Selected tables are ordinary global witness tables. The linker has already filtered the
    // input-module conformance index by exact requested tag, so this lexical global enumeration is
    // the canonical linked set; it neither follows arbitrary operand graphs nor scans names.
    List<IRWitnessTable*> taggedWitnessTables;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto table = as<IRWitnessTable>(inst))
        {
            if (table->findDecoration<IRStructuralRayTracingTaggedConformanceDecoration>())
                taggedWitnessTables.add(table);
        }
    }

    Dictionary<IRType*, UInt> diagnosedInvalidTagKinds;
    bool isValid = true;
    IRBuilder builder(module);
    for (auto request : openSections)
    {
        auto operation = request->getParent();
        SLANG_RELEASE_ASSERT(
            as<IRStructuralRayTracingTrace>(operation) ||
            as<IRStructuralRayTracingCallShader>(operation) ||
            as<IRStructuralRayTracingProgramSchema>(operation));
        auto kindValue = request->getSectionKind()->getValue();
        SLANG_RELEASE_ASSERT(
            kindValue >= 0 && kindValue < IRIntegerValue(StructuralRayTracingSectionKind::Count));
        auto kind = StructuralRayTracingSectionKind(kindValue);

        if (!request->getIsValidTag()->getValue())
        {
            auto kindBit = UInt(1) << UInt(kind);
            auto& diagnosedKinds = diagnosedInvalidTagKinds.getOrAddValue(request->getTagType(), 0);
            if ((diagnosedKinds & kindBit) == 0)
            {
                diagnosedKinds |= kindBit;
                sink->diagnose(Diagnostics::StructuralRayTracingOpenTagNotEntryInterfaceIr{
                    .section = _getStructuralRayTracingSectionDiagnosticName(kind),
                    .tag = request->getTagType(),
                    .entryInterface = _getStructuralRayTracingSectionEntryInterfaceName(kind),
                    .location = operation->sourceLoc});
            }
            isValid = false;
            request->removeAndDeallocate();
            continue;
        }

        List<_StructuralRayTracingLinkedEntryCandidate> candidates;
        for (auto table : taggedWitnessTables)
        {
            for (auto decoration : table->getDecorations())
            {
                auto tagged = as<IRStructuralRayTracingTaggedConformanceDecoration>(decoration);
                if (!tagged || tagged->getSectionKind()->getValue() != kindValue ||
                    tagged->getTagType() != request->getTagType())
                {
                    continue;
                }
                candidates.add(_getStructuralRayTracingLinkedEntryCandidate(table, kind));
            }
        }
        candidates.sort(
            [](const _StructuralRayTracingLinkedEntryCandidate& left,
               const _StructuralRayTracingLinkedEntryCandidate& right)
            {
                auto sourceNameOrder = compare(
                    left.sourceTypeName->getStringSlice(),
                    right.sourceTypeName->getStringSlice());
                if (sourceNameOrder != 0)
                    return sourceNameOrder < 0;
                // Qualified names are the specified public order. The opaque canonical identity
                // is only an injective tie-breaker for same-spelled declarations from distinct
                // modules or generic specializations; it is never parsed to recover semantics.
                return compare(
                           left.typeIdentity->getStringSlice(),
                           right.typeIdentity->getStringSlice()) < 0;
            });

        HashSet<UnownedStringSlice> typeIdentities;
        Dictionary<IRType*, Index> nextIndicesByPayload;
        Index nextCallableIndex = 0;
        _collectStructuralRayTracingListedEntryState(
            operation,
            kind,
            typeIdentities,
            nextIndicesByPayload,
            nextCallableIndex);

        builder.setInsertInto(operation);
        for (auto candidate : candidates)
        {
            if (!typeIdentities.add(candidate.typeIdentity->getStringSlice()))
                continue;

            switch (kind)
            {
            case StructuralRayTracingSectionKind::HitGroups:
                {
                    auto source =
                        cast<IRStructuralRayTracingHitGroupInfoDecoration>(candidate.entryInfo);
                    auto& nextIndex =
                        nextIndicesByPayload.getOrAddValue(source->getPayloadSemanticType(), 0);
                    _appendStructuralRayTracingLinkedHitGroup(
                        builder,
                        operation,
                        source,
                        nextIndex++);
                    break;
                }
            case StructuralRayTracingSectionKind::MissShaders:
                {
                    auto source =
                        cast<IRStructuralRayTracingMissShaderInfoDecoration>(candidate.entryInfo);
                    auto& nextIndex =
                        nextIndicesByPayload.getOrAddValue(source->getPayloadSemanticType(), 0);
                    _appendStructuralRayTracingLinkedMissShader(
                        builder,
                        operation,
                        source,
                        nextIndex++);
                    break;
                }
            case StructuralRayTracingSectionKind::CallableShaders:
                _appendStructuralRayTracingLinkedCallableShader(
                    builder,
                    operation,
                    cast<IRStructuralRayTracingCallableShaderInfoDecoration>(candidate.entryInfo),
                    nextCallableIndex++);
                break;
            default:
                SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
            }

            if (auto schema = as<IRStructuralRayTracingProgramSchema>(operation))
            {
                operation = _appendStructuralRayTracingProgramSchemaTypeIdentity(
                    builder,
                    schema,
                    kind,
                    candidate.typeIdentity);
                builder.setInsertInto(operation);
            }
        }

        // The request marker has served its only liveness purpose. The complete operation metadata
        // now roots every selected stage directly, so later per-entry linking and DCE need not
        // retain or rediscover the contributing witness tables.
        request->removeAndDeallocate();
    }
    return isValid;
}

struct _StructuralRayTracingEmptyPayloadCandidate
{
    IRType* payloadType = nullptr;
    IRType* payloadSemanticType = nullptr;
};

static void _collectDeferredStructuralRayTracingEmptyPayloadTraces(
    IRInst* root,
    List<IRStructuralRayTracingTrace*>& traces)
{
    for (auto child = root->getFirstChild(); child; child = child->getNextInst())
    {
        // The first specialization pass materializes every executable trace outside its retained
        // generic template. Resolving the template would manufacture a payload choice for an
        // uninstantiated program and would consume generic metadata that is intentionally not a
        // concrete linked-program identity.
        if (as<IRGeneric>(child))
            continue;

        if (auto trace = as<IRStructuralRayTracingTrace>(child))
        {
            if (trace->findDecoration<IRStructuralRayTracingDeferredEmptyPayloadDecoration>())
                traces.add(trace);
        }
        _collectDeferredStructuralRayTracingEmptyPayloadTraces(child, traces);
    }
}

static void _addStructuralRayTracingEmptyPayloadCandidate(
    _StructuralRayTracingEmptyPayloadCandidate candidate,
    _StructuralRayTracingEmptyPayloadCandidate& first,
    _StructuralRayTracingEmptyPayloadCandidate& second)
{
    SLANG_RELEASE_ASSERT(candidate.payloadType && candidate.payloadSemanticType);
    if (!isSemanticallyEmptyStructuralRayTracingPayloadType(candidate.payloadSemanticType))
        return;

    if (!first.payloadSemanticType)
    {
        first = candidate;
        return;
    }
    if (first.payloadSemanticType == candidate.payloadSemanticType)
    {
        // One semantic payload identity has exactly one target representation at this phase.
        // Hit and miss metadata may repeat that identity, but they must not reinterpret it.
        SLANG_RELEASE_ASSERT(first.payloadType == candidate.payloadType);
        return;
    }
    if (!second.payloadSemanticType)
        second = candidate;
}

// Returns the canonical schema type carried by each complete schema metadata owner.
//
// Trace and callable operations own the metadata needed by executable target lowering, while the
// program-schema root owns the same metadata for reflection-only requests. Open-section completion
// has already appended every linked entry to all three shapes before this function is used, so the
// semantic schema operand is the only identity needed to deduplicate them.
static IRType* _getStructuralRayTracingSchemaOwnerType(IRInst* owner)
{
    if (auto trace = as<IRStructuralRayTracingTrace>(owner))
        return as<IRType>(trace->getProgramLayout());
    if (auto call = as<IRStructuralRayTracingCallShader>(owner))
        return as<IRType>(call->getProgramLayout());
    if (auto schema = as<IRStructuralRayTracingProgramSchema>(owner))
        return schema->getSchemaType();
    return nullptr;
}

static void _collectStructuralRayTracingSchemaOwners(IRInst* root, List<IRInst*>& owners)
{
    // Specialization materializes every executable operation outside its generic template. A
    // template can still contain dependent payload types, so it is not a finalized schema and
    // must not participate in this whole-program invariant.
    if (as<IRGeneric>(root))
        return;

    if (_getStructuralRayTracingSchemaOwnerType(root))
        owners.add(root);
    for (auto child = root->getFirstChild(); child; child = child->getNextInst())
        _collectStructuralRayTracingSchemaOwners(child, owners);
}

// Checks the empty-payload invariant on complete schema metadata rather than on one trace call.
//
// Consider a schema that serves `EmptyA`, `EmptyB`, and `RadiancePayload`. An explicit
// `trace<RadiancePayload>` is locally unambiguous, but reflection and a later payload-less trace
// still observe one schema with two incompatible implicit payload identities. Every schema owner
// contains its complete hit and miss metadata at this boundary, so checking once per canonical
// schema type catches the conflict for executable and reflection-only programs alike.
static bool _validateStructuralRayTracingSchemaEmptyPayloads(
    IRModule* module,
    DiagnosticSink* sink,
    HashSet<IRType*>& outAmbiguousSchemas)
{
    List<IRInst*> owners;
    _collectStructuralRayTracingSchemaOwners(module->getModuleInst(), owners);

    bool isValid = true;
    HashSet<IRType*> validatedSchemas;
    for (auto owner : owners)
    {
        auto schemaType = _getStructuralRayTracingSchemaOwnerType(owner);
        SLANG_RELEASE_ASSERT(schemaType);
        if (!validatedSchemas.add(schemaType))
            continue;

        _StructuralRayTracingEmptyPayloadCandidate first;
        _StructuralRayTracingEmptyPayloadCandidate second;
        for (auto decoration : owner->getDecorations())
        {
            if (auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
            {
                _addStructuralRayTracingEmptyPayloadCandidate(
                    {group->getPayloadType(), group->getPayloadSemanticType()},
                    first,
                    second);
            }
            else if (auto miss = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration))
            {
                _addStructuralRayTracingEmptyPayloadCandidate(
                    {miss->getPayloadType(), miss->getPayloadSemanticType()},
                    first,
                    second);
            }
        }

        if (!second.payloadSemanticType)
            continue;

        sink->diagnose(Diagnostics::StructuralRayTracingLinkedAmbiguousEmptyPayload{
            .schemaType = schemaType,
            .firstPayloadType = first.payloadSemanticType,
            .secondPayloadType = second.payloadSemanticType,
            .location = owner->sourceLoc});
        outAmbiguousSchemas.add(schemaType);
        isValid = false;
    }
    return isValid;
}

static IRStructuralRayTracingTrace* _completeDeferredStructuralRayTracingEmptyPayloadTrace(
    IRModule* module,
    IRStructuralRayTracingTrace* trace,
    IRStructuralRayTracingDeferredEmptyPayloadDecoration* deferred,
    _StructuralRayTracingEmptyPayloadCandidate payload)
{
    SLANG_RELEASE_ASSERT(
        trace && deferred && payload.payloadType && payload.payloadSemanticType &&
        as<IRVoidLit>(trace->getFallback()) && as<IRVoidLit>(trace->getFallbackArguments()) &&
        as<IRVoidType>(trace->getPayloadType()) &&
        as<IRVoidType>(trace->getPayloadSemanticType()) && as<IRVoidLit>(trace->getPayload()) &&
        trace->getPayloadLocation()->getValue() < 0 &&
        StructuralRayTracingTraceMethodKind(trace->getTraceMethodKind()->getValue()) ==
            StructuralRayTracingTraceMethodKind::ImplicitEmptyPayload);

    IRBuilder builder(module);
    builder.setInsertBefore(trace);
    auto payloadVariable = builder.emitVar(payload.payloadType);
    builder.emitStore(payloadVariable, builder.emitDefaultConstruct(payload.payloadType));

    auto genericArgumentsWithoutPayload = deferred->getFallbackGenericArgumentsWithoutPayload();
    auto payloadGenericArgumentIndex =
        Index(deferred->getPayloadGenericArgumentIndex()->getValue());
    SLANG_RELEASE_ASSERT(
        payloadGenericArgumentIndex >= 0 &&
        payloadGenericArgumentIndex <= Index(genericArgumentsWithoutPayload->getOperandCount()));
    List<IRInst*> genericArguments;
    for (Index i = 0; i <= Index(genericArgumentsWithoutPayload->getOperandCount()); ++i)
    {
        if (i == payloadGenericArgumentIndex)
            genericArguments.add(payload.payloadType);
        if (i < Index(genericArgumentsWithoutPayload->getOperandCount()))
            genericArguments.add(genericArgumentsWithoutPayload->getOperand(UInt(i)));
    }

    auto fallbackGeneric = deferred->getFallbackGeneric();
    SLANG_RELEASE_ASSERT(fallbackGeneric && fallbackGeneric->getDataType());
    auto fallbackType = as<IRType>(builder.emitSpecializeInst(
        builder.getTypeKind(),
        fallbackGeneric->getDataType(),
        genericArguments));
    SLANG_RELEASE_ASSERT(fallbackType);
    auto fallback = builder.emitSpecializeInst(fallbackType, fallbackGeneric, genericArguments);

    auto argumentsWithoutPayload = deferred->getFallbackArgumentsWithoutPayload();
    auto payloadArgumentIndex = Index(deferred->getPayloadFallbackArgumentIndex()->getValue());
    SLANG_RELEASE_ASSERT(
        payloadArgumentIndex >= 0 &&
        payloadArgumentIndex <= Index(argumentsWithoutPayload->getOperandCount()));
    List<IRInst*> fallbackArguments;
    for (Index i = 0; i <= Index(argumentsWithoutPayload->getOperandCount()); ++i)
    {
        if (i == payloadArgumentIndex)
            fallbackArguments.add(payloadVariable);
        if (i < Index(argumentsWithoutPayload->getOperandCount()))
            fallbackArguments.add(argumentsWithoutPayload->getOperand(UInt(i)));
    }
    auto fallbackArgumentPack =
        builder.emitMakeValuePack(fallbackArguments.getCount(), fallbackArguments.getBuffer());

    // Rebuild from semantic accessors so this resolver stays independent of the trace's physical
    // operand layout. Only the five payload-dependent fields change; the trace remains the owner
    // of every completed entry record and of the source operation's result uses.
    IRInst* operands[] = {
        fallback,
        fallbackArgumentPack,
        trace->getProgramLayout(),
        trace->getProgramLayoutSourceTypeName(),
        trace->getTraceContext(),
        payload.payloadType,
        payload.payloadSemanticType,
        trace->getTraceMethodKind(),
        trace->getMotionKind(),
        trace->getHitGroups(),
        trace->getHitGroupTypes(),
        trace->getMissShaders(),
        trace->getMissShaderTypes(),
        trace->getCallableShaders(),
        trace->getCallableShaderTypes(),
        trace->getTracer(),
        trace->getDesc(),
        trace->getAccelerationStructure(),
        trace->getDescriptor(),
        payloadVariable,
        trace->getPayloadLocation(),
    };
    auto completedTrace = cast<IRStructuralRayTracingTrace>(builder.emitIntrinsicInst(
        trace->getDataType(),
        kIROp_StructuralRayTracingTrace,
        SLANG_COUNT_OF(operands),
        operands));
    completedTrace->sourceLoc = trace->sourceLoc;

    deferred->removeAndDeallocate();
    trace->transferDecorationsTo(completedTrace);
    trace->replaceUsesWith(completedTrace);
    trace->removeAndDeallocate();
    return completedTrace;
}

static bool _resolveDeferredStructuralRayTracingEmptyPayloads(
    IRModule* module,
    DiagnosticSink* sink,
    const HashSet<IRType*>& ambiguousSchemas)
{
    List<IRStructuralRayTracingTrace*> traces;
    _collectDeferredStructuralRayTracingEmptyPayloadTraces(module->getModuleInst(), traces);

    bool isValid = true;
    IRBuilder builder(module);
    for (auto trace : traces)
    {
        auto deferred =
            trace->findDecoration<IRStructuralRayTracingDeferredEmptyPayloadDecoration>();
        SLANG_RELEASE_ASSERT(deferred);

        _StructuralRayTracingEmptyPayloadCandidate first;
        _StructuralRayTracingEmptyPayloadCandidate second;
        for (auto decoration : trace->getDecorations())
        {
            if (auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
            {
                _addStructuralRayTracingEmptyPayloadCandidate(
                    {group->getPayloadType(), group->getPayloadSemanticType()},
                    first,
                    second);
            }
            else if (auto miss = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration))
            {
                _addStructuralRayTracingEmptyPayloadCandidate(
                    {miss->getPayloadType(), miss->getPayloadSemanticType()},
                    first,
                    second);
            }
        }

        if (!first.payloadSemanticType || second.payloadSemanticType)
        {
            if (!first.payloadSemanticType)
            {
                sink->diagnose(Diagnostics::StructuralRayTracingLinkedEmptyPayloadNotFound{
                    .schemaType = trace->getProgramLayout(),
                    .location = trace->sourceLoc});
            }
            else
            {
                // Schema validation owns this diagnostic even when the activating call happens to
                // use the implicit overload. The resolver still removes the invalid trace so the
                // failed manifest remains internally safe for the current request.
                SLANG_RELEASE_ASSERT(
                    ambiguousSchemas.contains(cast<IRType>(trace->getProgramLayout())));
            }

            builder.setInsertBefore(trace);
            trace->replaceUsesWith(builder.getVoidValue());
            trace->removeAndDeallocate();
            isValid = false;
            continue;
        }

        _completeDeferredStructuralRayTracingEmptyPayloadTrace(module, trace, deferred, first);
    }
    return isValid;
}

bool finalizeStructuralRayTracingSchemaPayloads(IRModule* module, DiagnosticSink* sink)
{
    HashSet<IRType*> ambiguousSchemas;
    bool isValid = _validateStructuralRayTracingSchemaEmptyPayloads(module, sink, ambiguousSchemas);
    if (!_resolveDeferredStructuralRayTracingEmptyPayloads(module, sink, ambiguousSchemas))
        isValid = false;
    return isValid;
}

} // namespace Slang
